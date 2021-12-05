
#pragma once

#include <memory>

#include <vsg/core/Inherit.h>
#include <vsg/nodes/Group.h>

// this class emulates the class: Aspect in DS
namespace ehb
{
    class Aspect : public vsg::Inherit<vsg::Group, Aspect>
    {
    public:

        struct Impl;

    public:

        // have to do this for now as we need access to the options for texture loading
        explicit Aspect(std::shared_ptr<Impl> impl, vsg::ref_ptr<const vsg::Options> options);

    protected:

        virtual ~Aspect() = default;

    private:

        friend class ReaderWriterASP;

        std::shared_ptr<Impl> d;
    };
}
