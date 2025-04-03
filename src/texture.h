#pragma once
#include <cstdint>
#include <string>

namespace Veloxr {

    struct Point {
        uint32_t x, y;
    };

class TextureInfo {
    public:
        TextureInfo() = default;
        TextureInfo(std::string filename);
        void init(std::string filename);

        inline const Point& getResolution() const { return _resolution; }
        inline const std::string& getFilename() const { return _filename; }
        inline const int& getNumChannels() const { return _numChannels; }

    private:
        Point _resolution;
        std::string _filename;
        int _numChannels;

};

}
