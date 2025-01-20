// Implement AIM commands

#include <cstdio>
#include <cstring>
#include <iostream>
#include <variant>
#include <optional>
#include <vector>
#include <string>
#include <regex>
#include <algorithm>
#include <filesystem>
#include <pkg.hpp>
#include <args.hpp>
#include <github.hpp>
#include <cmd.hpp>

using namespace cmd;

static void ensureAppFldr(const args::Args &cliArgs);
static void rmBackup(const args::Args &cliArgs);
static void makeBackup(const args::Args &cliArgs);
static bool asked(const args::Args &cliArgs);

int cmd::available(const args::Args &cliArgs) {
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
    try {
        ensureAppFldr(cliArgs);
    } catch (...) {
        return 1;
    }

    // Check if already installed
    const auto extension = std::string(".AppImage");
    std::stringstream path;
    path << std::string(std::getenv("HOME")) << "/Applications";
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

    if (!cliArgs.opt.quiet) {
        std::cout << "Installing " << pkgName << "..." << std::endl;
    }
    if (!asked(cliArgs)) {
        return 0;
    }

    if (cliArgs.opt.backup) {
        try {
            rmBackup(cliArgs);
            makeBackup(cliArgs);
        } catch (...) {
            return 1;
        }
    }

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
    try {
        ensureAppFldr(cliArgs);
    } catch (...) {
        return 1;
    }

    // Check if already installed
    const auto extension = std::string(".AppImage");
    std::stringstream path;
    path << std::string(std::getenv("HOME")) << "/Applications";
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

                if (!cliArgs.opt.quiet) {
                    std::cout << "Removing " << pkgName << "..." << std::endl;
                }
                if (!asked(cliArgs)) {
                    return 0;
                }

                if (cliArgs.opt.backup) {
                    try {
                        rmBackup(cliArgs);
                        makeBackup(cliArgs);
                    } catch (...) {
                        return 1;
                    }
                }

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

int cmd::upgrade(const args::Args &cliArgs) {
    // Make sure ~/Applications exists
    try {
        ensureAppFldr(cliArgs);
    } catch (...) {
        return 1;
    }

    // Get the available apps
    const auto pkgsRes = pkg::findAll();
    if (std::holds_alternative<std::string>(pkgsRes)) {
        const auto errMsg = std::get<std::string>(pkgsRes);
        std::cerr << "Error: " << errMsg << std::endl;
        return 1;
    }
    const auto pkgs = std::get<std::vector<pkg::Pkg>>(pkgsRes);
    std::vector<std::string> pkgNames;
    pkgNames.reserve(pkgs.size());
    for (const auto &pkg : pkgs) {
        pkgNames.push_back(pkg.name);
    }

    if (cliArgs.opt.backup) {
        try {
            rmBackup(cliArgs);
            makeBackup(cliArgs);
        } catch (...) {
            return 1;
        }
    }

    // Check if already installed
    const auto extension = std::string(".AppImage");
    std::stringstream path;
    path << std::string(std::getenv("HOME")) << "/Applications";
    std::regex pkgNamePattern(R"(^([a-zA-Z0-9_-]+)-(\d+\.\d+\.\d+)\.AppImage$)");
    try {
        for (const auto &entry : std::filesystem::directory_iterator(path.str())) {
            const auto fileName = entry.path().filename().string();
            std::smatch pkgNameMatch;
            if (!std::regex_match(fileName, pkgNameMatch, pkgNamePattern)) {
                continue;
            }
            if (pkgNameMatch.size() != 3) {
                continue;
            }
            const auto pkgName = pkgNameMatch[1];
            const auto versStr = pkgNameMatch[2];
            auto pkgNameIt = std::find(pkgNames.begin(), pkgNames.end(), pkgName);
            if (fileName.length() > extension.length()
                    && fileName.substr(fileName.length() - extension.length()) == extension
                    && pkgNameIt != pkgNames.end()) {
                const auto pkgIt = (pkgNameIt - pkgNames.begin()) + pkgs.begin();
                if (pkgIt->latestVers == versStr) {
                    if (!cliArgs.opt.quiet) {
                        std::cout
                            << "Skipping '" << pkgName << "' as it is already up to date."
                            << std::endl;
                    }
                    continue;
                }

                if (!cliArgs.opt.quiet) {
                    std::cout << "Upgrading " << pkgName << "..." << std::endl;
                }
                if (!asked(cliArgs)) {
                    continue;
                }

                if (!cliArgs.opt.quiet) {
                    std::cout
                        << "Upgrading '" << pkgName
                        << "' from version " << versStr << " to version " << pkgIt->latestVers
                        << std::endl;
                }

                if (std::remove(entry.path().c_str()) == 0) {
                    if (!cliArgs.opt.quiet) {
                        std::cout << "Successfully removed package." << std::endl;
                    }
                } else {
                    if (!cliArgs.opt.quiet) {
                        std::cerr << "Failed to remove package." << std::endl;
                    }
                    return 1;
                }

                // Download
                if (!cliArgs.opt.quiet) {
                    std::cout
                        << "Downloading " << pkgIt->name << " version "
                        << pkgIt->latestVers << "..."
                        << std::endl;
                }
                std::stringstream fileName;
                fileName
                    << "~/Applications/" << pkgIt->name << "-" << pkgIt->latestVers << ".AppImage";
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
                int success = system(setExec.str().c_str());
                if (success != 0) {
                    if (!cliArgs.opt.quiet) {
                        std::cerr << "Failed to set app to excutable!" << std::endl;
                    }
                    return 1;
                }
            }
        }
    } catch(...) {
        // Assume not installed
    }
    return 0;
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
    try {
        ensureAppFldr(cliArgs);
    } catch (...) {
        return 1;
    }

    // Check if already installed
    const auto extension = std::string(".AppImage");
    std::stringstream path;
    path << std::string(std::getenv("HOME")) << "/Applications";
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

int cmd::backup(const args::Args &cliArgs) {
    // Make sure ~/Applications exists
    try {
        ensureAppFldr(cliArgs);
    } catch (...) {
        return 1;
    }

    if (!cliArgs.opt.quiet) {
        std::cout << "Backing up..." << std::endl;
    }
    if (!asked(cliArgs)) {
        return 0;
    }

    try {
        rmBackup(cliArgs);
        makeBackup(cliArgs);
    } catch (...) {
        return 1;
    }
    return 0;
}

int cmd::restore(const args::Args &cliArgs) {
    // Make sure ~/Applications exists
    try {
        ensureAppFldr(cliArgs);
    } catch (...) {
        return 1;
    }

    if (!cliArgs.opt.quiet) {
        std::cout << "Removing old packages..." << std::endl;
    }
    const auto extension = std::string(".AppImage");
    std::stringstream path;
    path << std::string(std::getenv("HOME")) << "/Applications";
    try {
        for (const auto &entry : std::filesystem::directory_iterator(path.str())) {
            const auto fileName = entry.path().filename().string();
            if (fileName.length() > extension.length()
                    && fileName.substr(fileName.length() - extension.length()) == extension) {
                // Found an installed version
                if (!cliArgs.opt.quiet) {
                    std::cout
                        << "Found application '" << fileName << "' - '"
                        << fileName << "'."
                        << std::endl;
                }

                if (!cliArgs.opt.quiet) {
                    std::cout << "Removing " << fileName << "..." << std::endl;
                }
                if (!asked(cliArgs)) {
                    return 0;
                }

                if (!cliArgs.opt.quiet) {
                    std::cout << "Removing..." << std::endl;
                }
                if (std::remove(entry.path().c_str()) == 0) {
                    if (!cliArgs.opt.quiet) {
                        std::cout << "Successfully removed package." << std::endl;
                    }
                } else {
                    if (!cliArgs.opt.quiet) {
                        std::cerr << "Failed to remove package." << std::endl;
                    }
                    return 1;
                }
            }
        }
    } catch(...) {
        if (!cliArgs.opt.quiet) {
            std::cerr << "Failed to remove." << std::endl;
        }
        return 1;
    }

    std::stringstream tarCmd;
    tarCmd << "tar xzfv " << std::string(std::getenv("HOME")) << "/.aim-backup.tar.gz -C /";
    return system(tarCmd.str().c_str());
}

static void ensureAppFldr(const args::Args &cliArgs) {
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
            throw std::exception();
        }
    }
}

