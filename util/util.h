#pragma once

#include <stdio.h>
#include <string>
#include <functional>
#include "jarvis.h"

namespace Jarvis {
    enum UtilExceptionType {
        LoaderOpenFailed = UtilExceptionBase,
        LoaderParseError,
        LoaderFormatError,
    };
};

extern std::string property_text(const Jarvis::Property &i);
extern std::string property_text(const Jarvis::PropertyRef &i);
template <typename T> std::string tag_text(const T &n);

extern bool string_to_tm(const std::string &tstr, struct tm *user_tz_tm,
                     int *hr_offset, int *min_offset);
extern std::string time_to_string(const Jarvis::Time& t, bool utc=false);

extern void print_exception(const Jarvis::Exception &e, FILE *f = stdout);

extern void dump_nodes(Jarvis::Graph &db, FILE *f = stdout);
extern void dump_edges(Jarvis::Graph &db, FILE *f = stdout);
extern void dump(Jarvis::Graph &db, const Jarvis::Node &n, FILE *f = stdout);
extern void dump(Jarvis::Graph &db, const Jarvis::Edge &n, FILE *f = stdout);

extern void dump(Jarvis::Graph &db, Jarvis::NodeIterator &i, FILE *f = stdout);
extern void dump(Jarvis::Graph &db, Jarvis::EdgeIterator &i, FILE *f = stdout);
extern void dump(Jarvis::Graph &db, Jarvis::NodeIterator &&i, FILE *f = stdout);
extern void dump(Jarvis::Graph &db, Jarvis::EdgeIterator &&i, FILE *f = stdout);

extern void do_nothing_node(Jarvis::Node &);
extern void do_nothing_edge(Jarvis::Edge &);

extern void load_tsv(Jarvis::Graph &db, const char *filename,
                     std::function<void(Jarvis::Node &)> = do_nothing_node,
                     std::function<void(Jarvis::Edge &)> = do_nothing_edge);
extern void load_tsv(Jarvis::Graph &db, FILE *f,
                     std::function<void(Jarvis::Node &)> = do_nothing_node,
                     std::function<void(Jarvis::Edge &)> = do_nothing_edge);

extern void load(Jarvis::Graph &db, const char *filename,
                 std::function<void(Jarvis::Node &)> = do_nothing_node,
                 std::function<void(Jarvis::Edge &)> = do_nothing_edge);
extern void load(Jarvis::Graph &db, FILE *f,
                 std::function<void(Jarvis::Node &)> = do_nothing_node,
                 std::function<void(Jarvis::Edge &)> = do_nothing_edge);

extern void load_gson(Jarvis::Graph &db, const char *filename,
                      std::function<void(Jarvis::Node &)> = do_nothing_node,
                      std::function<void(Jarvis::Edge &)> = do_nothing_edge);
