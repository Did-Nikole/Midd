#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <ctime>

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void init(const std::string& filepath, bool enabled) {
        m_enabled = enabled;
        m_filepath = filepath;
        if (m_enabled) {
            std::ofstream file(m_filepath, std::ios::app);
            if (file.is_open()) {
                file << "\n--- Logger Initialized ---\n";
            }
        }
    }

    void log(const std::string& message) {
        if (!m_enabled) return;
        std::ofstream file(m_filepath, std::ios::app);
        if (file.is_open()) {
            std::time_t now = std::time(nullptr);
            char buf[20];
            std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
            file << "[" << buf << "] " << message << "\n";
        }
    }

private:
    Logger() : m_enabled(false) {}
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    bool m_enabled;
    std::string m_filepath;
};
