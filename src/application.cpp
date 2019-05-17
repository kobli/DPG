#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <map>
#include <chrono>
#include "libs.hpp"
#include "utils.hpp"
#include "application.hpp"
#include "camera.hpp"
#include "object.hpp"
#include "globals.hpp"
#include "circularBuffer.hpp"

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
	_runningTime{0}, 
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
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	if(glewInit()) {
		cerr << "Cannot initialize GLEW\n";
		exit(EXIT_FAILURE);
	}
	if(glDebugMessageCallback){
		cout << "Register OpenGL debug callback " << endl;
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(GLDEBUGPROC(openglCallbackFunction), nullptr);
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
	if(!_cameraRouteOutFileName.empty()) {
		if(_cameraRoute.save(_cameraRouteOutFileName))
			std::cout << "Camera route saved.\n";
		else
			std::cerr << "Failed to save camera route.\n";
	}
}

void Application::processArgs(int argc, char* argv[]) {
	using namespace std;
	map<string, string> argMap;
	auto valueIt = argMap.insert({string(),{}}).first;
	for(int i = 0; i < argc; ++i) {
		string w(argv[i]+1);
		if(argv[i][0] == '-' && std::all_of(w.begin(), w.end(), [](unsigned char c){return !(std::isdigit(c) || std::isblank(c));}))
			valueIt = argMap.insert({w, {}}).first;
		else {
			if(!valueIt->second.empty())
				valueIt->second += " ";
			valueIt->second += string(argv[i]);
		}
	}
	// process arguments
	if(argMap.count("c") != 0) {
		MAX_PRIMITIVES_IN_LEAF = stof(argMap["c"]);
	}
	if(argMap.count("s") != 0) {
		Object& o = _scene->addObject("../data/"+argMap["s"]);
		_scene->getCamera().setPosition(
				glm::vec3(
					o.getTransform() * glm::vec4(o.getAABB().centroid(), 1.0))
				);
	}
	else {
		cerr << "Scene name is a required argument.\n";
		exit(1);
	}
	if(argMap.count("vp")) {
		glm::vec3 camPos;
		if(!sToVec(argMap["vp"], camPos)) {
			cerr << "Could not parse camera position argument.\n";
			exit(1);
		}
		_scene->getCamera().setPosition(camPos);
	}
	if(argMap.count("vd")) {
		glm::vec3 camDir;
		if(!sToVec(argMap["vd"], camDir)) {
			cerr << "Could not parse camera direction argument.\n";
			exit(1);
		}
		_scene->getCamera().setLookDir(camDir);
	}
	if(argMap.count("vu")) {
		glm::vec3 up;
		if(!sToVec(argMap["vu"], up)) {
			cerr << "Could not parse camera UP argument.\n";
			exit(1);
		}
		_scene->getCamera().setUpVector(up);
	}
	if(argMap.count("vf")) {
		_scene->getCamera().setFOV(stof(argMap["vf"]));
	}
	float initialT = 0;
	if(argMap.count("t")) {
		initialT = stof(argMap["t"]);
	}
	if(argMap.count("p")) {
		_cameraRoute.load(argMap["p"]);
		_cameraPlayLineNode = _cameraRoute.getNode(initialT);
		_scene->getCamera().setPosition(_cameraPlayLineNode.position);
		_scene->getCamera().setLookDir(_cameraPlayLineNode.direction);
		_cameraPlaySpeed = CAMERA_PLAY_SPEED;
		if(initialT)
			_cameraPlayPaused = true;
	}
	if(argMap.count("r"))
		_cameraRouteOutFileName = argMap["r"];
	if(argMap.count("q"))
		_quitAfterPlayback = true;
	if(argMap.count("u"))
		_cameraPlaybackUniformStepSize = stof(argMap["u"]);
	if(argMap.count("m")) {
		_statsOutFile.open(argMap["m"]);
		if(!_statsOutFile)
			cerr << "Failed to open stats output file " << argMap["m"] << endl;
	}
	if(argMap.count("no-frustum-culling"))
		FRUSTUM_CULLING_ENABLED = false;
	if(argMap.count("no-octant-test"))
		OCTANT_TEST_ENABLED = false;
	if(argMap.count("no-plane-masking"))
		PLANE_MASKING_ENABLED = false;
	if(argMap.count("no-plane-coherency"))
		PLANE_COHERENCY_ENABLED = false;
	if(argMap.count("no-camera-coherency"))
		CAMERA_COHERENCY_ENABLED = false;
}

