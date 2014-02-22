#pragma once

#include <functional>
#include "iterator.h"

namespace Jarvis {
    template <typename B>
    class IteratorFilter : public B::Impl_type {
    protected:
        B *base_iter;
        std::function<Disposition(B &)> func;

    private:
        bool _done;

        virtual bool _next() {
            while (bool(*base_iter)) {
                switch (func(*base_iter)) {
                    case dont_pass: base_iter->next(); break;
                    case pass: return true;
                    case stop: base_iter->done(); return false;
                    case pass_stop: _done = true; return true;
                    default: throw e_not_implemented;
                }
            }
            return false;
        }

    public:
        IteratorFilter(B *i, std::function<Disposition(B &)> f)
            : base_iter(i), func(f) { _next(); }
        operator bool() const { return bool(*base_iter); }
        const typename B::Ref_type &operator*() const
            { return (*base_iter).operator*(); }
        const typename B::Ref_type *operator->() const
            { return (*base_iter).operator->(); }
        typename B::Ref_type &operator*() { return (*base_iter).operator*(); }
        typename B::Ref_type *operator->() { return (*base_iter).operator->(); }

        bool next()
        {
            if (_done) {
                base_iter->done();
                return false;
            }
            else {
                base_iter->next();
                return _next();
            }
        }
    };

    template <typename Iter> class PropertyFilter {
        const PropertyPredicate &pp;
    public:
        PropertyFilter(const PropertyPredicate &p) : pp(p) { }
        Disposition operator()(const Iter &);
    };

    class NodeIteratorFilter : public IteratorFilter<NodeIterator> {
    public:
        NodeIteratorFilter(NodeIterator *i, std::function<Disposition(NodeIterator &)> f)
            : IteratorFilter<NodeIterator>(i, f) { }
    };

    class NodeIteratorPropertyFilter : public IteratorFilter<NodeIterator> {
    public:
        NodeIteratorPropertyFilter(NodeIterator *i, const PropertyPredicate &pp)
            : IteratorFilter<NodeIterator>(i, PropertyFilter<NodeIterator>(pp)) { }
    };

    class EdgeIteratorFilter : public IteratorFilter<EdgeIterator> {
    public:
        EdgeIteratorFilter(EdgeIterator *i, std::function<Disposition(EdgeIterator &)> f)
            : IteratorFilter<EdgeIterator>(i, f) { }
    };

    class EdgeIteratorPropertyFilter : public IteratorFilter<EdgeIterator> {
    public:
        EdgeIteratorPropertyFilter(EdgeIterator *i, const PropertyPredicate &pp)
            : IteratorFilter<EdgeIterator>(i, PropertyFilter<EdgeIterator>(pp)) { }
    };

    class PropertyIteratorFilter : public IteratorFilter<PropertyIterator> {
    public:
        PropertyIteratorFilter(PropertyIterator *i, std::function<Disposition(PropertyIterator &)> f)
            : IteratorFilter<PropertyIterator>(i, f) { }
    };

    class PathIteratorFilter : public IteratorFilter<PathIterator> {
        using IteratorFilter<PathIterator>::base_iter;

    public:
        PathIteratorFilter(PathIterator *i, std::function<Disposition(PathIterator &)> f)
            : IteratorFilter<PathIterator>(i, f) { }

        NodeIterator end_nodes() const { return base_iter->end_nodes(); }
    };

    inline NodeIterator NodeIterator::filter(const PropertyPredicate &pp)
        { return NodeIterator(new NodeIteratorPropertyFilter(this, pp)); }

    inline NodeIterator NodeIterator::filter(std::function<Disposition(NodeIterator &)> f)
        { return NodeIterator(new NodeIteratorFilter(this, f)); }

    inline EdgeIterator EdgeIterator::filter(const PropertyPredicate &pp)
        { return EdgeIterator(new EdgeIteratorPropertyFilter(this, pp)); }

    inline EdgeIterator EdgeIterator::filter(std::function<Disposition(EdgeIterator &)> f)
        { return EdgeIterator(new EdgeIteratorFilter(this, f)); }

    inline PropertyIterator PropertyIterator::filter(std::function<Disposition(PropertyIterator &)> f)
        { return PropertyIterator(new PropertyIteratorFilter(this, f)); }

    inline PathIterator PathIterator::filter(std::function<Disposition(PathIterator &)> f)
        { return PathIterator(new PathIteratorFilter(this, f)); }
};
