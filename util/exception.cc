#include <stdio.h>
#include "jarvis.h"
#include "util.h"

using namespace Jarvis;

void print_exception(const Exception &e, FILE *f)
{
    fprintf(f, "[Exception] %s at %s:%d\n", e.name, e.file, e.line);
}
