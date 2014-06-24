#include "AvlTreeIndex.h"
#include "iterator.h"
#include "List.h"
#include "IndexString.h"

using namespace Jarvis;

namespace Jarvis {
    class IndexEq_NodeIteratorImpl : public NodeIteratorImplIntf {
        const List<Node *>::ListType *_pos;
    public:
        IndexEq_NodeIteratorImpl(List<Node *> *l)
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
