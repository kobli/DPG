#ifndef CAMERA_HPP_19_04_21_10_18_54
#define CAMERA_HPP_19_04_21_10_18_54 
#include <glm/glm.hpp>

class Camera {
	public:
		Camera();
		void setPosition(const glm::vec3& pos);
		void setLookDir(const glm::vec3& dir);

		/** Adds the delta to the position.
		 */
		void move(glm::vec3 delta);

		/** Rotates the camera around Y axis.
		 * setUpVector has no effect on the axis of rotation.
		 */
		void rotateHoriz(float delta);

		glm::mat4 getViewProjection() const;
		glm::vec3 getPosition() const;
		glm::vec3 getLookDir() const;
		float getNear() const;
		float getFar() const;
		void setUpVector(const glm::vec3& up);
		glm::vec3 getUpVector() const;

		/** Sets the horizontal and vertical field of view in radians.
		 */
		void setFOV(float fov);

	private:
		void updateView();

		glm::vec3 _position;
		glm::vec3 _lookDir;
		glm::mat4 _view;
		glm::mat4 _proj;
		glm::vec3 _up;
		float _fov; // radians
};
#endif /* CAMERA_HPP_19_04_21_10_18_54 */
