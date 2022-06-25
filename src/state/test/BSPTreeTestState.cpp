
#include "BSPTreeTestState.hpp"

#include "Systems.hpp"
#include "world/SiegeNode.hpp"

#include <vsg/io/read.h>
#include <vsg/nodes/MatrixTransform.h>

#include <spdlog/spdlog.h>

namespace ehb
{
    // untransformed vertices
    struct UV
    {
        float u, v;
    };

    struct sVertex
    {
        float x, y, z;
        uint32_t color;
        UV uv;

        bool operator==(const sVertex& s)
        {
            return(!memcmp(&s, this, sizeof(sVertex)));
        }

    };
    void generateBSPDrawRender(BSPTree* tree, const BSPNode* node) // the first param is because BSPTree holds a pointer to all the triangles
    {
        assert(node != nullptr);

        static auto log = spdlog::get("log");

        if (node->m_IsLeaf)
        {
            log->info("Adding Leaf to render");

            if (node->m_Triangles)
            {
                for (int32_t i = 0; i < node->m_NumTriangles; ++i)
                {
                    TriNorm& tri = tree->m_Triangles[node->m_Triangles[i]];

                    sVertex nodetris[3];
                    memset(nodetris, 0, sizeof(sVertex) * 3);

                    nodetris[0].color = 0xFFA0A0A0;
                    nodetris[1].color = 0xFFA0A0A0;
                    nodetris[2].color = 0xFFA0A0A0;

                    memcpy(&nodetris[0], &tri.m_Vertices[0], sizeof(vsg::vec3));
                    memcpy(&nodetris[1], &tri.m_Vertices[1], sizeof(vsg::vec3));
                    memcpy(&nodetris[2], &tri.m_Vertices[2], sizeof(vsg::vec3));

                    log->info("leaf added");
                }
            }
        }
        else
        {
            generateBSPDrawRender(tree, node->m_LeftChild);
            generateBSPDrawRender(tree, node->m_RightChild);
        }
    }
}

namespace ehb
{
    void BSPTreeTestState::enter()
    {
        log->info("Entered Test State");

        vsg::StateGroup& scene3d = *systems.scene3d;
        auto options = systems.options;

        static std::string siegeNode("t_grs01_houses_generic-a-log");

        TimePoint start = Timer::now();

        if (auto sno = vsg::read_cast<SiegeNodeMesh>(siegeNode, options); sno != nullptr)
        {
            vsg::ref_ptr<vsg::BindGraphicsPipeline> pipeline(options->getObject<vsg::BindGraphicsPipeline>("SiegeNodeGraphicsPipeline"));

            scene3d.addChild(pipeline);

            // transforms are required to connect two nodes together using doors
            auto t1 = vsg::MatrixTransform::create();

            t1->addChild(sno);

            scene3d.addChild(t1);

            generateBSPDrawRender(sno->tree(), sno->tree()->GetRoot());

            // workaround
            compile(systems, systems.scene3d);
        }
        else
        {
            log->critical("Unable to load SiegeNode: {}", siegeNode);
        }

        Duration duration = Timer::now() - start;
        log->info("SiegeNodeTest entire state profiled @ {} milliseconds", duration.count());
    }

    void BSPTreeTestState::leave() {}

    void BSPTreeTestState::update(double deltaTime) {}
} // namespace ehb
