#include "AvlTreeIndex.h"
#include "iterator.h"
#include "List.h"
#include "IndexString.h"

using namespace Jarvis;

namespace Jarvis {
    class IndexEq_NodeIteratorImpl : public NodeIteratorImplIntf {
        ListTraverser<Node *> _it;
    public:
        IndexEq_NodeIteratorImpl(List<Node *> *l)
            : _it(l)
        { }
        Node &operator*() { return **_it; }
        Node *operator->() { return *_it; }
        Node &operator*() const { return **_it; }
        Node *operator->() const { return *_it; }
        operator bool() const { return _it; }
        bool next() { return _it.next(); }
    };
}

template <typename K, typename V>
NodeIterator AvlTreeIndex<K,V>::get_nodes(const K &key, const PropertyPredicate &pp)
{
    V *value = this->find(key);
    if (value == NULL)
        return NodeIterator(NULL);
    else
        return NodeIterator(new IndexEq_NodeIteratorImpl(value));
}

// Explicitly instantiate any types that might be required
template class AvlTreeIndex<long long, List<Node *>>;
template class AvlTreeIndex<bool, List<Node *>>;
template class AvlTreeIndex<double, List<Node *>>;
template class AvlTreeIndex<IndexString, List<Node *>>;
