// Implement AIM commands

#include <iostream>
#include <variant>
#include <optional>
#include <vector>
#include <string>
#include <pkg.hpp>
#include <args.hpp>
#include <cmd.hpp>

using namespace cmd;

int cmd::available(const args::Args &cliArgs) {
    // TODO: Implement backup option
    if (cliArgs.opt.quiet) {
        // This command only prints stuff, so after backup, if they said quiet, don't print lol
        return 0;
    }
    const auto pkgsRes = pkg::findAll();
    if (std::holds_alternative<std::string>(pkgsRes)) {
        const auto errMsg = std::get<std::string>(pkgsRes);
        std::cerr << "Error: " << errMsg << std::endl;
        return 1;
    }
    const auto pkgs = std::get<std::vector<pkg::Pkg>>(pkgsRes);
    for (const auto &pkg : pkgs) {
        std::cout << pkg.name;
        if (pkg.appVers.has_value()) {
            std::cout << " (" << pkg.appVers.value() << ")";
        }
        std::cout << " " << pkg.latestVers << std::endl;
    }
    return 0;
}
