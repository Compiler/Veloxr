#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "VLogger.h"

namespace Veloxr {

    struct Point {
        uint64_t x, y;
    };

    class OIIOTexture {
        public:
            OIIOTexture() = default;
            OIIOTexture(std::string filename);
            void init(std::string filename);

            inline const Point& getResolution() const { return _resolution; }
            inline const std::string& getFilename() const { return _filename; }
            inline const int& getNumChannels() const { return _numChannels; }
            inline const int& getOrientation() const { return _orientation; }
            std::vector<unsigned char> load(std::string filename="");
            inline const bool isInitialized() const { return _loaded; }

        private:

            Veloxr::LLogger console{"[Veloxr][OIIOTexture]"};
            Point _resolution;
            std::string _filename;
            uint64_t _numChannels;
            int _orientation;
            bool _loaded{false};

    };

}
