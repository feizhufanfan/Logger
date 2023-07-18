#ifndef LOGGER_H
#define LOGGER_H


#include <iostream>
#include <sstream>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <stdarg.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <type_traits>
#if _WIN32
#include <direct.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <atomic>
#include <cstring>
#endif

#ifdef _WIN32
#define FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

//是否启用单例模式
#define ISSINGNEL 1

namespace feifei {
    class Logger {
    public:
#if ISSINGNEL

        static Logger &getinstance(std::string Path) {
            static Logger *instance = nullptr;
            if (!instance) {  // 第一次检查
                std::lock_guard<std::mutex> lock(mutex_);
                if (!instance) {  // 第二次检查
                    instance = new Logger(Path);
                }
            }
            return *instance;
        }

        inline void setPath(std::string Path = "./") {
            this->m_filePath = Path;
        }

    private:
        static std::mutex mutex_;//单例模式
        Logger() = delete;

    public:
        Logger(const Logger &) = delete;

        Logger &operator=(const Logger &) = delete;

#endif

        template<typename T>
        Logger& operator<<(const T& t) {
            std::unique_lock<std::mutex> lock(m_mutex);
            std::ostringstream oss;
            oss << t;
            m_queue.emplace(oss.str());
            m_condition.notify_all();
            return *this;
        }







        template<typename... Args>
        void PrintLog(const char *format, Args... args) {
            std::unique_lock<std::mutex> lock(m_mutex);
            std::ostringstream stream;
            std::shared_ptr<char> buffer(new char[1024]);
            memcpy(buffer.get(), "", 1024);
#if _WIN32
            sprintf_s(buffer.get(), 1024, format, args...);
#else
            snprintf(buffer.get(),1024,format,args...);
#endif
            stream << buffer;

            m_queue.emplace( stream.str());
            m_condition.notify_all();


        }

#if ISSINGNEL
    private:
#else

        public:
#endif

        Logger(std::string filePath) : m_stop(false), m_filePath(filePath) {
            m_thread = std::thread(&Logger::processEntries, this);
            create_directory(filePath);
        }

        ~Logger() {
            m_stop.store(true);
            m_condition.notify_all();
            m_thread.join();
        }

    private:




        bool create_directory(const std::string &path) {
#if _WIN32
            int status = _mkdir(path.c_str());
#else
            int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

            if (status == 0) {
                return true;
            } else {
                switch (errno) {
                    case ENOENT:
                        // parent didn't exist, try to create it
#if _WIN32
                        if (create_directory(path.substr(0, path.find_last_of('\\')))) {
#else
                            if (create_directory(path.substr(0, path.find_last_of('/')))) {
#endif
                            // try again to create the directory
#if _WIN32
                            return 0 == _mkdir(path.c_str());
#else
                            return 0 == mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

                        } else {
                            return false;
                        }
                        break;
                    case EEXIST:
                        // done!
                        return true;
                    default:
                        return false;
                }
            }
        }

        void processEntries() {
            std::unordered_map<std::string, std::chrono::system_clock::time_point> lastWriteTime;
            auto printInterval = std::chrono::milliseconds(500);

            while (true) {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_condition.wait(lock, [this]() { return !m_queue.empty() || m_stop; });
                if (m_stop.load()) {
                    break;
                }
                auto now = std::chrono::system_clock::now();
                auto time = std::chrono::system_clock::to_time_t(now);
                std::stringstream ss;
                ss << m_filePath << std::put_time(std::localtime(&time), "%Y-%m-%d") << ".log";
                std::string filename = ss.str();
                if (filename != m_filename) {
                    m_filename = filename;
                    m_file.close();
                    m_file.open(m_filename, std::ios::app);
                }
                while (!m_queue.empty()) {
                    auto logEntry = m_queue.front();
                    m_queue.pop();
                    if (lastWriteTime.find(logEntry) == lastWriteTime.end() ||
                        now - lastWriteTime[logEntry] >= printInterval) {
                        m_file << std::put_time(std::localtime(&time), "%Y-%m-%d %T") << " " << logEntry << std::endl;
                        std::cout << std::put_time(std::localtime(&time), "%Y-%m-%d %T") << " " << logEntry << std::endl;
                        lastWriteTime[logEntry] = now;
                    }
                }
            }
        }


    private:
        std::string m_filePath;
        std::string m_filename;
        std::ofstream m_file;
        std::thread m_thread;
        std::mutex m_mutex;
        std::condition_variable m_condition;
        std::queue<std::string> m_queue;
        std::atomic<bool> m_stop;


    };


};
#endif //LOGGER_H