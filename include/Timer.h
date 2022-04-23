//
// Created by dawid on 01.11.2021.
//

#pragma once

#include <chrono>

template <typename T>
class Timer {

    long begin = 0;
    long end = 0;
    bool running = false;

public:
    Timer() = default;

    ~Timer() = default;

    [[nodiscard]] bool isRunning() const {
        return running;
    }

    void start() {
        if (!isRunning()) {
            begin = std::chrono::duration_cast<T>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
            running = true;
        }
    }

    void stop() {
        if (isRunning()) {
            end = std::chrono::duration_cast<T>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
            running = false;
        }
    }

    [[nodiscard]] long duration() const {
        if (isRunning())
            return std::chrono::duration_cast<T>(
                    std::chrono::system_clock::now().time_since_epoch()).count() - begin;
        else
            return end - begin;
    }
};


