#include <iostream>
#include <sstream>
#include <unistd.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include "utils.hpp"
#include "application.hpp"
#include "camera.hpp"
#include "object.hpp"

static const float CAMERA_PLAY_SPEED = 100;

Application& Application::instance(int argc, char* argv[]) {
	static Application instance(argc, argv);
	return instance;
}

void Application::run() {
	glutMainLoop();
}

Application::Application(int argc, char* argv[]):
	_frameTime{0}, 
	_cameraSpeed{100},
	_cameraPlaySpeed{0},
	_quitAfterPlayback{false},
	_cameraPlayPaused{false},
	_cameraPlaybackUniformStepSize{0}
{
	using namespace std;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitContextVersion (4, 5);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(1024,1024);
	glutCreateWindow(argv[0]);

	if(glewInit()) {
		cerr << "Cannot initialize GLEW\n";
		exit(EXIT_FAILURE);
	}
	if(glDebugMessageCallback){
		cout << "Register OpenGL debug callback " << endl;
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(openglCallbackFunction, nullptr);
		GLuint unusedIds = 0;
		glDebugMessageControl(GL_DONT_CARE,
				GL_DONT_CARE,
				GL_DONT_CARE,
				0,
				&unusedIds,
				true);
	}
	else
		cout << "glDebugMessageCallback not available" << endl;

	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutKeyboardFunc(Application::keyDown);
	glutKeyboardUpFunc(Application::keyUp);
	glutIdleFunc(Application::idle);
	glutDisplayFunc(Application::display);

	_fr.reset(new FontRenderer("../data/arial.ttf", 1024, 1024));
	_scene.reset(new Scene());
	processArgs(argc, argv);
}

Application::~Application() {
	_cameraRoute.save(_cameraRouteOutFileName);
}

void Application::processArgs(int argc, char* argv[]) {
	using namespace std;
	int opt;
	std::string cameraRouteIn,
							cameraRouteOut;
	float initialT = 0;
	while((opt = getopt(argc, argv, "s:r:p:qm:u:t:")) != -1) {
		switch(opt) {
			case 's':
				{
					Object& o = _scene->addObject(optarg);
					_scene->getCamera().setPosition(
							glm::vec3(
								o.getTransform() * glm::vec4(o.getAABB().centroid(), 1.0))
							);
					break;
				}
			case 'r':
				cameraRouteOut = optarg;
				break;
			case 'p':
				cameraRouteIn = optarg;
				break;
			case 'q':
				_quitAfterPlayback = true;
				break;
			case 'm':
				_statsOutFile.open(optarg);
				if(!_statsOutFile)
					cerr << "Failed to open stats output file " << optarg << endl;
				break;
			case 'u':
				_cameraPlaybackUniformStepSize = std::atof(optarg);
				break;
			case 't':
				initialT = std::atof(optarg);
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: %s -s sceneFile [[-r cameraRouteRecordFile] | [-p cameraRoutePlayFile]] [-q] [-m statisticsOutputFile] [-u playbackUniformStepSize] [-t initialPlaybackT]\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	if(!cameraRouteIn.empty()) {
		_cameraRoute.load(cameraRouteIn);
		_cameraPlayLineNode = _cameraRoute.getNode(initialT);
		_scene->getCamera().setPosition(_cameraPlayLineNode.position);
		_scene->getCamera().setLookDir(_cameraPlayLineNode.direction);
		_cameraPlaySpeed = CAMERA_PLAY_SPEED;
		if(initialT)
			_cameraPlayPaused = true;
	}
	else if(!cameraRouteOut.empty())
		_cameraRouteOutFileName = cameraRouteOut;
}

void Application::displayStats() {
	using namespace std;
	stringstream ss;
	ss << "FPS: " << int(1/_frameTime) << endl;
	ss << "Draw time [ms]: " << _scene->totalObjectGPUDrawTime() << endl;
	_fr->RenderText(ss.str(), 10, 100, 0.4, {1,0,0});
}

void Application::display() {
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	instance()._scene->render();
	instance().displayStats();
	glutSwapBuffers();
}

void Application::moveCamera() {
	const float rotSpeed = 1.0f;
	Camera& cam = _scene->getCamera();
	glm::vec3 forward = cam.getLookDir();
	if(_keyDown[(unsigned char)'q'])
		cam.rotateHoriz(rotSpeed*_frameTime);
	if(_keyDown[(unsigned char)'e'])
		cam.rotateHoriz(-rotSpeed*_frameTime);
	if(_keyDown[(unsigned char)'a'])
		cam.move(-glm::normalize(glm::cross(forward,UP))*_frameTime*_cameraSpeed);
	if(_keyDown[(unsigned char)'d'])
		cam.move(glm::normalize(glm::cross(forward,UP))*_frameTime*_cameraSpeed);
	if(_keyDown[(unsigned char)'w'])
		cam.move(forward*_frameTime*_cameraSpeed);
	if(_keyDown[(unsigned char)'s'])
		cam.move(-forward*_frameTime*_cameraSpeed);
	if(_keyDown[(unsigned char)'k'])
		cam.move(UP*_frameTime*_cameraSpeed);
	if(_keyDown[(unsigned char)'j'])
		cam.move(-UP*_frameTime*_cameraSpeed);

	if(_cameraPlaySpeed != 0 && !_cameraPlayPaused) {
		if(_cameraPlaybackUniformStepSize)
			_cameraPlayLineNode = _cameraRoute.getNode(std::min(1.f, _cameraPlayLineNode.t + _cameraPlaybackUniformStepSize));
		else
			_cameraPlayLineNode = _cameraRoute.getNode(_cameraPlayLineNode, _cameraPlaySpeed*_frameTime);
		cam.setPosition(_cameraPlayLineNode.position);
		cam.setLookDir(_cameraPlayLineNode.direction);
	}
}

void Application::onKeyPressed(unsigned char key) {
	const Camera& cam = _scene->getCamera();
	switch(key) {
		case 'r':
			_cameraRoute.appendNode(PolylineNode(cam.getPosition(), cam.getLookDir()));
			break;
		case 'p':
			_cameraPlayPaused = !_cameraPlayPaused;
			break;
		case '+':
			_cameraSpeed *= 2;
			break;
		case '-':
			_cameraSpeed /= 2;
			break;
	}
}

void Application::logFrameStats() {
	if(_statsOutFile) {
		_statsOutFile << _cameraPlayLineNode.t << " " 
		 << _frameTime << " "
		 << std::endl;
		if(!_statsOutFile)
			std::cerr << "Writing frame stats failed\n";
	}
}

void Application::idle() {
	static float old_t = glutGet(GLUT_ELAPSED_TIME);
	int t;
	t = glutGet(GLUT_ELAPSED_TIME);
	instance()._frameTime = (t - old_t) / 1000.0;
	old_t = t;

	instance().logFrameStats();
	if(instance()._quitAfterPlayback && instance()._cameraPlayLineNode.t == 1)
		exit(0);
	instance().moveCamera();
	glutPostRedisplay();
}

void Application::keyDown(unsigned char key, int /*x*/, int /*y*/) {
	instance()._keyDown[key] = true;
	instance().onKeyPressed(key);
}

void Application::keyUp(unsigned char key, int /*x*/, int /*y*/) {
	instance()._keyDown[key] = false;
}
