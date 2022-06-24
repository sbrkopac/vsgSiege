
#include "Game.hpp"
#include "cfg/WritableConfig.hpp"
#include "state/InitState.hpp"

#ifdef SIEGE_TEST_STATES_ENABLED
#    include "state/test/ASPMeshTestState.hpp"
#    include "state/test/FullMapTestState.hpp"
#    include "state/test/ProfileLoadingState.hpp"
#    include "state/test/RegionStitchTestState.hpp"
#    include "state/test/RegionTestState.hpp"
#    include "state/test/SiegeNodeTestState.hpp"
#endif

#ifdef SIEGE_VSG_EXAMPLES_ENABLED
#    include "state/vsgExamples/vsgExamplesDraw.hpp"
#endif

#include <spdlog/spdlog.h>

#include <vsg/viewer/CloseHandler.h>
#include <vsg/viewer/CommandGraph.h>
#include <vsg/viewer/RenderGraph.h>
#include <vsg/viewer/Trackball.h>
#include <vsg/viewer/View.h>
#include <vsg/viewer/Window.h>

namespace ehb
{
    Game::Game(IConfig& config) :
        gameStateMgr(this), systems(config, gameStateMgr) {}

    int Game::exec()
    {
        const uint32_t width = systems.config.getInt("width", 800), height = systems.config.getInt("height", 600);

        viewer = vsg::Viewer::create();

        // setup custom window traits
        auto traits = vsg::WindowTraits::create();
        traits->windowTitle = "Open Siege supported by VSG";
        traits->width = width;
        traits->height = height;
        traits->debugLayer = systems.config.getBool("debuglayer");
        traits->apiDumpLayer = systems.config.getBool("apidumplayer");
        traits->swapchainPreferences.presentMode = VkPresentModeKHR::VK_PRESENT_MODE_IMMEDIATE_KHR; // disable vsync

        // setup our main window
        vsg::ref_ptr<vsg::Window> window = vsg::Window::create(traits);
        viewer->addWindow(window);

        // camera related details
        auto viewport = vsg::ViewportState::create(VkExtent2D{width, height});
        auto perspective = vsg::Perspective::create(45, static_cast<double>(width) / static_cast<double>(height), 0.1, 1000.0);
        auto lookAt = vsg::LookAt::create(vsg::dvec3(1.0, 15.0, 50.0), vsg::dvec3(0.0, 0.0, 0.0), vsg::dvec3(0.0, 1.0, 0.0));
        systems.camera = vsg::Camera::create(perspective, lookAt, viewport);

        systems.view = vsg::View::create(systems.camera);
        auto renderGraph = vsg::RenderGraph::create(window, systems.view);
        auto commandGraph = vsg::CommandGraph::create(window);
        commandGraph->addChild(renderGraph);

        systems.view->addChild(systems.scene3d);

        viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});

        viewer->addEventHandler(vsg::CloseHandler::create(viewer));
        viewer->addEventHandler(vsg::Trackball::create(systems.camera));

        systems.viewer = viewer;

        if (systems.config.getBool("profile")) { gameStateMgr.request("ProfileLoadingState"); }
        else
        {
            gameStateMgr.request("InitState");
        }

        // delta time
        auto lastTime = std::chrono::steady_clock::now();

        // used to track total fps
        auto before = std::chrono::steady_clock::now();

        while (viewer->advanceToNextFrame())
        {
            auto now = std::chrono::steady_clock::now();
            float delta = std::chrono::duration_cast<std::chrono::duration<float>>(now - lastTime).count();

            // the scene graph can be manipulated here
            gameStateMgr.update(delta);

            // i don't think we should have to compile on every frame?
            // TODO: swap this out for a dynamic way of adding this to the scene graph
            // Compile Request from VSG examples?
            // Don't call compile here as it causes a crash after the state compiles the objects needed by vulkan
            // Is the above note proper behavior? should multiple compile calls destroy the previously compiled vulkan objects?
            // viewer->compile();

            viewer->handleEvents();
            viewer->recordAndSubmit();
            viewer->present();

            lastTime = viewer->getFrameStamp()->time;
        }

        auto runtime = std::chrono::duration<double, std::chrono::seconds::period>(std::chrono::steady_clock::now() - before).count();
        spdlog::get("log")->info("avg fps: {}", 1.0 / (runtime / (double)viewer->getFrameStamp()->frameCount));

        return 0;
    }

    IGameState* Game::createGameState(const std::string& gameStateType, IGameStateMgr& gameStateMgr)
    {
        if (gameStateType == "InitState") { return new InitState(systems); }

#ifdef SIEGE_TEST_STATES_ENABLED
        if (gameStateType == "ProfileLoadingState")
        {
            return new ProfileLoadingState(systems);
        }

        if (gameStateType == "SiegeNodeTestState") { return new SiegeNodeTestState(systems); }

        if (gameStateType == "ASPMeshTestState") { return new ASPMeshTestState(systems); }

        if (gameStateType == "RegionTestState") { return new RegionTestState(systems); }

        if (gameStateType == "FullMapTestState") { return new FullMapTestState(systems); }

        if (gameStateType == "RegionStitchTestState") { return new RegionStitchTestState(systems); }
#endif

#ifdef SIEGE_VSG_EXAMPLES_ENABLED
        if (gameStateType == "vsgExamplesDraw")
        {
            return new vsgExamplesDrawState(systems);
        }
#endif

        return nullptr;
    }
} // namespace ehb