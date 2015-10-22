/**
 * Dump information about a graphstore to standard output
 */

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <unistd.h>

static bool raw = false;

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
    uint64_t tail_ptr;
    uint64_t free_ptr;
    int64_t num_allocated;
    uint64_t max_addr;
    uint32_t size;

    static bool footnote;

    void print1(const char *s, uint64_t base)
    {
        printf("%s\t%u\t%ld\t%#lx\t%#lx\t%#lx",
               s, size, num_allocated, base, tail_ptr, max_addr);
        if (free_ptr)
            printf("\t%#lx", free_ptr);
        printf("\n");
    }

    void print2(const char *s, uint64_t base)
    {
        switch (size) {
            case 16: base += 0x30; break;
            case 32: base += 0x40; break;
            case 64: base += 0x40; break;
            case 128: base += 0x80; break;
            case 256: base += 0x100; break;
        }
        uint64_t total = (max_addr - base) / size;
        uint64_t free = (max_addr - tail_ptr) / size;
        char c = ' ';
        if (free_ptr) {
            c = '*';
            footnote = true;
        }
        printf("%s\t%u\t%lu\t%lu%c\t%lu\n",
               s, size, num_allocated, free, c, total);
    }

    static void print_footnote()
    {
        if (footnote)
            printf("* Not counting elements on free list.\n");
    }
};

bool FixedAllocator::footnote = false;

void print_usage(FILE *stream);

template <typename T>
void read_file(const char *db_name, const char *file, T &buf, size_t offset = 0);

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

    if (!raw) {
        printf(".fi\n");
        printf(".TS\n");
        printf("nowarn;\n");
        printf("l c c c c c r\n");
        printf("l r r r r r r.\n");
        printf("\tsize\tcount\tbase\ttail\tmax\tfree\n");
    }
    else
        printf("\tsize\tcount\tbase\t\ttail\t\tmax\t\tfree\n");

    FixedAllocator nodes;
    FixedAllocator edges;
    FixedAllocator allocator[5];

    read_file(db_name, graph_info.node_info.name, nodes);
    nodes.print1("Nodes", graph_info.node_info.addr);

    read_file(db_name, graph_info.edge_info.name, edges);
    edges.print1("Edges", graph_info.edge_info.addr);

    for (uint i = 0; i < graph_info.num_fixed_allocators; i++) {
        read_file(db_name, graph_info.allocator_info.name, allocator[i],
                  graph_info.allocator_offsets[i]);
        allocator[i].print1("", graph_info.allocator_info.addr + graph_info.allocator_offsets[i]);
    }

    if (!raw)
        printf(".TE\n");

    printf("\n");

    if (!raw) {
        printf(".TS\n");
        printf("nowarn;\n");
        printf("l c c c c\n");
        printf("l r r r r.\n");
        printf("\tsize\tused\tfree\ttotal\n");
    }
    else
        printf("\tsize\tused\tfree\t\ttotal\n");

    nodes.print2("Nodes", graph_info.node_info.addr);
    edges.print2("Edges", graph_info.edge_info.addr);

    for (uint i = 0; i < graph_info.num_fixed_allocators; i++)
        allocator[i].print2("", graph_info.allocator_info.addr + graph_info.allocator_offsets[i]);

    if (!raw)
        printf(".TE\n");
    FixedAllocator::print_footnote();

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
