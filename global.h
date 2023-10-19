#ifndef GLOBAL_H
#define GLOBAL_H

#include <iostream>

inline void yourCommand() {
    std::cout << "Your command: ";
    std::cout.flush();
}

inline void print(const std::string &s) {
    if (!s.empty()) std::cout << "\r" << s.c_str() << std::endl;
    yourCommand();
}

template<typename ... Args>
inline void print(const std::string &s, Args ... args) {
    char buf[s.length() + 20];
    std::snprintf(buf, sizeof(buf), s.c_str(), args...);
    if (!s.empty()) std::cout << "\r" << buf << std::endl;
    yourCommand();
}

#endif //GLOBAL_H
