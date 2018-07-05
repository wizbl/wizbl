#if defined(HAVE_CONFIG_H)
#include "config/wizblcoin-config.h"
#endif

#include <stdexcept>
#include <algorithm>
#include <iostream>

#include <boost/optional.hpp>

#ifndef NO_UTIL_LOG
#include "util.h"
#else
#endif

#include "crypto/equihash.h"

EhSolverCancelledException solver_cancelled;

template bool Equihash<48,5>::IsValidSolution(const eh_HashState& base_state, std::vector<unsigned char> soln);

template bool Equihash<48,5>::OptimisedSolution(const eh_HashState& base_state,
                                             const std::function<bool(std::vector<unsigned char>)> validBlock,
                                             const std::function<bool(EhSolverCancelCheck)> cancelled);

template bool Equihash<48,5>::BasicSolution(const eh_HashState& base_state,
                                         const std::function<bool(std::vector<unsigned char>)> validBlock,
                                         const std::function<bool(EhSolverCancelCheck)> cancelled);

template int Equihash<48,5>::InitializeState(eh_HashState& base_state, bool wbl_salt);

template bool Equihash<96,5>::IsValidSolution(const eh_HashState& base_state, std::vector<unsigned char> soln);

template bool Equihash<96,5>::OptimisedSolution(const eh_HashState& base_state,
                                             const std::function<bool(std::vector<unsigned char>)> validBlock,
                                             const std::function<bool(EhSolverCancelCheck)> cancelled);

template bool Equihash<96,5>::BasicSolution(const eh_HashState& base_state,
                                         const std::function<bool(std::vector<unsigned char>)> validBlock,
                                         const std::function<bool(EhSolverCancelCheck)> cancelled);

template int Equihash<96,5>::InitializeState(eh_HashState& base_state, bool wbl_salt);

template bool Equihash<144,5>::IsValidSolution(const eh_HashState& base_state, std::vector<unsigned char> soln);

template bool Equihash<144,5>::OptimisedSolution(const eh_HashState& base_state,
                                              const std::function<bool(std::vector<unsigned char>)> validBlock,
                                              const std::function<bool(EhSolverCancelCheck)> cancelled);

template bool Equihash<144,5>::BasicSolution(const eh_HashState& base_state,
                                          const std::function<bool(std::vector<unsigned char>)> validBlock,
                                          const std::function<bool(EhSolverCancelCheck)> cancelled);

template int Equihash<144,5>::InitializeState(eh_HashState& base_state, bool wbl_salt);

template bool Equihash<200,9>::IsValidSolution(const eh_HashState& base_state, std::vector<unsigned char> soln);

template bool Equihash<200,9>::OptimisedSolution(const eh_HashState& base_state,
                                              const std::function<bool(std::vector<unsigned char>)> validBlock,
                                              const std::function<bool(EhSolverCancelCheck)> cancelled);

template int Equihash<200,9>::InitializeState(eh_HashState& base_state, bool wbl_salt);
template bool Equihash<200,9>::BasicSolution(const eh_HashState& base_state,
                                          const std::function<bool(std::vector<unsigned char>)> validBlock,
                                          const std::function<bool(EhSolverCancelCheck)> cancelled);

template bool Equihash<96,3>::IsValidSolution(const eh_HashState& base_state, std::vector<unsigned char> soln);

template bool Equihash<96,3>::OptimisedSolution(const eh_HashState& base_state,
                                             const std::function<bool(std::vector<unsigned char>)> validBlock,
                                             const std::function<bool(EhSolverCancelCheck)> cancelled);

template bool Equihash<96,3>::BasicSolution(const eh_HashState& base_state,
                                         const std::function<bool(std::vector<unsigned char>)> validBlock,
                                         const std::function<bool(EhSolverCancelCheck)> cancelled);

template int Equihash<96,3>::InitializeState(eh_HashState& base_state, bool wbl_salt);

