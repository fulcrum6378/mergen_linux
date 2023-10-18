#include "microphone.h"

Microphone::Microphone(std::atomic_bool *on) : on_(on) {}

Microphone::~Microphone() = default;
