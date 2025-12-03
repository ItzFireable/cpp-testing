#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <cmath>

namespace Utils {
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& s, char delimiter);
    bool hasEnding(std::string const &fullString, std::string const &ending);

    std::string readFile(const std::string& path);
    std::string formatMemorySize(size_t bytes);

    std::vector<std::string> getChartList();
    std::string getChartFile(const std::string& chartDirectory);
    static bool fileExists(const std::string& path);

    double pNorm(const std::vector<double>& values, const std::vector<double>& weights, double P);
    double calculateStandardDeviation(const std::vector<double>& array);
}

#endif