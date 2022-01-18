
#pragma once

#include "state/IGameState.hpp"

namespace ehb
{
    class LightTestState final : public IGameState
    {
    public:
        LightTestState(Systems& systems);

        virtual ~LightTestState() = default;

        virtual void enter() override;
        virtual void leave() override;
        virtual void update(double deltaTime) override;

        virtual const std::string name() const override { return "LightTestState"; }

    private:
        Systems& systems;
    };

    inline LightTestState::LightTestState(Systems& systems) :
        systems(systems) {}
} // namespace ehb
