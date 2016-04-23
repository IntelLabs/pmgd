#include <iostream>
#include <string.h>

#include "jarvis.h"
#include "../src/List.h"
#include "../src/os.h"
#include "../src/Allocator.h"
#include "../src/AvlTree.h"
#include "../src/GraphConfig.h"
#include "util.h"

using namespace Jarvis;
using namespace std;

namespace Jarvis {
    class AvlTreeTest {
        public:
            void print_recursive(AvlTree<int,int> &tree, AvlTree<int,int>::TreeNode *node,
                    char side, int rec_depth)
            {
                if (node == NULL)
                    return;
                for (int i = 0; i < rec_depth; ++i)
                    cout << " ";
                cout << side << ":" << *tree.key(node) << "," << *tree.value(node) << " [" << tree.height(node) << "]\n";
                print_recursive(tree, tree.left(node), 'L', rec_depth + 1);
                print_recursive(tree, tree.right(node), 'R', rec_depth + 1);
            }

            void print(AvlTree<int,int> &tree)
            {
                print_recursive(tree, tree._tree, 'C', 0);
                cout << endl;
            }
    };
}

int main()
{
    cout << "AVLTree unit test\n\n";
    uint64_t start_addr;
    uint64_t region_size, hdr_size;

    try {
        Graph db("avlgraph", Graph::Create);

        Transaction tx(db, Transaction::ReadWrite);
        bool create1 = true;

        start_addr = 0x100000000;
        region_size = 10485760;       // 10MB
        // We need a PM space for allocator header which normally
        // will reside in the GraphInfo structure which is quite
        // hidden. So creating a temporary space here to allow for
        // the header.
        uint64_t hdr_addr = start_addr + region_size;
        hdr_size = 1024;
        os::MapRegion region1(".", "region1", start_addr, region_size, create1, create1, false);
        os::MapRegion region2(".", "region2", hdr_addr, hdr_size, create1, create1, false);
        Allocator::RegionHeader *hdr = reinterpret_cast<Allocator::RegionHeader *>(hdr_addr);
        Allocator allocator1(start_addr, region_size, hdr, create1);

        AvlTree<int,int> tree;
        AvlTreeTest test;
        int insert_vals[] = {5, 5, 10, 15, 20, 25, 1, 4, 30, 35, 40, 45, 50,
            12, 17, 60, 70, 70, 55, 19};
        int num_inserted = 18 + 2;

        for (int i = 0; i < num_inserted; ++i) {
            int *value = tree.add(insert_vals[i], allocator1);
            *value = i + 1;
        }
        cout << "Num elements: " << tree.num_elems() << "\n";
        test.print(tree);

        int find_val = 66;
        int *value = tree.find(find_val);
        cout << "66: " << ((value == NULL) ? 0 : *value) << endl;
        find_val = 50;
        value = tree.find(find_val);
        cout << "50: " << ((value == NULL) ? 0 : *value) << endl;

        cout << "Testing remove\n";
        int remove_vals[] = {55, 55, 60, 45, 40, 15, 17, 10};
        int num_removed = 8;
        for (int i = 0; i < num_removed; ++i)
            tree.remove(remove_vals[i], allocator1);
        cout << "Num elements: " << tree.num_elems() << "\n";
        test.print(tree);
        // Now just delete the remaining
        int array[] = {5, 20, 25, 12, 19, 1, 4, 30, 35, 50, 70};
        for (int i = 0; i < 11; ++i)
            tree.remove(array[i], allocator1);
        cout << "Num elements: " << tree.num_elems() << "\n";
        test.print(tree);

        for (int i = 0; i < 8; ++i) {
            value = tree.add(201 + i, allocator1);
            *value = i + 1;
        }
        value = tree.add(210, allocator1);
        *value = 1;
        value = tree.add(212, allocator1);
        *value = 1;
        value = tree.add(200, allocator1);
        *value = 1;
        cout << "Num elements: " << tree.num_elems() << "\n";
        test.print(tree);
        for (int i = 0; i < 8; ++i)
            tree.remove(201 + i, allocator1);
        tree.remove(212, allocator1);
        tree.remove(210, allocator1);
        tree.remove(200, allocator1);
        cout << "Num elements: " << tree.num_elems() << "\n";
        test.print(tree);
    }
    catch (Exception e) {
        print_exception(e);
        return 1;
    }
    return 0;
}
