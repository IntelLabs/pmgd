#include "Index.h"
#include "AvlTreeIndex.h"
#include "List.h"
#include "exception.h"

using namespace Jarvis;

typedef AvlTreeIndex<long long,List<Node *>> LongValueIndex;
typedef AvlTreeIndex<double,List<Node *>> FloatValueIndex;
typedef AvlTreeIndex<bool,List<Node *>> BoolValueIndex;

void Index::init(PropertyType ptype)
{
    switch(ptype) {
        case t_integer:
            {
                LongValueIndex *idx = (LongValueIndex *)this;
                idx->init();
            }
            break;
        case t_float:
            {
                FloatValueIndex *idx = (FloatValueIndex *)this;
                idx->init();
            }
            break;
        case t_boolean:
            {
                BoolValueIndex *idx = (BoolValueIndex *)this;
                idx->init();
            }
            break;
        case t_time:
        case t_string:
        case t_novalue:
            throw Exception(not_implemented);
        case t_blob:
        default:
            throw Exception(property_type);
    }
    _ptype = ptype;
}

void Index::add(const Property &p, Node *n, Allocator &allocator)
{
    if (_ptype != p.type())
        throw Exception(property_type);
    switch(_ptype) {
        case t_integer:
            {
                LongValueIndex *prop_idx = (LongValueIndex *)this;
                List<Node*> *dest = prop_idx->add(p.int_value(), allocator) ;                
                if (dest->num_elems() == 0)
                    dest->init();
                dest->add(n, allocator);
            }
            break;
        case t_float:
            {
                FloatValueIndex *prop_idx = (FloatValueIndex *)this;
                List<Node*> *dest = prop_idx->add(p.float_value(), allocator) ;                
                if (dest->num_elems() == 0)
                    dest->init();
                dest->add(n, allocator);
            }
            break;
        case t_boolean:
            {
                BoolValueIndex *prop_idx = (BoolValueIndex *)this;
                // Now retrieve the list where node pointers are getting added
                // This particular property only has one value = true and should
                // be the first and only node in the tree.
                // TODO: Tree is unnecessary and Node* needs better arrangement for
                // quick search and remove operations
                List<Node*> *dest = prop_idx->add(p.bool_value(), allocator) ;                
                if (dest->num_elems() == 0)
                    dest->init();
                dest->add(n, allocator);
            }
            break;
        case t_time:
        case t_string:
        case t_novalue:
            throw Exception(not_implemented);
        case t_blob:
        default:
            throw Exception(property_type);
    }
}

namespace Jarvis {
    class NodeIndexIteratorImpl : public NodeIteratorImpl {
        const List<Node *>::ListType *_pos;
    public:
        NodeIndexIteratorImpl(List<Node *> *l)
            : _pos(l->begin())
        { }
        Node &operator*() const { return *_pos->value; }
        Node *operator->() const { return _pos->value; }
        Node &operator*() { return *_pos->value; }
        Node *operator->() { return _pos->value; }
        operator bool() const { return _pos != NULL; }
        bool next()
        {
            _pos = _pos->next; 
            return _pos != NULL;
        }
    };
}

NodeIterator Index::get_nodes(const PropertyPredicate &pp)
{
    // TODO: Only handling eq case right now and the check has
    // already been done when this is called. Use value v1 from pp.
    const Property &p = pp.v1;
    List<Node *> *list = NULL;

    if (_ptype != p.type())
        throw Exception(property_type);
    switch(_ptype) {
        case t_integer:
            {
                LongValueIndex *prop_idx = (LongValueIndex *)this;
                list = prop_idx->find(p.int_value()) ;                
            }
            break;
        case t_float:
            {
                FloatValueIndex *prop_idx = (FloatValueIndex *)this;
                list = prop_idx->find(p.float_value()) ;                
            }
            break;
        case t_boolean:
            {
                BoolValueIndex *idx = (BoolValueIndex *)this;
                list = idx->find(p.bool_value());
            }
            break;
        case t_time:
        case t_string:
        case t_novalue:
            throw Exception(not_implemented);
        case t_blob:
        default:
            throw Exception(property_type);
    }
    if (list == NULL)
        return NodeIterator(NULL);
    else
        return NodeIterator(new NodeIndexIteratorImpl(list));
}

void Index::remove(const Property &p, Node *n, Allocator &allocator)
{
    return;
}
