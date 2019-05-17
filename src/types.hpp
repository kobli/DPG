/** @file */
#ifndef TYPES_HPP_18_12_30_14_55_14
#define TYPES_HPP_18_12_30_14_55_14 

#include <sstream>
#include <limits>
#include "libs.hpp"

struct Color {
	float r;
	float g;
	float b;
	float a;
};

struct Material {
	Color color = {1,1,1,1};
	GLfloat ambientK;
	GLfloat diffuseK;
	GLfloat specularK;
	GLfloat shininess;
};

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
};

struct AABB {
	glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());

	/** Enlarges this AABB if necessary to contain the given point
	 */
	AABB& unite(const glm::vec3& p) {
		min = glm::min(min, p);
		max = glm::max(max, p);
		return *this;
	}

	glm::vec3 centroid() const {
		return (min+max)/2.f;
	}

	/** lower case = min, upper case = max
	 */
	enum VertexIndex {
		xyz,
		xyZ,
		xYz,
		xYZ,
		Xyz,
		XyZ,
		XYz,
		XYZ,
	};

	glm::vec3 operator[](VertexIndex vi) const {
		assert(vi >= 0 && vi <= VertexIndex::XYZ);
		glm::vec3 r = min;
		if(vi & 1<<2)
			r.x = max.x;
		if(vi & 1<<1)
			r.y = max.y;
		if(vi & 1<<0)
			r.z = max.z;
		return r;
	}
};

using Plane = glm::vec4;

inline void normalizePlane(Plane& p) {
	float l = glm::length(glm::vec3(p));
	p /= l;
}

/** Calculates a plane from normal and one contained point.
 * The normal must already be normalized
 */
inline Plane planeFromNormalAndPoint(const glm::vec3& normal, const glm::vec3& point) {
	return glm::vec4(normal, -glm::dot(normal, point));
}

inline bool sToVec(const std::string& s, glm::vec3& v) {
	std::stringstream ss(s);
	ss >> v.x >> v.y >> v.z;
	return !ss.fail();
}

enum FrustumPlane {
	Left,
	Right,
	Bot,
	Top,
	Near,
	Far,
};
#endif /* TYPES_HPP_18_12_30_14_55_14 */
