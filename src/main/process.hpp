#pragma once
#include <string>
#include <iostream>
#include <cstdlib>

namespace anvil {
    inline bool exec(const std::string& cmd) {
        std::cout << "  >> " << cmd << std::endl;
        const int result = std::system(cmd.c_str());
        return result == 0;
    }
}