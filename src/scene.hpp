#ifndef SCENE_HPP_19_04_21_09_17_56
#define SCENE_HPP_19_04_21_09_17_56 
#include <vector>
#include "object.hpp"
#include "camera.hpp"

class Scene {
	public:
		Scene();
		void render();

		/** Add object from file name.
		 * Only obj files with three vertices per face are supported.
		 */
		Object& addObject(const std::string& fileName);

		/** Returns the time in took for the GPU to draw all the objects in milliseconds in the last frame.
		 * It is measured using timer query.
		 */
		float totalObjectGPUDrawTime() const;

		/** Returns the number of triangles that were sent for rendering in the last frame.
		 */
		unsigned totalObjectTrianglesRendered() const;

		/** Returns the total number of triangles for all objects
		 */
		unsigned triangleCount() const;

		Camera& getCamera();

	private:
		/**
		 * Calculates view frustum planes from given projection matrix.
		 * If the matrix is a view-projection matrix, then the planes are in world space.
		 * If the matrix is a model-view-projection matrix, then the planes are in model space.
		 * The planes normals point inside.
		 *
		 * source:
		 * article from Gribb and Hartmann:
		 * Fast Extraction of Viewing Frustum Planes from the WorldView-Projection Matrix
		 */
		std::vector<Plane> viewFrustumPlanesFromProjMat(const glm::mat4& proj);

		std::vector<Object> _objects;
		Camera _camera;
		GLuint _program;
};
#endif /* SCENE_HPP_19_04_21_09_17_56 */
