
#pragma once

#include "IGameState.hpp"

namespace ehb
{
    class Systems;
    class EmptyTestState final : public IGameState
    {
    public:
        EmptyTestState(Systems& systems);

        virtual ~EmptyTestState() = default;

        virtual void enter() override;
        virtual void leave() override;
        virtual void update(double deltaTime) override;

        virtual const std::string name() const override { return "EmptyTestState"; }

    private:
        Systems& systems;
    };

    inline EmptyTestState::EmptyTestState(Systems& systems) :
        systems(systems)
    {
    }
} // namespace ehb
