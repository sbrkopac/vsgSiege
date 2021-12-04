
#pragma once

#include "Systems.hpp"
#include "io/LocalFileSys.hpp"
#include "io/NamingKeyMap.hpp"
#include "state/GameStateMgr.hpp"
#include "state/IGameStateProvider.hpp"
#include "world/WorldMapData.hpp"

#include <vsg/io/Options.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/viewer/Viewer.h>

namespace ehb
{
    class WritableConfig;
    class Game final : public IGameStateProvider
    {
    public:
        Game(IConfig& config);

        int exec();

    private:
        virtual IGameState* createGameState(const std::string& gameStateType, IGameStateMgr& gameStateMgr) override;

    private:
        GameStateMgr gameStateMgr;

        //! Temporary object to pass into states and refactor rapidly
        Systems systems;

        vsg::ref_ptr<vsg::Viewer> viewer;
    };
} // namespace ehb