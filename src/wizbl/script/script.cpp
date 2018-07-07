
#include "script.h"

#include "../util/str_encodings.h"
#include "../util/tiny_format.h"

bool CScript::HasValidOpeartions() const
{
    CScript::const_iterator it = begin();
    while (it < end()) {
        opertaionCodetype opeartionCode;
        std::vector<unsigned char> item;
        if (!GetOperator(it, opeartionCode, item) || opeartionCode > MAX_OPERATION_CODE || item.size() > MAX_SCRIPT_ELEMENT_SIZE) {
            return false;
        }
    }
    return true;
}

std::string CScriptWitnesses::ToString() const
{
    std::string ret = "CScriptWitnesses(";
    for (unsigned int i = 0; i < stack.size(); i++) {
        if (i) {
            ret += ", ";
        }
        ret += HexStr(stack[i]);
    }
    return ret + ")";
}

bool CScript::IsPushingOnly() const
{
    return this->IsPushingOnly(begin());
}

bool CScript::IsPushingOnly(const_iterator pc) const
{
    while (pc < end())
    {
        opertaionCodetype opeartionCode;
        if (!GetOperator(pc, opeartionCode))
            return false;
        if (opeartionCode > OPERATION_16)
            return false;
    }
    return true;
}

bool CScript::IsWitnessProgram(int& version, std::vector<unsigned char>& program) const
{
    if (this->size() < 4 || this->size() > 42) {
        return false;
    }
    if ((*this)[0] != OPERATION_0 && ((*this)[0] < OPERATION_1 || (*this)[0] > OPERATION_16)) {
        return false;
    }
    if ((size_t)((*this)[1] + 2) == this->size()) {
        version = DecodeOPERATION_N((opertaionCodetype)(*this)[0]);
        program = std::vector<unsigned char>(this->begin() + 2, this->end());
        return true;
    }
    return false;
}

bool CScript::IsPayToWitnessesScriptHash() const
{
    return (this->size() == 34 &&
            (*this)[0] == OPERATION_0 &&
            (*this)[1] == 0x20);
}

bool CScript::IsPayToHash() const
{
    return (this->size() == 23 &&
            (*this)[0] == OPERATION_HASH160 &&
            (*this)[1] == 0x14 &&
            (*this)[22] == OPERATION_EQUAL);
}

unsigned int CScript::GetSignatureOperationCount(const CScript& scriptSig) const
{
    if (!IsPayToHash())
        return GetSignatureOperationCount(true);

    const_iterator pc = scriptSig.begin();
    std::vector<unsigned char> vData;
    while (pc < scriptSig.end())
    {
        opertaionCodetype opcode;
        if (!scriptSig.GetOperator(pc, opcode, vData))
            return 0;
        if (opcode > OPERATION_16)
            return 0;
    }

    CScript subscript(vData.begin(), vData.end());
    return subscript.GetSignatureOperationCount(true);
}

unsigned int CScript::GetSignatureOperationCount(bool fAccurate) const
{
    unsigned int n = 0;
    const_iterator pc = begin();
    opertaionCodetype lastOpcode = OPERATION_INVALIDOPCODE;
    while (pc < end())
    {
        opertaionCodetype opcode;
        if (!GetOperator(pc, opcode))
            break;
        if (opcode == OPERATION_CHECKSIG || opcode == OPERATION_CHECKSIGVERIFY)
            n++;
        else if (opcode == OPERATION_CHECKMULTISIG || opcode == OPERATION_CHECKMULTISIGVERIFY)
        {
            if (fAccurate && lastOpcode >= OPERATION_1 && lastOpcode <= OPERATION_16)
                n += DecodeOPERATION_N(lastOpcode);
            else
                n += MAX_PUBLIC_KEYS_PER_MULTISIG;
        }
        lastOpcode = opcode;
    }
    return n;
}

