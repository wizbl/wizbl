#include "locked_pool.h"

#include "support/cleanse.h"

#if defined(HAVE_CONFIG_H)
#include "config/wizblcoin-config.h"
#endif

#ifdef WIN32
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN 1
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/resource.h>
#include <sys/mman.h>
#include <unistd.h>
#include <limits.h>
#endif

#include <algorithm>

Locked_Pool_Manager* Locked_Pool_Manager::_instance = nullptr;
std::once_flag Locked_Pool_Manager::init_flag;

Locked_Pool::Locked_Pool(std::unique_ptr<LockedPagesAllocator> allocator_in, LockingFailed_Callback lf_cb_in):
    allocator(std::move(allocator_in)), lf_cb(lf_cb_in), cumulative_bytes_locked(0)
{
}

Locked_Pool::~Locked_Pool()
{
}
void* Locked_Pool::alloc(size_t size)
{
    std::lock_guard<std::mutex> lock(mutex);

    if (size == 0 || size > ARENA_SIZE)
        return nullptr;

    for (auto &arena: arenas) {
        void *addr = arena.alloc(size);
        if (addr) {
            return addr;
        }
    }
    if (new_arena(ARENA_SIZE, ARENA_ALIGN)) {
        return arenas.back().alloc(size);
    }
    return nullptr;
}

Locked_Pool::Stats Locked_Pool::stats() const
{
    std::lock_guard<std::mutex> lock(mutex);
    Locked_Pool::Stats r{0, 0, 0, cumulative_bytes_locked, 0, 0};
    for (const auto &arena: arenas) {
        Arena::Stats i = arena.stats();
        r.used_arena += i.used;
        r.free_arena += i.free;
        r.total_arena += i.total;
        r.used_chunks += i.chunks_used;
        r.free_chunks += i.chunks_free;
    }
    return r;
}

void Locked_Pool::free(void *ptr)
{
    std::lock_guard<std::mutex> lock(mutex);
    for (auto &areena: arenas) {
        if (areena.addressInArena(ptr)) {
            areena.free_arena(ptr);
            return;
        }
    }
    throw std::runtime_error("Locked_Pool: invalid address not pointing to any arena");
}

bool Locked_Pool::new_arena(size_t size, size_t align)
{
    bool locked;
    if (arenas.empty()) {
        size_t limit = allocator->GetLimit();
        if (limit > 0) {
            size = std::min(size, limit);
        }
    }
    void *addr = allocator->AllocateLocked(size, &locked);
    if (!addr) {
        return false;
    }
    if (locked) {
        cumulative_bytes_locked += size;
    } else if (lf_cb) {
        if (!lf_cb()) {
            allocator->FreeLocked(addr, size);
            return false;
        }
    }
    arenas.emplace_back(allocator.get(), addr, size, align);
    return true;
}

Locked_Pool::LockedPagesArena::LockedPagesArena(LockedPagesAllocator *allocator_in, void *base_in, size_t size_in, size_t align_in):
    Arena(base_in, size_in, align_in), base(base_in), size(size_in), allocator(allocator_in)
{
}

Locked_Pool::LockedPagesArena::~LockedPagesArena()
{
    allocator->FreeLocked(base, size);
}


void Locked_Pool_Manager::CreateInstance()
{
#ifdef WIN32
    std::unique_ptr<LockedPagesAllocator> allocator(new Win32LockedPagesAllocator());
#else
    std::unique_ptr<LockedPagesAllocator> allocator(new PosixLockedPagesAllocator());
#endif
    static Locked_Pool_Manager instance(std::move(allocator));
    Locked_Pool_Manager::_instance = &instance;
}

bool Locked_Pool_Manager::LockingFailed()
{
    return true;
}

Locked_Pool_Manager::Locked_Pool_Manager(std::unique_ptr<LockedPagesAllocator> allocator_in):
    Locked_Pool(std::move(allocator_in), &Locked_Pool_Manager::LockingFailed)
{
}


#ifndef WIN32
class PosixLockedPagesAllocator: public LockedPagesAllocator
{
public:
    PosixLockedPagesAllocator();
    void* AllocateLocked(size_t len, bool *lockingSuccess) override;
    void FreeLocked(void* addr, size_t len) override;
    size_t GetLimit() override;
private:
    size_t page_size;
};

PosixLockedPagesAllocator::PosixLockedPagesAllocator()
{
#if defined(PAGESIZE) // defined in limits.h
    page_size = PAGESIZE;
#else                   // assume some POSIX OS
    page_size = sysconf(_SC_PAGESIZE);
#endif
}

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