template<unsigned int N, unsigned int K>
bool Equihash<N,K>::IsValidSolution(const eh_HashState& base_state, std::vector<unsigned char> soln)
{
    if (soln.size() != SolutionWidth) {
        return false;
    }

    std::vector<FullStepRow<FinalFullWidth>> X;
    X.reserve(1 << K);
    unsigned char tmpHash[HashOutput];
    for (eh_index i : GetIndicesFromMinimal(soln, LengthOfCollisionBit)) {
        GenerateHash(base_state, i/IndexesPerHashOutput, tmpHash, HashOutput);
        X.emplace_back(tmpHash+((i % IndexesPerHashOutput) * N/8),
                       N/8, HashLength, LengthOfCollisionBit, i);
    }

    size_t hashLen = HashLength;
    size_t lenIndices = sizeof(eh_index);
    while (X.size() > 1) {
        std::vector<FullStepRow<FinalFullWidth>> Xc;
        for (size_t i = 0; i < X.size(); i += 2) {
            if (!HasCollision(X[i], X[i+1], LengthOfCollisionByte)) {
                return false;
            }
            if (X[i+1].IndicesBefore(X[i], hashLen, lenIndices)) {
                return false;
            }
            if (!DistinctIndices(X[i], X[i+1], hashLen, lenIndices)) {
                return false;
            }
            Xc.emplace_back(X[i], X[i+1], hashLen, lenIndices, LengthOfCollisionByte);
        }
        X = Xc;
        hashLen -= LengthOfCollisionByte;
        lenIndices *= 2;
    }

    assert(X.size() == 1);
    return X[0].IsZero(hashLen);
}

