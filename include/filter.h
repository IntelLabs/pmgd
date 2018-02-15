/**
 * @file   filter.h
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#pragma once

#include <functional>
#include "iterator.h"
#include "node.h"

namespace PMGD {
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
                switch (func(*_base_impl->ref())) {
                    case DontPass: _base_impl->next(); break;
                    case Pass: return true;
                    case Stop: done(); return false;
                    case PassStop: _done = true; return true;
                    default: throw PMGDException(NotImplemented);
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

        typename B::Ref_type *ref()
        {
            if (_base_impl == NULL)
                throw PMGDException(NullIterator);
            return _base_impl->ref();
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
