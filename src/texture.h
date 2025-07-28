#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "VLogger.h"
#include "Common.h"

namespace Veloxr {

    class OIIOTexture {
        public:
            OIIOTexture() = default;
            OIIOTexture(std::string filename);
            void init(std::string filename);

            inline const Veloxr::Point& getResolution() const { return _resolution; }
            inline const std::string& getFilename() const { return _filename; }
            inline const uint64_t& getNumChannels() const { return _numChannels; }
            inline const uint64_t& getOrientation() const { return _orientation; }
            std::vector<unsigned char> load(std::string filename="");
            inline const bool isInitialized() const { return _loaded; }

        private:

            Veloxr::LLogger console{"[Veloxr][OIIOTexture]"};
            Veloxr::Point _resolution;
            std::string _filename;
            uint64_t _numChannels;
            uint64_t _orientation;
            bool _loaded{false};

    };

}