template<unsigned int N, unsigned int K>
bool Equihash<N,K>::OptimisedSolution(const eh_HashState& base_state,
                                   const std::function<bool(std::vector<unsigned char>)> blockValid,
                                   const std::function<bool(EhSolverCancelCheck)> cancelled)
{
    eh_index initial_size { 1 << (LengthOfCollisionBit + 1) };
    eh_index recreation_size { UntruncateIndex(1, 0, LengthOfCollisionBit + 1) };

    const eh_index soln_size { 1 << K };
    std::vector<std::shared_ptr<eh_trunc>> partialSolns;
    size_t invalidCount = 0;
    {

        size_t hashLength = HashLength;
        size_t lenIndexes = sizeof(eh_trunc);
        std::vector<TruncatedStepRow<TruncatedWidth>> Xt;
        Xt.reserve(initial_size);
        unsigned char tmpHash[HashOutput];
        for (eh_index g = 0; Xt.size() < initial_size; g++) {
            GenerateHash(base_state, g, tmpHash, HashOutput);
            for (eh_index i = 0; i < IndexesPerHashOutput && Xt.size() < initial_size; i++) {
                Xt.emplace_back(tmpHash+(i*N/8), N/8, HashLength, LengthOfCollisionBit,
                                (g*IndexesPerHashOutput)+i, LengthOfCollisionBit + 1);
            }
            if (cancelled(ListGeneration)) throw solver_cancelled;
        }

        for (size_t r = 1; r < K && Xt.size() > 0; r++) {
            std::sort(Xt.begin(), Xt.end(), CompareSR(LengthOfCollisionByte));
            if (cancelled(ListSorting)) throw solver_cancelled;

            size_t i = 0;
            size_t free = 0;
            std::vector<TruncatedStepRow<TruncatedWidth>> Xc;
            while (i < Xt.size() - 1) {
                size_t j = 1;
                while (i+j < Xt.size() &&
                        HasCollision(Xt[i], Xt[i+j], LengthOfCollisionByte)) {
                    j++;
                }

                for (size_t l = 0; l < j - 1; l++) {
                    for (size_t m = l + 1; m < j; m++) {
                        TruncatedStepRow<TruncatedWidth> Xi {Xt[i+l], Xt[i+m],
                                                             hashLength, lenIndexes,
                                                             LengthOfCollisionByte};
                        if (!(Xi.IsZero(hashLength-LengthOfCollisionByte) &&
                              IsProbablyDuplicate<soln_size>(Xi.GetTruncatedIndices(hashLength-LengthOfCollisionByte, 2*lenIndexes),
                                                             2*lenIndexes))) {
                            Xc.emplace_back(Xi);
                        }
                    }
                }

                while (free < i+j && Xc.size() > 0) {
                    Xt[free++] = Xc.back();
                    Xc.pop_back();
                }

                i += j;
                if (cancelled(ListColliding)) throw solver_cancelled;
            }

            while (free < Xt.size() && Xc.size() > 0) {
                Xt[free++] = Xc.back();
                Xc.pop_back();
            }

            if (Xc.size() > 0) {
                Xt.insert(Xt.end(), Xc.begin(), Xc.end());
            } else if (free < Xt.size()) {
                Xt.erase(Xt.begin()+free, Xt.end());
                Xt.shrink_to_fit();
            }

            hashLength -= LengthOfCollisionByte;
            lenIndexes *= 2;
            if (cancelled(RoundEnd)) throw solver_cancelled;
        }

        if (Xt.size() > 1) {
            std::sort(Xt.begin(), Xt.end(), CompareSR(hashLength));
            if (cancelled(FinalSorting)) throw solver_cancelled;
            size_t i = 0;
            while (i < Xt.size() - 1) {
                size_t j = 1;
                while (i+j < Xt.size() &&
                        HasCollision(Xt[i], Xt[i+j], hashLength)) {
                    j++;
                }

                for (size_t l = 0; l < j - 1; l++) {
                    for (size_t m = l + 1; m < j; m++) {
                        TruncatedStepRow<FinalTruncatedWidth> res(Xt[i+l], Xt[i+m],
                                                                  hashLength, lenIndexes, 0);
                        auto soln = res.GetTruncatedIndices(hashLength, 2*lenIndexes);
                        if (!IsProbablyDuplicate<soln_size>(soln, 2*lenIndexes)) {
                            partialSolns.push_back(soln);
                        }
                    }
                }

                i += j;
                if (cancelled(FinalColliding)) throw solver_cancelled;
            }
        } else

    }

    for (std::shared_ptr<eh_trunc> partialSoln : partialSolns) {
        std::set<std::vector<unsigned char>> solns;
        size_t hashLen;
        size_t lenIndices;
        unsigned char tmpHash[HashOutput];
        std::vector<boost::optional<std::vector<FullStepRow<FinalFullWidth>>>> X;
        X.reserve(K+1);

        for (eh_index i = 0; i < soln_size; i++) {
            std::vector<FullStepRow<FinalFullWidth>> icv;
            icv.reserve(recreation_size);
            for (eh_index j = 0; j < recreation_size; j++) {
                eh_index newIndex { UntruncateIndex(partialSoln.get()[i], j, LengthOfCollisionBit + 1) };
                if (j == 0 || newIndex % IndexesPerHashOutput == 0) {
                    GenerateHash(base_state, newIndex/IndexesPerHashOutput,
                                 tmpHash, HashOutput);
                }
                icv.emplace_back(tmpHash+((newIndex % IndexesPerHashOutput) * N/8),
                                 N/8, HashLength, LengthOfCollisionBit, newIndex);
                if (cancelled(PartialGeneration)) throw solver_cancelled;
            }
            boost::optional<std::vector<FullStepRow<FinalFullWidth>>> ic = icv;

            hashLen = HashLength;
            lenIndices = sizeof(eh_index);
            size_t rti = i;
            for (size_t r = 0; r <= K; r++) {
                if (r < X.size()) {
                    if (X[r]) {
                        ic->reserve(ic->size() + X[r]->size());
                        ic->insert(ic->end(), X[r]->begin(), X[r]->end());
                        std::sort(ic->begin(), ic->end(), CompareSR(hashLen));
                        if (cancelled(PartialSorting)) throw solver_cancelled;
                        size_t lti = rti-(1<<r);
                        CollideBranches(*ic, hashLen, lenIndices,
                                        LengthOfCollisionByte,
                                        LengthOfCollisionBit + 1,
                                        partialSoln.get()[lti], partialSoln.get()[rti]);

                        if (ic->size() == 0)
                            goto invalidsolution;

                        X[r] = boost::none;
                        hashLen -= LengthOfCollisionByte;
                        lenIndices *= 2;
                        rti = lti;
                    } else {
                        X[r] = *ic;
                        break;
                    }
                } else {
                    X.push_back(ic);
                    break;
                }
                if (cancelled(PartialSubtreeEnd)) throw solver_cancelled;
            }
            if (cancelled(PartialIndexEnd)) throw solver_cancelled;
        }

        assert(X.size() == K+1);
        for (FullStepRow<FinalFullWidth> row : *X[K]) {
            auto soln = row.GetIndices(hashLen, lenIndices, LengthOfCollisionBit);
            assert(soln.size() == equihash_solution_size(N, K));
            solns.insert(soln);
        }
        for (auto soln : solns) {
            if (blockValid(soln))
                return true;
        }
        if (cancelled(PartialEnd)) throw solver_cancelled;
        continue;

invalidsolution:
        invalidCount++;
    }

    return false;
}

