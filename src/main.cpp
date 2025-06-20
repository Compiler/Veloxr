#include <renderer.h>
#include <texture.h>
int main() {
    RendererCore app{};

    try {
        std::cout << "[DRIVER] Running from main.\n";
        //app.run();

        const std::string texturePath = "/Users/joshyoo/Desktop/test_data/old_woman-gigapixel-recover v2-2x-faceai v2-dust.png";
        std::cout << "[DRIVER] Loading texture from: " << texturePath << std::endl;
        
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
        app.init();
        app.setTextureBuffer(std::move(buf));
        app.spin();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
