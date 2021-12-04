
#pragma once

#include "IGameState.hpp"

namespace ehb
{
    class FullMapTestState final : public IGameState
    {
    public:
        FullMapTestState(Systems& systems);

        virtual ~FullMapTestState() = default;

        virtual void enter() override;
        virtual void leave() override;
        virtual void update(double deltaTime) override;

        virtual const std::string name() const override { return "FullMapTestState"; }

    private:
        Systems& systems;
    };

    inline FullMapTestState::FullMapTestState(Systems& systems) :
        systems(systems) {}
} // namespace ehb
