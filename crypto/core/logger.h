#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <chrono>
#include <ctime>

using namespace std;

class Logger {
public:
    static void log(const string& action) {
        ofstream f("encrypt.log", ios::app);
        if (f) {
            auto now = chrono::system_clock::now();
            time_t time = chrono::system_clock::to_time_t(now);
            f << ctime(&time) << " - " << action << "\n";
            f.close();
        }
    }
};

#endif
