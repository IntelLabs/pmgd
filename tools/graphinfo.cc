/**
 * Dump information about a graphstore to standard output
 */

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <unistd.h>

static bool raw = false;

template <typename T>
void read_file(const char *db_name, const char *file, T &buf, size_t offset = 0);

struct RegionInfo {
    static const int REGION_NAME_LEN = 32;
    char name[REGION_NAME_LEN];
    uint64_t addr;
    size_t len;

    void print()
    {
        if (raw)
            printf("%-18.*s%#lx\t%#zx\n", REGION_NAME_LEN, name, addr, len);
        else
            printf("%.*s\t%#lx\t%#zx\n", REGION_NAME_LEN, name, addr, len);
    }
};

struct GraphInfo {
    uint64_t version;

    RegionInfo transaction_info;
    RegionInfo journal_info;
    RegionInfo indexmanager_info;
    RegionInfo stringtable_info;
    RegionInfo node_info;
    RegionInfo edge_info;
    RegionInfo allocator_info;

    uint32_t max_stringid_length;

    char locale_name[32];

    uint32_t num_fixed_allocators;
    uint64_t allocator_offsets[8];

    void print()
    {
        printf("version: %lu\n", version);
        printf("locale: %.32s\n", locale_name);
        printf("max stringid length: %u\n", max_stringid_length);
        printf("\n");

        if (!raw) {
            printf(".fi\n");
            printf(".TS\n");
            printf("l c c\n");
            printf("l r r .\n");
            printf("\t  base\t    size\n");
        }
        else {
            printf("\t\t    base\t  size\n");
        }

        transaction_info.print();
        journal_info.print();
        indexmanager_info.print();
        stringtable_info.print();
        node_info.print();
        edge_info.print();
        allocator_info.print();

        if (! raw)
            printf(".TE\n");

        printf("\n");
    }
};

struct FixedAllocator {
    struct RegionHeader {
        uint64_t tail_ptr;
        uint64_t free_ptr;
        int64_t num_allocated;
        uint64_t max_addr;
        uint32_t size;
    };

    static bool has_free_list;

    const char *label;
    uint64_t base;
    RegionHeader hdr;

    void read(const char *name, const char *db_name,
              const RegionInfo &region_info, uint64_t offset = 0)
    {
        label = name;
        base = region_info.addr + offset;
        read_file(db_name, region_info.name, hdr, offset);
        if (hdr.free_ptr)
            has_free_list = true;
    }

    void print1() const
    {
        printf("%s\t%u\t%ld\t%#lx\t%#lx\t%#lx",
               label, hdr.size, hdr.num_allocated, base, hdr.tail_ptr, hdr.max_addr);
        if (hdr.free_ptr)
            printf("\t%#lx", hdr.free_ptr);
        printf("\n");
    }

    void print2() const
    {
        int offset = 0;
        switch (hdr.size) {
            case 16: offset += 0x30; break;
            case 32: offset += 0x40; break;
            case 64: offset += 0x40; break;
            case 128: offset += 0x80; break;
            case 256: offset += 0x100; break;
        }
        uint64_t first = base + offset;
        uint64_t total = (hdr.max_addr - first) / hdr.size;
        uint64_t free = (hdr.max_addr - hdr.tail_ptr) / hdr.size;
        uint64_t free_list = total - free - hdr.num_allocated;
        printf("%s\t%u\t%lu\t%lu\t%lu",
               label, hdr.size, total, hdr.num_allocated, free);
        if (free_list)
            printf("\t%lu", free_list);
        printf("\n");
    }
};

bool FixedAllocator::has_free_list = false;

void print_usage(FILE *stream);

