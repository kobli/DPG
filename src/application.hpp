#ifndef APPLICATION_HPP_19_04_21_09_04_19
#define APPLICATION_HPP_19_04_21_09_04_19 
#include <memory>
#include <fstream>
#include "scene.hpp"
#include "polyline.hpp"
#include "text.hpp"

/** The main class, which sets up glut, processes events and exports statistics.
 * It is a singleton, because glut should be set up only once - one window, one intput handling function, etc.
 */
class Application {
	public:
		static Application& instance(int argc = 0, char* argv[] = {});
		void run();

	private:
		Application(int argc, char* argv[]);
		~Application();

		void processArgs(int argc, char* argv[]);
		
		/** Draws the text with statistics and status of options.
		 */
		void displayStats();

		/** Move the camera based on which keys are being held down.
		 * Should be called every frame
		 */
		void moveCamera();

		/** This function is called once when a key is pressed.
		 * @param a GLUT character code of the pressed key
		 */
		void onKeyPressed(unsigned char key);

		/** Stores the statistics collected in the last frame.
		 * If no stats file was specified, the function does nothing.
		 */
		void logFrameStats();

		/** A GLUT display function callback.
		 * Does all the rendering.
		 */
		static void display();

		/** A GLUT idle function callback.
		 * Does all the necessary updates that should be done every frame, except rendering.
		 */
		static void idle();

		static void keyDown(unsigned char key, int /*x*/, int /*y*/);
		static void keyUp(unsigned char key, int /*x*/, int /*y*/);

		bool _keyDown[(unsigned char)(-1)];
		std::unique_ptr<Scene> _scene;
		std::unique_ptr<FontRenderer> _fr;
		float _frameTime;
		float _runningTime;
		float _cameraSpeed;
		std::ofstream _statsOutFile;

		PolyLine<PolyLineNode> _cameraRoute;
		std::string _cameraRouteOutFileName;
		float _cameraPlaySpeed;
		PolyLineNode _cameraPlayLineNode;
		bool _quitAfterPlayback;
		bool _cameraPlayPaused;
		float _cameraPlaybackUniformStepSize;
};
#endif /* APPLICATION_HPP_19_04_21_09_04_19 */
