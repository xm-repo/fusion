
#include "MegaBI.hpp"
#include "ThreadPool.hpp"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <regex>
#include <thread>
#include <string>
#include <vector>
#include <chrono>

namespace
{

std::pair<int,int>& operator+=(std::pair<int, int>& a, 
    const std::pair<int, int>& b)
{
    a.first += b.first;
    a.second += b.second;
    return a;
}

std::vector<std::filesystem::path> getLogFiles(const std::string& path)
{
    namespace fs = std::filesystem;

    std::vector<fs::path> logs;
    
    static const std::regex re("^file[1-9][0-9]*\\.log$");

    try
    {
        for (const auto& entry : fs::directory_iterator(path))
        {
            if (fs::is_regular_file(entry)
                && std::regex_match(entry.path().filename().string(), re))
            {
                logs.push_back(entry);
            }
        }
    }
    catch (const fs::filesystem_error&)
    {
        // log or something
    }

    return logs;
}

std::pair<int, int> parseLog(const std::filesystem::path& path, 
    MegaBI::Accumulator& storage)
{
    std::ifstream log(path);

    if (!log)
    {
        return { 0, 0 };
    }

    static const std::size_t MAX_SIZE = 1000;

    MegaBI::Accumulator acc;

    int totalRecords = 0;
    int parsedRecords = 0;

    for (std::string line; std::getline(log, line); totalRecords++)
    {
        if (acc.parse(line))
        {
            parsedRecords++;
        }

        if (std::size(acc) > MAX_SIZE)
        {
            storage += acc;
            acc.clear();
        }
    }

    storage += acc;

    return { totalRecords, parsedRecords };
}

}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "MegaBI path:str threads:int" << std::endl;
        return 0;
    }

    const std::string path = argv[1];
    const auto threads = std::stoi(argv[2]);

    const auto logs = getLogFiles(path);

    std::cout << "Reading " << std::size(logs)
        << " file(s) in " << std::filesystem::absolute(path);

    MegaBI::Accumulator acc;
    std::pair<int, int> counters;

    auto start = std::chrono::steady_clock::now();
    {
        MegaBI::ThreadPool pool(std::max(1, threads));
        std::vector<std::future<std::pair<int, int>>> futures;
        for (const auto& log : logs)
        {
            futures.push_back(pool.execute(parseLog, log, acc));
        }
        for (auto& future : futures)
        {
            counters += future.get();
        }
    }
    auto end = std::chrono::steady_clock::now();

    std::cout << std::endl << "Total records: " << counters.first << std::endl;
    std::cout << "Parsed records: " << counters.second << std::endl;
    std::cout << "Time: " << std::chrono::duration_cast<
        std::chrono::seconds>(end - start).count() << "s" << std::endl;

    std::cout << "Writing";
    {
        std::ofstream agr("agr.txt");
        agr << acc;
    }

    return 0;
}
