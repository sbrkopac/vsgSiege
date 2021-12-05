
#pragma once

#include "state/IGameState.hpp"

namespace ehb
{
    class Systems;
    class vsgExamplesDrawState final : public IGameState
    {
    public:
        vsgExamplesDrawState(Systems& systems);

        virtual ~vsgExamplesDrawState() = default;

        virtual void enter() override;
        virtual void leave() override;
        virtual void update(double deltaTime) override;

        virtual const std::string name() const override { return "vsgExamplesDrawState"; }

    private:
        Systems& systems;
    };

    inline vsgExamplesDrawState::vsgExamplesDrawState(Systems& systems) :
        systems(systems)
    {
    }
} // namespace ehb
