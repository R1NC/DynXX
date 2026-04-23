#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace DynXX::Test {
    std::vector<std::string> listSuits();
    std::vector<std::string> listTests(std::string_view suit);
    int runSuit(std::string_view suit);
    int runTest(std::string_view suit, std::string_view test);
    int runAll();
}
