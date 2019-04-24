#include "application.hpp"

int main(int argc, char* argv[]) {
	Application::instance(argc, argv).run();
}
