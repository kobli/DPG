#ifndef OBJECT_HPP_19_04_21_09_20_37
#define OBJECT_HPP_19_04_21_09_20_37 
#include <string>
#include "types.hpp"
#include "bvh.hpp"

class Object {
	friend class Scene;
	public:
		/** Add object from file name.
		 * Only obj files with three vertices per face are supported.
		 */
		Object(const std::string& fileName);

		Object(const Object& o) = delete;
		Object& operator=(const Object& o) = delete;

		Object(Object&& o) = default;
		Object& operator=(Object&& o) = default;

		~Object();

		void setPosition(glm::vec3 pos);

		glm::vec3 getPosition() const;

		/** Returns the time in took for the GPU to draw the object in milliseconds in the last frame.
		 * It is measured using timer query.
		 */
		double getDrawTime() const;

		/** Returns the number of triangles of this object that were sent for rendering in the last frame.
		 */
		unsigned getRenderedTriangleCount() const;

		/** Returns the number of triangles of the object.
		 */
		unsigned getTriangleCount() const;

		glm::mat4 getTransform() const;

		Material& getMaterial();

		/** Returns untransformed axis-aligned bounding box.
		 */
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
		AABB _aabb; /// used for frustum culling
		BVH _bvh;
		glm::vec3 _prevFrustumCenter;
		std::vector<unsigned> _visibleNodes; /// from last frame - caching used if the view did not change

		/** Optionally does the timer query and calls doDrawing.
		 * FrustumCenter, lookDir, up are all in model space.
		 */
		virtual void draw(const std::vector<Plane>& frustumPlanes, const glm::vec3& frustumCenter, const glm::vec3& lookDir, const glm::vec3& up, bool doTimerQuery = false);

		/** Actually draws the primitives.
		 * FrustumCenter, lookDir, up are all in model space.
		 */
		void doDrawing(const std::vector<Plane>& frustumPlanes, const glm::vec3& frustumCenter, const glm::vec3& lookDir, const glm::vec3& up);
};

#endif /* OBJECT_HPP_19_04_21_09_20_37 */
