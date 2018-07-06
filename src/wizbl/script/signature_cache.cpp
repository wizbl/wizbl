#include "signature_cache.h"

#include "random.h"
#include "util.h"
#include "uint256.h"
#include "memusage.h"

#include <boost/thread.hpp>

#include "../util/public_key.h"
#include "cuckoocache.h"

namespace {
class CSignatureCache
{
public:
	uint32_t bytes_setup(size_t n){
	        return setValid.bytes_setup(n);
	    }
	bool
	    Get(const uint256& entry, const bool erase)
	    {
	        return setValid.contains(entry, erase);
	    }
	void Set(uint256& entry)
	    {
	        setValid.insert(entry);
	    }
    void
        EntryCompute(uint256& entry, const uint256 &hash, const std::vector<unsigned char>& vchSignature, const CPublicKeyID& pubkey)
        {
            CSHA256().Write(nonce.begin(), 32).Write(hash.begin(), 32).Write(&pubkey[0], pubkey.size()).Write(&vchSignature[0], vchSignature.size()).Finalize(entry.begin());
        }
    CSignatureCache()
        {
            GetRandomBytes(nonce.begin(), 32);
        }
private:
    typedef CuckooCache::cache<uint256, SignatureCacheHasher> map_type;
    uint256 nonce;
    boost::shared_mutex cs_sigcache;
    map_type setValid;
};
static CSignatureCache signatureCache;
}


bool TxSignatureCachingChecker::VerifyingSignature(const std::vector<unsigned char>& vchSig, const CPublicKeyID& pubkey, const uint256& sighash) const
{
    uint256 entry;
    signatureCache.EntryCompute(entry, sighash, vchSig, pubkey);
    if (signatureCache.Get(entry, !store))
        return true;
    if (!TxSignatureChecker::VerifyingSignature(vchSig, pubkey, sighash))
        return false;
    if (store)
        signatureCache.Set(entry);
    return true;
}

void InitializeSignatureCache()
{
    size_t nMaxCacheSize = std::min(std::max((int64_t)0, gArgs.GetArg("-maxsigcachesize", DEFAULT_MAX_SIGNATURE_CACHE_SIZE) / 2), MAX_MAX_SIGNATURE_CACHE_SIZE) * ((size_t) 1 << 20);
    size_t nElems = signatureCache.bytes_setup(nMaxCacheSize);
}
                                                                         