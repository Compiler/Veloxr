
#pragma once


#include <string>
#include <iostream>
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <iostream>
#include <string>
#include <chrono>

OIIO_NAMESPACE_USING  

struct Test{

    int run2(std::string input_filename, std::string output_filename) {
        auto now = std::chrono::high_resolution_clock::now();
        

        auto in = ImageInput::open(input_filename);
        if (!in) {
            std::cerr << "Could not open input: " << input_filename << "\n";
            return 1;
        }

        std::cout << "ImageInput::Open => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms \n";

        now = std::chrono::high_resolution_clock::now();
        const ImageSpec &in_spec = in->spec();
        int in_width    = in_spec.width;
        int in_height   = in_spec.height;
        int nchannels   = in_spec.nchannels;

        std::cout << "Input image: " << in_width << "x" << in_height 
            << " channels=" << nchannels << std::endl;

        int skip = in_width / 4096;  
        int out_width  = in_width  / skip;  
        int out_height = in_height / skip;  


        std::cout << "ImageInput::Spec => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms \n"; now = std::chrono::high_resolution_clock::now();
        auto out = ImageOutput::create(output_filename);
        if (!out) {
            std::cerr << "Could not create output: " << output_filename << "\n";
            return 1;
        }
        std::cout << "ImageOutput::create => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms \n"; now = std::chrono::high_resolution_clock::now();
        ImageSpec out_spec(out_width, out_height, nchannels, TypeDesc::FLOAT);
        out->open(output_filename, out_spec);

        std::vector<float> in_scanline(in_width * nchannels);
        std::vector<float> out_scanline(out_width * nchannels);

        std::cout << "ImageOutput::spec => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms \n"; now = std::chrono::high_resolution_clock::now();
        for (int y = 0, out_y = 0; y < in_height && out_y < out_height; y += skip, ++out_y) {
            if (!in->read_scanline(y, 0, TypeDesc::FLOAT, in_scanline.data())) {
                std::cerr << "Read error at scanline " << y << ": " 
                    << in->geterror() << "\n";
                break;
            }

            for (int out_x = 0; out_x < out_width; ++out_x) {
                int in_x = out_x * skip;
                for (int c = 0; c < nchannels; ++c) {
                    out_scanline[out_x * nchannels + c] =
                        in_scanline[in_x * nchannels + c];
                }
            }

            if (!out->write_scanline(out_y, 0, TypeDesc::FLOAT, out_scanline.data())) {
                std::cerr << "Write error at scanline " << out_y << ": "
                    << out->geterror() << "\n";
                break;
            }
        }
        std::cout << "FilterDownsample::spec => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms \n"; now = std::chrono::high_resolution_clock::now();

        in->close();
        out->close();
        return 0;

    }

    int run(std::string input_filename) {
        ImageBuf inbuf(input_filename);
        if (!inbuf.init_spec(input_filename, 0, 0)) {
            std::cerr << "Error reading image spec: " << inbuf.geterror() << "\n";
            return 1;
        }

        const ImageSpec &in_spec = inbuf.spec();
        std::cout << "Input image: " << in_spec.width << "x" << in_spec.height
            << ", channels=" << in_spec.nchannels << std::endl;

        int new_width  = 4096;
        int new_height = 4096;

        ImageSpec out_spec(new_width, new_height, in_spec.nchannels, TypeDesc::FLOAT);
        ImageBuf outbuf(out_spec);

        bool ok = ImageBufAlgo::resize(outbuf, inbuf);
        if (!ok) {
            std::cerr << "Error during resize: " << geterror() << std::endl;
            return 1;
        }
        return 0;
    }

};
