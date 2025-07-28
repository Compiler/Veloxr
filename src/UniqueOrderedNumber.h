#pragma once

#include <queue>

namespace Veloxr {

    class OrderedNumberFactory {
        public:
            OrderedNumberFactory(int maxSlots=1024, int startNum=0) {
                _availableSlots = {};
                for(int i = startNum; i < maxSlots + startNum; i++) _availableSlots.push(i);
            }

            inline int getSlot() { auto val = _availableSlots.top(); _availableSlots.pop(); return val;}
            void removeSlot(int slot) {
                _availableSlots.push(slot);
            }

        private:
            std::priority_queue<int, std::vector<int>, std::greater<int>> _availableSlots;
    };
}

