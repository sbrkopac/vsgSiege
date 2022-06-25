
#pragma once

#include "state/IGameState.hpp"

namespace ehb
{
    class BSPTreeTestState final : public IGameState
    {
    public:
        BSPTreeTestState(Systems& systems);

        virtual ~BSPTreeTestState() = default;

        virtual void enter() override;
        virtual void leave() override;
        virtual void update(double deltaTime) override;

        virtual const std::string name() const override { return "BSPTreeTestState"; }

    private:
        Systems& systems;
    };

    inline BSPTreeTestState::BSPTreeTestState(Systems& systems) :
        systems(systems) {}
} // namespace ehb