template<size_t WIDTH>
void CollideBranches(std::vector<FullStepRow<WIDTH>>& X, const size_t hlen, const size_t lenIndices, const unsigned int clen, const unsigned int ilen, const eh_trunc lt, const eh_trunc rt)
{
    size_t i = 0;
    size_t posFree = 0;
    std::vector<FullStepRow<WIDTH>> Xc;
    while (i < X.size() - 1) {
        size_t j = 1;
        while (i+j < X.size() &&
                HasCollision(X[i], X[i+j], clen)) {
            j++;
        }

        for (size_t l = 0; l < j - 1; l++) {
            for (size_t m = l + 1; m < j; m++) {
                if (DistinctIndices(X[i+l], X[i+m], hlen, lenIndices)) {
                    if (IsValidBranch(X[i+l], hlen, ilen, lt) && IsValidBranch(X[i+m], hlen, ilen, rt)) {
                        Xc.emplace_back(X[i+l], X[i+m], hlen, lenIndices, clen);
                    } else if (IsValidBranch(X[i+m], hlen, ilen, lt) && IsValidBranch(X[i+l], hlen, ilen, rt)) {
                        Xc.emplace_back(X[i+m], X[i+l], hlen, lenIndices, clen);
                    }
                }
            }
        }

        while (posFree < i+j && Xc.size() > 0) {
            X[posFree++] = Xc.back();
            Xc.pop_back();
        }

        i += j;
    }

    while (posFree < X.size() && Xc.size() > 0) {
        X[posFree++] = Xc.back();
        Xc.pop_back();
    }

    if (Xc.size() > 0) {
        X.insert(X.end(), Xc.begin(), Xc.end());
    } else if (posFree < X.size()) {
        X.erase(X.begin()+posFree, X.end());
        X.shrink_to_fit();
    }
}

template<unsigned int N, unsigned int K>
bool Equihash<N,K>::BasicSolution(const eh_HashState& base_state,
                               const std::function<bool(std::vector<unsigned char>)> validBlock,
                               const std::function<bool(EhSolverCancelCheck)> cancelled)
{
    eh_index init_size { 1 << (LengthOfCollisionBit + 1) };

    size_t hashLen = HashLength;
    size_t lenIndices = sizeof(eh_index);
    std::vector<FullStepRow<FullWidth>> X;
    X.reserve(init_size);
    unsigned char tmpHash[HashOutput];
    for (eh_index g = 0; X.size() < init_size; g++) {
        GenerateHash(base_state, g, tmpHash, HashOutput);
        for (eh_index i = 0; i < IndexesPerHashOutput && X.size() < init_size; i++) {
            X.emplace_back(tmpHash+(i*N/8), N/8, HashLength,
                           LengthOfCollisionBit, (g*IndexesPerHashOutput)+i);
        }
        if (cancelled(ListGeneration)) throw solver_cancelled;
    }

    for (unsigned int r = 1; r < K && X.size() > 0; r++) {
        std::sort(X.begin(), X.end(), CompareSR(LengthOfCollisionByte));
        if (cancelled(ListSorting)) throw solver_cancelled;

        size_t i = 0;
        size_t posFree = 0;
        std::vector<FullStepRow<FullWidth>> Xc;
        while (i < X.size() - 1) {
            size_t j = 1;
            while (i+j < X.size() &&
                    HasCollision(X[i], X[i+j], LengthOfCollisionByte)) {
                j++;
            }

            for (size_t l = 0; l < j - 1; l++) {
                for (size_t m = l + 1; m < j; m++) {
                    if (DistinctIndices(X[i+l], X[i+m], hashLen, lenIndices)) {
                        Xc.emplace_back(X[i+l], X[i+m], hashLen, lenIndices, LengthOfCollisionByte);
                    }
                }
            }

            while (posFree < i+j && Xc.size() > 0) {
                X[posFree++] = Xc.back();
                Xc.pop_back();
            }

            i += j;
            if (cancelled(ListColliding)) throw solver_cancelled;
        }

        while (posFree < X.size() && Xc.size() > 0) {
            X[posFree++] = Xc.back();
            Xc.pop_back();
        }

        if (Xc.size() > 0) {
            X.insert(X.end(), Xc.begin(), Xc.end());
        } else if (posFree < X.size()) {
            X.erase(X.begin()+posFree, X.end());
            X.shrink_to_fit();
        }

        hashLen -= LengthOfCollisionByte;
        lenIndices *= 2;
        if (cancelled(RoundEnd)) throw solver_cancelled;
    }

    if (X.size() > 1) {
        std::sort(X.begin(), X.end(), CompareSR(hashLen));
        if (cancelled(FinalSorting)) throw solver_cancelled;
        size_t i = 0;
        while (i < X.size() - 1) {
            size_t j = 1;
            while (i+j < X.size() &&
                    HasCollision(X[i], X[i+j], hashLen)) {
                j++;
            }

            for (size_t l = 0; l < j - 1; l++) {
                for (size_t m = l + 1; m < j; m++) {
                    FullStepRow<FinalFullWidth> res(X[i+l], X[i+m], hashLen, lenIndices, 0);
                    if (DistinctIndices(X[i+l], X[i+m], hashLen, lenIndices)) {
                        auto soln = res.GetIndices(hashLen, 2*lenIndices, LengthOfCollisionBit);
                        assert(soln.size() == equihash_solution_size(N, K));
                        if (validBlock(soln)) {
                            return true;
                        }
                    }
                }
            }

            i += j;
            if (cancelled(FinalColliding)) throw solver_cancelled;
        }
    } else

    return false;
}

