#pragma once
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
namespace Veloxr {

    class RequestQueue {

        public:

            RequestQueue();

            void destroy();

            bool isEmpty();
            void pushRequest();
            bool popRequest();

            void handleRequest();
            int size() const{return m_reqQueue.size();}
            std::string getVerboseQueueData() const;
            std::string requestToString(int request) const ;

        protected:
            std::atomic_bool m_isRunning = true;

            std::mutex m_mutex;
            std::condition_variable m_condVar;
            std::queue<int> m_reqQueue{};
            std::thread m_requestThread;

            std::vector<uint32_t> m_requests;

    };



}
