#include <string>
#include <iostream>
#include "../sitawareness/sitawareness.hpp"

int main() {
    std::string info = getInfo();
    std::cout << info << std::endl;
    return 0;
}