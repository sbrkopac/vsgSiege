
#pragma once

#include "IGameState.hpp"

namespace ehb
{
    class RegionStitchTestState final : public IGameState
    {
    public:
        RegionStitchTestState(Systems& systems);

        virtual ~RegionStitchTestState() = default;

        virtual void enter() override;
        virtual void leave() override;
        virtual void update(double deltaTime) override;

        virtual const std::string name() const override { return "RegionStitchTestState"; }

    private:
        Systems& systems;
    };

    inline RegionStitchTestState::RegionStitchTestState(Systems& systems) :
        systems(systems) {}
} // namespace ehb
