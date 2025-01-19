// Implement argument parsing

#include <optional>
#include <variant>
#include <string>
#include <sstream>
#include <iostream>
#include <args.hpp>

using namespace args;

std::variant<Args, std::string> args::parse(int argc, char **argv) {
    std::stringstream errMsg;
    bool ask = false;
    bool backup = false;
    bool quiet = false;
    int i = 1;
    while (i < argc && argv[i][0] == '-') {
        const auto arg = std::string(argv[i]);
        if (arg == "-h" || arg == "--help") {
            return "";
        } else if (arg == "-a" || arg == "--ask") {
            ask = true;
        } else if (arg == "-b" || arg == "--backup") {
            backup = true;
        } else if (arg == "-q" || arg == "--quier") {
            quiet = true;
        } else {
            errMsg << "Unknown option '" << arg << "'";
            return errMsg.str();
        }
        i++;
    }
    if (i >= argc) {
        return "No command provided";
    }
    const auto cmdStr = std::string(argv[i]);
    i++;
    Command cmd = Command::Install;
    if (cmdStr == "install") {
        cmd = Command::Install;
    } else if (cmdStr == "remove") {
        cmd = Command::Remove;
    } else if (cmdStr == "upgrade") {
        cmd = Command::Upgrade;
    } else if (cmdStr == "run") {
        cmd = Command::Run;
    } else if (cmdStr == "available") {
        cmd = Command::Available;
    } else if (cmdStr == "backup") {
        cmd = Command::Backup;
    } else if (cmdStr == "restore") {
        cmd = Command::Restore;
    } else {
        errMsg << "Unknown command '" << cmdStr;
        return errMsg.str();
    }
    std::optional<std::string> pkg = std::nullopt;
    if (i < argc) {
        pkg.emplace(argv[i]);
        i++;
    }
    if (i < argc) {
        errMsg << "Extraneous text in command: '";
        while (i < argc) {
            errMsg << argv[i] << " ";
        }
        errMsg << "'";
        return errMsg.str();
    }
    return Args {
        Options { ask, backup, quiet },
        cmd, pkg
    };
}

void args::printUsage() {
    std::cout
        << "APPIMAGE MANAGER - v" << VERS << std::endl
        << "Created by Dylan Turner" << std::endl
        << std::endl
        << "USAGE: aim [OPTIONS...] COMMAND [PACKAGE]" << std::endl
        << "OPTIONS:" << std::endl
        << "    -a/--ask            Ask before executing each command" << std::endl
        << "    -b/--backup         Create a backup before execution" << std::endl
        << "    -q/--quiet          Do not output to stdout/stderr" << std::endl
        << "COMMAND:" << std::endl
        << "    install             Installs a package" << std::endl
        << "    remove              Removes a package" << std::endl
        << "    upgrade             Upgrade your packages" << std::endl
        << "    run                 Run installed package" << std::endl
        << "    available           List all available packages" << std::endl
        << "    backup              Create a backup of your packages" << std::endl
        << "    restore             Restore the backup of your packages" << std::endl;
}
