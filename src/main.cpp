#include <exception>
#include <renderer.h>
#include <stdexcept>
#include <texture.h>
int main(int argc, char* argv[]) {
    RendererCore app{};

    try {
        std::cout << "[DRIVER] Running from main.\n";
        //app.run();
        if(argc <= 1) throw std::runtime_error("No filepath supplied.");
        std::string filepath = argv[1];

        const std::string texturePath = filepath;
        std::cout << "[DRIVER] Loading texture from: " << texturePath << std::endl;
        
        /*
        Veloxr::OIIOTexture texture(texturePath);
        std::vector<unsigned char> data = texture.load();
        if (data.empty()) {
            throw std::runtime_error("Failed to load texture data from: " + texturePath);
        }
        
        Veloxr::VeloxrBuffer buf;
        buf.data = std::move(data);
        buf.width = texture.getResolution().x;
        buf.height = texture.getResolution().y;
        buf.numChannels = texture.getNumChannels();
        std::cout << "[DRIVER] Sending setTextureBuffer\n";
	    //app.setTextureFilePath(texturePath);
        app.setTextureBuffer(std::move(buf));
        */



        std::cout << "[DRIVER] Init\n";
        app.setTextureFilePath("C:/Users/luker/Downloads/test.png");
        app.init();
        app.spin();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
