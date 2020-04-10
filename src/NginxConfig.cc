#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "config_parser.h"

std::string NginxConfig::ToString(int depth) {
    std::string serialized_config;
    for (const auto& statement : statements_) {
        serialized_config.append(statement->ToString(depth));
    }
    return serialized_config;
}
