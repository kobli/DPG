#ifndef APPLICATION_HPP_19_04_21_09_04_19
#define APPLICATION_HPP_19_04_21_09_04_19 
#include <memory>
#include <fstream>
#include "scene.hpp"
#include "polyline.hpp"
#include "text.hpp"

class Application {
	public:
		static Application& instance(int argc = 0, char* argv[] = {});
		void run();

	private:
		Application(int argc, char* argv[]);
		~Application();

		void processArgs(int argc, char* argv[]);
		void displayStats();
		void moveCamera();
		void onKeyPressed(unsigned char key);
		void logFrameStats();

		static void display();
		static void idle();
		static void keyDown(unsigned char key, int /*x*/, int /*y*/);
		static void keyUp(unsigned char key, int /*x*/, int /*y*/);

		bool _keyDown[(unsigned char)(-1)];
		std::unique_ptr<Scene> _scene;
		std::unique_ptr<FontRenderer> _fr;
		float _frameTime;
		float _cameraSpeed;
		std::ofstream _statsOutFile;

		Polyline<PolylineNode> _cameraRoute;
		std::string _cameraRouteOutFileName;
		float _cameraPlaySpeed;
		PolylineNode _cameraPlayLineNode;
		bool _quitAfterPlayback;
		bool _cameraPlayPaused;
		float _cameraPlaybackUniformStepSize;
};
#endif /* APPLICATION_HPP_19_04_21_09_04_19 */
