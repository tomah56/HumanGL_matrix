#pragma once

#include <string>
#include <fstream>
#include <stdexcept>

namespace ResourcePath {
    inline std::string getPath(const std::string &path) {
        std::string macPath = "../" + path;
        std::ifstream testFile(macPath);
        if (testFile.good()) {
            testFile.close();
            return macPath;
        }

		return path;
    }
}
