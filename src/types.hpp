#ifndef TYPES_HPP_18_12_30_14_55_14
#define TYPES_HPP_18_12_30_14_55_14 
#include <limits>
#include <GL/gl.h>
#include <glm/glm.hpp>

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

	AABB& unite(const glm::vec3& p) {
		min = glm::min(min, p);
		max = glm::max(max, p);
		return *this;
	}

	glm::vec3 centroid() const {
		return (min+max)/2.f;
	}
};

#endif /* TYPES_HPP_18_12_30_14_55_14 */
