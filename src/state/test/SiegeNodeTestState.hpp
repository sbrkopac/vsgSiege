
#pragma once

#include "state/IGameState.hpp"

namespace ehb
{
    class SiegeNodeTestState final : public IGameState
    {
    public:
        SiegeNodeTestState(Systems& systems);

        virtual ~SiegeNodeTestState() = default;

        virtual void enter() override;
        virtual void leave() override;
        virtual void update(double deltaTime) override;

        virtual const std::string name() const override { return "SiegeNodeTestState"; }

    private:
        Systems& systems;
    };

    inline SiegeNodeTestState::SiegeNodeTestState(Systems& systems) :
        systems(systems) {}
} // namespace ehb
