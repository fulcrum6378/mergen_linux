#include "microphone.hpp"

Microphone::Microphone(std::atomic_bool *on) : on_(on) {}

Microphone::~Microphone() = default;
