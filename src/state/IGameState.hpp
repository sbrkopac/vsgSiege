
#pragma once

#include <chrono>
#include <spdlog/spdlog.h>
#include <string>

//! TODO: remove when dynamic scene graphs have been refactored
#include <vsg/core/Object.h>

namespace ehb
{
    // eventually we will move this into its profile / test project
    using Timer = std::chrono::high_resolution_clock;
    using TimePoint = Timer::time_point;
    using Duration = std::chrono::duration<float, std::milli>;

    class Systems;
    class IGameState
    {
    protected:
        std::shared_ptr<spdlog::logger> log;

    public:
        IGameState() { log = spdlog::get("log"); }
        virtual ~IGameState() = default;

        virtual void enter() {}
        virtual void leave() {}
        virtual void update(double deltaTime) {}
        // virtual bool handle(const osgGA::GUIEventAdapter & event, osgGA::GUIActionAdapter & action) { return false; }

        //! TODO: remove once VSG has refactored support for dynamic scene graphs
        //! this will recalculate the resource hints for the given object and
        void compile(Systems& systems, vsg::ref_ptr<vsg::Object> object);

        virtual const std::string name() const = 0;
    };

} // namespace ehb
