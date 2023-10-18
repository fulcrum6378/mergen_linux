#ifndef AUD_MICROPHONE_H
#define AUD_MICROPHONE_H

#include <atomic>

class Microphone {
private:
    std::atomic_bool *on_;

public:
    int exit = 0;

    explicit Microphone(std::atomic_bool *on);

    ~Microphone();
};

#endif //AUD_MICROPHONE_H
