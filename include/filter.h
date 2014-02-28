#pragma once

#include <functional>
#include "iterator.h"
#include "node.h"

namespace Jarvis {
    template <typename B>
    class IteratorFilter : public B::Impl_type {
    protected:
        B *base_iter;
        std::function<Disposition(const typename B::Ref_type &)> func;

    private:
        bool _done;

        virtual bool _next() {
            while (bool(*base_iter)) {
                switch (func(base_iter->operator*())) {
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
        IteratorFilter(B *i,
                std::function<Disposition(const typename B::Ref_type &)> f)
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

    template <typename Ref_type> class PropertyFilter {
        const PropertyPredicate &_pp;
    public:
        PropertyFilter(const PropertyPredicate &pp) : _pp(pp) { }
        Disposition operator()(const Ref_type &);
    };

    class NodeIteratorFilter : public IteratorFilter<NodeIterator> {
    public:
        NodeIteratorFilter(NodeIterator *i,
                std::function<Disposition(const NodeRef &)> f)
            : IteratorFilter<NodeIterator>(i, f)
        { }
    };

    class NodeIteratorPropertyFilter : public IteratorFilter<NodeIterator> {
    public:
        NodeIteratorPropertyFilter(NodeIterator *i,
                                   const PropertyPredicate &pp)
            : IteratorFilter<NodeIterator>(i,
                    PropertyFilter<NodeRef>(pp))
        { }
    };

    class EdgeIteratorFilter : public IteratorFilter<EdgeIterator> {
    public:
        EdgeIteratorFilter(EdgeIterator *i,
                std::function<Disposition(const EdgeRef &)> f)
            : IteratorFilter<EdgeIterator>(i, f)
        { }
    };

    class EdgeIteratorPropertyFilter : public IteratorFilter<EdgeIterator> {
    public:
        EdgeIteratorPropertyFilter(EdgeIterator *i,
                                   const PropertyPredicate &pp)
            : IteratorFilter<EdgeIterator>(i,
                    PropertyFilter<EdgeRef>(pp))
        { }
    };

    class PropertyIteratorFilter : public IteratorFilter<PropertyIterator> {
    public:
        PropertyIteratorFilter(PropertyIterator *i,
                std::function<Disposition(const PropertyRef &)> f)
            : IteratorFilter<PropertyIterator>(i, f)
        { }
    };

    class PathIteratorFilter : public IteratorFilter<PathIterator> {
        using IteratorFilter<PathIterator>::base_iter;

    public:
        PathIteratorFilter(PathIterator *i,
                std::function<Disposition(const PathRef &)> f)
            : IteratorFilter<PathIterator>(i, f)
        { }

        NodeIterator end_nodes() const { return base_iter->end_nodes(); }
    };

    inline NodeIterator NodeIterator::filter(const PropertyPredicate &pp)
        { return NodeIterator(new NodeIteratorPropertyFilter(this, pp)); }

    inline NodeIterator NodeIterator::filter(std::function<Disposition(const Ref_type &)> f)
        { return NodeIterator(new NodeIteratorFilter(this, f)); }

    inline EdgeIterator EdgeIterator::filter(const PropertyPredicate &pp)
        { return EdgeIterator(new EdgeIteratorPropertyFilter(this, pp)); }

    inline EdgeIterator EdgeIterator::filter(std::function<Disposition(const Ref_type &)> f)
        { return EdgeIterator(new EdgeIteratorFilter(this, f)); }

    inline PropertyIterator PropertyIterator::filter(std::function<Disposition(const Ref_type &)> f)
        { return PropertyIterator(new PropertyIteratorFilter(this, f)); }

    inline PathIterator PathIterator::filter(std::function<Disposition(const Ref_type &)> f)
        { return PathIterator(new PathIteratorFilter(this, f)); }


    template <typename Ref_type>
    Disposition PropertyFilter<Ref_type>::operator()(const Ref_type &p)
    {
        bool r = false;
        Property val;
        if (p.check_property(_pp.id, val)) {
            switch (_pp.op) {
                case PropertyPredicate::dont_care: r = true; break;
                case PropertyPredicate::eq: r = val == _pp.v1; break;
                case PropertyPredicate::ne: r = val != _pp.v1; break;
                case PropertyPredicate::gt: r = val > _pp.v1; break;
                case PropertyPredicate::ge: r = val >= _pp.v1; break;
                case PropertyPredicate::lt: r = val < _pp.v1; break;
                case PropertyPredicate::le: r = val <= _pp.v1; break;
                case PropertyPredicate::gele: r = val >= _pp.v1 && val <= _pp.v2; break;
                case PropertyPredicate::gelt: r = val >= _pp.v1 && val < _pp.v2; break;
                case PropertyPredicate::gtle: r = val > _pp.v1 && val <= _pp.v2; break;
                case PropertyPredicate::gtlt: r = val > _pp.v1 && val < _pp.v2; break;
                default: throw Exception(internal_error);
            }
        }
        return r ? pass : dont_pass;
    }

    template Disposition PropertyFilter<NodeRef>::operator()(const NodeRef &);
    template Disposition PropertyFilter<EdgeRef>::operator()(const EdgeRef &);
};
