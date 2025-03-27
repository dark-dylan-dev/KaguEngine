#include "KaguEngine.hpp"

int main() {
	KaguEngine::App Engine;

	try {
		Engine.run();
	}
	catch (std::exception& error) {
		std::cerr << error.what() << std::endl;
	}
}