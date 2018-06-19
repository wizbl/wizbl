#include "consensus_wizblcoin.h"

#include "../util/public_key.h"
#include "primitives/transaction.h"
#include "script/interpreter.h"
#include "version.h"

namespace {

class TxInputStream
{
public:
    TxInputStream(int nTypeIn, int nVersionIn, const unsigned char *txTo, size_t txToLen) :
    m_typeIn(nTypeIn),
    m_versionIn(nVersionIn),
    m_txData(txTo),
    m_remains(txToLen)
    {}

    void read(char* pch, size_t nSize)
    {
        if (nSize > m_remains)
            throw std::ios_base::fail(std::string(__func__) + ": end of data");

        if (pch == nullptr)
            throw std::ios_base::fail(std::string(__func__) + ": bad destination buffer");

        if (m_txData == nullptr)
            throw std::ios_base::fail(std::string(__func__) + ": bad source buffer");

        memcpy(pch, m_txData, nSize);
        m_remains -= nSize;
        m_txData += nSize;
    }

    template<typename T>
    TxInputStream& operator>>(T& obj)
    {
        ::Unserialize(*this, obj);
        return *this;
    }

    int GetVersion() const { return m_versionIn; }
    int GetType() const { return m_typeIn; }
private:
    const int m_typeIn;
    const int m_versionIn;
    const unsigned char* m_txData;
    size_t m_remains;
};

inline int set_error(wizblcoinconsensus_error* ret, wizblcoinconsensus_error serror)
{
    if (ret)
        *ret = serror;
    return 0;
}

struct ECCryptoClosure
{
    ECCVerifyHandle handle;
};

ECCryptoClosure instance_of_eccryptoclosure;
}

static int verify_script(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen, CAmount amount,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    unsigned int nIn, unsigned int flags, wizblcoinconsensus_error* err)
{
    if (!verify_flags(flags)) {
        return wizblcoinconsensus_ERR_INVALID_FLAGS;
    }
    try {
        TxInputStream stream(SER_NETWORK, PROTOCOL_VERSION, txTo, txToLen);
        ChkTx tx(deserialize, stream);
        if (nIn >= tx.vin.size())
            return set_error(err, wizblcoinconsensus_ERR_TX_INDEX);
        if (GetSerialSize(tx, SER_NETWORK, PROTOCOL_VERSION) != txToLen)
            return set_error(err, wizblcoinconsensus_ERR_TX_SIZE_MISMATCH);

        set_error(err, wizblcoinconsensus_ERR_OK);

        PrecomputedTxData txdata(tx);
        return VerifyScript(tx.vin[nIn].scriptSig, CScript(scriptPubKey, scriptPubKey + scriptPubKeyLen), &tx.vin[nIn].scriptWitness, flags, TxSignatureChecker(&tx, nIn, amount, txdata), nullptr);
    } catch (const std::exception&) {
        return set_error(err, wizblcoinconsensus_ERR_TX_DESERIALIZE);
    }
}

static bool verify_flags(unsigned int flags)
{
    return (flags & ~(wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_ALL)) == 0;
}

int wizblcoinconsensus_verify_script_with_amount(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen, int64_t amount,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    unsigned int nIn, unsigned int flags, wizblcoinconsensus_error* err)
{
    CAmount am(amount);
    return ::verify_script(scriptPubKey, scriptPubKeyLen, am, txTo, txToLen, nIn, flags, err);
}

int wizblcoinconsensus_verify_script(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen,
                                   const unsigned char *txTo        , unsigned int txToLen,
                                   unsigned int nIn, unsigned int flags, wizblcoinconsensus_error* err)
{
    if (flags & wizblcoinconsensus_SCRIPT_FLAGS_VERIFY_WITNESS) {
        return set_error(err, wizblcoinconsensus_ERR_AMOUNT_REQUIRED);
    }

    CAmount am(0);
    return ::verify_script(scriptPubKey, scriptPubKeyLen, am, txTo, txToLen, nIn, flags, err);
}

unsigned int wizblcoinconsensus_version()
{
    return WIZBLCOINCONSENSUS_API_VER;
}

    