
#include "SiegeNode.hpp"

#include <spdlog/spdlog.h>

#include <vsg/maths/quat.h>
#include <vsg/maths/transform.h> // required for type deduction of inverse

namespace ehb
{
    void SiegeNode::connect(const vsg::MatrixTransform* targetNode, uint32_t targetDoor, vsg::MatrixTransform* connectNode, uint32_t connectDoor)
    {
        auto log = spdlog::get("log");

        const auto targetMesh = targetNode->children[0].cast<SiegeNode>();
        const auto connectMesh = connectNode->children[0].cast<SiegeNode>();

        if (!targetMesh)
        {
            log->error("SiegeNode::connect - targetNode has no SiegeNode parent");
            return;
        }
        if (!connectMesh)
        {
            log->error("SiegeNode::connect - connectNode has no SiegeNode parent");
            return;
        }

        auto dTargetDoor = targetMesh->mesh()->doorById(targetDoor);
        auto& m1 = dTargetDoor->transform();

        auto dConnectDoor = connectMesh->mesh()->doorById(connectDoor);
        auto& m2 = dConnectDoor->transform();

        // log->info("Supporting information found for SiegeNode::Connect - attempting connection of doors {} to {}", connectDoor, targetDoor);

        /*
         * lets start at the location of the destination door
         * we want to use an inverse here to account for the fact that
         * we're currently placing the center of connectNode at the location
         * of its door and will be connecting this to door 1
         */
        vsg::dmat4 xform = vsg::inverse(m2);

        static auto oneEightyRotate = vsg::dquat(0.00000000000000000, 1.00000000000000000, 0.0, 6.1232339957367660e-17);

        // account for flipping from door 1 to door 2
        xform = vsg::rotate(oneEightyRotate) * xform;

        // now transform by the first door...
        xform = (m1)*xform;

        // and adjust for the node we're connecting to
        xform = targetNode->matrix * xform;

        // "Hold on to your butts." - Ray Arnold
        connectNode->matrix = xform;
    }

    void SiegeNode::connect(const vsg::MatrixTransform* targetRegion, vsg::MatrixTransform* targetNode, uint32_t targetDoor, vsg::MatrixTransform* connectRegion, const vsg::MatrixTransform* connectNode, uint32_t connectDoor)
    {
        auto log = spdlog::get("log");

        const auto targetMesh = targetNode->children[0].cast<SiegeNode>();
        const auto connectMesh = connectNode->children[0].cast<SiegeNode>();

        if (!targetMesh)
        {
            log->error("SiegeNode::connect - targetNode has no SiegeNode parent");
            return;
        }
        if (!connectMesh)
        {
            log->error("SiegeNode::connect - connectNode has no SiegeNode parent");
            return;
        }

        auto dTargetDoor = targetMesh->mesh()->doorById(targetDoor);
        auto& m1 = dTargetDoor->transform();

        auto dConnectDoor = connectMesh->mesh()->doorById(connectDoor);
        auto& m2 = dConnectDoor->transform();

        // log->info("Supporting information found for SiegeNode::Connect - attempting connection of doors {} to {}", connectDoor, targetDoor);

        auto t1 = targetRegion->matrix * targetNode->matrix * (m1);
        auto t2 = connectNode->matrix * (m2);

        /*
         * lets start at the location of the destination door
         * we want to use an inverse here to account for the fact that
         * we're currently placing the center of connectNode at the location
         * of its door and will be connecting this to door 1
         */
        vsg::dmat4 xform = vsg::inverse(t2);

        static auto oneEightyRotate = vsg::dquat(0.00000000000000000, 1.00000000000000000, 0.0, 6.1232339957367660e-17);

        // account for flipping from door 1 to door 2
        xform = vsg::rotate(oneEightyRotate) * xform;

        // now transform by the first door...
        xform = (t1)*xform;

        // "Hold on to your butts." - Ray Arnold
        connectRegion->matrix = xform;
    }

} // namespace ehb