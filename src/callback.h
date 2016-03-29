#pragma once
#include <list>
#include <functional>
#include <algorithm>

namespace Jarvis {
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
