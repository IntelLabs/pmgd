/**
 * @file   callback.h
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
#include <list>
#include <functional>
#include <algorithm>

namespace PMGD {
    template <typename O, typename P> class CallbackList {
        typedef std::function<void(P)> F;
        struct Callback { O key; F f; };

        std::list<Callback> _list;

    public:
        void register_callback(O key, F f)
            { _list.push_back(Callback{key, f}); }

        void unregister_callback(O key)
            { _list.remove_if([key](Callback &p) { return p.key == key; }); }

        F *lookup_callback(O key)
        {
            auto r = std::find_if(_list.begin(), _list.end(),
                                  [key](Callback &a) { return a.key == key; });
            return r == _list.end() ? NULL : &r->f;
        }

        void do_callbacks(P p) const
        {
            for (auto cb : _list)
                cb.f(p);
        }
    };

    template <typename O> class CallbackList<O, void> {
        typedef std::function<void()> F;
        struct Callback { O key; F f; };

        std::list<Callback> _list;

    public:
        void register_callback(O key, F f)
            { _list.push_back(Callback{key, f}); }

        void unregister_callback(O key)
            { _list.remove_if([key](Callback &p) { return p.key == key; }); }

        F *lookup_callback(O key)
        {
            auto r = std::find_if(_list.begin(), _list.end(),
                                  [key](Callback &a) { return a.key == key; });
            return r == _list.end() ? NULL : &r->f;
        }

        void do_callbacks() const
        {
            for (auto cb : _list)
                cb.f();
        }
    };

    template <typename P> class CallbackList<void, P> {
        typedef std::function<void(P)> F;
        struct Callback { F f; };

        std::list<Callback> _list;

    public:
        void register_callback(F f) { _list.push_back(Callback{f}); }

        void do_callbacks(P p) const
        {
            for (auto cb : _list)
                cb.f(p);
        }
    };

    template <> class CallbackList<void, void> {
        typedef std::function<void()> F;
        struct Callback { F f; };

        std::list<Callback> _list;

    public:
        void register_callback(F f) { _list.push_back(Callback{f}); }

        void do_callbacks() const
        {
            for (auto cb : _list)
                cb.f();
        }
    };
};
