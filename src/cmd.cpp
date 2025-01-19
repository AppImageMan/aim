// Implement AIM commands

#include <cstdio>
#include <cstring>
#include <iostream>
#include <variant>
#include <optional>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <pkg.hpp>
#include <args.hpp>
#include <github.hpp>
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

int cmd::install(const args::Args &cliArgs) {
    if (!cliArgs.pkg.has_value()) {
        if (!cliArgs.opt.quiet) {
            std::cerr << "Error: No package provided, but 'install' requires one." << std::endl;
        }
        return 1;
    }
    const auto pkgName = cliArgs.pkg.value();

    // Make sure ~/Applications exists
    std::stringstream path;
    path << std::string(std::getenv("HOME")) << "/Applications";
    if (!std::filesystem::exists(path.str())) {
        if (!cliArgs.opt.quiet) {
            std::cout << "No such ~/Applications!" << std::endl;
        }
        if (std::filesystem::create_directory(path.str()) && !cliArgs.opt.quiet) {
            std::cout << "Created ~/Applications" << std::endl;
        } else {
            if (!cliArgs.opt.quiet) {
                std::cerr <<  "Failed to create ~/Applications";
            }
            return 1;
        }
    }

    // Check if already installed
    const auto extension = std::string(".AppImage");
    try {
        for (const auto &entry : std::filesystem::directory_iterator(path.str())) {
            const auto fileName = entry.path().filename().string();
            if (strncmp(fileName.c_str(), pkgName.c_str(), pkgName.length()) == 0
                    && fileName.length() > extension.length()
                    && fileName.substr(fileName.length() - extension.length()) == extension) {
                // Found an installed version
                if (!cliArgs.opt.quiet) {
                    std::cout
                        << "The application '" << pkgName << "' is already installed!"
                        << std::endl;
                    return 0;
                }
            }
        }
    } catch(...) {
        // Assume not installed
    }

    // Find the package on the repo
    const auto pkgsRes = pkg::findAll();
    if (std::holds_alternative<std::string>(pkgsRes)) {
        if (!cliArgs.opt.quiet) {
            const auto errMsg = std::get<std::string>(pkgsRes);
            std::cerr << "Error: " << errMsg << std::endl;
        }
        return 1;
    }
    const auto pkgs = std::get<std::vector<pkg::Pkg>>(pkgsRes);
    const auto pkgIt = std::find_if(
        pkgs.begin(), pkgs.end(),
        [pkgName](pkg::Pkg pkg) {
            return pkg.name == pkgName;
        }
    );
    if (pkgIt == pkgs.end()) {
        std::cerr << "No such package '" << pkgName << "'" << std::endl;
        return 1;
    }

    // TODO: Ask before installing
    // TODO: Implement backup option (run after we make sure package exists)

    // Download
    if (!cliArgs.opt.quiet) {
        std::cout
            << "Downloading " << pkgIt->name << " version " << pkgIt->latestVers << "..."
            << std::endl;
    }
    std::stringstream fileName;
    fileName << "~/Applications/" << pkgIt->name << "-" << pkgIt->latestVers << ".AppImage";
    const auto err = git::dloadLfsFile(
        pkgIt->latestCommit, "application.AppImage", fileName.str(),
        cliArgs.opt.quiet
    );
    if (err.has_value()) {
        if (!cliArgs.opt.quiet) {
            std::cerr << "Error downloading file: " << err.value() << std::endl;
        }
        return 1;
    }
    std::stringstream setExec;
    setExec << "chmod +x " << fileName.str();
    return system(setExec.str().c_str());
}

