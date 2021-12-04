
#include "ProfileLoadingState.hpp"

#include "Systems.hpp"

namespace ehb
{
    void ProfileLoadingState::enter()
    {
        // don't use auto since we dont want a weird deduction by the compiler

        log->info("Entered {} State", name());

        {
            TimePoint start = Timer::now();
            systems.fileSys.init(systems.config);
            Duration duration = Timer::now() - start;
            log->info("FileSys profiled @ {} milliseconds", duration.count());
        }

        {
            TimePoint start = Timer::now();
            systems.namingKeyMap.init(systems.fileSys);
            Duration duration = Timer::now() - start;
            log->info("NamingKeyMap profiled @ {} milliseconds", duration.count());
        }
    }

    void ProfileLoadingState::leave() {}

    void ProfileLoadingState::update(double deltaTime) {}
} // namespace ehb
