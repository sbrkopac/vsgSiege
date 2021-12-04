
#include "GameStateMgr.hpp"

#include "IGameState.hpp"
#include "IGameStateProvider.hpp"

#include <spdlog/spdlog.h>

namespace ehb
{
    GameStateMgr::GameStateMgr(IGameStateProvider* provider) :
        provider(provider) { log = spdlog::get("log"); }

    GameStateMgr::~GameStateMgr()
    {
        if (currState.second) { currState.second->leave(); }
    }

    void GameStateMgr::request(const std::string& gameStateType)
    {
        if (IGameState* state = provider->createGameState(gameStateType, *this))
        {
            if (pendState.second) { log->warn("GameStateMgr still has a pending state {} while requesting a new state ({})", pendingStateName(), gameStateType); }

            pendState.first = gameStateType;
            pendState.second.reset(state);
        }
        else
        {
            log->warn("GameState [{}] is unavailable", gameStateType);
        }
    }

    void GameStateMgr::update(double deltaTime)
    {
        if (pendState.second)
        {
            if (currState.second)
            {
                log->info("==========================================================================================================================");
                log->info("| BEGIN - WORLD STATE TRANSITION: {} --> {}", currentStateName(), pendingStateName());
                log->info("|");

                currState.second->leave();
            }
            else
            {
                log->info("==========================================================================================================================");
                log->info("| BEGIN - WORLD STATE TRANSITION: {}", pendingStateName());
                log->info("|");
            }

            prevState.first = currState.first;
            prevState.second = std::move(currState.second);

            log->info("|");
            log->info("| END - WORLD STATE TRANSITION. State = {}", pendingStateName());
            log->info("==========================================================================================================================");

            currState.first = pendState.first;
            currState.second = std::move(pendState.second);

            currState.second->enter();
        }

        if (currState.second) { currState.second->update(deltaTime); }
    }

    std::string GameStateMgr::currentStateName() const { return currState.second->name(); }

    std::string GameStateMgr::previousStateName() const { return prevState.second->name(); }

    std::string GameStateMgr::pendingStateName() const { return pendState.second->name(); }
} // namespace ehb
