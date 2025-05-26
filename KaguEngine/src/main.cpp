#include "Core.hpp"

int main() {
    try {
        KaguEngine::Core Engine{};
        Engine.run();
    }
    catch (std::exception& error) {
        std::cerr << error.what() << std::endl;
    }
}