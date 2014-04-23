#pragma once
#include "property.h"
#include "allocator.h"
#include "node.h"

namespace Jarvis {
    // Base class for all the property value indices
    class Index {
    protected:
        PropertyType _ptype;
    public:
        void init(PropertyType ptype);
        void add(const Property &p, const Node *n, Allocator &allocator);
        void remove(const Property &p, const Node *n, Allocator &allocator);
        // Find functions will be for the iterators
    };
}
