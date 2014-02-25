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
                    default: throw Exception(not_implemented);
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


    template <typename Iter>
    Disposition PropertyFilter<Iter>::operator()(const Iter &i)
    {
        bool r = false;
        Property p;
        if (i->check_property(pp.id, p)) {
            const PropertyValue &val = p.value();
            switch (pp.op) {
                case PropertyPredicate::dont_care: r = true; break;
                case PropertyPredicate::eq: r = val == pp.v1; break;
                case PropertyPredicate::ne: r = val != pp.v1; break;
                case PropertyPredicate::gt: r = val > pp.v1; break;
                case PropertyPredicate::ge: r = val >= pp.v1; break;
                case PropertyPredicate::lt: r = val < pp.v1; break;
                case PropertyPredicate::le: r = val <= pp.v1; break;
                case PropertyPredicate::gele: r = val >= pp.v1 && val <= pp.v2; break;
                case PropertyPredicate::gelt: r = val >= pp.v1 && val < pp.v2; break;
                case PropertyPredicate::gtle: r = val > pp.v1 && val <= pp.v2; break;
                case PropertyPredicate::gtlt: r = val > pp.v1 && val < pp.v2; break;
                default: throw Exception(internal_error);
            }
        }
        return r ? pass : dont_pass;
    }

    template Disposition PropertyFilter<NodeIterator>::operator()(const NodeIterator &);
    template Disposition PropertyFilter<EdgeIterator>::operator()(const EdgeIterator &);
};
