#pragma once

#include <stdio.h>
#include <string>
#include <functional>
#include "jarvis.h"

extern std::string property_text(const Jarvis::Property &i);
extern std::string property_text(const Jarvis::PropertyIterator &i);
template <typename T> std::string tag_text(const T &n);

extern void print_exception(const Jarvis::Exception &e, FILE *f = stdout);

extern void dump_nodes(Jarvis::Graph &db, FILE *f = stdout);
extern void dump_edges(Jarvis::Graph &db, FILE *f = stdout);
extern void dump(Jarvis::Graph &db, const Jarvis::Node &n, FILE *f = stdout);
extern void dump(Jarvis::Graph &db, const Jarvis::Edge &n, FILE *f = stdout);

extern void dump(Jarvis::Graph &db, Jarvis::NodeIterator i, FILE *f = stdout);
extern void dump(Jarvis::Graph &db, Jarvis::EdgeIterator i, FILE *f = stdout);

extern void load_tsv(Jarvis::Graph &db, const char *filename,
                     std::function<void(Jarvis::Node &)> = NULL,
                     std::function<void(Jarvis::Edge &)> = NULL);
extern void load_tsv(Jarvis::Graph &db, FILE *f,
                     std::function<void(Jarvis::Node &)> = NULL,
                     std::function<void(Jarvis::Edge &)> = NULL);

extern void load(Jarvis::Graph &db, const char *filename,
                 std::function<void(Jarvis::Node &)> = NULL,
                 std::function<void(Jarvis::Edge &)> = NULL);
extern void load(Jarvis::Graph &db, FILE *f,
                 std::function<void(Jarvis::Node &)> = NULL,
                 std::function<void(Jarvis::Edge &)> = NULL);
