/**
 * @file   Index.cc
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

#include <string>
#include "Index.h"
#include "AvlTreeIndex.h"
#include "List.h"
#include "exception.h"
#include "TransactionImpl.h"
#include "GraphImpl.h"
#include "IndexString.h"

using namespace PMGD;

void Index::add(const Property &p, void *n, GraphImpl *db)
{
    if (_ptype != p.type())
        throw PMGDException(PropertyTypeMismatch);

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
            throw PMGDException(NotImplemented);
        case PropertyType::Blob:
        default:
            throw PMGDException(PropertyTypeInvalid);
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
        throw PMGDException(PropertyTypeMismatch);

    Allocator &allocator = db->allocator();

    List<void*> *dest;
    switch(_ptype) {
        case PropertyType::Integer:
            {
                LongValueIndex *prop_idx = static_cast<LongValueIndex *>(this);
                dest = prop_idx->find(p.int_value(), true);
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
                dest = prop_idx->find(p.float_value(), true);
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
                dest = prop_idx->find(p.bool_value(), true);
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
                dest = prop_idx->find(p.time_value(), true);
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
                dest = prop_idx->find(istr, true);
                if (dest) {
                    dest->remove(n, allocator);
                    // TODO: Re-traversal of tree.
                    if (dest->num_elems() == 0)
                        prop_idx->remove(istr, allocator);
                }
            }
            break;
        case PropertyType::NoValue:
            throw PMGDException(NotImplemented);
        case PropertyType::Blob:
        default:
            throw PMGDException(PropertyTypeInvalid);
    }
}

Index::Index_IteratorImplIntf *Index::get_iterator(Graph::IndexType index_type,
                                        const PropertyPredicate &pp, std::locale *loc,
                                        bool reverse)
{
    const Property &p1 = pp.v1;
    const Property &p2 = pp.v2;

    if (pp.op != PropertyPredicate::DontCare) {
        if (_ptype != p1.type())
            throw PMGDException(PropertyTypeMismatch);
        if (pp.op >= PropertyPredicate::GeLe) {
            if (_ptype != p2.type())
                throw PMGDException(PropertyTypeMismatch);
        }
    }

    switch(_ptype) {
        case PropertyType::Integer:
            {
                LongValueIndex *This = static_cast<LongValueIndex *>(this);
                if (pp.op >= PropertyPredicate::GeLe)
                    return This->get_iterator(index_type, p1.int_value(), p2.int_value(), pp.op, reverse);
                else if (pp.op == PropertyPredicate::DontCare)
                    return This->get_iterator(index_type, reverse);
                else
                    return This->get_iterator(index_type, p1.int_value(), pp.op, reverse);
            }
            break;
        case PropertyType::Float:
            {
                FloatValueIndex *This = static_cast<FloatValueIndex *>(this);
                if (pp.op >= PropertyPredicate::GeLe)
                    return This->get_iterator(index_type, p1.float_value(), p2.float_value(), pp.op, reverse);
                else if (pp.op == PropertyPredicate::DontCare)
                    return This->get_iterator(index_type, reverse);
                else
                    return This->get_iterator(index_type, p1.float_value(), pp.op, reverse);
            }
            break;
        case PropertyType::Boolean:
            {
                BoolValueIndex *This = static_cast<BoolValueIndex *>(this);
                if (pp.op >= PropertyPredicate::GeLe)
                    return This->get_iterator(index_type, p1.bool_value(), p2.bool_value(), pp.op, reverse);
                else if (pp.op == PropertyPredicate::DontCare)
                    return This->get_iterator(index_type, reverse);
                else
                    return This->get_iterator(index_type, p1.bool_value(), pp.op, reverse);
            }
            break;
        case PropertyType::Time:
            {
                TimeValueIndex *This = static_cast<TimeValueIndex *>(this);
                if (pp.op >= PropertyPredicate::GeLe)
                    return This->get_iterator(index_type, p1.time_value(), p2.time_value(), pp.op, reverse);
                else if (pp.op == PropertyPredicate::DontCare)
                    return This->get_iterator(index_type, reverse);
                else
                    return This->get_iterator(index_type, p1.time_value(), pp.op, reverse);
            }
            break;
        case PropertyType::String:
            {
                StringValueIndex *This = static_cast<StringValueIndex *>(this);
                if (pp.op == PropertyPredicate::DontCare)
                    return This->get_iterator(index_type, reverse);
                TransientIndexString istr(p1.string_value(), *loc);
                if (pp.op >= PropertyPredicate::GeLe) {
                    TransientIndexString istr2(p2.string_value(), *loc);
                    return This->get_iterator(index_type, istr, istr2, pp.op, reverse);
                }
                else
                    return This->get_iterator(index_type, istr, pp.op, reverse);
            }
            break;
        case PropertyType::NoValue:
            throw PMGDException(NotImplemented);
        case PropertyType::Blob:
        default:
            throw PMGDException(PropertyTypeInvalid);
    }
}

Graph::IndexStats Index::get_stats()
{
    Graph::IndexStats stats;
    switch(_ptype) {
        case PropertyType::Integer:
            {
                LongValueIndex *This   = static_cast<LongValueIndex *>(this);
                This->index_stats_info(stats);
            }
            break;
        case PropertyType::Float:
            {
                FloatValueIndex *This = static_cast<FloatValueIndex *>(this);
                This->index_stats_info(stats);
            }
            break;
        case PropertyType::Boolean:
            {
                BoolValueIndex *This = static_cast<BoolValueIndex *>(this);
                This->index_stats_info(stats);
            }
            break;
        case PropertyType::Time:
            {
                TimeValueIndex *This = static_cast<TimeValueIndex *>(this);
                This->index_stats_info(stats);
            }
            break;
        case PropertyType::String:
            {
                StringValueIndex *This = static_cast<StringValueIndex *>(this);
                This->index_stats_info(stats);
            }
            break;
        case PropertyType::NoValue:
            throw PMGDException(NotImplemented);
        case PropertyType::Blob:
            throw PMGDException(NotImplemented);
        default:
            throw PMGDException(PropertyTypeInvalid);
    }

    return stats;
}
