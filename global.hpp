#ifndef MERGEN_GLOBAL_H
#define MERGEN_GLOBAL_H

#include <atomic>
#include <cstring> // memset
#include <iostream>

/** Boolean which allows recording of further frames.
 * it should not be static. */
inline std::atomic_bool on = false;

inline void footerMsg() {
    std::cout << "Press Enter to stop...";
    std::cout.flush();
}

inline void print(const std::string &s) {
    if (!s.empty()) std::cout << "\r" << s.c_str() << std::endl;
    if (on) footerMsg();
}

template<typename ... Args>
inline void print(const std::string &s, Args ... args) {
    char buf[100];
    std::snprintf(buf, sizeof(buf), s.c_str(), args...);
    if (!s.empty()) std::cout << "\r" << buf << std::endl;
    if (on) footerMsg();
}

#endif //MERGEN_GLOBAL_H
