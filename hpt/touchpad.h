#ifndef HPT_TOUCHPAD_H
#define HPT_TOUCHPAD_H

#include <atomic>

class Touchpad {
private:
    std::atomic_bool *on_;

public:
    int exit = 0;

    explicit Touchpad(std::atomic_bool *on);

    ~Touchpad();
};

#endif //HPT_TOUCHPAD_H
