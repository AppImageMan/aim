// Parse command line arguments

#pragma once

#include <optional>
#include <variant>
#include <string>

#define VERS        1

namespace args {
    struct Options {
        const bool ask;
        const bool backup;
        const bool quiet;
    };

    enum class Command {
        Install,
        Remove,
        Upgrade,
        List,
        Run,
        Available,
        Backup,
        Restore
    };

    struct Args {
        const Options opt;
        const Command cmd;
        const std::optional<std::string> pkg;
    };

    std::variant<Args, std::string> parse(int argc, char **argv);
    void printUsage(void);
};