static void rmBackup(const args::Args &cliArgs) {
    std::stringstream backupPath;
    backupPath << std::string(std::getenv("HOME")) << "/.aim-backup.tar.gz";
    if (std::remove(backupPath.str().c_str()) == 0) {
        if (!cliArgs.opt.quiet) {
            std::cout << "Removed old backup." << std::endl;
        }
    } else if (std::filesystem::exists(backupPath.str())) {
        if (!cliArgs.opt.quiet) {
            std::cerr << "Failed to remove old backup." << std::endl;
        }
        throw std::exception();
    }
}

static void makeBackup(const args::Args &cliArgs) {
    std::stringstream tarCmd;
    tarCmd
        << "tar cvfz "
        << std::string(std::getenv("HOME")) << "/.aim-backup.tar.gz "
        << std::string(std::getenv("HOME")) << "/Applications/*.AppImage";
    if (system(tarCmd.str().c_str()) != 0) {
        if (!cliArgs.opt.quiet) {
            std::cerr << "Failed to make backup!" << std::endl;
        }
        throw std::exception();
    }
    std::cout << "Created backup at ~/.aim-backup.tar.gz" << std::endl;
}

static bool asked(const args::Args &cliArgs) {
    if (!cliArgs.opt.ask) {
        // Skip ask if the user didn't add the tag
        return true;
    }
    std::cout << "Do you want to perform this action? (yes/no):";
    std::string input;
    std::cin >> input;
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);
    if (input == "y" || input == "yes") {
        return true;
    }
    return false;
}
