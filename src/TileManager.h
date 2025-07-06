#pragma once


#include <queue>
namespace Veloxr {

    class TileManager {

        public:
            TileManager() {
                for(int i = 0; i < 32; i++) _availableTextureSlots.push(i);
            }

            inline int getTextureSlot() { auto val = _availableTextureSlots.front(); _availableTextureSlots.pop(); return val;}
            void removeTextureSlot(int slot) {
                _availableTextureSlots.push(slot);
            }


        private:
            std::queue<int> _availableTextureSlots;
        

        
    };


}
