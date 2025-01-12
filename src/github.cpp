// Implement reading from AIL git repo

#include <sstream>
#include <vector>
#include <string>
#include <optional>
#include <variant>
#include <memory>
#include <filesystem>
#include <iostream>
#include <github.hpp>

using namespace git;

#define AIL_REPO        "https://github.com/AppImageMan/ail"
#define AIL_RAW         "https://raw.githubusercontent.com/AppImageMan/ail"
#define AIL_LFS         "https://media.githubusercontent.com/media/AppImageMan/ail"

static std::optional<std::vector<std::string>> execCliCmd(const std::string &cmd);
static std::string trimStr(const std::string &ln);

std::variant<std::vector<Ref>, std::string> git::fetchAilRefs() {
    std::stringstream gitCmd;
    gitCmd << "git ls-remote --heads '" << AIL_REPO << '\'';
    const auto cmdOutRes = execCliCmd(gitCmd.str());
    if (!cmdOutRes.has_value()) {
        return "Failed to fetch app list";
    }
    const auto cmdLns = cmdOutRes.value();
    std::vector<Ref> refs;
    refs.reserve(cmdLns.size());
    for (const auto &refStr : cmdLns) {
        size_t space = refStr.find('\t'); // Find first space
        if (space == std::string::npos) {
            return "Invalid app list";
        }
        const auto commit = trimStr(refStr.substr(0, space));
        const auto branch = trimStr(refStr.substr(space + 1))
            .substr(std::string("refs/heads/").length());
        refs.push_back(Ref { branch, commit });
    }
    return refs;
}

std::string git::metadataVal(const std::string &commit, const std::string &fileName) {
    std::stringstream curlCmd;
    curlCmd << "curl -s '" << AIL_RAW << '/' << commit << '/' << fileName << '\'';
    const auto cmdOutRes = execCliCmd(curlCmd.str());
    if (!cmdOutRes.has_value() || cmdOutRes.value().size() != 1
            || cmdOutRes.value()[0] == "404: Not Found") {
        std::stringstream errMsg;
        errMsg << "Failed to get file '" << fileName << "' on commit '" << commit << "'";
        return errMsg.str();
    }
    const auto cmdOutLns = cmdOutRes.value();
    std::stringstream ret;
    ret << "<" << cmdOutLns[0] << ">";
    return ret.str();
}

std::optional<std::string> git::dloadLfsFile(
        const std::string &commit, const std::string &fileName,
        const std::string &output, const bool isQuiet) {
    std::stringstream curlCmd;
    curlCmd
        << "curl -L" << (isQuiet ? "s '" : " '")
        << AIL_LFS << '/' << commit << '/' << fileName << "?download=true' -o "
        << output;
    const auto cmdOutRes = execCliCmd(curlCmd.str());
    if (!cmdOutRes.has_value()) {
        std::stringstream errMsg;
        errMsg << "Curl failed to download '" << fileName << "' on commit '" << commit << "'";
        return errMsg.str();
    }
    return std::nullopt;
}

static std::optional<std::vector<std::string>> execCliCmd(const std::string &cmd) {
    auto pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return std::nullopt;
    }
    std::stringstream output;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output << buffer;
    }
    pclose(pipe);
    std::vector<std::string> lns;
    std::string ln;
    while (std::getline(output, ln)) {
        ln = trimStr(ln);
        if (!ln.empty()) {
            lns.push_back(ln);
        }
    }
    return lns;
}

std::string trimStr(const std::string &ln) {
    size_t start = ln.find_first_not_of(" \t\n\r");
    size_t end = ln.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : ln.substr(start, end - start + 1);
}
