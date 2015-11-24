#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "jarvis.h"
#include "util.h"

using namespace Jarvis;

static void dump_nodes(Graph &db, FILE *f);
static void dump_edges(Graph &db, FILE *f);

void dump_debug(Graph &db, FILE *f)
{
    dump_nodes(db, f);
    dump_edges(db, f);
}

static void dump_nodes(Graph &db, FILE *f)
{
    for (NodeIterator i = db.get_nodes(); i; i.next())
        dump(*i, f);
}

static void dump_edges(Graph &db, FILE *f)
{
    for (EdgeIterator i = db.get_edges(); i; i.next())
        dump(*i, f);
}

void dump(NodeIterator i, FILE *f)
{
    while (i) {
        dump(*i, f);
        i.next();
    }
}

void dump(EdgeIterator i, FILE *f)
{
    while (i) {
        dump(*i, f);
        i.next();
    }
}

void dump(const Node &n, FILE *f)
{
    fprintf(f, "Node %" PRIu64 "%s:\n", n.get_id(), tag_text(n).c_str());
    n.get_properties()
        .process([&f](PropertyRef &p) {
            fprintf(f, "  %s: %s\n", p.id().name().c_str(), property_text(p).c_str());
        });
    n.get_edges(Outgoing)
        .process([&f](EdgeRef &e) {
            fprintf(f, " %s -> n%" PRIu64 " (e%" PRIu64 ")\n",
                    tag_text(e).c_str(),
                    e.get_destination().get_id(), e.get_id());
        });
    n.get_edges(Incoming)
        .process([&f](EdgeRef &e) {
            fprintf(f, " %s <- n%" PRIu64 " (e%" PRIu64 ")\n",
                    tag_text(e).c_str(),
                    e.get_source().get_id(), e.get_id());
        });
}

void dump(const Edge &e, FILE *f)
{
    fprintf(f, "Edge %" PRIu64 "%s: n%" PRIu64 " -> n%" PRIu64 "\n",
            e.get_id(), tag_text(e).c_str(),
            e.get_source().get_id(), e.get_destination().get_id());
    e.get_properties()
        .process([&f](PropertyRef &p) {
            fprintf(f, "  %s: %s\n", p.id().name().c_str(), property_text(p).c_str());
        });
}
