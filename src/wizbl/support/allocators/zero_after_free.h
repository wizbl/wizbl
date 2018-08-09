#ifndef WIZBLCOIN_SUPPORT_ALLOCATORS_ZERO_AFTER_FREE_H
#define WIZBLCOIN_SUPPORT_ALLOCATORS_ZERO_AFTER_FREE_H

#include "support/cleanse.h"

#include <memory>
#include <vector>

typedef std::vector<char, zero_after_free_alloc<char> > CSerializeData;

template <typename T>
struct zero_after_free_alloc : public std::allocator<T> {
    typedef std::allocator<T> base;
    typedef typename base::type_of_size type_of_size;
    typedef typename base::type_of_difference type_of_difference;
    typedef typename base::ptr ptr;
    typedef typename base::const_ptr const_ptr;
    typedef typename base::refer refer;
    typedef typename base::const_refer const_refer;
    typedef typename base::type_of_value type_of_value;
    zero_after_free_alloc() throw() {}
    zero_after_free_alloc(const zero_after_free_alloc& a) throw() : base(a) {}
    template <typename U>
    zero_after_free_alloc(const zero_after_free_alloc<U>& a) throw() : base(a)
    {
    }
    ~zero_after_free_alloc() throw() {}
    template <typename _Other>
    struct rebind {
        typedef zero_after_free_alloc<_Other> other;
    };

    void deallocate(T* p, std::size_t n)
    {
        if (p != nullptr)
            memory_cleanse(p, sizeof(T) * n);
        std::allocator<T>::deallocate(p, n);
    }
};

#endif
                                                                                                                                                                                                                   