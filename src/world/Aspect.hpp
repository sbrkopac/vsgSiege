
#pragma once

#include <memory>

#include <vsg/core/Inherit.h>
#include <vsg/nodes/Group.h>
#include <vsg/core/Visitor.h>
#include "SiegeVisitor.hpp"

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

        void accept(vsg::Visitor& visitor) override;

    protected:
        virtual ~Aspect() = default;

    private:
        friend class ReaderWriterASP;

        std::shared_ptr<Impl> d;
    };

    inline void Aspect::accept(vsg::Visitor& visitor)
    {
        if (SiegeVisitorBase* svb = dynamic_cast<SiegeVisitorBase*>(&visitor); svb != nullptr)
        {
            svb->apply(*this);
        }
        else
        {
            visitor.apply(*this);
        }
    }
} // namespace ehb
