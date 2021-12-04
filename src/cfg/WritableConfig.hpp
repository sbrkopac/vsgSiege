
#pragma once

#include "IConfig.hpp"
#include <string>
#include <unordered_map>

#include <spdlog/spdlog.h>

namespace ehb
{
    class WritableConfig : public IConfig
    {
    public:
        // TODO: remove this when we have proper config implementations
        WritableConfig();

        WritableConfig(int argc, char* argv[]);

        virtual bool getBool(const std::string& key, bool defaultValue) const override;
        virtual float getFloat(const std::string& key, float defaultValue) const override;
        virtual int getInt(const std::string& key, int defaultValue) const override;
        virtual const std::string& getString(const std::string& key, const std::string& defaultValue) const override;

        void setBool(const std::string& key, bool value);
        void setFloat(const std::string& key, float value);
        void setInt(const std::string& key, int value);
        void setString(const std::string& key, const std::string& value);

        // TODO: clean me up, maybe have a report context or whatever as an argument?
        virtual void dump(const std::string& context) override;

    private:
        std::unordered_map<std::string, bool> boolMap;
        std::unordered_map<std::string, float> floatMap;
        std::unordered_map<std::string, int> intMap;
        std::unordered_map<std::string, std::string> stringMap;

        std::shared_ptr<spdlog::logger> log;
    };

    inline void WritableConfig::setBool(const std::string& key, bool value)
    {
        if (!key.empty()) { boolMap[key] = value; }
    }

    inline void WritableConfig::setFloat(const std::string& key, float value)
    {
        if (!key.empty()) { floatMap[key] = value; }
    }

    inline void WritableConfig::setInt(const std::string& key, int value)
    {
        if (!key.empty()) { intMap[key] = value; }
    }

    inline void WritableConfig::setString(const std::string& key, const std::string& value)
    {
        if (!key.empty()) { stringMap[key] = value; }
    }
} // namespace ehb
