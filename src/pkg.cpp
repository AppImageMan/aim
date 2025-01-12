// Implement package pulling

#include <cstring>
#include <string>
#include <optional>
#include <sstream>
#include <vector>
#include <variant>
#include <github.hpp>
#include <pkg.hpp>

using namespace pkg;

std::variant<std::vector<Pkg>, std::string> pkg::findAll() {
    const auto branchesRes = git::fetchAilRefs();
    if (std::holds_alternative<std::string>(branchesRes)) {
        return std::get<std::string>(branchesRes);
    }
    const auto refs = std::get<std::vector<git::Ref>>(branchesRes);
    std::vector<Pkg> pkgs;
    pkgs.reserve(refs.size());
    for (const auto &ref : refs) {
        if (strncmp(ref.branch.c_str(), "apps/", 5) != 0) {
            continue;
        }
        const auto name = ref.branch.substr(5);
        const auto commit = ref.commit;
        const auto versRes = git::metadataVal(commit, "date.txt");
        if (versRes[0] != '<') {
            std::stringstream errMsg;
            errMsg << "App issue for '" << name << "' - " << versRes;
            return errMsg.str();
        }
        const auto vers = versRes.substr(1, versRes.length() - 2);
        const auto appVersRes = git::metadataVal(commit, "version.txt");
        std::optional<std::string> appVers = std::nullopt;
        if (appVersRes[0] == '<') {
            appVers.emplace(appVersRes.substr(1, appVersRes.length() - 2));
        }
        pkgs.push_back(Pkg { name, vers, commit, appVers });
    }
    return pkgs;
}
