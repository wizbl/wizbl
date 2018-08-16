#ifndef WIZBLCOIN_EQUIHASH_H
#define WIZBLCOIN_EQUIHASH_H

#include "crypto/sha_wbl.h"
#include "compat/endian.h"

#include "sodium.h"

#include <exception>
#include <cstring>
#include <memory>
#include <functional>
#include <vector>
#include <set>
#include <boost/static_assert.hpp>

#include "../util/str_encodings.h"

typedef uint8_t eh_trunc;
typedef uint32_t eh_index;
typedef crypto_generichash_blake2b_state eh_HashState;

template<unsigned int N, unsigned int K>
class Equihash
{
private:
    BOOST_STATIC_ASSERT(K < N);
    BOOST_STATIC_ASSERT(N % 8 == 0);
    BOOST_STATIC_ASSERT((N/(K+1)) + 1 < 8*sizeof(eh_index));

public:
    enum : size_t { IndexesPerHashOutput=512/N };
    enum : size_t { HashOutput=IndexesPerHashOutput*N/8 };
    enum : size_t { LengthOfCollisionBit=N/(K+1) };
    enum : size_t { LengthOfCollisionByte=(LengthOfCollisionBit+7)/8 };
    enum : size_t { HashLength=(K+1)*LengthOfCollisionByte };
    enum : size_t { FullWidth=2*LengthOfCollisionByte+sizeof(eh_index)*(1 << (K-1)) };
    enum : size_t { FinalFullWidth=2*LengthOfCollisionByte+sizeof(eh_index)*(1 << (K)) };
    enum : size_t { TruncatedWidth=max(HashLength+sizeof(eh_trunc), 2*LengthOfCollisionByte+sizeof(eh_trunc)*(1 << (K-1))) };
    enum : size_t { FinalTruncatedWidth=max(HashLength+sizeof(eh_trunc), 2*LengthOfCollisionByte+sizeof(eh_trunc)*(1 << (K))) };
    enum : size_t { SolutionWidth=(1 << K)*(LengthOfCollisionBit+1)/8 };

    Equihash() { }

    int InitializeState(eh_HashState& base_state, bool wbl_salt);
    bool BasicSolution(const eh_HashState& base_state,
                    const std::function<bool(std::vector<unsigned char>)> validBlock,
                    const std::function<bool(EhSolverCancelCheck)> cancelled);
    bool OptimisedSolution(const eh_HashState& base_state,
                        const std::function<bool(std::vector<unsigned char>)> validBlock,
                        const std::function<bool(EhSolverCancelCheck)> cancelled);
    bool IsValidSolution(const eh_HashState& base_state, std::vector<unsigned char> soln);
};

#include "equihash.tcc"

static Equihash<200,9> Eh200_9;
static Equihash<144,5> Eh144_5;
static Equihash<48,5> Eh48_5;
static Equihash<96,5> Eh96_5;
static Equihash<96,3> Eh96_3;

inline unsigned int EhSolutionWidth(int n, int k)
{
    unsigned int ret;
    if (n == 96 && k == 3) {
        ret = Eh96_3.SolutionWidth;
    } else if (n == 200 && k == 9) {
        ret = Eh200_9.SolutionWidth;
    } else if (n == 144 && k == 5) {
        ret = Eh144_5.SolutionWidth;
    } else if (n == 96 && k == 5) {
        ret = Eh96_5.SolutionWidth;
    } else if (n == 48 && k == 5) {
        ret = Eh48_5.SolutionWidth;
    } else {
        throw std::invalid_argument("Unsupported Equihash parameters");
    }
    return ret;
}

#define EhIsValidSolution(n, k, base_state, soln, ret)   \
    if (n == 96 && k == 3) {                             \
        ret = Eh96_3.IsValidSolution(base_state, soln);  \
    } else if (n == 200 && k == 9) {                     \
        ret = Eh200_9.IsValidSolution(base_state, soln); \
    } else if (n == 144 && k == 5) {                     \
        ret = Eh144_5.IsValidSolution(base_state, soln); \
    } else if (n == 96 && k == 5) {                      \
        ret = Eh96_5.IsValidSolution(base_state, soln);  \
    } else if (n == 48 && k == 5) {                      \
        ret = Eh48_5.IsValidSolution(base_state, soln);  \
    } else {                                             \
        throw std::invalid_argument("Unsupported Equihash parameters"); \
    }

