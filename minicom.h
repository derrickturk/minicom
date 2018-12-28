#ifndef MINICOM_H

#define INTERFACE_(nm) struct nm
#define INTERFACE(nm, base) struct nm : public base

#define REGISTER_INTERFACE(iface) \
    template<> inline const char *minicom::interface_name<iface> = #iface

#define REGISTER_CLASS(klass) \
    template<> inline const char *minicom::class_name<klass> = #klass

#define TRY_DOWNCAST(iface, iname) \
    if (std::strcmp(minicom::interface_name<iface>, iname) == 0) { \
        auto p = static_cast<iface *>(this); \
        p->add_ref(); \
        return p; \
    }

#define TRY_CONSTRUCT(klass, kname) \
    if (std::strcmp(minicom::class_name<klass>, kname) == 0) \
        return new (std::nothrow) klass;

#ifdef __clang__
#define MINIEXPORT(ret, fn, ...) \
    extern "C" [[gnu::cdecl, gnu::dllexport]] ret fn(__VA_ARGS__) noexcept
#define MINIEXPORTDEF(ret, fn, ...) \
    extern "C" [[gnu::cdecl]] typedef ret (*fn)(__VA_ARGS__) noexcept
#define MINIMETHOD(ret, fn, ...) \
    [[gnu::cdecl]] virtual ret fn(__VA_ARGS__) noexcept = 0
#define MINIMETHODIMPL(ret, fn, ...) ret fn(__VA_ARGS__) noexcept

#elif defined(__GNUC__)
#define MINIEXPORT(ret, fn, ...) \
    extern "C" [[gnu::cdecl, gnu::dllexport]] ret fn(__VA_ARGS__) noexcept
#define MINIEXPORTDEF(ret, fn, ...) \
    extern "C" [[gnu::cdecl]] typedef ret (*fn)(__VA_ARGS__) noexcept
#define MINIMETHOD(ret, fn, ...) \
    [[gnu::cdecl]] virtual ret fn(__VA_ARGS__) noexcept = 0
#define MINIMETHODIMPL(ret, fn, ...) ret fn(__VA_ARGS__) noexcept

#elif defined(_MSC_VER)
#define MINIEXPORT(ret, fn, ...) \
    extern "C" __declspec(dllexport) ret __cdecl fn(__VA_ARGS__) noexcept
#define MINIEXPORTDEF(ret, fn, ...) \
    extern "C" typedef ret __cdecl (*fn)(__VA_ARGS__) noexcept
#define MINIMETHOD(ret, fn, ...) \
    virtual ret __cdecl fn(__VA_ARGS__) noexcept = 0
#define MINIMETHODIMPL(ret, fn, ...) ret fn(__VA_ARGS__) noexcept
#endif

#include <cstring>
#include <cstddef>
#include <atomic>
#include <memory>
#include <new>
#include <type_traits>

#define WIN32_LEAN_AND_MEAN
#include "windows.h"
// damn you windows.h
#undef cdecl

namespace minicom {

template<class T> const char *interface_name;
template<class T> const char *class_name;

INTERFACE_(i_miniobj) {
    MINIMETHOD(i_miniobj*, downcast, const char* iname);
    MINIMETHOD(void, add_ref, void);
    MINIMETHOD(void, release_ref, void);
};

template<class T> inline T* downcast(i_miniobj* obj) noexcept
{
    return static_cast<T*>(obj->downcast(minicom::interface_name<T>));
}

template<class T, class C> class rc {
  public:
    rc() noexcept
        : rc_(1) {}

    void add_ref() noexcept { ++rc_; }

    void release_ref() noexcept
    {
        if (--rc_ == 0)
            delete static_cast<T*>(this);
    }

  private:
    C rc_;
};

template<class T> using atomic_rc = rc<T, std::atomic<std::size_t>>;
template<class T> using basic_rc = rc<T, std::size_t>;

template<class C, class... Is> struct downcast_to;

template<class C, class I, class... Is> struct downcast_to<C, I, Is...>
        : public downcast_to<C, Is...> {
    i_miniobj* downcast(const char* iname) noexcept
    {
        if (std::strcmp(minicom::interface_name<I>, iname) == 0) {
            auto p = static_cast<I*>(static_cast<C*>(this));
            p->add_ref();
            return p;
        }

        return downcast_to<C, Is...>::downcast(iname);
    }
};

template<class C> struct downcast_to<C> {
    i_miniobj* downcast(const char*) noexcept { return nullptr; }
};

MINIEXPORTDEF(i_miniobj*, minicom_factory_t, const char*);

template<class... Cs> struct factory_for;

template<class C, class... Cs> struct factory_for<C, Cs...> {
    static i_miniobj* construct(const char* klass) noexcept
    {
        TRY_CONSTRUCT(C, klass);
        return factory_for<Cs...>::construct(klass);
    }
};

template<> struct factory_for<> {
    static i_miniobj* construct(const char*) noexcept { return nullptr; }
};

class module {
  public:
    module(const char* path) noexcept
        : dll_(), factory_()
    {
        if (auto hmodule = LoadLibrary(path)) {
            dll_ = std::shared_ptr<std::remove_pointer_t<HMODULE>>(
                    hmodule, &FreeLibrary);
            factory_ = reinterpret_cast<minicom::minicom_factory_t>(
                    GetProcAddress(dll_.get(), "minicom_factory"));
            if (!factory_)
                dll_ = nullptr;
        }
    }

    explicit operator bool() noexcept { return static_cast<bool>(dll_); }

    minicom_factory_t factory() noexcept { return factory_; }

    i_miniobj* make_class(const char* klass) noexcept
    {
        if (!factory_)
            return nullptr;
        return factory_(klass);
    }

    template<class I>
    I* make_class_as(const char* klass) noexcept
    {
        auto obj = make_class(klass);
        if (!obj)
            return nullptr;
        auto p = downcast<I>(obj);
        obj->release_ref();
        return p;
    }

  private:
    std::shared_ptr<std::remove_pointer_t<HMODULE>> dll_;
    minicom_factory_t factory_;
};

}

REGISTER_INTERFACE(minicom::i_miniobj);

#define MINICOM_H
#endif
