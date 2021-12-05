
#pragma once

#include "state/IGameState.hpp"

namespace ehb
{
    class ASPMeshTestState final : public IGameState
    {
    public:
        ASPMeshTestState(Systems& systems);

        virtual ~ASPMeshTestState() = default;

        virtual void enter() override;
        virtual void leave() override;
        virtual void update(double deltaTime) override;

        virtual const std::string name() const override { return "ASPMeshTestState"; }

    private:
        Systems& systems;
    };

    inline ASPMeshTestState::ASPMeshTestState(Systems& systems) :
        systems(systems) {}
} // namespace ehb
