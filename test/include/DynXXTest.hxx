#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace DynXX::Test {
    std::vector<std::string> list();
    int run(std::string_view test);
    int runAll();
}
