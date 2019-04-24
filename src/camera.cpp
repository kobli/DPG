#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "camera.hpp"
#include "utils.hpp"

Camera::Camera():
_lookDir{glm::vec3(1,0,0)}
{
	_proj = glm::perspective(glm::radians(60.0f), 1.0f, 1.f, 10000.0f);
	updateView();
}

void Camera::setPosition(const glm::vec3& pos) {
	_position = pos;
	updateView();
}

void Camera::setLookDir(const glm::vec3& dir) {
	_lookDir = dir;
	updateView();
}

void Camera::move(glm::vec3 delta) {
	_position += delta;
	updateView();
}

void Camera::rotateHoriz(float delta) {
	_lookDir = glm::rotateY(_lookDir, delta);
	updateView();
}

glm::mat4 Camera::getViewProjection() const {
	return _proj*_view;
}

glm::vec3 Camera::getPosition() const {
	return _position;
}

glm::vec3 Camera::getLookDir() const {
	return _lookDir;
}

void Camera::updateView() {
	_view	= glm::lookAt(_position, _position+getLookDir(), UP);
}
