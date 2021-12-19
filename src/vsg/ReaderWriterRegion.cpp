
#include "ReaderWriterRegion.hpp"
#include "io/IFileSys.hpp"
#include "vsg/ReaderWriterSNO.hpp"
#include "world/Region.hpp"
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
        if (vsg::fileExtension(filename) != ".region") return {};

        // the below feels a bit hacky but we need to be able to access all the files in the region on load - for now
        // you can think of this loader as a proxy loader for the rest of the elements in a region

        log->info("about to load region with path : {} and simpleFilename {}", filename, simpleFilename);

        auto path = vsg::removeExtension(filename);
        auto maindotgas = path + "/main.gas";
        auto nodesdotgas = path + "/terrain_nodes/nodes.gas";
        auto objectsPath = path + "/objects/regular";
        auto actordotgas = objectsPath + "/actor.gas";

        InputStream main = fileSys.createInputStream(maindotgas);
        InputStream nodes = fileSys.createInputStream(nodesdotgas);

        if (main == nullptr || nodes == nullptr)
        {
            log->critical("main.gas or nodes.gas are missing for region {}", filename);

            return {};
        }

        if (auto region = read_cast<Region>(*main, options))
        {
            // this read loads in our siege nodes and assigns properties
            if (auto nodeData = vsg::read_cast<vsg::Group>(nodesdotgas, options))
            {
                region->addChild(nodeData);

                // since we didn't have access to the loader we will visit each node and store the transform at a high level
                // TODO: should we just store the transforms as object data against the nodeData and query from there?
                GenerateGlobalSiegeNodeGuidToNodeXformMap v(region->placedNodeXformMap);
                region->accept(v);

                return region;
            }
        }
            }

#if 0
            if (fileSys.openGasFile(actordotgas))
            {
                log->info("loading actors from {}", actordotgas);
                
                // load up the file here and create a transform for each of the objects
                // assign the TemplateName to the transform and use a visitor later on to do the positioning
                // that's a little less efficient because in the visitor we will need to grab the name again
            }
#endif

            return region;
        }

        return {};
    }

    vsg::ref_ptr<vsg::Object> ReaderWriterRegion::read(std::istream& stream, vsg::ref_ptr<const vsg::Options> options) const
    {
        if (Fuel doc; doc.load(stream))
        {
            auto region = Region::create();

            region->guid = doc.valueAsUInt("region:guid");

            return region;
        }
        else
        {
            log->critical("failed to parse main.gas");
        }

        return {};
    };
} // namespace ehb
