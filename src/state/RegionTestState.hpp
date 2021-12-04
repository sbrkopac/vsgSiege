
#pragma once

#include "IGameState.hpp"

namespace ehb
{
    class RegionTestState final : public IGameState
    {
    public:
        RegionTestState(Systems& systems);

        virtual ~RegionTestState() = default;

        virtual void enter() override;
        virtual void leave() override;
        virtual void update(double deltaTime) override;

        virtual const std::string name() const override { return "RegionTestState"; }

    private:
        Systems& systems;
    };

    inline RegionTestState::RegionTestState(Systems& systems) :
        systems(systems) {}
} // namespace ehb
