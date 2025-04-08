#include "RequestQueue.h"

using namespace Veloxr;


RequestQueue::RequestQueue() {
        m_requestThread = std::thread(&RequestQueue::handleRequest, this);
}
