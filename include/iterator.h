#pragma once

#include "exception.h"
#include "stringid.h"
#include "property.h"

namespace Jarvis {
    enum Disposition { dont_pass, pass, stop, pass_stop, prune, pass_prune };

    template <typename R> class IteratorImpl {
    public:
        typedef R Ref_type;
        virtual ~IteratorImpl() { }
        virtual operator bool() const = 0;
        virtual const Ref_type &operator*() const = 0;
        virtual const Ref_type *operator->() const = 0;
        virtual void next() = 0;
    };

    template <typename Impl> class Iterator {
    protected:
        Impl *impl;

    public:
        typedef Impl Impl_type;
        typedef typename Impl::Ref_type Ref_type;

        explicit Iterator(Impl *i) : impl(i) { }
        ~Iterator() { delete impl; }
        void done() { delete impl; impl = NULL; }

        operator bool() const { return impl && bool(*impl); }
        const Ref_type &operator*() const
            { if (!impl) throw e_null_iterator; return (*impl).operator*(); }
        const Ref_type *operator->() const
            { if (!impl) throw e_null_iterator; return (*impl).operator->(); }
        void next() { if (impl) impl->next(); }
    };
};

namespace Jarvis {
    class Node;
    typedef IteratorImpl<Node> NodeIteratorImpl;

    class NodeIterator : public Iterator<NodeIteratorImpl> {
    public:
        explicit NodeIterator(NodeIteratorImpl *i)
            : Iterator<NodeIteratorImpl>(i) { }

        NodeIterator filter(const PropertyPredicate &pp);
        template <typename F> NodeIterator filter(F f);
    };
};

namespace Jarvis {
    class Edge;
    typedef IteratorImpl<Edge> EdgeIteratorImpl;

    class EdgeIterator : public Iterator<EdgeIteratorImpl> {
    public:
        explicit EdgeIterator(EdgeIteratorImpl *i)
            : Iterator<EdgeIteratorImpl>(i) { }

        EdgeIterator filter(const PropertyPredicate &pp);
        template <typename F> EdgeIterator filter(F f);
    };
};

namespace Jarvis {
    class PropertyIteratorImpl;

    class PropertyValueRef {
        PropertyIteratorImpl *iter;
    public:
        PropertyValueRef(PropertyIteratorImpl *i) : iter(i) { }
        operator PropertyValue() const;
        bool bool_value() const;
        unsigned long long int_value() const;
        std::string string_value() const;
        double float_value() const;
        Time time_value() const;
        void *blob_value() const;
    };

    class PropertyRef {
        PropertyIteratorImpl *iter;
    public:
        PropertyRef(PropertyIteratorImpl *i) : iter(i) { }
        operator Property() const;
        StringID id() const;
        PropertyValueRef value() const { return PropertyValueRef(iter); }
    };

    class PropertyIteratorImpl : public IteratorImpl<PropertyRef> {
        PropertyRef ref;
    public:
        PropertyIteratorImpl() : ref(this) { }
        const PropertyRef &operator*() const { return ref; }
        const PropertyRef *operator->() const { return &ref; }
        virtual StringID id_() const = 0;
        virtual bool bool_value_() const = 0;
        virtual unsigned long long int_value_() const = 0;
        virtual std::string string_value_() const = 0;
        virtual double float_value_() const = 0;
        virtual Time time_value_() const = 0;
        virtual void *blob_value_() const = 0;
    };

    class PropertyIterator : public Iterator<PropertyIteratorImpl> {
    public:
        explicit PropertyIterator(PropertyIteratorImpl *i)
            : Iterator<PropertyIteratorImpl>(i) { }

        template <typename F> PropertyIterator filter(F f);
    };
};

namespace Jarvis {
    class Path;
    class PathIteratorImpl;

    class PathRef {
        PathIteratorImpl *iter;
    public:
        PathRef(PathIteratorImpl *i) : iter(i) { }
        operator Path() const;
        Node &start_node() const;
        Node &end_node() const;
        int length() const;
        EdgeIterator get_edges() const;
    };

    class PathIteratorImpl : public IteratorImpl<PathRef> {
        PathRef ref;
    public:
        PathIteratorImpl() : ref(this) { }
        const PathRef &operator*() const { return ref; }
        const PathRef *operator->() const { return &ref; }
        virtual NodeIterator end_nodes() const = 0;

        virtual Node &start_node_() const = 0;
        virtual Node &end_node_() const = 0;
        virtual int length_() const = 0;
        virtual EdgeIterator get_edges_() const = 0;
    };

    class PathIterator : public Iterator<PathIteratorImpl> {
    public:
        explicit PathIterator(PathIteratorImpl *i)
            : Iterator<PathIteratorImpl>(i) { }

        template <typename F> PathIterator filter(F f);

        NodeIterator end_nodes() const
            { if (!impl) return NodeIterator(NULL); return impl->end_nodes(); }
    };
};

namespace Jarvis {
    inline StringID PropertyRef::id() const { return iter->id_(); }
    inline bool PropertyValueRef::bool_value() const { return iter->bool_value_(); }
    inline unsigned long long PropertyValueRef::int_value() const { return iter->int_value_(); }
    inline std::string PropertyValueRef::string_value() const { return iter->string_value_(); }
    inline double PropertyValueRef::float_value() const { return iter->float_value_(); }
    inline Time PropertyValueRef::time_value() const { return iter->time_value_(); }
    inline void *PropertyValueRef::blob_value() const { return iter->blob_value_(); }
};

namespace Jarvis {
    inline Node &PathRef::start_node() const { return iter->start_node_(); }
    inline Node &PathRef::end_node() const { return iter->end_node_(); }
    inline int PathRef::length() const { return iter->length_(); }
    inline EdgeIterator PathRef::get_edges() const { return iter->get_edges_(); }
};
