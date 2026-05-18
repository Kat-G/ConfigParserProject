#pragma once

#include <vector>
#include <string>
#include <regex>
#include <filesystem>

#include "configparser.h"

struct ParsedValue {
    std::string rawValue;
    double numericValue = 0;

    bool valid = true;
};

struct SensorData {
    std::string fileName;
    std::string sensorName;

    std::unordered_map<std::string, ParsedValue> values;
};

class ReportParser {
private:
    const ConfigParser& config;

    inline static const
    std::unordered_map<std::string, double> speedMultipliers =
    {
        {"bit", 1},
        {"Kbit", 1000},
        {"Mbit", 1000000},
        {"Gbit", 1000000000}
    };

    ParsedValue parseValue(const Rule& rule, const std::smatch& match) {
        ParsedValue value;

        try {
            if (rule.type == "value")
            {
                value.rawValue = match[1];
                value.numericValue =
                    std::stod(match[1]);
            }
            else if (rule.type == "bool")
            {
                value.rawValue = match[1];

                if (match[1] == rule.trueVal)
                {
                    value.numericValue = 1;
                } else if (match[1] == rule.falseVal) {
                    value.numericValue = 0;
                }
                else {
                    value.valid = false;
                }
            }
            else if (rule.type == "speed")
            {
                if (match.size() < 3) {
                    value.valid = false;
                    return value;
                }
                std::string numberStr = match[1];
                std::string unit = match[2];

                value.rawValue =
                    numberStr + " " + unit + "/s";

                double number =
                    std::stod(numberStr);

                number *= speedMultipliers.at(unit);

                value.numericValue = number;
            }
        } catch (const std::exception& ex) {
            value.valid = false;
            std::cerr << "Ошибка парсинга: " << rule.name << " -> " << ex.what() << std::endl;
        }

        return value;
    }

public:
    ReportParser(const ConfigParser& cfg): config(cfg) { }

    std::vector<SensorData> parseFile(const std::string& path) {
        std::ifstream file(path);

        if (!file.is_open()) {
            std::cerr << "Не удалось открыть файл\n";
            return {};
        }

        std::string line;

        const auto& rules = config.getRules();
        Sensor currentSensor;
        std::vector<SensorData> datas;
        SensorData data;

        bool hasActiveSensor = false;
        std::smatch match;

        while (std::getline(file, line)) {
            bool sensorFound = false;

            for (auto& [name, sensor] : config.getSensors())
            {
                if (line.find(sensor.ruleKey) != std::string::npos)
                {
                    if (hasActiveSensor) {
                        datas.push_back(data);
                    }
                    data = SensorData{};

                    currentSensor = sensor;
                    sensorFound = true;

                    data.fileName = std::filesystem::path(path).filename().string();
                    data.sensorName = currentSensor.name;

                    hasActiveSensor = true;
                    break;
                }
            }

            if (sensorFound)
                continue;

            if (!hasActiveSensor)
                continue;
            
            for (auto& ruleName : currentSensor.rules) {
                const Rule& rule = rules.at(ruleName);

                if (std::regex_search(line, match, rule.regexPattern)){
                    data.values[rule.name] = parseValue(rule, match);
                }
            }
        }
        if (hasActiveSensor) {
            datas.push_back(data);
        }

        return datas;
    }
};