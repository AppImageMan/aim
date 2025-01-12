// Entry point for AIM

#include <iostream>
#include <string>
#include <variant>
#include <args.hpp>
#include <pkg.hpp>
#include <cmd.hpp>

int main(int argc, char **argv) {
    const auto cliArgRes = args::parse(argc, argv);
    if (std::holds_alternative<std::string>(cliArgRes)) {
        const auto errMsg = std::get<std::string>(cliArgRes);
        if (errMsg == "") {
            args::printUsage();
            return 0;
        } else {
            std::cerr << "Error: " << errMsg << std::endl;
            args::printUsage();
            return 1;
        }
    }
    const auto cliArgs = std::get<args::Args>(cliArgRes);
    switch (cliArgs.cmd) {
        case args::Command::Install:
            std::cerr << "Not implemented!" << std::endl;
            return 1;
            break;
        case args::Command::Remove:
            std::cerr << "Not implemented!" << std::endl;
            return 1;
            break;
        case args::Command::Upgrade:
            std::cerr << "Not implemented!" << std::endl;
            return 1;
            break;
        case args::Command::List:
            std::cerr << "Not implemented!" << std::endl;
            return 1;
            break;
        case args::Command::Run:
            std::cerr << "Not implemented!" << std::endl;
            return 1;
            break;
        case args::Command::Available:
            return cmd::available(cliArgs);
        case args::Command::Backup:
            std::cerr << "Not implemented!" << std::endl;
            return 1;
            break;
        case args::Command::Restore:
            std::cerr << "Not implemented!" << std::endl;
            return 1;
            break;
    }
    return 0;
}
