#include <renderer.h>
#include <texture.h>
int main() {
    RendererCore app{};

    try {
        std::cout << "[DRIVER] Running from main.\n";
        //app.run();

        Veloxr::OIIOTexture texture("C:/Users/luker/Downloads/nyx_thumbnail.png");
        std::vector<unsigned char> data = texture.load();
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
