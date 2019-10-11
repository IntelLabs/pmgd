#define GET_ARGS                                                   \
    bool timing = false;                                           \
    bool verbose = false;                                          \
                                                                   \
    int argi = 1;                                                  \
    while (argi < argc && argv[argi][0] == '-') {                  \
        if (strcmp(argv[argi], "-t") == 0)                         \
            timing = true, argi++;                                 \
        else if (strcmp(argv[argi], "-v") == 0)                    \
            verbose = true, argi++;                                \
        else                                                       \
            fprintf(stderr, "invalid parameter %s\n", argv[argi]); \
    }                                                              \
                                                                   \
    assert(argi + 1 < argc);                                       \
    const char *graph_name = argv[argi++];                         \
    const char *filename = argv[argi++];                           \

