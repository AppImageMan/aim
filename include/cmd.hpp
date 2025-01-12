// Run the AIM commands

#pragma once

#include <pkg.hpp>
#include <args.hpp>

namespace cmd {
    int available(const args::Args &cliArgs);
    int install(const args::Args &cliArgs);
}
