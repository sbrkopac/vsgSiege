
#include "Aspect.hpp"
#include "AspectImpl.hpp"

#include <spdlog/spdlog.h>

namespace ehb
{
    Aspect::Aspect(std::shared_ptr<Impl> impl) : d(std::move(impl))
    {
        auto log = spdlog::get("log");

        log->debug("asp has {} sub meshes", d->subMeshes.size());

        for (const auto& mesh : d->subMeshes)
        {
            log->debug("asp subMesh has {} textures", mesh.textureCount);

            uint32_t f = 0; // track which face the loader is loading across the sub mesh
            for (uint32_t i = 0; i < mesh.textureCount; ++i)
            {
            }
        }
    }
}