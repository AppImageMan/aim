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
            return cmd::install(cliArgs);
        case args::Command::Remove:
            return cmd::remove(cliArgs);
        case args::Command::Upgrade:
            return cmd::upgrade(cliArgs);
        case args::Command::Run:
            return cmd::run(cliArgs);
        case args::Command::Available:
            return cmd::available(cliArgs);
        case args::Command::Backup:
            return cmd::backup(cliArgs);
        case args::Command::Restore:
            return cmd::restore(cliArgs);
    }
    return 0;
}
