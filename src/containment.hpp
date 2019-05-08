#ifndef CONTAINMENT_HPP_19_05_08_11_50_21
#define CONTAINMENT_HPP_19_05_08_11_50_21 
#include <vector>
#include "types.hpp"

enum ContainmentType {
	Inside,
	Intersecting,
	Outside
};

ContainmentType pointInPlane(const glm::vec3& point, const Plane& plane);
ContainmentType AAboxInPlane(const AABB& box, const Plane& plane);
ContainmentType AAboxInPlanes(const AABB& box, const std::vector<Plane>& planes);

class AAboxInPlanesTesterBase {
	public:
		virtual ContainmentType boxInPlanes(const AABB& box, uint8_t* failPlane) = 0;
};

class AAboxInPlanesTester: public AAboxInPlanesTesterBase {
	public:
		AAboxInPlanesTester(const std::vector<Plane>& planes);
		virtual ContainmentType boxInPlanes(const AABB& box, uint8_t* failPlane = nullptr) override;

	private:
		const std::vector<Plane>& _planes;
};


// may report "intersecting" when the box is outside the planes
// source article: Optimized View Frustum Culling Algorithms for Bounding Boxes (Ulf Assarsson and Tomas Möller)
class AAboxInPlanesTester_conservative: public AAboxInPlanesTesterBase
{
	// P is the index of farthest box vertex along the plane normal, N is the index of the opposite vertex
	struct NP {
		AABB::VertexIndex n;
		AABB::VertexIndex p;
	};

	public:
		AAboxInPlanesTester_conservative(const std::vector<Plane>& planes);
		virtual ContainmentType boxInPlanes(const AABB& box, uint8_t* failPlane = nullptr) override;

	private:
		NP npIndicesForPlane(const Plane& p);
		// inside means that box is on the side of the plane which the normal points to
		ContainmentType AAboxInPlane(const AABB& box, unsigned i) const;

		// index is formed by taking the signs of the plane normal components (+ -> 1, - -> 0) as bit vector xyz
		static AABB::VertexIndex _pIndex[2*2*2];

		const std::vector<Plane> _planes;
		std::vector<NP> _np; // N and P - indices of AABB vertices for each plane
};

#endif /* CONTAINMENT_HPP_19_05_08_11_50_21 */
