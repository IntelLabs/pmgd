#include <stdlib.h>  // for rand
#include <iostream>
#include <string.h>

#include "List.h"
#include "allocator.h"

using namespace Jarvis;
using namespace std;

int main()
{
    cout << "List unit test\n\n";

    // Need the allocator
    struct AllocatorInfo info1;
    strcpy(info1.name, "region1");
    info1.addr = 0x100000000;
    info1.len = 1024;
    info1.size = 16;
    FixedAllocator region1(".", info1, true);
    long base1 = info1.addr + /* sizeof(struct RegionHeader) */64;

    cout << "Allocator created starting at: " << (void *)base1 << "\n";
    List<int> *list = (List<int> *)region1.alloc();
    cout << "Size of list obj: " << sizeof(List<int>) << "\n";
    list->init();

    // List array elements
    int elems[] = {7, 5, 3, 7, 4, 13, 3};
    for (int i = 0; i < 5; ++i)
        list->add(elems[i], region1);
/*
    for (int i = 0; i < 100; ++i)
        list.add(rand() % 1000);
        */
    list->remove(3, region1);
    for (int i = 5; i < 7; ++i)
        list->add(elems[i], region1);

    for (int i = 0; i < 7; ++i)
        list->remove(elems[i], region1);
    return 0;
}
