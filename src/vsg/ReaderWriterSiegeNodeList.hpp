
#pragma once

#include "io/NamingKeyMap.hpp"
#include <vsg/io/ReaderWriter.h>

#include <spdlog/spdlog.h>

namespace ehb
{
    class IFileSys;
    class ReaderWriterSiegeNodeList final : public vsg::Inherit<vsg::ReaderWriter, ReaderWriterSiegeNodeList>
    {
    public:
        ReaderWriterSiegeNodeList(IFileSys& fileSys);

        virtual vsg::ref_ptr<vsg::Object> read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> = {}) const override;
        virtual vsg::ref_ptr<vsg::Object> read(std::istream& stream, vsg::ref_ptr<const vsg::Options> = {}) const override;

    private:
        const std::string& resolveFileName(const std::string& filename) const;

    private:
        IFileSys& fileSys;

        std::unordered_map<std::string, std::string> keyMap;

        std::shared_ptr<spdlog::logger> log;
    };

    inline const std::string& ReaderWriterSiegeNodeList::resolveFileName(const std::string& filename) const
    {
        const auto itr = keyMap.find(filename);

        return itr != keyMap.end() ? itr->second : filename;
    }
} // namespace ehb
