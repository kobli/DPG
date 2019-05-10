#include <stack>
#include <algorithm>
#include <iostream>
#include "bvh.hpp"
#include "containment.hpp"
#include "globals.hpp"

std::vector<unsigned> BVH::build(const std::vector<Vertex>& vertices, const std::vector<PrimitiveInfo>& primitivesInfo, unsigned maxPrimitivesInLeaf) {
	std::vector<unsigned> primitives(primitivesInfo.size());
	for(unsigned i = 0; i < primitives.size(); ++i)
		primitives[i] = i;
	BVHBuildNode root;
	root.depth = 0;
	root.firstPrimitive = 0;
	root.primitiveCount = primitives.size();
	std::stack<BVHBuildNode*> nodes;
	nodes.push(&root);

	AABB centroidAABB;
	while(!nodes.empty()) {
		BVHBuildNode* n = nodes.top();
		nodes.pop();
		// TODO OPTIMIZE? - calculate primitive AABBs bottom up only for the compressed version?
		auto nodePrimsBegin = primitives.begin() + n->firstPrimitive;
		auto nodePrimsEnd = primitives.begin() + n->firstPrimitive + n->primitiveCount;
		primitivesAndCentroidsAABB(
				vertices,
				primitivesInfo,
				nodePrimsBegin,
				nodePrimsEnd,
				n->bounds,
				centroidAABB);
		if(n->primitiveCount > maxPrimitivesInLeaf) {
			unsigned short splittingAxis = 0;
			glm::vec3 extents = centroidAABB.max-centroidAABB.min;
			if(extents.y > extents.x)
				splittingAxis = 1;
			if(extents.z > extents.y && extents.z > extents.x)
				splittingAxis = 2;
			float splitVal = centroidAABB.min[splittingAxis] + extents[splittingAxis]/2;
			auto secondGroupBegin = std::partition(nodePrimsBegin, nodePrimsEnd, [&](unsigned primitiveIndex){
					return primitivesInfo[primitiveIndex].centroid[splittingAxis] < splitVal;
					});
			BVHBuildNode *l = new BVHBuildNode,
									 *r = new BVHBuildNode;
			n->children[0].reset(l);
			n->children[1].reset(r);
			l->firstPrimitive = n->firstPrimitive;
			l->primitiveCount = std::distance(nodePrimsBegin, secondGroupBegin);
			l->depth = n->depth+1;
			r->firstPrimitive = l->firstPrimitive + l->primitiveCount;
			r->primitiveCount = n->primitiveCount - l->primitiveCount;
			r->depth = n->depth+1;
			nodes.push(r);
			nodes.push(l);
			FC_TREE_DEPTH = std::max(FC_TREE_DEPTH, n->depth+1);
		}
	}
	compress(std::move(root));
	return primitives;
}

void BVH::compress(BVHBuildNode&& root) {
	std::stack<BVHBuildNode*> nodes;
	nodes.push(&root);
	auto addNode = [&](const AABB& bounds, unsigned firstPrimitive, unsigned primitiveCount){
		_nodes.push_back({bounds, unsigned(-1), 0, bounds.centroid(), glm::length(bounds.centroid()-bounds.min)});
		_nodePrimitives.push_back({firstPrimitive, primitiveCount});
	};
	addNode(root.bounds, root.firstPrimitive, root.primitiveCount);
	root.compressedNodeI = 0;
	while(!nodes.empty()) {
		BVHBuildNode* n = nodes.top();
		if(n->children[0]) {
			n = n->children[0].get();
			addNode(n->bounds, n->firstPrimitive, n->primitiveCount);
			n->compressedNodeI = _nodes.size()-1;
			nodes.push(n);
		}
		else {
			// if the node does not have a left child, it does not have a right child either
			while(!nodes.empty() && !nodes.top()->children[1])
				nodes.pop();
			if(!nodes.empty()) {
				_nodes[nodes.top()->compressedNodeI].rightChild = _nodes.size();
				n = nodes.top()->children[1].get();
				addNode(n->bounds, n->firstPrimitive, n->primitiveCount);
				n->compressedNodeI = _nodes.size()-1;
				nodes.pop();
				nodes.push(n);
			}
		}
	}
	FC_NODE_COUNT = _nodes.size();
}

void BVH::primitivesAndCentroidsAABB(
		const std::vector<Vertex>& vertices,
		const std::vector<PrimitiveInfo>& primitivesInfo,
		std::vector<unsigned>::iterator primitiveIndexBegin,
		std::vector<unsigned>::iterator primitiveIndexEnd,
		AABB& primitivesAABBout,
		AABB& centroidsAABBout) {
	primitivesAABBout = {};
	centroidsAABBout = {};
	for(auto it = primitiveIndexBegin; it != primitiveIndexEnd; ++it) {
		const PrimitiveInfo& info = primitivesInfo[*it];
		primitivesAABBout.unite(vertices[info.indices[0]].position);
		primitivesAABBout.unite(vertices[info.indices[1]].position);
		primitivesAABBout.unite(vertices[info.indices[2]].position);
		centroidsAABBout.unite(info.centroid);
	}
}

