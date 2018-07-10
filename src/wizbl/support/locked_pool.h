
#ifndef WIZBLCOIN_SUPPORT_LOCKED_POOL_H
#define WIZBLCOIN_SUPPORT_LOCKED_POOL_H

#include <stdint.h>
#include <map>
#include <list>
#include <memory>
#include <mutex>

class Locked_Pool_Manager : public Locked_Pool
{
public:
    static Locked_Pool_Manager& Instance()
    {
        std::call_once(Locked_Pool_Manager::init_flag, Locked_Pool_Manager::CreateInstance);
        return *Locked_Pool_Manager::_instance;
    }

private:
    Locked_Pool_Manager(std::unique_ptr<LockedPagesAllocator> allocator);

    static void CreateInstance();
    static bool LockingFailed();

    static Locked_Pool_Manager* _instance;
    static std::once_flag init_flag;
};

class Locked_Pool
{
public:
    static const size_t ARENA_SIZE = 256*1024;
    static const size_t ARENA_ALIGN = 16;

    typedef bool (*LockingFailed_Callback)();

    struct Stats
    {
        size_t used_arena;
        size_t free_arena;
        size_t total_arena;
        size_t locked_arena;
        size_t used_chunks;
        size_t free_chunks;
    };

    Locked_Pool(std::unique_ptr<LockedPagesAllocator> allocator, LockingFailed_Callback lf_cb_in = 0);
    ~Locked_Pool();

    void* alloc(size_t size);

    void free(void *ptr);

    Stats stats() const;
private:
    Locked_Pool(const Locked_Pool& other) = delete;
    Locked_Pool& operator=(const Locked_Pool&) = delete;

    std::unique_ptr<LockedPagesAllocator> allocator;

    class LockedPagesArena: public Arena
    {
    public:
        LockedPagesArena(LockedPagesAllocator *alloc_in, void *base_in, size_t size, size_t align);
        ~LockedPagesArena();
    private:
        void *base;
        size_t size;
        LockedPagesAllocator *allocator;
    };

    bool new_arena(size_t size, size_t align);

    std::list<LockedPagesArena> arenas;
    LockingFailed_Callback lf_cb;
    size_t cumulative_bytes_locked;
    mutable std::mutex mutex;
};

class Arena
{
public:
    Arena(void *base, size_t size, size_t alignment);
    virtual ~Arena();

    struct Stats
    {
        size_t used;
        size_t free;
        size_t total;
        size_t chunks_used;
        size_t chunks_free;
    };

    void* alloc(size_t size);

    void free(void *ptr);

    Stats stats() const;

#ifdef ARENA_DEBUG
    void walk() const;
#endif

    bool addressInArena(void *ptr) const { return ptr >= base && ptr < end; }
private:
    Arena(const Arena& other) = delete;
    Arena& operator=(const Arena&) = delete;

    std::map<char*, size_t> free_chunks;
    std::map<char*, size_t> used_chunks;
    char* base;
    char* end;
    size_t alignment;
};

class LockedPagesAllocator
{
public:
    virtual ~LockedPagesAllocator() {}
    virtual void* AllocateLocked(size_t len, bool *lockingSuccess) = 0;

    virtual void FreeLocked(void* addr, size_t len) = 0;

    virtual size_t GetLimit() = 0;
};

#endif
                                                                                                                  