#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <chrono>
#include <ctime>

class Logger {
public:
    static void log(const std::string& action) {
        std::ofstream f("encrypt.log", std::ios::app);
        if (f) {
            auto now = std::chrono::system_clock::now();
            std::time_t time = std::chrono::system_clock::to_time_t(now);
            f << std::ctime(&time) << " - " << action << "\n";
            f.close();
        }
    }
};

#endif
