#pragma once

#include <stddef.h>
#include <functional>
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
        virtual Ref_type &operator*() = 0;
        virtual Ref_type *operator->() = 0;
        virtual bool next() = 0;
    };

    template <typename Impl> class Iterator {
    protected:
        Impl *_impl;
        void done() { delete _impl; _impl = NULL; }

    public:
        typedef Impl Impl_type;
        typedef typename Impl::Ref_type Ref_type;

        explicit Iterator(Impl *i)
            : _impl(i)
        {
            if (_impl && !bool(*_impl))
                done();
        }

        ~Iterator() { delete _impl; }

        operator bool() const { return _impl != NULL; }
        const Ref_type &operator*() const
            { if (!_impl) throw Exception(null_iterator); return (*_impl).operator*(); }
        const Ref_type *operator->() const
            { if (!_impl) throw Exception(null_iterator); return (*_impl).operator->(); }
        Ref_type &operator*()
            { if (!_impl) throw Exception(null_iterator); return (*_impl).operator*(); }
        Ref_type *operator->()
            { if (!_impl) throw Exception(null_iterator); return (*_impl).operator->(); }
        void next() { if (_impl) if (!_impl->next()) done(); }

        void process(std::function<void(Ref_type &)> f)
        {
            while (operator bool()) {
                f(operator*());
                next();
            }
        }
    };
};

namespace Jarvis {
    class Node;
    typedef Node NodeRef;
    typedef IteratorImpl<NodeRef> NodeIteratorImpl;

    class NodeIterator : public Iterator<NodeIteratorImpl> {
    public:
        explicit NodeIterator(NodeIteratorImpl *i)
            : Iterator<NodeIteratorImpl>(i) { }

        NodeIterator filter(const PropertyPredicate &pp);
        NodeIterator filter(std::function<Disposition(const Ref_type &)> f);
    };
};

namespace Jarvis {
    class Allocator;
    class PropertyList;

    class PropertyRef {
        uint8_t *_chunk;
        unsigned _offset;

        friend class PropertyList;
        friend class PropertyListIterator;

        struct BlobRef;

        enum { p_unused, p_end, p_link,
               p_novalue, p_boolean_false, p_boolean_true,
               p_integer, p_float, p_time, p_string, p_string_ptr, p_blob };

        unsigned chunk_size() const { return _chunk[0]; }

        StringID &get_id() { return *(StringID *)(&_chunk[_offset]); }

        operator bool() const { return _offset != 0; }

        uint8_t &type_size() { return _chunk[_offset+2]; }
        const uint8_t &type_size() const
            { return const_cast<PropertyRef *>(this)->type_size(); }

        int ptype() const { return type_size() & 0xf; }
        int size() const { return type_size() >> 4; }

        Property get_value() const;

        uint8_t *val() const { return &_chunk[_offset + 3]; }

        bool check_space(unsigned size) const
            { return _offset + size + 3 <= chunk_size(); }

        bool next();
        bool not_done() const
            { return _offset <= chunk_size() - 3 && ptype() != p_end; }

        void set_id(StringID id) { this->get_id() = id; }

        void set_type(int type)
            { type_size() = uint8_t((type_size() & 0xf0) | type); }

        void set_size(int new_size);
        void set_value(const Property &, Allocator &);
        void set_link(PropertyList *p_chunk);
        void set_blob(const void *value, std::size_t size,
                      Allocator &allocator);
        void set_end() { set_id(0); type_size() = p_end; }
        void free() { type_size() &= 0xf0; /* keep size and clear type */ }

        void free(int type, int size)
        {
            if (type == p_end)
                type_size() = p_end;
            else
                type_size() = uint8_t(size << 4) | p_unused;
        }

        void follow_link() { *this = *(PropertyList **)val(); }

        void make_space(PropertyRef &);
        int get_space() const;

        PropertyRef() : _chunk(0), _offset(0) { }
        PropertyRef(const PropertyList *l) : _chunk((uint8_t *)l), _offset(1) {}
        PropertyRef(const PropertyRef &p, unsigned size)
            : _chunk(p._chunk), _offset(p._offset + size + 3)
            { assert(_offset <= chunk_size()); }

    public:
        StringID id() const { return const_cast<PropertyRef *>(this)->get_id(); }
        operator Property() const { return get_value(); }