template<size_t WIDTH>
std::shared_ptr<eh_trunc> TruncatedStepRow<WIDTH>::GetTruncatedIndices(size_t len, size_t lenIndices) const
{
    std::shared_ptr<eh_trunc> p (new eh_trunc[lenIndices], std::default_delete<eh_trunc[]>());
    std::copy(hash+len, hash+len+lenIndices, p.get());
    return p;
}

template<size_t WIDTH>
TruncatedStepRow<WIDTH>& TruncatedStepRow<WIDTH>::operator=(const TruncatedStepRow<WIDTH>& a)
{
    std::copy(a.hash, a.hash+WIDTH, hash);
    return *this;
}

template<size_t WIDTH> template<size_t W>
TruncatedStepRow<WIDTH>::TruncatedStepRow(const TruncatedStepRow<W>& a, const TruncatedStepRow<W>& b, size_t len, size_t lenIndices, size_t trim) :
        StepRow<WIDTH> {a}
{
    assert(len+lenIndices <= W);
    assert(len-trim+(2*lenIndices) <= WIDTH);
    for (size_t i = trim; i < len; i++)
        hash[i-trim] = a.hash[i] ^ b.hash[i];
    if (a.IndicesBefore(b, len, lenIndices)) {
        std::copy(a.hash+len, a.hash+len+lenIndices, hash+len-trim);
        std::copy(b.hash+len, b.hash+len+lenIndices, hash+len-trim+lenIndices);
    } else {
        std::copy(b.hash+len, b.hash+len+lenIndices, hash+len-trim);
        std::copy(a.hash+len, a.hash+len+lenIndices, hash+len-trim+lenIndices);
    }
}

template<size_t WIDTH>
TruncatedStepRow<WIDTH>::TruncatedStepRow(const unsigned char* hashIn, size_t hInLen,
                                          size_t hLen, size_t cBitLen,
                                          eh_index i, unsigned int ilen) :
        StepRow<WIDTH> {hashIn, hInLen, hLen, cBitLen}
{
    hash[hLen] = TruncateIndex(i, ilen);
}

template<size_t WIDTH>
bool HasCollision(StepRow<WIDTH>& a, StepRow<WIDTH>& b, size_t l)
{
    for (size_t j = 0; j < l; j++) {
        if (a.hash[j] != b.hash[j])
            return false;
    }
    return true;
}

template<size_t WIDTH>
std::vector<unsigned char> FullStepRow<WIDTH>::GetIndices(size_t len, size_t lenIndices,
                                                          size_t cBitLen) const
{
    assert(((cBitLen+1)+7)/8 <= sizeof(eh_index));
    size_t minLen { (cBitLen+1)*lenIndices/(8*sizeof(eh_index)) };
    size_t bytePad { sizeof(eh_index) - ((cBitLen+1)+7)/8 };
    std::vector<unsigned char> ret(minLen);
    CompressArray(hash+len, lenIndices, ret.data(), minLen, cBitLen+1, bytePad);
    return ret;
}

