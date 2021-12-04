
#pragma once

#include "IGameState.hpp"

namespace ehb
{
    class InitState final : public IGameState
    {
    public:
        InitState(Systems& systems);

        virtual ~InitState() = default;

        virtual void enter() override;
        virtual void leave() override;
        virtual void update(double deltaTime) override;

        virtual const std::string name() const override { return "InitState"; }

    private:
        Systems& systems;
    };

    inline InitState::InitState(Systems& systems) :
        systems(systems)
    {
    }
} // namespace ehb
