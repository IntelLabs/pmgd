#pragma once

#include <stdio.h>
#include <string>
#include "jarvis.h"

extern std::string property_text(const Jarvis::Property &i);
extern std::string property_text(const Jarvis::PropertyIterator &i);

extern void print_exception(const Jarvis::Exception &e, FILE *f = stdout);

extern void dump_nodes(Jarvis::Graph &db, FILE *f = stdout);
extern void dump_edges(Jarvis::Graph &db, FILE *f = stdout);
extern void dump(Jarvis::Graph &db, const Jarvis::Node &n, FILE *f = stdout);
extern void dump(Jarvis::Graph &db, const Jarvis::Edge &n, FILE *f = stdout);

extern void dump(Jarvis::Graph &db, Jarvis::NodeIterator i, FILE *f = stdout);
extern void dump(Jarvis::Graph &db, Jarvis::EdgeIterator i, FILE *f = stdout);
