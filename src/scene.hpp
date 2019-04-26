#ifndef SCENE_HPP_19_04_21_09_17_56
#define SCENE_HPP_19_04_21_09_17_56 
#include <vector>
#include "object.hpp"
#include "camera.hpp"

class Scene {
	public:
		Scene();
		void render();
		Object& addObject(const std::string& fileName);
		float totalObjectGPUDrawTime() const;
		unsigned totalObjectTrianglesRendered() const;
		unsigned triangleCount() const;
		Camera& getCamera();

	private:
		std::vector<Plane> viewFrustumPlanesFromProjMat(const glm::mat4& proj);

		std::vector<Object> _objects;
		Camera _camera;
		GLuint _program;
};
#endif /* SCENE_HPP_19_04_21_09_17_56 */
