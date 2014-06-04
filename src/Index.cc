#include <string>
#include "Index.h"
#include "AvlTreeIndex.h"
#include "List.h"
#include "exception.h"
#include "TransactionImpl.h"
#include "GraphImpl.h"
#include "IndexString.h"

using namespace Jarvis;

void Index::add(const Property &p, void *n, GraphImpl *db)
{
    if (_ptype != p.type())
        throw Exception(PropertyTypeMismatch);

    Allocator &allocator = db->allocator();

    // TODO: Tree is unnecessary and Node* needs better arrangement for
    // quick search and remove operations
    List<void*> *dest = NULL;

    switch(_ptype) {
        case PropertyType::Integer:
            dest = static_cast<LongValueIndex *>(this)->add(p.int_value(),
                                                            allocator);
            break;
        case PropertyType::Float:
            dest = static_cast<FloatValueIndex *>(this)->add(p.float_value(),
                                                             allocator);
            break;
        case PropertyType::Boolean:
            dest = static_cast<BoolValueIndex *>(this)->add(p.bool_value(),
                                                            allocator);
            break;
        case PropertyType::Time:
            dest = static_cast<TimeValueIndex *>(this)->add(p.time_value(),
                                                            allocator);
            break;
        case PropertyType::String:
            {
                TransientIndexString istr(p.string_value(), db->locale());
                dest = static_cast<StringValueIndex *>(this)->add(istr, allocator);
            }
            break;
        case PropertyType::NoValue:
            throw Exception(NotImplemented);
        case PropertyType::Blob:
        default:
            throw Exception(PropertyTypeInvalid);
    }
    // dest will never be null since it gets allocated at the add time.
    // Also, if it was a new element, the add code does a placement new.
    // The List->init() function does not do a transaction flush but
    // we are going to add an element right after before the transaction
    // gets over and that does the right logging of the very same
    // elements that get modified in init().
    dest->add(n, allocator);
}

void Index::remove(const Property &p, void *n, GraphImpl *db)
{
    if (_ptype != p.type())
        throw Exception(PropertyTypeMismatch);

    Allocator &allocator = db->allocator();

    List<void*> *dest;
    switch(_ptype) {
        case PropertyType::Integer:
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
        case PropertyType::Float:
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
        case PropertyType::Boolean:
            {
                BoolValueIndex *prop_idx = static_cast<BoolValueIndex *>(this);
                dest = prop_idx->find(p.bool_value());
                if (dest) {
                    dest->remove(n, allocator);
                    // TODO: Re-traversal of tree.
                    if (dest->num_elems() == 0)
                        prop_idx->remove(p.bool_value(), allocator);
                }
            }
            break;
        case PropertyType::Time:
            {
                TimeValueIndex *prop_idx = static_cast<TimeValueIndex *>(this);
                dest = prop_idx->find(p.time_value());
                if (dest) {
                    dest->remove(n, allocator);
                    // TODO: Re-traversal of tree.
                    if (dest->num_elems() == 0)
                        prop_idx->remove(p.time_value(), allocator);
                }
            }
            break;
        case PropertyType::String:
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
        case PropertyType::NoValue:
            throw Exception(NotImplemented);
        case PropertyType::Blob:
        default:
            throw Exception(PropertyTypeInvalid);
    }
}

NodeIterator Index::get_nodes(const PropertyPredicate &pp, std::locale *loc, bool reverse)
{
    const Property &p1 = pp.v1;
    const Property &p2 = pp.v2;

    if (pp.op != PropertyPredicate::DontCare) {
        if (_ptype != p1.type())
            throw Exception(PropertyTypeMismatch);
        if (pp.op >= PropertyPredicate::GeLe) {
            if (_ptype != p2.type())
                throw Exception(PropertyTypeMismatch);
        }
    }

    switch(_ptype) {
        case PropertyType::Integer:
            {
                LongValueIndex *This = static_cast<LongValueIndex *>(this);
                if (pp.op >= PropertyPredicate::GeLe)
                    return This->get_nodes(p1.int_value(), p2.int_value(), pp.op, reverse);
                else if (pp.op == PropertyPredicate::DontCare)
                    return This->get_nodes(reverse);
                else
                    return This->get_nodes(p1.int_value(), pp.op, reverse);
            }
            break;
        case PropertyType::Float:
            {
                FloatValueIndex *This = static_cast<FloatValueIndex *>(this);
                if (pp.op >= PropertyPredicate::GeLe)
                    return This->get_nodes(p1.float_value(), p2.float_value(), pp.op, reverse);
                else if (pp.op == PropertyPredicate::DontCare)
                    return This->get_nodes(reverse);
                else
                    return This->get_nodes(p1.float_value(), pp.op, reverse);
            }
            break;
        case PropertyType::Boolean:
            {
                BoolValueIndex *This = static_cast<BoolValueIndex *>(this);
                if (pp.op >= PropertyPredicate::GeLe)
                    return This->get_nodes(p1.bool_value(), p2.bool_value(), pp.op, reverse);
                else if (pp.op == PropertyPredicate::DontCare)
                    return This->get_nodes(reverse);
                else
                    return This->get_nodes(p1.bool_value(), pp.op, reverse);
            }
            break;
        case PropertyType::Time:
            {
                TimeValueIndex *This = static_cast<TimeValueIndex *>(this);
                if (pp.op >= PropertyPredicate::GeLe)
                    return This->get_nodes(p1.time_value(), p2.time_value(), pp.op, reverse);
                else if (pp.op == PropertyPredicate::DontCare)
                    return This->get_nodes(reverse);
                else
                    return This->get_nodes(p1.time_value(), pp.op, reverse);
            }
            break;
        case PropertyType::String:
            {
                TransientIndexString istr(p1.string_value(), *loc);
                StringValueIndex *This = static_cast<StringValueIndex *>(this);
                if (pp.op >= PropertyPredicate::GeLe) {
                    TransientIndexString istr2(p2.string_value(), *loc);
                    return This->get_nodes(istr, istr2, pp.op, reverse);
                }
                else if (pp.op == PropertyPredicate::DontCare)
                    return This->get_nodes(reverse);
                else
                    return This->get_nodes(istr, pp.op, reverse);
            }
            break;
        case PropertyType::NoValue:
            throw Exception(NotImplemented);
        case PropertyType::Blob:
        default:
            throw Exception(PropertyTypeInvalid);
    }
}
