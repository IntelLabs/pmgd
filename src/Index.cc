#include <string>
#include "Index.h"
#include "AvlTreeIndex.h"
#include "List.h"
#include "exception.h"
#include "TransactionImpl.h"
#include "GraphImpl.h"
#include "IndexString.h"

using namespace Jarvis;

void Index::add(const Property &p, Node *n, GraphImpl *db)
{
    if (_ptype != p.type())
        throw Exception(property_type);

    Allocator &allocator = db->allocator();

    // TODO: Tree is unnecessary and Node* needs better arrangement for
    // quick search and remove operations
    List<Node*> *dest = NULL;

    switch(_ptype) {
        case t_integer:
            dest = static_cast<LongValueIndex *>(this)->add(p.int_value(),
                                                            allocator);
            break;
        case t_float:
            dest = static_cast<FloatValueIndex *>(this)->add(p.float_value(),
                                                             allocator);
            break;
        case t_boolean:
            dest = static_cast<BoolValueIndex *>(this)->add(p.bool_value(),
                                                            allocator);
            break;
        case t_string:
            {
                TransientIndexString istr(p.string_value(), db->locale());
                dest = static_cast<StringValueIndex *>(this)->add(istr, allocator);
            }
            break;
        case t_time:
        case t_novalue:
            throw Exception(not_implemented);
        case t_blob:
        default:
            throw Exception(property_type);
    }
    // dest will never be null since it gets allocated at the add time.
    // Also, if it was a new element, the add code does a placement new.
    // The List->init() function does not do a transaction flush but
    // we are going to add an element right after before the transaction
    // gets over and that does the right logging of the very same
    // elements that get modified in init().
    dest->add(n, allocator);
}

void Index::remove(const Property &p, Node *n, GraphImpl *db)
{
    if (_ptype != p.type())
        throw Exception(property_type);

    Allocator &allocator = db->allocator();

    List<Node*> *dest;
    switch(_ptype) {
        case t_integer:
            {
                LongValueIndex *prop_idx = static_cast<LongValueIndex *>(this);
                dest = prop_idx->find(p.int_value());
                if (dest) {
                    dest->remove(n, allocator);
                    // TODO: Re-traversal of tree.
                    if (dest->num_elems() == 0)
                        prop_idx->remove(p.int_value(), allocator);
                }
            }
            break;
        case t_float:
            {
                FloatValueIndex *prop_idx = static_cast<FloatValueIndex *>(this);
                dest = prop_idx->find(p.float_value());
                if (dest) {
                    dest->remove(n, allocator);
                    // TODO: Re-traversal of tree.
                    if (dest->num_elems() == 0)
                        prop_idx->remove(p.float_value(), allocator);
                }
            }
            break;
        case t_boolean:
            {
                BoolValueIndex *prop_idx = static_cast<BoolValueIndex *>(this);
                dest = prop_idx->find(p.bool_value());
                if (dest) {
                    dest->remove(n, allocator);
                    // TODO: Re-traversal of tree.
                    if (dest->num_elems() == 0)
                        prop_idx->remove(p.float_value(), allocator);
                }
            }
            break;
        case t_string:
            {
                TransientIndexString istr(p.string_value(), db->locale());
                StringValueIndex *prop_idx = static_cast<StringValueIndex *>(this);
                dest = prop_idx->find(istr);
                if (dest) {
                    dest->remove(n, allocator);
                    // TODO: Re-traversal of tree.
                    if (dest->num_elems() == 0)
                        prop_idx->remove(istr, allocator);
                }
            }
            break;
        case t_time:
        case t_novalue:
            throw Exception(not_implemented);
        case t_blob:
        default:
            throw Exception(property_type);
    }
}

NodeIterator Index::get_nodes(const PropertyPredicate &pp, std::locale *loc)
{
    const Property &p1 = pp.v1;
    const Property &p2 = pp.v2;

    if (pp.op != PropertyPredicate::dont_care) {
        if (_ptype != p1.type())
            throw Exception(property_type);
        if (pp.op >= PropertyPredicate::gele) {
            if (_ptype != p2.type())
                throw Exception(property_type);
        }
    }

    switch(_ptype) {
        case t_integer:
            {
                LongValueIndex *This = static_cast<LongValueIndex *>(this);
                if (pp.op >= PropertyPredicate::gele)
                    return This->get_nodes(p1.int_value(), p2.int_value(), pp.op);
                else if (pp.op == PropertyPredicate::dont_care)
                    return This->get_nodes();
                else
                    return This->get_nodes(p1.int_value(), pp.op);
            }
            break;
        case t_float:
            {
                FloatValueIndex *This = static_cast<FloatValueIndex *>(this);
                if (pp.op >= PropertyPredicate::gele)
                    return This->get_nodes(p1.float_value(), p2.float_value(), pp.op);
                else if (pp.op == PropertyPredicate::dont_care)
                    return This->get_nodes();
                else
                    return This->get_nodes(p1.float_value(), pp.op);
            }
            break;
        case t_boolean:
            {
                BoolValueIndex *This = static_cast<BoolValueIndex *>(this);
                if (pp.op >= PropertyPredicate::gele)
                    return This->get_nodes(p1.bool_value(), p2.bool_value(), pp.op);
                else if (pp.op == PropertyPredicate::dont_care)
                    return This->get_nodes();
                else
                    return This->get_nodes(p1.bool_value(), pp.op);
            }
            break;
        case t_string:
            {
                TransientIndexString istr(p1.string_value(), *loc);
                StringValueIndex *This = static_cast<StringValueIndex *>(this);
                if (pp.op >= PropertyPredicate::gele) {
                    TransientIndexString istr2(p2.string_value(), *loc);
                    return This->get_nodes(istr, istr2, pp.op);
                }
                else if (pp.op == PropertyPredicate::dont_care)
                    return This->get_nodes();
                else
                    return This->get_nodes(istr, pp.op);
            }
            break;
        case t_time:
        case t_novalue:
            throw Exception(not_implemented);
        case t_blob:
        default:
            throw Exception(property_type);
    }
}
