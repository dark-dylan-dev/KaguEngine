import App;

import std;

int main() {
    try {
        KaguEngine::App Engine{};
        Engine.run();
    }
    catch (std::exception& error) {
        std::cerr << error.what() << '\n';
    }
}