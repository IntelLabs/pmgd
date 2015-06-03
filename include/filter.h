#pragma once

#include <functional>
#include "iterator.h"
#include "node.h"

namespace Jarvis {
    template <typename B>
    class IteratorFilter : public B::Impl_type {
    protected:
        typename B::Impl_type *_base_impl;
        std::function<Disposition(const typename B::Ref_type &)> func;

    private:
        bool _done;

        void done() { delete _base_impl; _base_impl = NULL; }

        virtual bool _next() {
            while (bool(*_base_impl)) {
                switch (func(_base_impl->operator*())) {
                    case dont_pass: _base_impl->next(); break;
                    case pass: return true;
                    case stop: done(); return false;
                    case pass_stop: _done = true; return true;
                    default: throw Exception(NotImplemented);
                }
            }
            return false;
        }

    public:
        IteratorFilter(typename B::Impl_type *i,
                std::function<Disposition(const typename B::Ref_type &)> f)
            : _base_impl(i), func(f), _done(false)
            { _next(); }

        ~IteratorFilter() { delete _base_impl; }

        operator bool() const { return _base_impl && bool(*_base_impl); }

        const typename B::Ref_type &operator*() const
        {
            if (_base_impl == NULL)
                throw Exception(NullIterator);
            return (*_base_impl).operator*();
        }

        const typename B::Ref_type *operator->() const
        {
            if (_base_impl == NULL)
                throw Exception(NullIterator);
            return (*_base_impl).operator->();
        }

        typename B::Ref_type &operator*()
        {
            if (_base_impl == NULL)
                throw Exception(NullIterator);
            return (*_base_impl).operator*();
        }

        typename B::Ref_type *operator->()
        {
            if (_base_impl == NULL)
                throw Exception(NullIterator);
            return (*_base_impl).operator->();
        }

        bool next()
        {
            if (_done) {
                done();
                return false;
            }
            else {
                _base_impl->next();
                return _next();
            }
        }
    };

    template <typename Ref_type> class PropertyFilter {
        const PropertyPredicate _pp;
    public:
        PropertyFilter(const PropertyPredicate &pp) : _pp(pp) { }
        Disposition operator()(const Ref_type &);
    };

    class NodeIteratorFilter : public IteratorFilter<NodeIterator> {
    public:
        NodeIteratorFilter(NodeIterator::Impl_type *i,
                std::function<Disposition(const NodeRef &)> f)
            : IteratorFilter<NodeIterator>(i, f)
        { }
    };

    class NodeIteratorPropertyFilter : public IteratorFilter<NodeIterator> {
    public:
        NodeIteratorPropertyFilter(NodeIterator::Impl_type *i,
                                   const PropertyPredicate &pp)
            : IteratorFilter<NodeIterator>(i,
                    PropertyFilter<NodeRef>(pp))
        { }
    };

    class EdgeIteratorFilter : public IteratorFilter<EdgeIterator> {
    public:
        EdgeIteratorFilter(EdgeIterator::Impl_type *i,
                std::function<Disposition(const EdgeRef &)> f)
            : IteratorFilter<EdgeIterator>(i, f)
        { }
    };

    class EdgeIteratorPropertyFilter : public IteratorFilter<EdgeIterator> {
    public:
        EdgeIteratorPropertyFilter(EdgeIterator::Impl_type *i,
                                   const PropertyPredicate &pp)
            : IteratorFilter<EdgeIterator>(i,
                    PropertyFilter<EdgeRef>(pp))
        { }
    };

    class PropertyIteratorFilter : public IteratorFilter<PropertyIterator> {
    public:
        PropertyIteratorFilter(PropertyIterator::Impl_type *i,
                std::function<Disposition(const PropertyRef &)> f)
            : IteratorFilter<PropertyIterator>(i, f)
        { }
    };

    class PathIteratorFilter : public IteratorFilter<PathIterator> {
        using IteratorFilter<PathIterator>::_base_impl;

    public:
        PathIteratorFilter(PathIterator::Impl_type *i,
                std::function<Disposition(const PathRef &)> f)
            : IteratorFilter<PathIterator>(i, f)
        { }

        // Removed the body of this function to allow compilation on Windows.
        // To add the code back, the implementation had the following 1 line:
        // return _base_impl ? _base_impl->end_nodes() : NodeIterator(NULL);
        NodeIterator end_nodes() const;
    };

    inline NodeIterator NodeIterator::filter(const PropertyPredicate &pp)
    {
        Impl_type *impl = _impl;
        _impl = NULL;
        return NodeIterator(impl ? new NodeIteratorPropertyFilter(impl, pp) : NULL);
    }

    inline NodeIterator NodeIterator::filter(std::function<Disposition(const Ref_type &)> f)
    {
        Impl_type *impl = _impl;
        _impl = NULL;
        return NodeIterator(impl ? new NodeIteratorFilter(impl, f) : NULL);
    }

    inline EdgeIterator EdgeIterator::filter(const PropertyPredicate &pp)
    {
        Impl_type *impl = _impl;
        _impl = NULL;
        return EdgeIterator(impl ? new EdgeIteratorPropertyFilter(impl, pp) : NULL);
    }

    inline EdgeIterator EdgeIterator::filter(std::function<Disposition(const Ref_type &)> f)
    {
        Impl_type *impl = _impl;
        _impl = NULL;
        return EdgeIterator(impl ? new EdgeIteratorFilter(impl, f) : NULL);
    }

    inline PropertyIterator PropertyIterator::filter(std::function<Disposition(const Ref_type &)> f)
    {
        Impl_type *impl = _impl;
        _impl = NULL;
        return PropertyIterator(impl ? new PropertyIteratorFilter(impl, f) : NULL);
    }

    inline PathIterator PathIterator::filter(std::function<Disposition(const Ref_type &)> f)
    {
        Impl_type *impl = _impl;
        _impl = NULL;
        return PathIterator(impl ? new PathIteratorFilter(impl, f) : NULL);
    }


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
                default: assert(0);
            }
        }
        return r ? pass : dont_pass;
    }

    template Disposition PropertyFilter<NodeRef>::operator()(const NodeRef &);
    template Disposition PropertyFilter<EdgeRef>::operator()(const EdgeRef &);
};
