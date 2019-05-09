#include <iostream>
#include <GL/glew.h>
#include <glm/gtc/matrix_access.hpp>
#include "scene.hpp"
#include "utils.hpp"

Scene::Scene() {
	// load and prepare shaders
	_program = loadShaderProgram({
			{ GL_VERTEX_SHADER, "shaders/pt.vert" },
			{ GL_FRAGMENT_SHADER, "shaders/cameraLight.frag" },
			});
	if(!_program)
		std::cerr << "Failed to load shader program\n";
}

void Scene::render() {
	glUseProgram(_program);
	// set uniforms
	GLint camPosLoc = glGetUniformLocation(_program, "CameraPos");
	if(camPosLoc == -1)
		std::cerr << "Could not set uniform value CameraPos - uniform not found.\n";
	glUniform3fv(camPosLoc, 1, glm::value_ptr(_camera.getPosition()));

	setUniform(_program, _camera.getViewProjection(), "ViewProject");

	float near = _camera.getNear();
	float far = _camera.getFar();
	glm::vec3 frustumCenterWorld = _camera.getPosition() + _camera.getLookDir()*(near + (far-near)/2);

	// draw objects
	for(Object& o : _objects) {
		glm::mat4 modelInverse = glm::inverse(o.getTransform());
		setUniform(_program, o.getTransform(), "Model");
		setUniform(_program, glm::transpose(modelInverse), "ModelInvT");

		const Material& m = o.getMaterial();
		glUniform4fv(glGetUniformLocation(_program, "Mat.color"), 1, &m.color.r);
		glUniform1f(glGetUniformLocation(_program, "Mat.ambientK"), m.ambientK);
		glUniform1f(glGetUniformLocation(_program, "Mat.diffuseK"), m.diffuseK);
		glUniform1f(glGetUniformLocation(_program, "Mat.specularK"), m.specularK);
		glUniform1f(glGetUniformLocation(_program, "Mat.shininess"), m.shininess);

		glm::mat4 mvp = _camera.getViewProjection()*o.getTransform();
		o.draw(
				viewFrustumPlanesFromProjMat(mvp),
				glm::vec3(glm::vec4(frustumCenterWorld, 1)*modelInverse),
				glm::vec3(glm::vec4(_camera.getLookDir(), 0)*modelInverse),
				glm::vec3(glm::vec4(_camera.getUpVector(), 0)*modelInverse),
				true
				);
	}
}

Object& Scene::addObject(const std::string& fileName) {
	_objects.emplace_back(fileName);
	return _objects.back();
}

float Scene::totalObjectGPUDrawTime() const {
	float drawTime = 0;
	for(const Object& o : _objects)
		drawTime += o.getDrawTime();
	return drawTime;
}

unsigned Scene::totalObjectTrianglesRendered() const {
	unsigned triCount = 0;
	for(const Object& o : _objects)
		triCount += o.getRenderedTriangleCount();
	return triCount;
}

unsigned Scene::triangleCount() const {
	unsigned triCount = 0;
	for(const Object& o : _objects)
		triCount += o.getTriangleCount();
	return triCount;
}

Camera& Scene::getCamera() {
	return _camera;
}

// see article from Gribb and Hartmann:
// Fast Extraction of Viewing Frustum Planes from the WorldView-Projection Matrix
// the planes normals point inside
std::vector<Plane> Scene::viewFrustumPlanesFromProjMat(const glm::mat4& mat) {
	using namespace glm;
	std::vector<Plane> planes(6);
	planes[LEFT]  = row(mat,3) + row(mat,0);
	planes[RIGHT] = row(mat,3) - row(mat,0);
	planes[BOT]   = row(mat,3) + row(mat,1);
	planes[TOP]   = row(mat,3) - row(mat,1);
	planes[NEAR]  = row(mat,3) + row(mat,2);
	planes[FAR]   = row(mat,3) - row(mat,2);
	// Normalize the plane equations
	for(Plane& p : planes)
		normalizePlane(p);
	return planes;
}
