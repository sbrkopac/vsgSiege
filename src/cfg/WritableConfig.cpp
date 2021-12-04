
#include "WritableConfig.hpp"

namespace ehb
{
    // TODO: make an IConfigSrc class
    // TODO: convert these into implementations of the IConfigSrc class
    //! configuration backend declarations
    void steamConfig(WritableConfig& config);
    void registryConfig(WritableConfig& config);
    void userConfig(WritableConfig& config);
    void argsConfig(WritableConfig& config, int argc, char* argv[]);

    WritableConfig::WritableConfig() { log = spdlog::get("log"); }

    WritableConfig::WritableConfig(int argc, char* argv[])
    {
        log = spdlog::get("log");

        steamConfig(*this);
        registryConfig(*this);
        userConfig(*this);
        argsConfig(*this, argc, argv);
    }

    bool WritableConfig::getBool(const std::string& key, bool defaultValue) const
    {
        const auto itr = boolMap.find(key);

        return itr != boolMap.end() ? itr->second : defaultValue;
    }

    float WritableConfig::getFloat(const std::string& key, float defaultValue) const
    {
        const auto itr = floatMap.find(key);

        return itr != floatMap.end() ? itr->second : defaultValue;
    }

    int WritableConfig::getInt(const std::string& key, int defaultValue) const
    {
        const auto itr = intMap.find(key);

        return itr != intMap.end() ? itr->second : defaultValue;
    }

    const std::string& WritableConfig::getString(const std::string& key, const std::string& defaultValue) const
    {
        const auto itr = stringMap.find(key);

        return itr != stringMap.end() ? itr->second : defaultValue;
    }

    void WritableConfig::dump(const std::string& context)
    {
        auto contxt = spdlog::get(context);

        for (const auto& entry : boolMap)
        {
            contxt->info("{} = {}", entry.first, entry.second);
        }

        for (const auto& entry : floatMap)
        {
            contxt->info("{} = {}", entry.first, entry.second);
        }

        for (const auto& entry : intMap)
        {
            contxt->info("{} = {}", entry.first, entry.second);
        }

        for (const auto& entry : stringMap)
        {
            contxt->info("{} = {}", entry.first, entry.second);
        }
    }
} // namespace ehb
