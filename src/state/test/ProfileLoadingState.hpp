
#pragma once

#include "state/IGameState.hpp"

namespace ehb
{
    class ProfileLoadingState final : public IGameState
    {
    public:
        ProfileLoadingState(Systems& systems);

        virtual ~ProfileLoadingState() = default;

        virtual void enter() override;
        virtual void leave() override;
        virtual void update(double deltaTime) override;

        virtual const std::string name() const override { return "ProfileLoadingState"; }

    private:
        Systems& systems;
    };

    inline ProfileLoadingState::ProfileLoadingState(Systems& systems) :
        systems(systems) {}
} // namespace ehb
