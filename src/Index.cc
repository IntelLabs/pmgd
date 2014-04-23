#include "Index.h"
#include "AvlTree.h"
#include "List.h"

using namespace Jarvis;

typedef AvlTree<long long,List<const Node*>> LongValueIndex;
typedef AvlTree<bool,List<const Node*>> BoolValueIndex;

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

void Index::add(const Property &p, const Node *n, Allocator &allocator)
{
    switch(_ptype) {
        case t_integer:
            {
                LongValueIndex *prop_idx = (LongValueIndex *)this;
                List<const Node*> *dest = prop_idx->add(p.int_value(), allocator) ;                
                if (dest->num_elems() == 0) {
                    dest->init();
                }
                dest->add(n, allocator);
            }
            break;
        case t_float:
            break;
        case t_boolean:
            {
                BoolValueIndex *idx = (BoolValueIndex *)this;
                // Now retrieve the list where node pointers are getting added
                // This particular property only has one value = true and should
                // be the first and only node in the tree.
                // TODO: Tree is unnecessary and Node* needs better arrangement for
                // quick search and remove operations
                List<const Node *> *list = idx->value(idx->find(p.bool_value()) );
                list->add(n, allocator);
            }
            break;
        case t_time:
        case t_string:
            throw Exception(not_implemented);
        case t_blob:
        case t_novalue:
            throw Exception(property_type);
    }
}

void Index::remove(const Property &p, const Node *n, Allocator &allocator)
{
    return;
}
