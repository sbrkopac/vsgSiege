
#pragma once

#include "cfg/IConfig.hpp"
#include "io/LocalFileSys.hpp"
#include "io/NamingKeyMap.hpp"
#include "io/TankFileSys.hpp"

#include "state/GameStateMgr.hpp"
#include "world/WorldMapData.hpp"
#include "ContentDb.hpp"

#include <vsg/io/Options.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/viewer/Camera.h>
#include <vsg/viewer/View.h>
#include <vsg/viewer/Viewer.h>

namespace ehb
{
    // Temp god object to pass everywhere for easy prototyping
    class Systems final
    {

    public:
        Systems(IConfig& config, GameStateMgr& mgr);

        IConfig& config;

        GameStateMgr& gameStateMgr;

        //! this is the only IFileSys object that should ever exist
        //! Any local file system operations should be done via std::filesystem
        TankFileSys fileSys;

        vsg::ref_ptr<NamingKeyMap> namingKeyMap = NamingKeyMap::create();
        ContentDb contentDb;
        WorldMapDataCache worldMapData;

        //! created in Game.cpp
        vsg::ref_ptr<vsg::Camera> camera;
        vsg::ref_ptr<vsg::Viewer> viewer; // putting this as a system to attempt to work around dynamic graph bugs
        vsg::ref_ptr<vsg::View> view;

        //! specific to this object
        vsg::ref_ptr<vsg::Options> options = vsg::Options::create();
        vsg::ref_ptr<vsg::StateGroup> scene3d = vsg::StateGroup::create();
        vsg::ref_ptr<vsg::CompileTraversal> compileTraversal = vsg::CompileTraversal::create();
    };

    inline Systems::Systems(IConfig& config, GameStateMgr& mgr) :
        config(config), gameStateMgr(mgr) {}
} // namespace ehb