#include <exception>
#include "renderer.h"
#include <stdexcept>
#include "texture.h"
int main(int argc, char* argv[]) {
    RendererCore app{};

    try {
        std::cout << "[DRIVER] Running from main.\n";
        //app.run();
        if(argc <= 1) throw std::runtime_error("No filepath supplied.");
        std::string filepath = argv[1];

        const std::string texturePath = filepath;
        
//        Veloxr::OIIOTexture texture(texturePath);
        




        std::cout << "[DRIVER] Init\n";
        app.init();
        auto em = app.getEntityManager();

        {
            Veloxr::OIIOTexture texture("C:/Users/ljuek/Downloads/56000.jpg");
            auto entityHandle = em->createEntity("main");

            Veloxr::VeloxrBuffer buf;
            buf.data = texture.load(texturePath);
            buf.width = texture.getResolution().x;
            buf.height = texture.getResolution().y;
            buf.numChannels = texture.getNumChannels();
            buf.orientation = texture.getOrientation();

            entityHandle->setTextureBuffer(buf);
        }

        // {
        //     Veloxr::OIIOTexture texture("C:/Users/ljuek/Downloads/fox.jpg");
        //     auto entityHandle = em->createEntity("main");
        //
        //     Veloxr::VeloxrBuffer buf;
        //     buf.data = texture.load(texturePath);
        //     buf.width = texture.getResolution().x;
        //     buf.height = texture.getResolution().y;
        //     buf.numChannels = texture.getNumChannels();
        //     buf.orientation = texture.getOrientation();
        //
        //     entityHandle->setTextureBuffer(buf);
        //     entityHandle->setResolution({500, 500});
        // }
        //
        // {
        //     auto entityHandle = em->createEntity("main2");
        //     Veloxr::OIIOTexture texture("C:/Users/ljuek/Downloads/fox_after.jpeg");
        //
        //     Veloxr::VeloxrBuffer buf;
        //     buf.data = texture.load();
        //     buf.width = texture.getResolution().x;
        //     buf.height = texture.getResolution().y;
        //     buf.numChannels = texture.getNumChannels();
        //     buf.orientation = texture.getOrientation();
        //
        //     entityHandle->setTextureBuffer(buf);
        //     entityHandle->setResolution(em->getEntity("main")->getResolution());
        // }


        em->initialize();
        app.setupGraphics();
        app.spin();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
