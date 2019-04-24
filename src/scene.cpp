#include <iostream>
#include <GL/glew.h>
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

	// draw objects
	for(Object& o : _objects) {
		setUniform(_program, o.getTransform(), "Model");
		setUniform(_program, glm::transpose(glm::inverse(o.getTransform())), "ModelInvT");

		const Material& m = o.getMaterial();
		glUniform4fv(glGetUniformLocation(_program, "Mat.color"), 1, &m.color.r);
		glUniform1f(glGetUniformLocation(_program, "Mat.ambientK"), m.ambientK);
		glUniform1f(glGetUniformLocation(_program, "Mat.diffuseK"), m.diffuseK);
		glUniform1f(glGetUniformLocation(_program, "Mat.specularK"), m.specularK);
		glUniform1f(glGetUniformLocation(_program, "Mat.shininess"), m.shininess);

		o.draw(true);
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

Camera& Scene::getCamera() {
	return _camera;
}