inline bool EhOptimisedSolutionUncancellable(unsigned int n, unsigned int k, const eh_HashState& base_state,
                    const std::function<bool(std::vector<unsigned char>)> validBlock)
{
    return EhOptimisedSolution(n, k, base_state, validBlock,
                            [](EhSolverCancelCheck pos) { return false; });
}

inline bool EhOptimisedSolution(unsigned int n, unsigned int k, const eh_HashState& base_state,
                    const std::function<bool(std::vector<unsigned char>)> validBlock,
                    const std::function<bool(EhSolverCancelCheck)> cancelled)
{
    if (n == 96 && k == 3) {
        return Eh96_3.OptimisedSolution(base_state, validBlock, cancelled);
    } else if (n == 200 && k == 9) {
        return Eh200_9.OptimisedSolution(base_state, validBlock, cancelled);
    } else if (n == 144 && k == 5) {
        return Eh144_5.OptimisedSolution(base_state, validBlock, cancelled);
    } else if (n == 96 && k == 5) {
        return Eh96_5.OptimisedSolution(base_state, validBlock, cancelled);
    } else if (n == 48 && k == 5) {
        return Eh48_5.OptimisedSolution(base_state, validBlock, cancelled);
    } else {
        throw std::invalid_argument("Unsupported Equihash parameters");
    }
}

inline bool EhBasicSolutionUncancellable(unsigned int n, unsigned int k, const eh_HashState& base_state,
                    const std::function<bool(std::vector<unsigned char>)> validBlock)
{
    return EhBasicSolution(n, k, base_state, validBlock,
                        [](EhSolverCancelCheck pos) { return false; });
}

inline bool EhBasicSolution(unsigned int n, unsigned int k, const eh_HashState& base_state,
                    const std::function<bool(std::vector<unsigned char>)> validBlock,
                    const std::function<bool(EhSolverCancelCheck)> cancelled)
{
    if (n == 96 && k == 3) {
        return Eh96_3.BasicSolution(base_state, validBlock, cancelled);
    } else if (n == 200 && k == 9) {
        return Eh200_9.BasicSolution(base_state, validBlock, cancelled);
    } else if (n == 144 && k == 5) {
        return Eh144_5.BasicSolution(base_state, validBlock, cancelled);
    } else if (n == 96 && k == 5) {
        return Eh96_5.BasicSolution(base_state, validBlock, cancelled);
    } else if (n == 48 && k == 5) {
        return Eh48_5.BasicSolution(base_state, validBlock, cancelled);
    } else {
        throw std::invalid_argument("Unsupported Equihash parameters");
    }
}

#define EhInitializeState(n, k, base_state, wbl_salt)  \
    if (n == 96 && k == 3) {                           \
        Eh96_3.InitializeState(base_state, wbl_salt);  \
    } else if (n == 200 && k == 9) {                   \
        Eh200_9.InitializeState(base_state, wbl_salt); \
    } else if (n == 144 && k == 5) {                   \
        Eh144_5.InitializeState(base_state, wbl_salt); \
    } else if (n == 96 && k == 5) {                    \
        Eh96_5.InitializeState(base_state, wbl_salt);  \
    } else if (n == 48 && k == 5) {                    \
        Eh48_5.InitializeState(base_state, wbl_salt);  \
    } else {                                           \
        throw std::invalid_argument("Unsupported Equihash parameters"); \
    }

inline constexpr size_t equihash_solution_size(unsigned int N, unsigned int K) {
    return (1 << K)*(N/(K+1)+1)/8;
}

inline constexpr size_t max(const size_t A, const size_t B) { return A > B ? A : B; }

class EhSolverCancelledException : public std::exception
{
    virtual const char* what() const throw() {
        return "Equihash solver was cancelled";
    }
};

enum EhSolverCancelCheck
{
    ListGeneration,
    ListSorting,
    ListColliding,
    RoundEnd,
    FinalSorting,
    FinalColliding,
    PartialGeneration,
    PartialSorting,
    PartialSubtreeEnd,
    PartialIndexEnd,
    PartialEnd
};