PlaneMask octantToFrustumPlaneMask(const glm::vec3& point, const Plane& octantPlaneTop, const Plane& octantPlaneFront, const Plane& octantPlaneRight) {
	glm::vec4 p(point, 1);
	float dTop   = glm::dot(p, octantPlaneTop);
	float dFront = glm::dot(p, octantPlaneFront);
	float dRight = glm::dot(p, octantPlaneRight);
	PlaneMask r = 0;
	if(dTop > 0)
		r |= 1<<FrustumPlane::Top;
	else
		r |= 1<<FrustumPlane::Bot;
	if(dFront > 0)
		r |= 1<<FrustumPlane::Far;
	else
		r |= 1<<FrustumPlane::Near;
	if(dRight > 0)
		r |= 1<<FrustumPlane::Right;
	else
		r |= 1<<FrustumPlane::Left;
	return r;
}

const std::vector<unsigned>& BVH::nodesInFrustum(const std::vector<Plane>& frustumPlanes, const glm::vec3& frustumCenter, const glm::vec3& lookDir, const glm::vec3& up) {
	Plane octantPlaneFront = planeFromNormalAndPoint(lookDir, frustumCenter);
	Plane octantPlaneRight = planeFromNormalAndPoint(glm::cross(lookDir, up), frustumCenter);
	Plane octantPlaneTop = planeFromNormalAndPoint(glm::cross(glm::vec3(octantPlaneRight), lookDir), frustumCenter);
	float frustCenterPlaneDistMin = std::numeric_limits<float>::max();
	for(const Plane& p : frustumPlanes)
		frustCenterPlaneDistMin = fmin(frustCenterPlaneDistMin, glm::dot(p, glm::vec4(frustumCenter, 1)));
	struct NodeInfo {
		unsigned id;
		PlaneMask testedPlanes;
	};
	static std::vector<unsigned> nodesInFrustum;
	nodesInFrustum.clear();
	std::stack<NodeInfo> forward;
	AAboxInPlanesTester_conservative aabbTester(frustumPlanes);
	auto goForward = [&](NodeInfo& n)->bool {
		while(!forward.empty() && forward.top().id <= n.id && forward.top().id != unsigned(-1))
			forward.pop();
		if(forward.empty())
			return false;
		n = forward.top();
		forward.pop();
		return true;
	};
	NodeInfo n = {0, PLANESMASK_ALL};
	while(n.id < _nodes.size()) {
		++FC_NODE_VISITED_COUNT;
		ContainmentType boxFrustumCont;
		if(_nodes[n.id].boundingSphereRadius < frustCenterPlaneDistMin && OCTANT_TEST_ENABLED) { // can do octant test
			const PlaneMask octantPlanesMask = octantToFrustumPlaneMask(_nodes[n.id].centroid, octantPlaneTop, octantPlaneFront, octantPlaneRight);
			PlaneMask planeMask = octantPlanesMask & n.testedPlanes; // do not test against planes disabled by plane masking optimization
			boxFrustumCont = aabbTester.boxInPlanes(_nodes[n.id].bounds, &_nodes[n.id].firstFrustumTestPlane, &planeMask);
			// plane masking might have disabled some additional planes - update the mask stored inside the node
			PlaneMask newlyDisabledPlanes = octantPlanesMask^planeMask;
			n.testedPlanes &= ~newlyDisabledPlanes;
		}
		else
			boxFrustumCont = aabbTester.boxInPlanes(_nodes[n.id].bounds, &_nodes[n.id].firstFrustumTestPlane, &n.testedPlanes);
		if(boxFrustumCont == ContainmentType::Inside) {
			nodesInFrustum.push_back(n.id);
			if(!goForward(n))
				break;
		}
		else if(boxFrustumCont == ContainmentType::Intersecting) {
			if(_nodes[n.id].rightChild == n.id+1 || _nodes[n.id].rightChild == unsigned(-1)) { // the current node is a leaf
				nodesInFrustum.push_back(n.id);
				if(!goForward(n))
					break;
			}
			else {
				forward.push({_nodes[n.id].rightChild, n.testedPlanes});
				++n.id;
			}
		}
		else if(boxFrustumCont == ContainmentType::Outside) {
			if(!goForward(n))
				break;
		}
		else {
			assert(false);
		}
	}
	return nodesInFrustum;
}

const std::vector<NodePrimitives>& BVH::getNodePrimitiveRanges() const {
	return _nodePrimitives;
}
