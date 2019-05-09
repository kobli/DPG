#include <functional>
#include "containment.hpp"
#include "globals.hpp"

ContainmentType pointInPlane(const glm::vec3& point, const Plane& plane) {
	float d = glm::dot(glm::vec4(point, 1), plane);
	if(d == 0)
		return ContainmentType::Intersecting;
	else if(d > 0)
		return ContainmentType::Inside;
	else
		return ContainmentType::Outside;
}

ContainmentType AAboxInPlane(const AABB& box, const Plane& plane) {
	std::vector<glm::vec3> vertices{
		box.min,
		{box.max.x, box.min.y, box.min.z},
		{box.min.x, box.max.y, box.min.z},
		{box.max.x, box.max.y, box.min.z},
		box.max,
		{box.max.x, box.min.y, box.max.z},
		{box.min.x, box.max.y, box.max.z},
		{box.max.x, box.max.y, box.max.z},
	};

	bool someInside = false;
	bool someOutside = false;
	for(const glm::vec3& v : vertices) {
		ContainmentType c = pointInPlane(v, plane);
		if(c == ContainmentType::Inside)
			someInside = true;
		else if(c == ContainmentType::Outside)
			someOutside = true;
	}
	if(!someInside)
		return ContainmentType::Outside;
	else if(!someOutside)
		return ContainmentType::Inside;
	else
		return ContainmentType::Intersecting;
}

ContainmentType AAboxInPlanes_impl(unsigned planeCount, std::function<ContainmentType(unsigned planeI)> aaboxInPlane, uint8_t* failPlane, PlaneMask* enabledPlanes) {
	bool intersecting = false;
	for(unsigned i = 0; i < planeCount; ++i) {
		unsigned planeI = i;
		if(enabledPlanes && !((*enabledPlanes) & 1<<planeI) && PLANE_MASKING_ENABLED)
			continue;
		if(failPlane && PLANE_COHERENCY_ENABLED)
			planeI = (*failPlane+i)%planeCount;
		ContainmentType c = aaboxInPlane(planeI);
		if(c == ContainmentType::Outside) {
			if(failPlane)
				*failPlane = planeI;
			return ContainmentType::Outside;
		}
		else if(c == ContainmentType::Intersecting)
			intersecting = true;
		else {
			if(enabledPlanes)
				*enabledPlanes &= ~(1<<planeI);
		}
	}
	if(intersecting)
		return ContainmentType::Intersecting;
	else
		return ContainmentType::Inside;
}

ContainmentType AAboxInPlanes(const AABB& box, const std::vector<Plane>& planes, uint8_t* failPlane, PlaneMask* enabledPlanes) {
	return AAboxInPlanes_impl(
			planes.size(),
			[&](unsigned planeI){ return AAboxInPlane(box, planes[planeI]); },
			failPlane,
			enabledPlanes);
}


AAboxInPlanesTester::AAboxInPlanesTester(const std::vector<Plane>& planes): _planes{planes} {
}

ContainmentType AAboxInPlanesTester::boxInPlanes(const AABB& box, uint8_t* failPlane, PlaneMask* enabledPlanes) {
	return AAboxInPlanes(box, _planes, failPlane, enabledPlanes);
}


AAboxInPlanesTester_conservative::AAboxInPlanesTester_conservative(const std::vector<Plane>& planes): _planes{planes} {
	_np.reserve(planes.size());
	for(const Plane& p : planes)
		_np.push_back(npIndicesForPlane(p));
}

ContainmentType AAboxInPlanesTester_conservative::boxInPlanes(const AABB& box, uint8_t* failPlane, PlaneMask* enabledPlanes) {
	return AAboxInPlanes_impl(
			_planes.size(),
			[&](unsigned planeI){ return this->AAboxInPlane(box, planeI); },
			failPlane,
			enabledPlanes);
}

AAboxInPlanesTester_conservative::NP AAboxInPlanesTester_conservative::npIndicesForPlane(const Plane& p) {
	unsigned pTableIndex = 
		(p.x > 0)*4 + 
		(p.y > 0)*2 + 
		(p.z > 0)*1;
	unsigned nTableIndex = ~pTableIndex & 7;
	return {_pIndex[nTableIndex], _pIndex[pTableIndex]};
}

// inside means that box is on the side of the plane which the normal points to
ContainmentType AAboxInPlanesTester_conservative::AAboxInPlane(const AABB& box, unsigned i) const {
	if(pointInPlane(box[_np[i].n], _planes[i]) == ContainmentType::Inside)
		return ContainmentType::Inside;
	else if(pointInPlane(box[_np[i].p], _planes[i]) == ContainmentType::Outside)
		return ContainmentType::Outside;
	else
		return ContainmentType::Intersecting;
}


AABB::VertexIndex AAboxInPlanesTester_conservative::_pIndex[] = {
	// x y z plane normal signs
	AABB::VertexIndex::xyz, // - - -  = 0
	AABB::VertexIndex::xyZ, // - - +  = 1
	AABB::VertexIndex::xYz, // - + -  = 2
	AABB::VertexIndex::xYZ, // - + +  = 3
	AABB::VertexIndex::Xyz, // + - -  = 4
	AABB::VertexIndex::XyZ, // + - +  = 5
	AABB::VertexIndex::XYz, // + + -  = 6
	AABB::VertexIndex::XYZ, // + + +  = 7
};
