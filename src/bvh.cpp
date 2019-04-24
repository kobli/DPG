#include <stack>
#include <algorithm>
#include "bvh.hpp"

std::vector<unsigned> BVH::build(const std::vector<Vertex>& vertices, const std::vector<PrimitiveInfo>& primitivesInfo, unsigned maxPrimitivesInLeaf) {
	std::vector<unsigned> primitives(primitivesInfo.size());
	for(unsigned i = 0; i < primitives.size(); ++i)
		primitives[i] = i;
	BVHBuildNode root;
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
			r->firstPrimitive = l->firstPrimitive + l->primitiveCount;
			r->primitiveCount = n->primitiveCount - l->primitiveCount;
			nodes.push(r);
			nodes.push(l);
		}
	}
	compress(std::move(root));
	return primitives;
}

void BVH::compress(BVHBuildNode&& root) {
	std::stack<BVHBuildNode*> nodes;
	nodes.push(&root);
	_nodes.emplace_back();
	_nodes.back().bounds = root.bounds;
	root.compressedNodeI = 0;
	while(!nodes.empty()) {
		BVHBuildNode* n = nodes.top();
		if(n->children[0]) {
			n = n->children[0].get();
			_nodes.emplace_back();
			_nodes.back().bounds = n->bounds;
			_nodePrimitives.push_back({n->firstPrimitive, n->primitiveCount});
			_nodes.back().rightChild = -1;
			n->compressedNodeI = _nodes.size()-1;
			nodes.push(n);
		}
		else {
			// if the node does not have a left child, it does not have a right child either
			while(!nodes.empty() && !nodes.top()->children[1])
				nodes.pop();
			if(!nodes.empty()) {
				_nodes.emplace_back();
				_nodes[nodes.top()->compressedNodeI].rightChild = _nodes.size()-1;
				n = nodes.top()->children[1].get();
				_nodes.back().bounds = n->bounds;
				_nodes.back().rightChild = -1;
				_nodePrimitives.push_back({n->firstPrimitive, n->primitiveCount});
				n->compressedNodeI = _nodes.size()-1;
				nodes.pop();
				nodes.push(n);
			}
		}
	}
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

