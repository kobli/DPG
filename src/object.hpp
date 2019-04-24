#ifndef OBJECT_HPP_19_04_21_09_20_37
#define OBJECT_HPP_19_04_21_09_20_37 
#include <string>
#include "types.hpp"

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
		bool _queryActive;
		AABB _aabb;

		virtual void draw(bool doTimerQuery = false);
};

#endif /* OBJECT_HPP_19_04_21_09_20_37 */
