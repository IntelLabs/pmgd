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
                    case DontPass: _base_impl->next(); break;
                    case Pass: return true;
                    case Stop: done(); return false;
                    case PassStop: _done = true; return true;
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


    template <typename Ref_type>
    Disposition PropertyFilter<Ref_type>::operator()(const Ref_type &p)
    {
        bool r = false;
        Property val;
        if (p.check_property(_pp.id, val)) {
            switch (_pp.op) {
                case PropertyPredicate::DontCare: r = true; break;
                case PropertyPredicate::Eq: r = val == _pp.v1; break;
                case PropertyPredicate::Ne: r = val != _pp.v1; break;
                case PropertyPredicate::Gt: r = val > _pp.v1; break;
                case PropertyPredicate::Ge: r = val >= _pp.v1; break;
                case PropertyPredicate::Lt: r = val < _pp.v1; break;
                case PropertyPredicate::Le: r = val <= _pp.v1; break;
                case PropertyPredicate::GeLe: r = val >= _pp.v1 && val <= _pp.v2; break;
                case PropertyPredicate::GeLt: r = val >= _pp.v1 && val < _pp.v2; break;
                case PropertyPredicate::GtLe: r = val > _pp.v1 && val <= _pp.v2; break;
                case PropertyPredicate::GtLt: r = val > _pp.v1 && val < _pp.v2; break;
                default: assert(0);
            }
        }
        return r ? Pass : DontPass;
    }

    template Disposition PropertyFilter<NodeRef>::operator()(const NodeRef &);
    template Disposition PropertyFilter<EdgeRef>::operator()(const EdgeRef &);
};
