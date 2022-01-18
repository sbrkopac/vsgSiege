
#include "ReaderWriterRegion.hpp"
#include "io/IFileSys.hpp"
#include "vsg/ReaderWriterSNO.hpp"
#include "world/Region.hpp"
#include "world/SiegeNode.hpp"

#include <vsg/io/read.h>
#include <vsg/nodes/Light.h>

#include <iostream>

namespace ehb
{
    //! TODO: add alpha
    vsg::vec3 convert(uint32_t color)
    {
        float b = static_cast<float>((color & 0x000000FF) >> 0) / 255;
        float g = static_cast<float>((color & 0x0000FF00) >> 8) / 255;
        float r = static_cast<float>((color & 0x00FF0000) >> 16) / 255;
        float a = static_cast<float>((color & 0xFF000000) >> 24) / 255;
        return vsg::vec3(r, g, b);
    }

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
        auto lightsdotgas = path + "/lights/lights.gas";

        // mandatory files required to draw a region
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

                // optional data
                if (auto lights = fileSys.openGasFile(lightsdotgas))
                {
                    log->info("Loading {}", lightsdotgas);

                    // parent to hold all the light information
                    auto lightXform = vsg::MatrixTransform::create();

                    // there is only 1 child in this file that is the main [lights] fuel black
                    if (auto children = lights->child("lights"))
                    {
                        //! TODO: gas->eachChildOfType?
                        for (auto light : children->eachChild())
                        {
                            if (light->type() == "point")
                            {
                                auto vsgLight = vsg::PointLight::create();

                                vsgLight->name = light->name();
                                vsgLight->color = convert(light->valueAsUInt("color"));
                            }
                            else if (light->type() == "directional")
                            {
                                auto vsgLight = vsg::DirectionalLight::create();

                                vsgLight->name = light->name();
                                vsgLight->color = convert(light->valueAsUInt("color"));
                            }
                            else if (light->type() == "spot")
                            {
                                auto vsgLight = vsg::SpotLight::create();

                                vsgLight->name = light->name();
                                vsgLight->color = convert(light->valueAsUInt("color"));
                            }
                            else
                            {
                                log->error("Unhandled light type: {}", light->type());
                            }
                        }
                    }
                }
                else { log->warn("No lights.gas defined for {}", path); }

                return region;
            }
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
