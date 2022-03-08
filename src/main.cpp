
#include "Game.hpp"
#include "cfg/WritableConfig.hpp" // has a spdlog include in it

// clang-format off
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <vsg/core/Version.h>
// clang-format on

#include <filesystem>

// http://www.codersnotes.com/sleepy/

// it's important that all siege loggers are setup prior to the game executing as we do sink modifications that aren't thread safe
void registerSiegeLogger(ehb::WritableConfig& config, const std::string& name, spdlog::sinks_init_list slist = {});

int main(int argc, char* argv[])
{
    // create a catch all logger for now
    auto log = spdlog::stdout_color_mt("log");

    using namespace ehb;

    WritableConfig config(argc, argv);
    config.dump("log");

    log->info("VSG Version = {}", vsgGetVersionString());

    // Currently we have to have bits as tank reading hasn't been implemented yet
    bool hasBits = !config.getString("bits", "").empty();
    if (!hasBits) log->info("No bits directory detected. Bits are NOT required.");

    // all loggers should be registered here before the Game class gets instantiated
    registerSiegeLogger(config, "filesystem");
    registerSiegeLogger(config, "game");
    registerSiegeLogger(config, "scene");
    registerSiegeLogger(config, "world");

    return Game(config).exec();
}

void registerSiegeLogger(ehb::WritableConfig& config, const std::string& name, spdlog::sinks_init_list slist)
{
    static std::string_view path = config.getString("logs_path", "");

    char dateTime[128] = {'\0'};
    std::time_t now = std::time(nullptr);
    strftime(dateTime, sizeof(dateTime), "%m/%d/%Y %I:%M:%S %p", std::localtime(&now));

    // TODO: what are the last 4 digits of the msqa
    char msqa[128] = {'\0'};
    strftime(msqa, sizeof(msqa), "%Y.%m.0202", std::localtime(&now));

    std::filesystem::path fullpath(path);
    fullpath = fullpath / std::string(name + ".log");

    try
    {
        auto log = spdlog::basic_logger_mt(name, fullpath.string(), true);

        // this is here so the header is printed to the file but not to the console
        log->info("-==--==--==--==--==--==--==--==--==--==--==--==--==--==--==--==--==--==--==-");
        log->info("-== App          : Open Siege ({} - Retail)", std::filesystem::current_path().string());
        log->info("-== Log category : {}", log->name());
        log->info("-== Session      : {}", dateTime);
        log->info("-== Build        : [] (1.11.1.1486 (msqa:{}))", msqa);
        log->info("-==--==--==--==--==--==--==--==--==--==--==--==--==--==--==--==--==--==--==-");

        // get our console sink which is created by default
        auto console = spdlog::get("log")->sinks().back();

        // make sure all logs also print out to our console
        log->sinks().push_back(console);

        // add any custom sinks passed in to the function, should the console be passed in for clarity?
        for (auto sink : slist)
        {
            log->sinks().push_back(sink);
        }

        // logged here so we can see it in the console as well
        log->info("Registered '{}' as a logger", name);
    }
    catch (std::exception& e)
    {
        spdlog::get("log")->error("could not create {} logger: {}", name, e.what());
    }
}