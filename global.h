#ifndef GLOBAL_H
#define GLOBAL_H

#include <iostream>

void print(std::string s) {
    if (!s.empty()) std::cout << "\r" << s.c_str() << std::endl;
    std::cout << "Your command: ";
    std::cout.flush();
}

#endif //GLOBAL_H
