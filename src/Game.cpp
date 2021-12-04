
#include "Game.hpp"
#include "cfg/WritableConfig.hpp"
#include "state/FullMapTestState.hpp"
#include "state/InitState.hpp"
#include "state/ProfileLoadingState.hpp"
#include "state/RegionStitchTestState.hpp"
#include "state/RegionTestState.hpp"
#include "state/SiegeNodeTestState.hpp"

#include <spdlog/spdlog.h>

#include <vsg/viewer/CloseHandler.h>
#include <vsg/viewer/Trackball.h>
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

        auto commandGraph = vsg::createCommandGraphForView(window, systems.camera, systems.scene3d);
        viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});

        viewer->addEventHandler(vsg::CloseHandler::create(viewer));
        viewer->addEventHandler(vsg::Trackball::create(systems.camera));

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
            viewer->compile();

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
        if (gameStateType == "ProfileLoadingState") { return new ProfileLoadingState(systems); }

        if (gameStateType == "SiegeNodeTestState") { return new SiegeNodeTestState(systems); }

        if (gameStateType == "RegionTestState") { return new RegionTestState(systems); }

        if (gameStateType == "InitState") { return new InitState(systems); }

        if (gameStateType == "FullMapTestState") { return new FullMapTestState(systems); }

        if (gameStateType == "RegionStitchTestState") { return new RegionStitchTestState(systems); }

        return nullptr;
    }
} // namespace ehb