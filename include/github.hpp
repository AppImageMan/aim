// Fetch info from AIL (Github)

#pragma once

#include <vector>
#include <string>
#include <variant>

namespace git {
    struct Ref {
        const std::string branch;
        const std::string commit;
    };
    std::variant<std::vector<Ref>, std::string> fetchAilRefs();
    std::string metadataVal(const std::string &commit, const std::string &fileName);
}
