#include <exception>
#include <renderer.h>
#include <stdexcept>
#include <texture.h>
#include <thread>
int main(int argc, char* argv[]) {
    RendererCore app{};

    try {
        std::cout << "[DRIVER] Running from main.\n";
        //app.run();
        if(argc <= 1) throw std::runtime_error("No filepath supplied.");
        std::string filepath = argv[1];

        const std::string texturePath = filepath;
        std::cout << "[DRIVER] Loading texture from: " << texturePath << std::endl;
        
        Veloxr::OIIOTexture texture(texturePath);
        std::cout << "Loading data...";
        
        Veloxr::VeloxrBuffer buf;
        std::cout << "Moving data...";
        buf.data = texture.load(texturePath);
        std::cout << "... done!\n";
        buf.width = texture.getResolution().x;
        buf.height = texture.getResolution().y;
        buf.numChannels = texture.getNumChannels();
        std::cout << "[DRIVER] Sending setTextureBuffer\n";
        app.setTextureBuffer(std::move(buf));



        std::cout << "[DRIVER] Init\n";
        app.init();
        app.spin();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
