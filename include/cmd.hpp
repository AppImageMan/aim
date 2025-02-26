// Run the AIM commands

#pragma once

#include <pkg.hpp>
#include <args.hpp>

namespace cmd {
    int available(const args::Args &cliArgs);
    int install(const args::Args &cliArgs);
    int remove(const args::Args &cliArgs);
    int upgrade(const args::Args &cliArgs);
    int run(const args::Args &cliArgs);
    int backup(const args::Args &cliArgs);
    int restore(const args::Args &cliArgs);
}
