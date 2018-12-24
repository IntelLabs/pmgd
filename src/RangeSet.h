/**
 * @file   RangeSet.h
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

#include <stddef.h>
#include <stdint.h>
#include <set>

namespace PMGD {
    class RangeSet
    {
        struct Range {
            uint64_t start;
            uint64_t end;

            // Order based on start first and then put smaller end before.
            bool operator< (const Range &r) const
            {
                return start < r.start || (start == r.start && end < r.end);
            }

            Range(const uint64_t &s, const uint64_t &e) : start(s), end(e) {}
            Range() : start(0), end(0) {}
        };

        struct RangeComp {
            bool operator() (const Range &lhs, const Range &rhs) const
            { return lhs < rhs; }
        };

        typedef std::set<Range, RangeComp> RangeSetType;

        RangeSetType _rangeset;

    public:
        RangeSetType::iterator begin() { return _rangeset.begin(); }
        RangeSetType::iterator end() { return _rangeset.end(); }
        void clear() { _rangeset.clear(); }

        // Normally this big a function would go in a CC file but it is appropriate
        // here for the following reasons:
        // a) By putting here, no .cc file is needed for this class
        // b) It is only called from one place, so there is no cost to inlining it.
        void add(uint64_t start, uint64_t end)
        {
            if (start == end)  // Not a range but not clear if we need to complain more.
                return;

            auto ret = _rangeset.insert(Range(start, end));
            if (!ret.second)   // complete overlap
                return;

            auto it = ret.first;
            auto fwd_it = it;
            fwd_it++;
            if (it != _rangeset.begin()) {
                auto back_it = it;
                back_it--;

                // Find if there is anything behind to merge. It can happen if the end
                // of the previous one was before the end of the new range and their
                // starts were equal. If the previous position has both smaller start
                // and non overlapping end, we can skip traversal.
                // Just one step behind is enough. If there was more
                // overlap of this kind, the previos two entries would
                // have been merged.
                if (start == back_it->start) {
                    _rangeset.erase(back_it);
                }
                // start cannot be < back_it start based on our compare function.
                else {
                    if (back_it->end >= start) { // These can be merged
                        // Since set elements cannot be modified, insert
                        // a new entry and erase the existing two entries.
                        // Since we have iterators, it should be decent.
                        _rangeset.erase(it);
                        it = _rangeset.insert(fwd_it, Range(back_it->start, end));
                        _rangeset.erase(back_it);  // erase the new element. Now only fwd checks.
                    }
                    // else no overlap
                }
                // If starts were equal, end could not be smaller than back_it->end
            }

            // Now see if there is anything to change going forward.
            // This we might have to iterate till we find a non-overlapping
            // interval
            while (fwd_it != _rangeset.end()) {
                uint64_t starti = it->start, endi = it->end;
                uint64_t startf = fwd_it->start, endf = fwd_it->end;
                if (starti == startf) {
                    _rangeset.erase(it);
                    break;
                }
                // start cannot be > fwd_it start based on our compare function.
                else if (endi >= startf) {  // overlap
                    _rangeset.erase(it);

                    // Since set elements cannot be modified, insert
                    // a new entry and erase the existing two entries.
                    // Since we have iterators, it should be decent.
                    it = _rangeset.insert(fwd_it, Range(starti, std::max(endi, endf)));
                    fwd_it = _rangeset.erase(fwd_it);
                }
                else  // non-overlapping intervals
                    break;
            }
        }
    };
}
