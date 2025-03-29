
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

#include <vector>
#include <mutex>
#include <atomic>
#include <iostream>
#include <OpenImageIO/imageio.h>

struct Test{

    struct ScanlineData {
        int out_y;
        std::vector<float> data;
    };

    static void process_chunk(ImageInput* in, 
                      int start_y, int end_y, int in_width, int out_width, 
                      int skip, int nchannels, std::mutex& io_mutex,
                      std::vector<ScanlineData>& results,
                      std::atomic<int>& progress_counter,
                      std::atomic<bool>& error_occurred) {
        
        for (int y = start_y, out_y = start_y / skip; 
             y < end_y && out_y < (end_y + skip - 1) / skip && !error_occurred; 
             y += skip, ++out_y) {
            
            std::vector<float> in_scanline(in_width * nchannels);
            std::vector<float> out_scanline(out_width * nchannels);
            
            {
                std::lock_guard<std::mutex> lock(io_mutex);
                if (!in->read_scanline(y, 0, TypeDesc::FLOAT, in_scanline.data())) {
                    std::cerr << "Read error at scanline " << y << ": " 
                             << in->geterror() << "\n";
                    error_occurred = true;
                    return;
                }
            }
            
            for (int out_x = 0; out_x < out_width; ++out_x) {
                int in_x = out_x * skip;
                for (int c = 0; c < nchannels; ++c) {
                    out_scanline[out_x * nchannels + c] = 
                        in_scanline[in_x * nchannels + c];
                }
            }
            
            ScanlineData scanline_data;
            scanline_data.out_y = out_y;
            scanline_data.data = std::move(out_scanline);
            
            {
                std::lock_guard<std::mutex> lock(io_mutex);
                results.push_back(std::move(scanline_data));
                
                // Increment and report progress
                int current = ++progress_counter;
                if (current % 100 == 0) {
                    std::cout << "Processed " << current << " scanlines\r" << std::flush;
                }
            }
        }
    }

    bool downsample_image(const std::string& in_filename, const std::string& out_filename, 
                          int skip, int num_threads = 16) {
        auto start_total = std::chrono::high_resolution_clock::now();
        auto now = std::chrono::high_resolution_clock::now();
        
        auto in = ImageInput::open(in_filename);
        if (!in) {
            std::cerr << "Could not open input file: " << in_filename << std::endl;
            return false;
        }
        std::cout << "ImageInput::Open => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms\n";
        now = std::chrono::high_resolution_clock::now();
        
        const ImageSpec& spec = in->spec();
        int in_width = spec.width;
        int in_height = spec.height;
        int nchannels = spec.nchannels;
        
        std::cout << "Input image: " << in_width << "x" << in_height 
                  << " channels=" << nchannels << std::endl;
        
        int out_width = (in_width + skip - 1) / skip;
        int out_height = (in_height + skip - 1) / skip;
        
        std::cout << "Output image: " << out_width << "x" << out_height << std::endl;
        std::cout << "ImageInput::Spec => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms\n";
        now = std::chrono::high_resolution_clock::now();
        
        ImageSpec out_spec(out_width, out_height, nchannels, TypeDesc::FLOAT);
        auto out = ImageOutput::create(out_filename);
        if (!out) {
            std::cerr << "Could not create output file: " << out_filename << std::endl;
            return false;
        }
        std::cout << "ImageOutput::create => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms\n";
        now = std::chrono::high_resolution_clock::now();
        
        if (!out->open(out_filename, out_spec)) {
            std::cerr << "Could not open output file: " << out_filename << std::endl;
            return false;
        }
        std::cout << "ImageOutput::open => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms\n";
        now = std::chrono::high_resolution_clock::now();
        
        int chunk_size = (in_height + num_threads - 1) / num_threads;
        chunk_size = (chunk_size + skip - 1) / skip * skip;
        
        std::vector<std::thread> threads;
        std::mutex io_mutex;
        std::vector<ScanlineData> results;
        results.reserve(out_height); // Pre-allocate for better performance
        std::atomic<int> progress_counter(0);
        std::atomic<bool> error_occurred(false);
        
        std::cout << "Setup time => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms\n";
        std::cout << "Expected scanlines: " << out_height << std::endl;
        now = std::chrono::high_resolution_clock::now();
        
        for (int t = 0; t < num_threads; ++t) {
            int start_y = t * chunk_size;
            int end_y = std::min(start_y + chunk_size, in_height);
            
            if (start_y >= in_height) break;
            
            threads.emplace_back(Test::process_chunk, in.get(),
                                start_y, end_y, in_width, out_width, 
                                skip, nchannels, std::ref(io_mutex), 
                                std::ref(results), std::ref(progress_counter),
                                std::ref(error_occurred));
        }
        
        std::cout << "Thread creation => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms\n";
        now = std::chrono::high_resolution_clock::now();
        
        // Monitor progress while threads are running
        std::thread progress_monitor([&]() {
            auto last_update = std::chrono::high_resolution_clock::now();
            int last_count = 0;
            
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                
                int current = progress_counter.load();
                auto current_time = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - last_update).count();
                
                if (elapsed > 0) {
                    double scanlines_per_sec = (current - last_count) / (double)elapsed;
                    double progress_percent = (current * 100.0) / out_height;
                    
                    std::cout << "Progress: " << current << "/" << out_height 
                              << " scanlines (" << progress_percent << "%), "
                              << scanlines_per_sec << " scanlines/sec" << std::endl;
                    
                    last_count = current;
                    last_update = current_time;
                    
                    // Check if all threads are done
                    bool all_done = true;
                    for (const auto& t : threads) {
                        if (t.joinable()) {
                            all_done = false;
                            break;
                        }
                    }
                    
                    if (all_done) break;
                }
            }
        });
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        if (progress_monitor.joinable()) {
            progress_monitor.join();
        }
        
        std::cout << "Parallel processing => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms\n";
        std::cout << "Processed " << progress_counter.load() << " of " << out_height << " scanlines" << std::endl;
        now = std::chrono::high_resolution_clock::now();
        
        if (error_occurred) {
            std::cerr << "Error occurred during processing." << std::endl;
            return false;
        }
        
        std::cout << "Starting sort of " << results.size() << " scanlines..." << std::endl;
        std::sort(results.begin(), results.end(), 
                 [](const ScanlineData& a, const ScanlineData& b) {
                     return a.out_y < b.out_y;
                 });
        
        std::cout << "Sorting results => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms\n";
        now = std::chrono::high_resolution_clock::now();
        
        std::cout << "Writing " << results.size() << " scanlines..." << std::endl;
        int lines_written = 0;
        for (const auto& scanline : results) {
            if (!out->write_scanline(scanline.out_y, 0, TypeDesc::FLOAT, scanline.data.data())) {
                std::cerr << "Write error at scanline " << scanline.out_y << ": "
                         << out->geterror() << "\n";
                error_occurred = true;
                break;
            }
            
            lines_written++;
            if (lines_written % 500 == 0) {
                std::cout << "Wrote " << lines_written << " scanlines\r" << std::flush;
            }
        }
        
        std::cout << "\nSequential writing => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms\n";
        now = std::chrono::high_resolution_clock::now();
        
        out->close();
        in->close();
        
        std::cout << "Closing files => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() << " ms\n";
        std::cout << "Total time => " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_total).count() << " ms\n";
        
        return !error_occurred;
    }

    int run3(std::string input_filename, std::string output_filename) {
        bool success = downsample_image(input_filename, output_filename, 25);
        return success ? 0 : 1;
    }

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