void Application::displayStats() {
	using namespace std;
	const unsigned circSize = 5;
	static CircularBuffer<float> drawTimeCirc(circSize);
	static CircularBuffer<float> travTimeCirc(circSize);
	static CircularBuffer<float> frameTimeCirc(circSize);
	static auto lastUpdate = chrono::steady_clock::now();

	if(chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now()-lastUpdate).count() > 200) {
		lastUpdate = chrono::steady_clock::now();
		drawTimeCirc.add(_scene->totalObjectGPUDrawTime());
		travTimeCirc.add(FC_TRAVERSE_TIME);
		frameTimeCirc.add(_frameTime);
	}

	stringstream ss;
	ss.precision(3);
	ss << "FPS: " << int(1/frameTimeCirc.avg()) << endl;
	ss << "Draw time [ms]: " << drawTimeCirc.avg() << endl;
	ss << "Triangles rendered / total: " << _scene->totalObjectTrianglesRendered() << " / " << _scene->triangleCount() << endl;
	ss << "Culling time [ms]: " << travTimeCirc.avg() << endl;
	ss << "Visited node count / total: " << FC_NODE_VISITED_COUNT << " / " << FC_NODE_COUNT << endl;
	ss << "Tree depth: " << FC_TREE_DEPTH << endl;
	ss << "Max tris per leaf: " << MAX_PRIMITIVES_IN_LEAF << endl;
	ss << "Backface culling: " << BF_CULLING_ENABLED << endl;
	ss << "VF culling: " << FRUSTUM_CULLING_ENABLED;
	if(FRUSTUM_CULLING_ENABLED) {
		ss << "(";
		if(OCTANT_TEST_ENABLED)
			ss << " + octant t.";
		if(PLANE_MASKING_ENABLED)
			ss << " + plane masking";
		if(PLANE_COHERENCY_ENABLED)
			ss << " + plane coh.";
		if(CAMERA_COHERENCY_ENABLED)
			ss << " + camera coh.";
		ss << ")";
	}
	ss << endl;
	_fr->RenderText(ss.str(), 10, 150, 0.4, {1,0,0});
}

void Application::display() {
	FC_NODE_VISITED_COUNT = 0;
	FC_TRAVERSE_TIME = 0;
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
	glm::vec3 up = cam.getUpVector();
	if(_keyDown[(unsigned char)'q'])
		cam.rotateHoriz(rotSpeed*_frameTime);
	if(_keyDown[(unsigned char)'e'])
		cam.rotateHoriz(-rotSpeed*_frameTime);
	if(_keyDown[(unsigned char)'a'])
		cam.move(-glm::normalize(glm::cross(forward,up))*_frameTime*_cameraSpeed);
	if(_keyDown[(unsigned char)'d'])
		cam.move(glm::normalize(glm::cross(forward,up))*_frameTime*_cameraSpeed);
	if(_keyDown[(unsigned char)'w'])
		cam.move(forward*_frameTime*_cameraSpeed);
	if(_keyDown[(unsigned char)'s'])
		cam.move(-forward*_frameTime*_cameraSpeed);
	if(_keyDown[(unsigned char)'k'])
		cam.move(up*_frameTime*_cameraSpeed);
	if(_keyDown[(unsigned char)'j'])
		cam.move(-up*_frameTime*_cameraSpeed);

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
			_cameraRoute.appendNode(PolyLineNode(cam.getPosition(), cam.getLookDir()));
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
		case 'b':
			BF_CULLING_ENABLED = !BF_CULLING_ENABLED;
			break;
		case 'f':
			FRUSTUM_CULLING_ENABLED = !FRUSTUM_CULLING_ENABLED;
			break;
		case 'o':
			OCTANT_TEST_ENABLED = !OCTANT_TEST_ENABLED;
			break;
		case 'm':
			PLANE_MASKING_ENABLED = !PLANE_MASKING_ENABLED;
			break;
		case 'l':
			PLANE_COHERENCY_ENABLED = !PLANE_COHERENCY_ENABLED;
			break;
		case 'c':
			CAMERA_COHERENCY_ENABLED = !CAMERA_COHERENCY_ENABLED;
			break;
	}
}

void Application::logFrameStats() {
	if(_statsOutFile.is_open()) {
		_statsOutFile << _cameraPlayLineNode.t << " " 
		 << _frameTime << " "
		 << _scene->totalObjectGPUDrawTime() << " "
		 << _scene->totalObjectTrianglesRendered() << " "
		 << _scene->triangleCount() << " "
		 << FC_NODE_VISITED_COUNT << " "
		 << FC_NODE_COUNT << " "
		 << FC_TRAVERSE_TIME << " "
		 << std::endl;
		if(!_statsOutFile)
			std::cerr << "Writing frame stats failed\n";
	}
}

void Application::idle() {
	static float old_t = glutGet(GLUT_ELAPSED_TIME);
	float t = glutGet(GLUT_ELAPSED_TIME);
	instance()._frameTime = (t - old_t) / 1000.0;
	old_t = t;
	instance()._runningTime += instance()._frameTime;

	instance().logFrameStats();
	if(instance()._quitAfterPlayback && instance()._cameraPlayLineNode.t == 1)
		exit(0);
	if(instance()._quitAfterPlayback && instance()._runningTime >= 1)
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
