#ifndef GLOBAL_H
#define GLOBAL_H

#include <iostream>

template<typename ... Args>
void print(const std::string &s, Args ... args) {
    char buf[s.length() + 20];
    std::snprintf(buf, sizeof(buf), s.c_str(), args...);
    if (!s.empty()) std::cout << "\r" << buf << std::endl;
    std::cout << "Your command: ";
    std::cout.flush();
}

#endif //GLOBAL_H