template<size_t WIDTH>
FullStepRow<WIDTH>& FullStepRow<WIDTH>::operator=(const FullStepRow<WIDTH>& a)
{
    std::copy(a.hash, a.hash+WIDTH, hash);
    return *this;
}

template<size_t WIDTH> template<size_t W>
FullStepRow<WIDTH>::FullStepRow(const FullStepRow<W>& a, const FullStepRow<W>& b, size_t len, size_t lenIndices, size_t trim) :
        StepRow<WIDTH> {a}
{
    assert(len+lenIndices <= W);
    assert(len-trim+(2*lenIndices) <= WIDTH);
    for (size_t i = trim; i < len; i++)
        hash[i-trim] = a.hash[i] ^ b.hash[i];
    if (a.IndicesBefore(b, len, lenIndices)) {
        std::copy(a.hash+len, a.hash+len+lenIndices, hash+len-trim);
        std::copy(b.hash+len, b.hash+len+lenIndices, hash+len-trim+lenIndices);
    } else {
        std::copy(b.hash+len, b.hash+len+lenIndices, hash+len-trim);
        std::copy(a.hash+len, a.hash+len+lenIndices, hash+len-trim+lenIndices);
    }
}

template<size_t WIDTH>
FullStepRow<WIDTH>::FullStepRow(const unsigned char* hashIn, size_t hInLen,
                                size_t hLen, size_t cBitLen, eh_index i) :
        StepRow<WIDTH> {hashIn, hInLen, hLen, cBitLen}
{
    EhIndexToArray(i, hash+hLen);
}

template<size_t WIDTH>
bool StepRow<WIDTH>::IsZero(size_t len)
{
    for (size_t i = 0; i < len; i++) {
        if (hash[i] != 0)
            return false;
    }
    return true;
}

template<size_t WIDTH> template<size_t W>
StepRow<WIDTH>::StepRow(const StepRow<W>& a)
{
    BOOST_STATIC_ASSERT(W <= WIDTH);
    std::copy(a.hash, a.hash+W, hash);
}

template<size_t WIDTH>
StepRow<WIDTH>::StepRow(const unsigned char* hashIn, size_t hInLen,
                        size_t hLen, size_t cBitLen)
{
    assert(hLen <= WIDTH);
    ExpandArray(hashIn, hInLen, hash, hLen, cBitLen);
}

std::vector<unsigned char> GetMinimalFromIndices(std::vector<eh_index> indices,
                                                 size_t cBitLen)
{
    assert(((cBitLen+1)+7)/8 <= sizeof(eh_index));
    size_t lenIndices { indices.size()*sizeof(eh_index) };
    size_t minLen { (cBitLen+1)*lenIndices/(8*sizeof(eh_index)) };
    size_t bytePad { sizeof(eh_index) - ((cBitLen+1)+7)/8 };
    std::vector<unsigned char> array(lenIndices);
    for (size_t i = 0; i < indices.size(); i++) {
        EhIndexToArray(indices[i], array.data()+(i*sizeof(eh_index)));
    }
    std::vector<unsigned char> ret(minLen);
    CompressArray(array.data(), lenIndices,
                  ret.data(), minLen, cBitLen+1, bytePad);
    return ret;
}

std::vector<eh_index> GetIndicesFromMinimal(std::vector<unsigned char> minimal,
                                            size_t cBitLen)
{
    assert(((cBitLen+1)+7)/8 <= sizeof(eh_index));
    size_t lenIndices { 8*sizeof(eh_index)*minimal.size()/(cBitLen+1) };
    size_t bytePad { sizeof(eh_index) - ((cBitLen+1)+7)/8 };
    std::vector<unsigned char> array(lenIndices);
    ExpandArray(minimal.data(), minimal.size(),
                array.data(), lenIndices, cBitLen+1, bytePad);
    std::vector<eh_index> ret;
    for (size_t i = 0; i < lenIndices; i += sizeof(eh_index)) {
        ret.push_back(ArrayToEhIndex(array.data()+i));
    }
    return ret;
}

eh_index UntruncateIndex(const eh_trunc t, const eh_index r, const unsigned int ilen)
{
    eh_index i{t};
    return (i << (ilen - 8)) | r;
}

