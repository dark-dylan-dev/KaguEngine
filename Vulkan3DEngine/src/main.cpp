#include "Vulkan3DEngine.hpp"

int main() {
	Vulkan3DEngine Engine;

	try {
		Engine.run();
	}
	catch (std::exception& error) {
		std::cerr << error.what() << std::endl;
	}
}