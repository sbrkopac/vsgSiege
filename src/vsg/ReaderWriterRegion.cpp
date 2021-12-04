
#include "ReaderWriterRegion.hpp"
#include "io/IFileSys.hpp"
#include "vsg/ReaderWriterSNO.hpp"
#include "world/SiegeNode.hpp"

#include <vsg/io/read.h>

namespace ehb
{
    ReaderWriterRegion::ReaderWriterRegion(IFileSys& fileSys) :
        fileSys(fileSys) { log = spdlog::get("log"); }

    vsg::ref_ptr<vsg::Object> ReaderWriterRegion::read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options) const
    {
        const std::string simpleFilename = vsg::simpleFilename(filename);

        // check to make sure this is a nodes.gas file
        if (vsg::fileExtension(filename) != "region") return {};

        // the below feels a bit hacky but we need to be able to access all the files in the region on load - for now
        // you can think of this loader as a proxy loader for the rest of the elements in a region

        log->info("about to load region with path : {} and simpleFilename {}", filename, simpleFilename);

        auto path = vsg::removeExtension(filename);
        auto maindotgas = path + "/main.gas";
        auto nodesdotgas = path + "/terrain_nodes/nodes.gas";

        InputStream main = fileSys.createInputStream(maindotgas);
        InputStream nodes = fileSys.createInputStream(nodesdotgas);

        if (main == nullptr || nodes == nullptr)
        {
            log->critical("main.gas or nodes.gas are missing for region {}", filename);

            return {};
        }

        if (auto region = read_cast<vsg::MatrixTransform>(*main, options))
        {
            if (auto nodeData = vsg::read_cast<vsg::Group>(nodesdotgas, options))
            {
                region->addChild(nodeData);

                return region;
            }
        }

        return {};
    }

    vsg::ref_ptr<vsg::Object> ReaderWriterRegion::read(std::istream& stream, vsg::ref_ptr<const vsg::Options> options) const
    {
        if (Fuel doc; doc.load(stream))
        {
            auto transform = vsg::MatrixTransform::create();

            transform->setValue("guid", doc.valueAsUInt("region:guid"));

            return transform;
        }
        else
        {
            log->critical("failed to parse main.gas");
        }

        return {};
    };
} // namespace ehb