eh_trunc TruncateIndex(const eh_index i, const unsigned int ilen)
{
    BOOST_STATIC_ASSERT(sizeof(eh_trunc) == 1);
    return (i >> (ilen - 8)) & 0xff;
}

eh_index ArrayToEhIndex(const unsigned char* array)
{
    BOOST_STATIC_ASSERT(sizeof(eh_index) == 4);
    eh_index bei;
    memcpy(&bei, array, sizeof(eh_index));
    return be32toh(bei);
}

void EhIndexToArray(const eh_index i, unsigned char* array)
{
    BOOST_STATIC_ASSERT(sizeof(eh_index) == 4);
    eh_index bei = htobe32(i);
    memcpy(array, &bei, sizeof(eh_index));
}

void ExpandArray(const unsigned char* in, size_t in_len,
                 unsigned char* out, size_t out_len,
                 size_t bit_len, size_t byte_pad)
{
    assert(bit_len >= 8);
    assert(8*sizeof(uint32_t) >= 7+bit_len);

    size_t out_width { (bit_len+7)/8 + byte_pad };
    assert(out_len == 8*out_width*in_len/bit_len);

    uint32_t bit_len_mask { ((uint32_t)1 << bit_len) - 1 };

    size_t acc_bits = 0;
    uint32_t acc_value = 0;

    size_t j = 0;
    for (size_t i = 0; i < in_len; i++) {
        acc_value = (acc_value << 8) | in[i];
        acc_bits += 8;

        if (acc_bits >= bit_len) {
            acc_bits -= bit_len;
            for (size_t x = 0; x < byte_pad; x++) {
                out[j+x] = 0;
            }
            for (size_t x = byte_pad; x < out_width; x++) {
                out[j+x] = (
                    acc_value >> (acc_bits+(8*(out_width-x-1)))
                ) & (
                    (bit_len_mask >> (8*(out_width-x-1))) & 0xFF
                );
            }
            j += out_width;
        }
    }
}

void CompressArray(const unsigned char* in, size_t in_len,
                   unsigned char* out, size_t out_len,
                   size_t bit_len, size_t byte_pad)
{
    assert(bit_len >= 8);
    assert(8*sizeof(uint32_t) >= 7+bit_len);

    size_t in_width { (bit_len+7)/8 + byte_pad };
    assert(out_len == bit_len*in_len/(8*in_width));

    uint32_t bit_len_mask { ((uint32_t)1 << bit_len) - 1 };

    size_t acc_bits = 0;
    uint32_t acc_value = 0;

    size_t j = 0;
    for (size_t i = 0; i < out_len; i++) {
        if (acc_bits < 8) {
            acc_value = acc_value << bit_len;
            for (size_t x = byte_pad; x < in_width; x++) {
                acc_value = acc_value | (
                    (
                        in[j+x] & ((bit_len_mask >> (8*(in_width-x-1))) & 0xFF)
                    ) << (8*(in_width-x-1)));
            }
            j += in_width;
            acc_bits += bit_len;
        }

        acc_bits -= 8;
        out[i] = (acc_value >> acc_bits) & 0xFF;
    }
}

void GenerateHash(const eh_HashState& base_state, eh_index g,
                  unsigned char* hash, size_t hLen)
{
    eh_HashState state;
    state = base_state;
    eh_index lei = htole32(g);
    crypto_generichash_blake2b_update(&state, (const unsigned char*) &lei,
                                      sizeof(eh_index));
    crypto_generichash_blake2b_final(&state, hash, hLen);
}

template<unsigned int N, unsigned int K>
int Equihash<N,K>::InitializeState(eh_HashState& base_state, bool wbl_salt)
{
    uint32_t le_N = htole32(N);
    uint32_t le_K = htole32(K);
    unsigned char personalization[crypto_generichash_blake2b_PERSONALBYTES] = {};
    if (wbl_salt) {
        memcpy(personalization, "wblPoW", 8);
    } else {
        memcpy(personalization, "ZcashPoW", 8);
    }
    memcpy(personalization+8,  &le_N, 4);
    memcpy(personalization+12, &le_K, 4);
    return crypto_generichash_blake2b_init_salt_personal(&base_state,
                                                         NULL, 0,
                                                         (512/N)*N/8,
                                                         NULL,
                                                         personalization);
}

                                                                         