#ifndef CAMERA_HPP_19_04_21_10_18_54
#define CAMERA_HPP_19_04_21_10_18_54 
#include <glm/glm.hpp>

class Camera {
	public:
		Camera();
		void setPosition(const glm::vec3& pos);
		void setLookDir(const glm::vec3& dir);
		void move(glm::vec3 delta);
		void rotateHoriz(float delta);
		glm::mat4 getViewProjection() const;
		glm::vec3 getPosition() const;
		glm::vec3 getLookDir() const;

	private:
		void updateView();

		glm::vec3 _position;
		glm::vec3 _lookDir;
		glm::mat4 _view;
		glm::mat4 _proj;
};
#endif /* CAMERA_HPP_19_04_21_10_18_54 */
