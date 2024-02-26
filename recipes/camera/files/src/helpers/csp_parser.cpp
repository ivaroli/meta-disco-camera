#include "csp_parser.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "types.hpp"

void parseMessage(const std::string& input, CaptureMessage& message) {
    std::vector<std::string> pairs;
    std::stringstream ss(input);
    std::string pair;
    while (std::getline(ss, pair, ';')) {
        pairs.push_back(pair);
    }

    for (const auto& p : pairs) {
        std::istringstream iss(p);
        std::string variable, value;
        std::getline(iss, variable, '=');
        std::getline(iss, value);
        
        variable.erase(0, variable.find_first_not_of(" \t\n\r\f\v"));
        variable.erase(variable.find_last_not_of(" \t\n\r\f\v") + 1);
        value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));
        value.erase(value.find_last_not_of(" \t\n\r\f\v") + 1);
        
        // Check variable name and assign value -> kinda dirty...
        if (variable == "NUM_IMAGES") {
            message.NumberOfImages = std::stoi(value);
        } else if (variable == "ISO") {
            message.ISO = std::stof(value);
        } else if (variable == "EXPOSURE") {
            message.Exposure = std::stoi(value);
        } else if (variable == "CAMERA") {
            message.Camera = value;
        } else {
            continue;
        }
    }
}