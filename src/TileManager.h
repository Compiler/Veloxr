#pragma once


#include <queue>
namespace Veloxr {

    class TileManager {

        public:
            TileManager(int maxSlots=1024) {
                _availableTextureSlots = {};
                for(int i = 0; i < maxSlots; i++) _availableTextureSlots.push(i);
            }

            inline int getTextureSlot() { auto val = _availableTextureSlots.top(); _availableTextureSlots.pop(); return val;}
            void removeTextureSlot(int slot) {
                _availableTextureSlots.push(slot);
            }


        private:
            std::priority_queue<int, std::vector<int>, std::greater<int>> _availableTextureSlots;
        

        
    };


}