int cmd::remove(const args::Args &cliArgs) {
    if (!cliArgs.pkg.has_value()) {
        if (!cliArgs.opt.quiet) {
            std::cerr << "Error: No package provided, but 'remove' requires one." << std::endl;
        }
        return 1;
    }
    const auto pkgName = cliArgs.pkg.value();

    // Make sure ~/Applications exists
    std::stringstream path;
    path << std::string(std::getenv("HOME")) << "/Applications";
    if (!std::filesystem::exists(path.str())) {
        if (!cliArgs.opt.quiet) {
            std::cout << "No such ~/Applications!" << std::endl;
        }
        if (std::filesystem::create_directory(path.str()) && !cliArgs.opt.quiet) {
            std::cout << "Created ~/Applications" << std::endl;
        } else {
            if (!cliArgs.opt.quiet) {
                std::cerr <<  "Failed to create ~/Applications";
            }
            return 1;
        }
    }

    // Check if already installed
    const auto extension = std::string(".AppImage");
    try {
        for (const auto &entry : std::filesystem::directory_iterator(path.str())) {
            const auto fileName = entry.path().filename().string();
            if (strncmp(fileName.c_str(), pkgName.c_str(), pkgName.length()) == 0
                    && fileName.length() > extension.length()
                    && fileName.substr(fileName.length() - extension.length()) == extension) {
                // Found an installed version
                if (!cliArgs.opt.quiet) {
                    std::cout
                        << "Found application '" << pkgName << "' - '"
                        << fileName << "'."
                        << std::endl;
                }

                // TODO: Ask before removal if told to
                // TODO: Implement backup

                if (!cliArgs.opt.quiet) {
                    std::cout << "Removing..." << std::endl;
                }
                if (std::remove(entry.path().c_str()) == 0) {
                    if (!cliArgs.opt.quiet) {
                        std::cout << "Successfully removed package." << std::endl;
                    }
                    return 0;
                } else {
                    if (!cliArgs.opt.quiet) {
                        std::cerr << "Failed to remove package." << std::endl;
                    }
                    return 1;
                }
            }
        }
    } catch(...) {
        // Assume not installed
        if (!cliArgs.opt.quiet) {
            std::cerr
                << "Failed to remove '" << pkgName << ".' Are you sure it's installed?"
                << std::endl;
        }
        return 1;
    }
    if (!cliArgs.opt.quiet) {
        std::cerr
            << "Couldn't find package '" << pkgName << ".' Are you sure it's installed?"
            << std::endl;
    }
    return 1;
}

int cmd::run(const args::Args &cliArgs) {
    if (!cliArgs.pkg.has_value()) {
        if (!cliArgs.opt.quiet) {
            std::cerr << "Error: No package provided, but 'run' requires one." << std::endl;
        }
        return 1;
    }
    const auto pkgName = cliArgs.pkg.value();

    // Make sure ~/Applications exists
    std::stringstream path;
    path << std::string(std::getenv("HOME")) << "/Applications";
    if (!std::filesystem::exists(path.str())) {
        if (!cliArgs.opt.quiet) {
            std::cout << "No such ~/Applications!" << std::endl;
        }
        if (std::filesystem::create_directory(path.str()) && !cliArgs.opt.quiet) {
            std::cout << "Created ~/Applications" << std::endl;
        } else {
            if (!cliArgs.opt.quiet) {
                std::cerr <<  "Failed to create ~/Applications";
            }
            return 1;
        }
    }

    // Check if already installed
    const auto extension = std::string(".AppImage");
    try {
        for (const auto &entry : std::filesystem::directory_iterator(path.str())) {
            const auto fileName = entry.path().filename().string();
            if (strncmp(fileName.c_str(), pkgName.c_str(), pkgName.length()) == 0
                    && fileName.length() > extension.length()
                    && fileName.substr(fileName.length() - extension.length()) == extension) {
                // Found an installed version
                if (!cliArgs.opt.quiet) {
                    std::cout
                        << "Found application '" << pkgName << "' - '"
                        << fileName << "'."
                        << std::endl;
                }

                // TODO: Ask before removal if told to
                // TODO: Implement backup

                if (!cliArgs.opt.quiet) {
                    std::cout << "Starting..." << std::endl;
                }
                return system(entry.path().c_str());
            }
        }
    } catch(...) {
        if (!cliArgs.opt.quiet) {
            std::cerr
                << "Failed to remove '" << pkgName << ".' Are you sure it's installed?"
                << std::endl;
        }
        return 1;
    }
    if (!cliArgs.opt.quiet) {
        std::cerr
            << "Couldn't find package '" << pkgName << ".' Are you sure it's installed?"
            << std::endl;
    }
    return 1;
}