template<size_t WIDTH>
class TruncatedStepRow : public StepRow<WIDTH>
{
    template<size_t W>
    friend class TruncatedStepRow;

    using StepRow<WIDTH>::hash;

public:
    TruncatedStepRow(const unsigned char* hashIn, size_t hInLen,
                     size_t hLen, size_t cBitLen,
                     eh_index i, unsigned int ilen);
    ~TruncatedStepRow() { }

    TruncatedStepRow(const TruncatedStepRow<WIDTH>& a) : StepRow<WIDTH> {a} { }
    template<size_t W>
    TruncatedStepRow(const TruncatedStepRow<W>& a, const TruncatedStepRow<W>& b, size_t len, size_t lenIndices, size_t trim);
    TruncatedStepRow& operator=(const TruncatedStepRow<WIDTH>& a);

    inline bool IndicesBefore(const TruncatedStepRow<WIDTH>& a, size_t len, size_t lenIndices) const { return memcmp(hash+len, a.hash+len, lenIndices) < 0; }
    std::shared_ptr<eh_trunc> GetTruncatedIndices(size_t len, size_t lenIndices) const;
};

template<size_t WIDTH>
class FullStepRow : public StepRow<WIDTH>
{
    template<size_t W>
    friend class FullStepRow;

    using StepRow<WIDTH>::hash;

public:
    FullStepRow(const unsigned char* hashIn, size_t hInLen,
                size_t hLen, size_t cBitLen, eh_index i);
    ~FullStepRow() { }

    FullStepRow(const FullStepRow<WIDTH>& a) : StepRow<WIDTH> {a} { }
    template<size_t W>
    FullStepRow(const FullStepRow<W>& a, const FullStepRow<W>& b, size_t len, size_t lenIndices, size_t trim);
    FullStepRow& operator=(const FullStepRow<WIDTH>& a);

    inline bool IndicesBefore(const FullStepRow<WIDTH>& a, size_t len, size_t lenIndices) const { return memcmp(hash+len, a.hash+len, lenIndices) < 0; }
    std::vector<unsigned char> GetIndices(size_t len, size_t lenIndices,
                                          size_t cBitLen) const;

    template<size_t W>
    friend bool DistinctIndices(const FullStepRow<W>& a, const FullStepRow<W>& b,
                                size_t len, size_t lenIndices);
    template<size_t W>
    friend bool IsValidBranch(const FullStepRow<W>& a, const size_t len, const unsigned int ilen, const eh_trunc t);
};

template<size_t WIDTH>
bool HasCollision(StepRow<WIDTH>& a, StepRow<WIDTH>& b, size_t l);

class CompareSR
{
private:
    size_t len;

public:
    CompareSR(size_t l) : len {l} { }

    template<size_t W>
    inline bool operator()(const StepRow<W>& a, const StepRow<W>& b) { return memcmp(a.hash, b.hash, len) < 0; }
};

template<size_t WIDTH>
class StepRow
{
    template<size_t W>
    friend class StepRow;
    friend class CompareSR;

protected:
    unsigned char hash[WIDTH];

public:
    StepRow(const unsigned char* hashIn, size_t hInLen,
            size_t hLen, size_t cBitLen);
    ~StepRow() { }

    template<size_t W>
    StepRow(const StepRow<W>& a);

    bool IsZero(size_t len);
    std::string GetHex(size_t len) { return HexStr(hash, hash+len); }

    template<size_t W>
    friend bool HasCollision(StepRow<W>& a, StepRow<W>& b, size_t l);
};

std::vector<unsigned char> GetMinimalFromIndices(std::vector<eh_index> indices,
                                                 size_t cBitLen);

std::vector<eh_index> GetIndicesFromMinimal(std::vector<unsigned char> minimal,
                                            size_t cBitLen);

eh_trunc TruncateIndex(const eh_index i, const unsigned int ilen);

eh_index ArrayToEhIndex(const unsigned char* array);

void CompressArray(const unsigned char* in, size_t in_len,
                   unsigned char* out, size_t out_len,
                   size_t bit_len, size_t byte_pad=0);

void ExpandArray(const unsigned char* in, size_t in_len,
                 unsigned char* out, size_t out_len,
                 size_t bit_len, size_t byte_pad=0);

#endif // WIZBLCOIN_EQUIHASH_H
                                                                                                                                                                                                               