
#pragma once

#include <string>
#include <unordered_map>

#include <vsg/core/Object.h>
#include <vsg/core/Inherit.h>

// NOTE: this class violates several of my design decisions, but the code was already written and we can come back around to clean it up later
namespace ehb
{
    class IFileSys;

    // this is a vsg::Object so we can attach it to vsg::Options and reference it in loaders
    class NamingKeyMap final : public vsg::Inherit<vsg::Object, NamingKeyMap>
    {
    public:
        NamingKeyMap() = default;
        virtual ~NamingKeyMap() = default;

        void init(IFileSys& fileSys);

        std::string findDataFile(const std::string& filename) const;

    private:
        std::unordered_map<std::string, std::string> keyMap;
    };
} // namespace ehb
