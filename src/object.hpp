#ifndef OBJECT_HPP_19_04_21_09_20_37
#define OBJECT_HPP_19_04_21_09_20_37 
#include <string>
#include "types.hpp"
#include "bvh.hpp"

class Object {
	friend class Scene;
	public:
		Object(const std::string& fileName);

		Object(const Object& o) = delete;
		Object& operator=(const Object& o) = delete;

		Object(Object&& o) = default;
		Object& operator=(Object&& o) = default;

		~Object();

		void setPosition(glm::vec3 pos);
		glm::vec3 getPosition() const;
		double getDrawTime() const;
		unsigned getRenderedTriangleCount() const;
		glm::mat4 getTransform() const;
		Material& getMaterial();
		const AABB& getAABB() const;

	private:
		GLuint _queryID;
		GLuint _vao;
		GLuint _indexBuffer;
		GLuint _vertexBuffer;
		GLuint _vertexCount;
		GLuint _triangleCount;
		glm::mat4 _transform;
		Material _material;
		double _drawTime;
		unsigned _renderedTriangleCount;
		bool _queryActive;
		AABB _aabb;
		BVH _bvh;

		virtual void draw(const std::vector<Plane>& frustumPlanes, bool doTimerQuery = false);
		void doDrawing(const std::vector<Plane>& frustumPlanes);
};

#endif /* OBJECT_HPP_19_04_21_09_20_37 */