const char* GetOperationName(opertaionCodetype opcode)
{
    switch (opcode)
    {
    case OPERATION_0                      : return "0";
    case OPERATION_PUSHDATA1              : return "OPERATION_PUSHDATA1";
    case OPERATION_PUSHDATA2              : return "OPERATION_PUSHDATA2";
    case OPERATION_PUSHDATA4              : return "OPERATION_PUSHDATA4";
    case OPERATION_1NEGATE                : return "-1";
    case OPERATION_RESERVED               : return "OPERATION_RESERVED";
    case OPERATION_1                      : return "1";
    case OPERATION_2                      : return "2";
    case OPERATION_3                      : return "3";
    case OPERATION_4                      : return "4";
    case OPERATION_5                      : return "5";
    case OPERATION_6                      : return "6";
    case OPERATION_7                      : return "7";
    case OPERATION_8                      : return "8";
    case OPERATION_9                      : return "9";
    case OPERATION_10                     : return "10";
    case OPERATION_11                     : return "11";
    case OPERATION_12                     : return "12";
    case OPERATION_13                     : return "13";
    case OPERATION_14                     : return "14";
    case OPERATION_15                     : return "15";
    case OPERATION_16                     : return "16";

    case OPERATION_NOP                    : return "OPERATION_NOP";
    case OPERATION_VER                    : return "OPERATION_VER";
    case OPERATION_IF                     : return "OPERATION_IF";
    case OPERATION_NOTIF                  : return "OPERATION_NOTIF";
    case OPERATION_VERIF                  : return "OPERATION_VERIF";
    case OPERATION_VERNOTIF               : return "OPERATION_VERNOTIF";
    case OPERATION_ELSE                   : return "OPERATION_ELSE";
    case OPERATION_ENDIF                  : return "OPERATION_ENDIF";
    case OPERATION_VERIFY                 : return "OPERATION_VERIFY";
    case OPERATION_RETURN                 : return "OPERATION_RETURN";

    case OPERATION_TOALTSTACK             : return "OPERATION_TOALTSTACK";
    case OPERATION_FROMALTSTACK           : return "OPERATION_FROMALTSTACK";
    case OPERATION_2DROP                  : return "OPERATION_2DROP";
    case OPERATION_2DUP                   : return "OPERATION_2DUP";
    case OPERATION_3DUP                   : return "OPERATION_3DUP";
    case OPERATION_2OVER                  : return "OPERATION_2OVER";
    case OPERATION_2ROT                   : return "OPERATION_2ROT";
    case OPERATION_2SWAP                  : return "OPERATION_2SWAP";
    case OPERATION_IFDUP                  : return "OPERATION_IFDUP";
    case OPERATION_DEPTH                  : return "OPERATION_DEPTH";
    case OPERATION_DROP                   : return "OPERATION_DROP";
    case OPERATION_DUP                    : return "OPERATION_DUP";
    case OPERATION_NIP                    : return "OPERATION_NIP";
    case OPERATION_OVER                   : return "OPERATION_OVER";
    case OPERATION_PICK                   : return "OPERATION_PICK";
    case OPERATION_ROLL                   : return "OPERATION_ROLL";
    case OPERATION_ROT                    : return "OPERATION_ROT";
    case OPERATION_SWAP                   : return "OPERATION_SWAP";
    case OPERATION_TUCK                   : return "OPERATION_TUCK";

    case OPERATION_CAT                    : return "OPERATION_CAT";
    case OPERATION_SUBSTR                 : return "OPERATION_SUBSTR";
    case OPERATION_LEFT                   : return "OPERATION_LEFT";
    case OPERATION_RIGHT                  : return "OPERATION_RIGHT";
    case OPERATION_SIZE                   : return "OPERATION_SIZE";

    case OPERATION_INVERT                 : return "OPERATION_INVERT";
    case OPERATION_AND                    : return "OPERATION_AND";
    case OPERATION_OR                     : return "OPERATION_OR";
    case OPERATION_XOR                    : return "OPERATION_XOR";
    case OPERATION_EQUAL                  : return "OPERATION_EQUAL";
    case OPERATION_EQUALVERIFY            : return "OPERATION_EQUALVERIFY";
    case OPERATION_RESERVED1              : return "OPERATION_RESERVED1";
    case OPERATION_RESERVED2              : return "OPERATION_RESERVED2";

    case OPERATION_1ADD                   : return "OPERATION_1ADD";
    case OPERATION_1SUB                   : return "OPERATION_1SUB";
    case OPERATION_2MUL                   : return "OPERATION_2MUL";
    case OPERATION_2DIV                   : return "OPERATION_2DIV";
    case OPERATION_NEGATE                 : return "OPERATION_NEGATE";
    case OPERATION_ABS                    : return "OPERATION_ABS";
    case OPERATION_NOT                    : return "OPERATION_NOT";
    case OPERATION_0NOTEQUAL              : return "OPERATION_0NOTEQUAL";
    case OPERATION_ADD                    : return "OPERATION_ADD";
    case OPERATION_SUB                    : return "OPERATION_SUB";
    case OPERATION_MUL                    : return "OPERATION_MUL";
    case OPERATION_DIV                    : return "OPERATION_DIV";
    case OPERATION_MOD                    : return "OPERATION_MOD";
    case OPERATION_LSHIFT                 : return "OPERATION_LSHIFT";
    case OPERATION_RSHIFT                 : return "OPERATION_RSHIFT";
    case OPERATION_BOOLAND                : return "OPERATION_BOOLAND";
    case OPERATION_BOOLOR                 : return "OPERATION_BOOLOR";
    case OPERATION_NUMEQUAL               : return "OPERATION_NUMEQUAL";
    case OPERATION_NUMEQUALVERIFY         : return "OPERATION_NUMEQUALVERIFY";
    case OPERATION_NUMNOTEQUAL            : return "OPERATION_NUMNOTEQUAL";
    case OPERATION_LESSTHAN               : return "OPERATION_LESSTHAN";
    case OPERATION_GREATERTHAN            : return "OPERATION_GREATERTHAN";
    case OPERATION_LESSTHANOREQUAL        : return "OPERATION_LESSTHANOREQUAL";
    case OPERATION_GREATERTHANOREQUAL     : return "OPERATION_GREATERTHANOREQUAL";
    case OPERATION_MIN                    : return "OPERATION_MIN";
    case OPERATION_MAX                    : return "OPERATION_MAX";
    case OPERATION_WITHIN                 : return "OPERATION_WITHIN";

    case OPERATION_RIPEMD160              : return "OPERATION_RIPEMD160";
    case OPERATION_SHA1                   : return "OPERATION_SHA1";
    case OPERATION_SHA256                 : return "OPERATION_SHA256";
    case OPERATION_HASH160                : return "OPERATION_HASH160";
    case OPERATION_HASH256                : return "OPERATION_HASH256";
    case OPERATION_CODESEPARATOR          : return "OPERATION_CODESEPARATOR";
    case OPERATION_CHECKSIG               : return "OPERATION_CHECKSIG";
    case OPERATION_CHECKSIGVERIFY         : return "OPERATION_CHECKSIGVERIFY";
    case OPERATION_CHECKMULTISIG          : return "OPERATION_CHECKMULTISIG";
    case OPERATION_CHECKMULTISIGVERIFY    : return "OPERATION_CHECKMULTISIGVERIFY";

    case OPERATION_NOP1                   : return "OPERATION_NOP1";
    case OPERATION_CHECKLOCKTIMEVERIFY    : return "OPERATION_CHECKLOCKTIMEVERIFY";
    case OPERATION_CHECKSEQUENCEVERIFY    : return "OPERATION_CHECKSEQUENCEVERIFY";
    case OPERATION_NOP4                   : return "OPERATION_NOP4";
    case OPERATION_NOP5                   : return "OPERATION_NOP5";
    case OPERATION_NOP6                   : return "OPERATION_NOP6";
    case OPERATION_NOP7                   : return "OPERATION_NOP7";
    case OPERATION_NOP8                   : return "OPERATION_NOP8";
    case OPERATION_NOP9                   : return "OPERATION_NOP9";
    case OPERATION_NOP10                  : return "OPERATION_NOP10";

    case OPERATION_INVALIDOPCODE          : return "OPERATION_INVALIDOPCODE";


    default:
        return "OPERATION_UNKNOWN";
    }
}
                                                                                                    