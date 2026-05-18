#include <iostream>
#include <future>

#include "configparser.h"
#include "reportparser.h"
#include "analyzer.h"

void printData(const std::vector<SensorData>& datas) {
    std::cout << std::endl << "Data:\n";
    for(auto& d : datas) {
        std::cout << d.fileName << std::endl << d.sensorName << std::endl;
        for (auto& [name, value] : d.values) {
            std::cout << "  " << name << ": (" << value.rawValue << ", " << value.numericValue << ")\n";
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr
            << "Формат: parser <config.json> <files>\n";

        return 1;
    }

    std::string configPath = argv[1];

    std::vector<std::string> files;
    for (int i = 2; i < argc; ++i) {
        files.push_back(argv[i]);
    }

    ConfigParser parser;
    
    try {
        parser.loadConfig(configPath);
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    ReportParser reportParser(parser);

    std::vector<SensorData> allData;
    std::vector<std::future<std::vector<SensorData>>> futures;

    for (const auto& file : files)
    {
        futures.push_back(
            std::async(
                std::launch::async,
                &ReportParser::parseFile,
                &reportParser,
                file
            )
        );
    }

    for (auto& future : futures) {
        auto datas = future.get();

        allData.insert(allData.end(), datas.begin(), datas.end());
    }

    Analyzer analyzer;
    analyzer.findMinMax(allData);
    analyzer.print();

    return 0;
}