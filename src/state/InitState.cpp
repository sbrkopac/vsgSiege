
#include "InitState.hpp"
#include "Systems.hpp"
#include "state/GameStateMgr.hpp"

#include "vsg/ReaderWriterASP.hpp"
#include "vsg/ReaderWriterRAW.hpp"
#include "vsg/ReaderWriterRegion.hpp"
#include "vsg/ReaderWriterSiegeMesh.hpp"
#include "vsg/ReaderWriterSiegeNodeList.hpp"

#include <vsg/utils/SharedObjects.h>

#include <stdlib.h> // strtoul
#include <spdlog/spdlog.h>

namespace ehb
{
    // hook into our virtual filesystem
    vsg::Path findFileCallback(const vsg::Path& filename, const vsg::Options* options)
    {
        static auto namingKeyMap = options->getObject<NamingKeyMap>("NamingKeyMap");

        return namingKeyMap->findDataFile(filename.string());
    }

    void InitState::enter()
    {
        log->info("Entered Init State");

        IFileSys& fileSys = systems.fileSys;
        IConfig& config = systems.config;
        NamingKeyMap& namingKeyMap = *systems.namingKeyMap;
        MeshDatabase& meshDatabase = *systems.meshDatabase;
        auto options = systems.options;

        // attempt to load the tank filesystem and if it's not available check for bits
        // if bits also aren't available then we have to go to the vsgExamples
        bool dsContentAvailable = fileSys.init(config);
        if (!dsContentAvailable)
        {
            log->error("DS Tank files were unable to be loaded. Attempting fallback to bits");

            dsContentAvailable = !config.getString("bits", "").empty();
        }

        // sanity check to stop errors from being thrown when using vsgExamples
        if (dsContentAvailable)
        {
            namingKeyMap.init(fileSys);

            systems.worldMap = std::make_unique<WorldMap>(fileSys);

            options->setObject("NamingKeyMap", &namingKeyMap);

            // setup the node database for our mesh guid to filename mappings
            {
                static const std::string directory = "/world/global/siege_nodes";

                fileSys.eachGasFile(
                    directory,
                    [this, &meshDatabase](const std::string& filename, auto doc) {
                        for (auto root : doc->eachChild())
                        {
                            for (auto node : root->eachChild())
                            {

                                // auto test = std::stoul(node->valueOf("guid"));
                                auto guid = std::strtoul(node->valueOf("guid").c_str(), nullptr, 0);
                                auto filename = stringtool::convertToLowerCase(node->valueOf("filename"));

                                meshDatabase.InsertMeshMapping(guid, filename);
                            }
                        }
                    });
            }

            options->setObject("MeshDatabase", &meshDatabase);

            // experimenting with putting the GraphicsPipeline setup code in the Reader / Writers
            auto readerWriterRAW = ReaderWriterRAW::create(fileSys);
            auto readerWriterSNO = ReaderWriterSiegeMesh::create(fileSys);
            auto readerWriterASP = ReaderWriterASP::create(fileSys);
            auto readerWriterSiegeNodeList = ReaderWriterSiegeNodeList::create(fileSys);
            auto readerWriterRegion = ReaderWriterRegion::create(fileSys);

            // accessed by the ReaderWriters to setup pipelines and layouts
            options->setObject("SiegeNodeGraphicsPipeline", readerWriterSNO->createOrShareGraphicsPipeline());
            options->setObject("SiegeNodeLayout", readerWriterSNO->createOrShareGraphicsPipeline()->pipeline->layout);

            options->readerWriters = {readerWriterRAW, readerWriterSNO, readerWriterASP, readerWriterSiegeNodeList, readerWriterRegion};

            options->findFileCallback = &findFileCallback;

            options->sharedObjects = vsg::SharedObjects::create();
        }
        else
        {
            log->error("Tank files and bits directory are unavailable. Falling back to vsgExamples");
        }

        if (const std::string& state = config.getString("state"); !state.empty())
        {
            systems.gameStateMgr.request(state);
        }
        else
        {
#if SIEGE_VSG_EXAMPLES_ENABLED
            // Dungeon Siege content is unavailable so default to something the user can run
            if (!dsContentAvailable)
            {
                systems.gameStateMgr.request("vsgExamplesDraw");

                return;
            }
#endif

#if SIEGE_TEST_STATES_ENABLED
            // default to region test state if nothing passed to command line
            systems.gameStateMgr.request("RegionTestState");
#endif
        }
    }

    void InitState::leave() {}

    void InitState::update(double deltaTime) {}
} // namespace ehb
