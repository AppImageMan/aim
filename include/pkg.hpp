// Represent packages and pull from AIL

#pragma once

#include <string>
#include <optional>
#include <variant>
#include <vector>
#include <pkg.hpp>

namespace pkg {
    struct Pkg {
        const std::string name;
        const std::string latestVers;
        const std::string latestCommit;
        const std::optional<std::string> appVers;
    };

    std::variant<std::vector<Pkg>, std::string> findAll();
}
