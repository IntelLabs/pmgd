#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "jarvis.h"
#include "util.h"

using namespace Jarvis;

void dump_nodes(Graph &db, FILE *f)
{
    for (NodeIterator i = db.get_nodes(); i; i.next()) {
        dump(db, *i, f);
    }
}

void dump_edges(Graph &db, FILE *f)
{
    for (EdgeIterator i = db.get_edges(); i; i.next()) {
        dump(db, *i, f);
    }
}

void dump(Graph &db, NodeIterator &i, FILE *f)
{
    while (i) {
        dump(db, *i, f);
        i.next();
    }
}

void dump(Graph &db, NodeIterator &&i, FILE *f)
{
    while (i) {
        dump(db, *i, f);
        i.next();
    }
}

void dump(Graph &db, EdgeIterator &i, FILE *f)
{
    while (i) {
        dump(db, *i, f);
        i.next();
    }
}

void dump(Graph &db, EdgeIterator &&i, FILE *f)
{
    while (i) {
        dump(db, *i, f);
        i.next();
    }
}

void dump(Graph &db, const Node &n, FILE *f)
{
    fprintf(f, "Node %" PRIu64 "%s:\n", db.get_id(n), tag_text(n).c_str());
    n.get_properties()
        .process([&db,&f](PropertyRef &p) {
            fprintf(f, "  %s: %s\n", p.id().name().c_str(), property_text(p).c_str());
        });
    n.get_edges(Outgoing)
        .process([&db,&f](EdgeRef &e) {
            fprintf(f, " %s -> n%" PRIu64 " (e%" PRIu64 ")\n", tag_text(e).c_str(),
                    db.get_id(e.get_destination()), db.get_id(e));
        });
    n.get_edges(Incoming)
        .process([&db,&f](EdgeRef &e) {
            fprintf(f, " %s <- n%" PRIu64 " (e%" PRIu64 ")\n", tag_text(e).c_str(),
                    db.get_id(e.get_source()), db.get_id(e));
        });
}

void dump(Graph &db, const Edge &e, FILE *f)
{
    fprintf(f, "Edge %" PRIu64 "%s: n%" PRIu64 " -> n%" PRIu64 "\n",
            db.get_id(e), tag_text(e).c_str(),
            db.get_id(e.get_source()), db.get_id(e.get_destination()));
    e.get_properties()
        .process([&db,&f](PropertyRef &p) {
            fprintf(f, "  %s: %s\n", p.id().name().c_str(), property_text(p).c_str());
        });
}