int main(int argc, char **argv)
{
    int argi = 1;

    while (argi < argc && argv[argi][0] == '-') {
        switch (argv[argi][1]) {
            case 'h':
                print_usage(stdout);
                return 0;

            case 'r':
                raw = true;
                break;

            default:
                fprintf(stderr, "dumpgraph: %s: Unrecognized option\n", argv[argi]);
                print_usage(stderr);
                return 1;
        }
        argi++;
    }

    if (!(argi < argc)) {
        fprintf(stderr, "dumpgraph: No graphstore specified\n");
        print_usage(stderr);
        return 1;
    }


    FILE *pipe = NULL;
    if (!raw) {
        pipe = popen("tbl | nroff | uniq", "w");
        dup2(fileno(pipe), 1);
        printf(".nf\n");
    }

    const char *db_name = argv[argi];

    union {
        GraphInfo graph_info;
        uint8_t graph_info_raw[4096];
    };
    read_file(db_name, "graph.jdb", graph_info_raw);
    graph_info.print();

    FixedAllocator nodes;
    FixedAllocator edges;
    FixedAllocator allocator[5];

    nodes.read("Nodes", db_name, graph_info.node_info);
    edges.read("Edges", db_name, graph_info.edge_info);
    for (uint i = 0; i < graph_info.num_fixed_allocators; i++)
        allocator[i].read("", db_name, graph_info.allocator_info,
                          graph_info.allocator_offsets[i]);

    if (!raw) {
        printf(".fi\n");
        printf(".TS\n");
        printf("nowarn;\n");
        if (FixedAllocator::has_free_list) {
            printf("l c c c c c c\n");
            printf("l r r r r r r .\n");
            printf("\tsize\tcount\tbase\ttail\tmax\tfree list\n");
        }
        else {
            printf("l c c c c c\n");
            printf("l r r r r r .\n");
            printf("\tsize\tcount\tbase\ttail\tmax\n");
        }
    }
    else {
        if (FixedAllocator::has_free_list)
            printf("\tsize\tcount\tbase\t\ttail\t\tmax\t\tfree list\n");
        else
            printf("\tsize\tcount\tbase\t\ttail\t\tmax\n");
    }

    nodes.print1();
    edges.print1();
    for (uint i = 0; i < graph_info.num_fixed_allocators; i++)
        allocator[i].print1();

    if (!raw)
        printf(".TE\n");

    printf("\n");

    if (!raw) {
        printf(".TS\n");
        printf("nowarn;\n");
        if (FixedAllocator::has_free_list) {
            printf("l c c c c c\n");
            printf("l r r r r r .\n");
            printf("\tsize\ttotal\tused\tfree\tfree list\n");
        }
        else {
            printf("l c c c c\n");
            printf("l r r r r .\n");
            printf("\tsize\ttotal\tused\tfree\n");
        }
    }
    else {
        if (FixedAllocator::has_free_list)
            printf("\tsize\ttotal\t\tused\tfree\t\tfree list\n");
        else
            printf("\tsize\ttotal\t\tused\tfree\n");
    }

    nodes.print2();
    edges.print2();
    for (uint i = 0; i < graph_info.num_fixed_allocators; i++)
        allocator[i].print2();

    if (!raw)
        printf(".TE\n");

    if (!raw) {
        fclose(stdout);
        pclose(pipe);
    }

    return 0;
}

void print_usage(FILE *stream)
{
    fprintf(stream, "Usage: dumpgraph [OPTION]... GRAPHSTORE\n");
    fprintf(stream, "Dump the content of GRAPHSTORE.\n");
    fprintf(stream, "\n");
    fprintf(stream, "  -h  print this help and exit\n");
    fprintf(stream, "  -r  open the graph read/write, so recovery can be performed if necessary\n");
    fprintf(stream, "  -d  debug mode: list all nodes then all edges (default)\n");
    fprintf(stream, "  -x  dump in the GEXF file format\n");
    fprintf(stream, "  -j  dump in the Jarvis Lake graph text format\n");
}

template <typename T>
void read_file(const char *db_name, const char *name, T &buf, size_t offset)
{
    std::string file_name = std::string(db_name) + "/" + name;
    FILE *file = fopen(file_name.c_str(), "rb");
    if (file == NULL) {
        perror(file_name.c_str());
        exit(1);
    }

    fseek(file, offset, 0);

    if (fread(&buf, sizeof buf, 1, file) != 1) {
        perror(file_name.c_str());
        exit(1);
    }
    fclose(file);
}
