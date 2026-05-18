#pragma once

#include <string>
#include "reportparser.h"

struct MinMaxResult
{
    ParsedValue minValue;
    std::string minFile;

    ParsedValue maxValue;
    std::string maxFile;

    bool initialized = false;
};

class Analyzer {
private:
    std::map<std::string, std::unordered_map<std::string, MinMaxResult>> results;

public:
    void findMinMax(const std::vector<SensorData>& allData) {
        for (const auto& data : allData) {
            for (const auto& [ruleName, value]: data.values) {
                auto& result = results[data.sensorName][ruleName];

                if (!value.valid)
                    continue;

                if (!result.initialized) {
                    result.minValue = value;
                    result.maxValue = value;

                    result.minFile = data.fileName;
                    result.maxFile = data.fileName;

                    result.initialized = true;
                } else {
                    if (value.numericValue < result.minValue.numericValue) {
                        result.minValue = value;
                        result.minFile = data.fileName;
                    }
                    if (value.numericValue > result.maxValue.numericValue) {
                        result.maxValue = value;
                        result.maxFile = data.fileName;
                    }
                }
            }
        }
    }

    void print() const {
        for (const auto& [name, value]: results) {
            std::cout << name << ":\n";
            for (const auto& [rname, rvalue]: value) {
                std::cout << rname << ": max=" << rvalue.maxValue.rawValue << "(" << rvalue.maxFile << "), min=" <<
                    rvalue.minValue.rawValue << "(" << rvalue.minFile  << ")\n";
            }
        }
    }
};