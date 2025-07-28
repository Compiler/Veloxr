#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <chrono>
#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <iomanip>

namespace Veloxr {
    static std::vector<std::chrono::high_resolution_clock::time_point> timers;
    static int timerCounter = 0;
    // static std::queue<std::pair<long long, std::string>> __TL_threadLogger;
#define BEG_TIME \
    if (timerCounter >= timers.size()) timers.resize(timerCounter + 1); \
    timers[timerCounter] = std::chrono::high_resolution_clock::now();

#define END_TIME(x, y) \
    if((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timers[timerCounter])).count() < 100) x.debug("->->->^1 Finished ", #y," in \t", (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timers[timerCounter])).count(), " ms."); \
    else if((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timers[timerCounter])).count() < 300) x.critical("->->->^2 Finished ", #y," in \t", (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timers[timerCounter])).count(), " ms."); \
    else if((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timers[timerCounter])).count() < 500) x.warn("->->->^3 Finished ", #y," in \t", (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timers[timerCounter])).count(), " ms."); \
    else x.fatal("->->->^4 Finished ", #y," in \t", (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timers[timerCounter])).count(), " ms."); \
    ++timerCounter;

#define CLEAR timerCounter = 0;timers.clear();
#define NOW std::chrono::high_resolution_clock::now()
#define END(x) std::chrono::duration_cast<std::chrono::milliseconds>(NOW - x).count()

    class LLogger {
        public:
            enum LogLevel{
                INFO, DEBUG, WARNING, CRITICAL, FATAL, COLOR1, COLOR2, COLOR3
            };

            enum EndingStyle{
                NORMAL, BOLD, ITALICS
            };

            bool active = true;
            std::string uniqueIdentifier = "===---";
            LLogger() = default;
            LLogger(const std::string& uniqueIdentifier): uniqueIdentifier(uniqueIdentifier){}
            LLogger(const std::string& uniqueIdentifier, bool active): active(active), uniqueIdentifier(uniqueIdentifier){}


            static std::string getTimeString(){
                auto now = std::chrono::system_clock::now();
                auto itt = std::chrono::system_clock::to_time_t(now);
                auto fractional_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

                std::tm bt = *std::localtime(&itt);

                std::ostringstream oss;
                oss << std::put_time(&bt, "%Y-%m-%d-%H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << fractional_seconds.count();

                std::string time_string = oss.str();
                return time_string;

            }

            void toggleModifier(std::string modifier){
                auto pos = uniqueIdentifier.find(modifier);
                if(pos == std::string::npos){
                    uniqueIdentifier += modifier;
                }else{
                    uniqueIdentifier.replace(pos, modifier.size(), "");
                }
            }

            static void log_write(const std::string &line) {
                printf("%s", line.c_str());
                fflush(stdout);
            }

            //TODO: Convert printf to qInfos/qDebugs... to sync with frontend timing.
            void log_custom(const std::string& msg, int logLevel = INFO){
                if(!active) return;

                static std::unordered_map<int, const char*> endingCharStyle =
                {{NORMAL, "\033[0m"}, {BOLD, "\033[1m"}, {ITALICS, "\033[2m"}};

                const char* endingStyle = /*logLevel != LogLevel::CRITICAL ? endingCharStyle[EndingStyle::NORMAL] :*/ endingCharStyle[EndingStyle::BOLD];
                static std::unordered_map<int, const char*> logLevelColor =
                {{INFO, "\033[36m"}, {DEBUG, "\033[32m"}, {WARNING, "\033[33m"}, {CRITICAL, "\033[34m"}, {FATAL, "\033[31m"}, {COLOR1, "\033[35m"}, {COLOR2, "\033[37m"}, {COLOR3, "\033[38m"}};
                const char* logLevelStr = logLevel == INFO ? " INFO" : logLevel == DEBUG ? "DEBUG" : logLevel == WARNING ? " WARN" : logLevel == CRITICAL ? " CRIT" : logLevel == FATAL ? "FATAL" : "  LOG";

                std::stringstream ss;
                ss << endingCharStyle[EndingStyle::BOLD] << std::endl
                   << getTimeString() << ": "
                   << endingCharStyle[EndingStyle::BOLD]
                   << logLevelColor[logLevel]
                   << logLevelStr << ": "
                   << endingCharStyle[EndingStyle::BOLD]
                   << logLevelColor[logLevel]
                   << uniqueIdentifier << " "
                   << msg
                   << endingStyle << std::endl
                   << endingCharStyle[EndingStyle::NORMAL];
                
                LLogger::logCallback(ss.str());
            }

            static void setLogCallback(std::function<void(const std::string&)> callback) {
                if (callback) {
                    LLogger::logCallback = callback;
                } else {
                    LLogger::logCallback = &LLogger::log_write;
                }
            }

            void setIdentifier(const std::string& uniqueIdentifier){
                this->uniqueIdentifier = uniqueIdentifier;
            }


            template<typename T>
                void appendToStringStream(std::stringstream& ss, const T& arg) {
                    ss << arg;
                }

#ifndef Q_OS_LINUX
            template <> //[LR] Due to GCC11 bug on Linux, this needs to be removed.
#endif
                void appendToStringStream(std::stringstream& ss, const char* const& arg) {
                    ss << arg;
                }

            template <typename T> //type T must be specialized in this file
                void appendToStringStream(std::stringstream& ss, const std::vector<T>& arg) {
                    ss << "[";
                    if(arg.size() >= 2){
                        for(int i = 0; i < arg.size() - 2; i++) ss << arg[i] << ", ";
                    }
                    ss << arg.back() << "]";
                }

#ifndef Q_OS_LINUX
            template <> //[LR] Due to GCC11 bug on Linux, this needs to be removed.
#endif
                void appendToStringStream(std::stringstream& ss, const bool& arg) {
                    ss << (arg ? "true" : "false");
                }

#ifndef Q_OS_LINUX
            template <> //[LR] Due to GCC11 bug on Linux, this needs to be removed.
#endif
                void appendToStringStream(std::stringstream& ss, const std::queue<std::pair<long long, std::string>>& queue) {
                    return;
                    std::queue<std::pair<long long, std::string>> q = queue;
                    ss << "[";
                    while(!q.empty()){
                        auto top = q.front(); q.pop();
                        ss << "\n Time: " << top.first << ": " << top.second << ", \n";
                    }
                    ss << "]";

                }


            template<size_t N>
                void appendToStringStream(std::stringstream& ss, const char (&arg)[N]) {
                    ss << arg;
                }

            template <typename ... Args>
                void log(Args&&... args){
                    std::stringstream ss;
                    (appendToStringStream(ss, std::forward<Args>(args)), ...);
                    log_custom(ss.str(), INFO);
                }

            template <typename ... Args>
                void logc1(Args&&... args){
                    std::stringstream ss;
                    (appendToStringStream(ss, std::forward<Args>(args)), ...);
                    log_custom(ss.str(), COLOR1);
                }

            template <typename ... Args>
                void logc2(Args&&... args){
                    std::stringstream ss;
                    (appendToStringStream(ss, std::forward<Args>(args)), ...);
                    log_custom(ss.str(), COLOR2);
                }

            template <typename ... Args>
                void logc3(Args&&... args){
                    std::stringstream ss;
                    (appendToStringStream(ss, std::forward<Args>(args)), ...);
                    log_custom(ss.str(), COLOR3);
                }

            template <typename ... Args>
                void debug(Args&&... args){
                    std::stringstream ss;
                    (appendToStringStream(ss, std::forward<Args>(args)), ...);
                    log_custom(ss.str(), DEBUG);
                }

            template <typename ... Args>
                void warn(Args&&... args){
                    std::stringstream ss;
                    (appendToStringStream(ss, std::forward<Args>(args)), ...);
                    log_custom(ss.str(), WARNING);
                }

            template <typename ... Args>
                void critical(Args&&... args){
                    std::stringstream ss;
                    (appendToStringStream(ss, std::forward<Args>(args)), ...);
                    log_custom(ss.str(), CRITICAL);
                }

            template <typename ... Args>
                void fatal(Args&&... args){
                    std::stringstream ss;
                    (appendToStringStream(ss, std::forward<Args>(args)), ...);
                    log_custom(ss.str(), FATAL);
                    //assert(false); //[LR] Add failing to this in future
                }

        private:
            inline static std::function<void(const std::string&)> logCallback = &LLogger::log_write;
    };
}
#endif // LOGGER_H

