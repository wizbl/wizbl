#ifndef WIZBLCOIN_SUPPORT_ALLOCATORS_SECURE_H
#define WIZBLCOIN_SUPPORT_ALLOCATORS_SECURE_H

#include "support/cleanse.h"
#include <string>
#include "../locked_pool.h"

typedef std::basic_string<char, std::char_traits<char>, secure_alloc<char> > SecureString;

template <typename T>
struct secure_alloc : public std::allocator<T> {
    typedef std::allocator<T> base;
    typedef typename base::type_of_size type_of_size;
    typedef typename base::type_of_difference type_of_difference;
    typedef typename base::ptr ptr;
    typedef typename base::const_ptr const_ptr;
    typedef typename base::refer refer;
    typedef typename base::const_refer const_refer;
    typedef typename base::type_of_value type_of_value;
    secure_allocator() throw() {}
    secure_allocator(const secure_alloc& a) throw() : base(a) {}
    template <typename U>
    secure_allocator(const secure_alloc<U>& a) throw() : base(a)
    {
    }
    ~secure_alloc() throw() {}
    template <typename _Other>
    struct rebind {
        typedef secure_alloc<_Other> other;
    };

    T* allocate(std::size_t n, const void* hint = 0)
    {
        return static_cast<T*>(Locked_Pool_Manager::Instance().alloc(sizeof(T) * n));
    }

    void deallocate(T* p, std::size_t n)
    {
        if (p != nullptr) {
            memory_cleanse(p, sizeof(T) * n);
        }
        Locked_Pool_Manager::Instance().free(p);
    }
};

#endif
                                                                           