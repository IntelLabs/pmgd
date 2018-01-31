/**
 * @file   util.h
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#pragma once

#include <stdio.h>
#include <string>
#include <functional>
#include "pmgd.h"

namespace PMGD {
    enum UtilExceptionType {
        LoaderOpenFailed = UtilExceptionBase,
        LoaderParseError,
        LoaderFormatError,
    };
};

extern std::string property_text(const PMGD::Property &i);
extern std::string property_text(const PMGD::PropertyRef &i);
template <typename T> std::string tag_text(const T &n);

extern bool string_to_tm(const std::string &tstr, struct tm *user_tz_tm,
                         unsigned long *usec, int *hr_offset, int *min_offset);

inline bool string_to_tm(const std::string &tstr, struct tm *user_tz_tm,
                         int *hr_offset, int *min_offset)
{
    unsigned long usec;
    return string_to_tm(tstr, user_tz_tm, &usec, hr_offset, min_offset);
}

extern std::string time_to_string(const PMGD::Time& t, bool utc=false);

extern void print_exception(const PMGD::Exception &e, FILE *f = stdout);

extern void dump_debug(PMGD::Graph &db, FILE *f = stdout);
extern void dump_gexf(PMGD::Graph &db, FILE *f = stdout);
extern void dump_pmgd(PMGD::Graph &db, FILE *f = stdout);

extern void dump(const PMGD::Node &n, FILE *f = stdout);
extern void dump(const PMGD::Edge &e, FILE *f = stdout);
extern void dump(PMGD::NodeIterator ni, FILE *f = stdout);
extern void dump(PMGD::EdgeIterator ei, FILE *f = stdout);

extern void do_nothing_node(PMGD::Node &);
extern void do_nothing_edge(PMGD::Edge &);

extern void load_tsv(PMGD::Graph &db, const char *filename,
                     std::function<void(PMGD::Node &)> = do_nothing_node,
                     std::function<void(PMGD::Edge &)> = do_nothing_edge);
extern void load_tsv(PMGD::Graph &db, FILE *f,
                     std::function<void(PMGD::Node &)> = do_nothing_node,
                     std::function<void(PMGD::Edge &)> = do_nothing_edge);

extern void load(PMGD::Graph &db, const char *filename, bool use_index = false,
                 std::function<void(PMGD::Node &)> = do_nothing_node,
                 std::function<void(PMGD::Edge &)> = do_nothing_edge);
extern void load(PMGD::Graph &db, FILE *f, bool use_index = false,
                 std::function<void(PMGD::Node &)> = do_nothing_node,
                 std::function<void(PMGD::Edge &)> = do_nothing_edge);

extern void load_gson(PMGD::Graph &db, const char *filename,
                      std::function<void(PMGD::Node &)> = do_nothing_node,
                      std::function<void(PMGD::Edge &)> = do_nothing_edge);
