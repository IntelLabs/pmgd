#pragma once

#include "iterator.h"

namespace Jarvis {
    template <typename B, typename F>
    class IteratorFilter : public B::Impl_type {
    protected:
        B *base_iter;
        F func;

    private:
        bool done;

        virtual void _next() {
            if (done) base_iter->done();
            while (base_iter) {
                switch (func(*base_iter)) {
                    case dont_pass: base_iter->next(); continue;
                    case pass: break;
                    case stop: base_iter->done(); break;
                    case pass_stop: done = true; break;
                    default: throw e_not_implemented;
                }
                break;
            }
        }

    public:
        IteratorFilter(B *i, F f) : base_iter(i), func(f) { _next(); }
        operator bool() const { return bool(*base_iter); }
        typename B::Ref_type &operator*() const { return (*base_iter).operator*(); }
        typename B::Ref_type *operator->() const { return (*base_iter).operator->(); }
        void next() { if (base_iter) { base_iter->next(); _next(); } }
    };

    template <typename Iter> class PropertyFilter {
        const PropertyPredicate &pp;
    public:
        PropertyFilter(const PropertyPredicate &p) : pp(p) { }
        Disposition operator()(const Iter &);
    };

    template <typename F>
    class NodeIteratorFilter : public IteratorFilter<NodeIterator, F> {
    public:
        NodeIteratorFilter(NodeIterator *i, F f)
            : IteratorFilter<NodeIterator, F>(i, f) { }
    };

    template <typename F>
    class NodeIteratorPropertyFilter : public IteratorFilter<NodeIterator, F> {
    public:
        NodeIteratorPropertyFilter(NodeIterator *i, const PropertyPredicate &pp)
            : IteratorFilter<NodeIterator, F>(i, PropertyFilter<NodeIterator>(pp)) { }
    };

    template <typename F>
    class EdgeIteratorFilter : public IteratorFilter<EdgeIterator, F> {
    public:
        EdgeIteratorFilter(EdgeIterator *i, F f)
            : IteratorFilter<EdgeIterator, F>(i, f) { }
    };

    template <typename F>
    class EdgeIteratorPropertyFilter : public IteratorFilter<EdgeIterator, F> {
    public:
        EdgeIteratorPropertyFilter(EdgeIterator *i, const PropertyPredicate &pp)
            : IteratorFilter<EdgeIterator, F>(i, PropertyFilter<EdgeIterator>(pp)) { }
    };

    template <typename F>
    class PropertyIteratorFilter : public IteratorFilter<PropertyRef, F> {
    public:
        PropertyIteratorFilter(PropertyIterator *i, F f)
            : IteratorFilter<PropertyRef, F>(i, f) { }
    };

    template <typename F>
    class PathIteratorFilter : public IteratorFilter<PathIterator, F> {
        using IteratorFilter<PathIterator, F>::base_iter;

    public:
        PathIteratorFilter(PathIterator *i, F f)
            : IteratorFilter<PathIterator, F>(i, f) { }

        NodeIterator end_nodes() const { return base_iter->end_nodes(); }

        Node &start_node() const { return base_iter->start_node(); }
        Node &end_node() const { return base_iter->end_node(); }
        int length() const { return base_iter->length(); }
        EdgeIterator get_edges() const { return base_iter->get_edges(); }

        Node &start_node_() const { throw e_internal_error; }
        Node &end_node_() const { throw e_internal_error; }
        int length_() const { throw e_internal_error; }
        EdgeIterator get_edges_() const { throw e_internal_error; }
    };

    inline NodeIterator NodeIterator::filter(const PropertyPredicate &pp)
        { return NodeIterator(new NodeIteratorPropertyFilter<
                     PropertyFilter<NodeIterator> >(this, pp)); }

    template <typename F> inline NodeIterator NodeIterator::filter(F f)
        { return NodeIterator(new NodeIteratorFilter<F>(this, f)); }

    inline EdgeIterator EdgeIterator::filter(const PropertyPredicate &pp)
        { return EdgeIterator(new EdgeIteratorPropertyFilter<
                     PropertyFilter<EdgeIterator> >(this, pp)); }

    template <typename F> inline EdgeIterator EdgeIterator::filter(F f)
        { return EdgeIterator(new EdgeIteratorFilter<F>(this, f)); }

    template <typename F> inline PropertyIterator PropertyIterator::filter(F f)
        { return PropertyIterator(new PropertyIteratorFilter<F>(this, f)); }

    template <typename F> inline PathIterator PathIterator::filter(F f)
        { return PathIterator(new PathIteratorFilter<F>(this, f)); }
};