        PropertyType type() const
        {
            assert(ptype() >= p_novalue && ptype() <= p_blob);
            static constexpr PropertyType type_map[] = {
                t_novalue, t_boolean, t_boolean, t_integer, t_float,
                t_time, t_string, t_string, t_blob
            };
            return type_map[ptype() - p_novalue];
        }

        bool bool_value() const;
        long long int_value() const;
        std::string string_value() const;
        double float_value() const;
        Time time_value() const;
        Property::blob_t blob_value() const;
    };

    typedef IteratorImpl<PropertyRef> PropertyIteratorImpl;
    class PropertyIterator : public Iterator<PropertyIteratorImpl> {
    public:
        explicit PropertyIterator(PropertyIteratorImpl *i)
            : Iterator<PropertyIteratorImpl>(i) { }

        PropertyIterator filter(std::function<Disposition(const Ref_type &)> f);
    };

    class PropertyList {
        static constexpr unsigned chunk_size = 64;

        uint8_t _chunk0[];

        class PropertySpace;
        bool find_property(StringID property, PropertyRef &p,
                           PropertySpace *space = 0) const;
        void find_space(PropertySpace &space, Allocator &) const;
        static PropertySpace get_space(const Property &);

    public:
        void init(std::size_t size);
        bool check_property(StringID property, Property &result) const;
        Property get_property(StringID id) const;
        PropertyIterator get_properties() const;
        void set_property(StringID id, const Property &);
        void remove_property(StringID name);
    };
};

namespace Jarvis {
    class Edge;
    class EdgeRef;

    class EdgeIteratorImpl : public IteratorImpl<EdgeRef> {
        friend class EdgeRef;
        virtual Edge *get_edge() const = 0;
        virtual StringID get_tag() const = 0;
        virtual Node &get_source() const = 0;
        virtual Node &get_destination() const = 0;
    };

    class EdgeRef {
        EdgeIteratorImpl *_impl;

        Edge *edge() const { return _impl->get_edge(); }

    public:
        EdgeRef(EdgeIteratorImpl *impl) : _impl(impl) {}
        operator Edge &() { return *edge(); }
        StringID get_tag() const { return _impl->get_tag(); }
        Node &get_source() const { return _impl->get_source(); }
        Node &get_destination() const { return _impl->get_destination(); }
        bool check_property(StringID id, Property &result) const;
        Property get_property(StringID id) const;
        PropertyIterator get_properties() const;
        void set_property(StringID id, const Property &property);
        void remove_property(StringID id);
    };

    class EdgeIterator : public Iterator<IteratorImpl<EdgeRef>> {
    public:
        explicit EdgeIterator(IteratorImpl<EdgeRef> *i)
            : Iterator<IteratorImpl<EdgeRef>>(i) { }

        EdgeIterator filter(const PropertyPredicate &pp);
        EdgeIterator filter(std::function<Disposition(const Ref_type &)> f);
    };
};

namespace Jarvis {
    class Path;
    class PathRef;

    class PathIteratorImplBase : public IteratorImpl<PathRef> {
    public:
        virtual NodeIterator end_nodes() const = 0;
    };

    class PathIteratorImpl : public PathIteratorImplBase {
        friend class PathRef;
        virtual Node &start_node() const = 0;
        virtual Node &end_node() const = 0;
        virtual int length() const = 0;
        virtual EdgeIterator get_edges() const = 0;
    };

    class PathRef {
        PathIteratorImpl *_impl;
    public:
        PathRef(PathIteratorImpl *i) : _impl(i) { }
        operator Path() const;
        Node &start_node() const { return _impl->start_node(); }
        Node &end_node() const { return _impl->end_node(); }
        int length() const { return _impl->length(); }
        EdgeIterator get_edges() const { return _impl->get_edges(); }
    };

    class PathIterator : public Iterator<PathIteratorImplBase> {
    public:
        explicit PathIterator(PathIteratorImplBase *i)
            : Iterator<PathIteratorImplBase>(i) { }

        PathIterator filter(std::function<Disposition(const Ref_type &)> f);

        NodeIterator end_nodes() const
            { if (!_impl) return NodeIterator(NULL); return _impl->end_nodes(); }
    };
};
