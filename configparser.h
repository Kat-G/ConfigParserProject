#pragma once

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <regex>
#include "json.hpp"

using json = nlohmann::json;

struct Rule {
    std::string name;
    std::string type;
    std::string pattern;

    std::regex regexPattern;

    std::string trueVal;
    std::string falseVal;
};

struct Sensor {
    std::string name;
    std::string ruleKey;
    std::vector<std::string> rules;
};

class ConfigParser {
private:
    std::unordered_map<std::string, Rule> rules;
    std::unordered_map<std::string, Sensor> sensors;

    void parseSensors(const json& config) {
        for (auto& s : config["sensors"]) {
            Sensor sensor;

            sensor.name = s["name"].get<std::string>();
            sensor.ruleKey = s["rule"].get<std::string>();

            sensors[sensor.name] = sensor;
        }
    }

    void parseRules(const json& config) {
        for (auto& r : config["rules"]) {
            Rule rule;

            rule.name = r["name"].get<std::string>();
            rule.type = r["type"].get<std::string>();
            rule.pattern = r["rule"].get<std::string>();
            rule.regexPattern = std::regex(rule.pattern);

            if (rule.type == "bool") {
                rule.trueVal = r["true"].get<std::string>();
                rule.falseVal = r["false"].get<std::string>();
            }

            rules[rule.name] = rule;
        }
    }

    void parseExtractors(const json& config) {
        for (auto& e : config["extractors"]) {
            std::string sensorName = e["sensor"];

            for (auto& r : e["rules"]) {
                sensors[sensorName].rules.push_back(
                    r.get<std::string>()
                );
            }
        }
    }

public:
    void loadConfig(const std::string& path) {
        std::ifstream file(path);

        if (!file.is_open()) {
            throw std::runtime_error("Cannot open config file");
        }

        json config;
        file >> config;

        parseSensors(config);
        parseRules(config);
        parseExtractors(config);
    }

    const std::unordered_map<std::string, Sensor>& getSensors() const {
        return sensors;
    }

    const std::unordered_map<std::string, Rule>& getRules() const {
        return rules;
    }

    void printConfig() const {
        std::cout << "=== Sensors ===\n";

        for (const auto& [name, sensor] : sensors) {
            std::cout << name << " -> " << sensor.ruleKey << "\n";
        }

        std::cout << "\n=== Extractors ===\n";

        for (const auto& [name, sensor] : sensors) {
            std::cout << name << ":\n";

            for (const auto& rule : sensor.rules) {
                std::cout << "  " << rule << "\n";
            }
        }

        std::cout << "\n=== Rules ===\n";

        for (const auto& [name, rule] : rules) {
            std::cout << name
                      << " - "
                      << rule.type
                      << " - "
                      << rule.pattern;

            if (rule.type == "bool") {
                std::cout << " - "
                          << rule.trueVal
                          << "/"
                          << rule.falseVal;
            }

            std::cout << "\n";
        }
    }
};