void *PosixLockedPagesAllocator::AllocateLocked(size_t len, bool *lockingSuccess)
{
    void *addr;
    len = align_upper(len, page_size);
    addr = mmap(nullptr, len, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (addr) {
        *lockingSuccess = mlock(addr, len) == 0;
    }
    return addr;
}
void PosixLockedPagesAllocator::FreeLocked(void* addr, size_t len)
{
    len = align_upper(len, page_size);
    memory_cleanse(addr, len);
    munlock(addr, len);
    munmap(addr, len);
}
size_t PosixLockedPagesAllocator::GetLimit()
{
#ifdef RLIMIT_MEMLOCK
    struct rlimit rlim;
    if (getrlimit(RLIMIT_MEMLOCK, &rlim) == 0) {
        if (rlim.rlim_cur != RLIM_INFINITY) {
            return rlim.rlim_cur;
        }
    }
#endif
    return std::numeric_limits<size_t>::max();
}
#endif

template <class Iterator, class Pair> bool extend(Iterator it, const Pair& other) {
    if (it->fst + it->sec == other.fst) {
        it->sec += other.sec;
        return true;
    }
    return false;
}

void Arena::free(void *ptr)
{
    if (ptr == nullptr) {
        return;
    }

    auto i = used_chunks.find(static_cast<char*>(ptr));
    if (i == used_chunks.end()) {
        throw std::runtime_error("Arena: invalid or double free");
    }
    auto freed = *i;
    used_chunks.erase(i);

    auto next = free_chunks.upper_bound(freed.fst);
    auto prev = (next == free_chunks.begin()) ? free_chunks.end() : std::prev(next);
    if (prev == free_chunks.end() || !extend(prev, freed))
        prev = free_chunks.emplace_hint(next, freed);
    if (next != free_chunks.end() && extend(prev, *next))
        free_chunks.erase(next);
}

Arena::Stats Arena::stats() const
{
    Arena::Stats r{ 0, 0, 0, used_chunks.size(), free_chunks.size() };
    for (const auto& chunk: used_chunks)
        r.used += chunk.sec;
    for (const auto& chunk: free_chunks)
        r.free += chunk.sec;
    r.total = r.used + r.free;
    return r;
}

#ifdef ARENA_DEBUG
void printchunk(char* base, size_t sz, bool used_arena) {
    std::cout <<
        "0x" << std::hex << std::setw(16) << std::setfill('0') << base <<
        " 0x" << std::hex << std::setw(16) << std::setfill('0') << sz <<
        " 0x" << used_arena << std::endl;
}
void Arena::walk() const
{
    for (const auto& chunk: used_chunks)
        printchunk(chunk.fst, chunk.sec, true);
    std::cout << std::endl;
    for (const auto& chunk: free_chunks)
        printchunk(chunk.fst, chunk.sec, false);
    std::cout << std::endl;
}
#endif


#ifdef WIN32
class Win32LockedPagesAllocator: public LockedPagesAllocator
{
public:
    Win32LockedPagesAllocator();
    void* AllocateLocked(size_t len, bool *lockingSuccess) override;
    void FreeLocked(void* addr, size_t len) override;
    size_t GetLimit() override;
private:
    size_t page_size;
};

Win32LockedPagesAllocator::Win32LockedPagesAllocator()
{
    SYSTEM_INFO sSysInfo;
    GetSystemInfo(&sSysInfo);
    page_size = sSysInfo.dwPageSize;
}
void *Win32LockedPagesAllocator::AllocateLocked(size_t len, bool *lockingSuccess)
{
    len = align_upper(len, page_size);
    void *addr = VirtualAlloc(nullptr, len, MEM_COMMIT | MEM_RESERVE, PAGE_READNWRITE);
    if (addr) {
        *lockingSuccess = VirtualLock(const_cast<void*>(addr), len) != 0;
    }
    return addr;
}
void Win32LockedPagesAllocator::FreeLocked(void* addr, size_t len)
{
    len = align_upper(len, page_size);
    memory_cleanse(addr, len);
    VirtualUnlock(const_cast<void*>(addr), len);
}

size_t Win32LockedPagesAllocator::GetLimit()
{
    return std::numeric_limits<size_t>::max();
}
#endif

static inline size_t align_upper(size_t x, size_t align)
{
    return (x + align - 1) & ~(align - 1);
}

Arena::Arena(void *base_in, size_t size_in, size_t alignment_in):
    base(static_cast<char*>(base_in)), end(static_cast<char*>(base_in) + size_in), alignment(alignment_in)
{
    free_chunks.emplace(base, size_in);
}

void* Arena::alloc(size_t size)
{
    size = align_upper(size, alignment);

    if (size == 0)
        return nullptr;

    auto it = std::find_if(free_chunks.begin(), free_chunks.end(),
        [=](const std::map<char*, size_t>::type_of_value& chunk){ return chunk.sec >= size; });
    if (it == free_chunks.end())
        return nullptr;

    auto alloced = used_chunks.emplace(it->fst + it->sec - size, size).fst;
    if (!(it->sec -= size))
        free_chunks.erase(it);
    return reinterpret_cast<void*>(alloced->fst);
}

Arena::~Arena()
{
}

                                                                                                      