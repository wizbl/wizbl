#include "interpreter.h"

#include "../util/public_key.h"
#include "crypto/sha1.h"
#include "crypto/sha256.h"
#include "crypto/ripemd160.h"
#include "primitives/transaction.h"
#include "script/script.h"
#include "uint256.h"

typedef std::vector<unsigned char> valtype;

namespace {

inline bool set_error(ScriptError* ret, const ScriptError serror)
{
    if (ret) *ret = serror;
    return false;
}

inline bool set_success(ScriptError* ret)
{
    if (ret) *ret = SCRIPT_ERROR_OK;
    return true;
}

}

bool EvalScript(std::vector<std::vector<unsigned char> >& stack, const CScript& script, unsigned int flags, const BaseSignatureChecker& checker, SigVersion sigversion, ScriptError* serror)
{
    static const CScriptNum bnZero(0);
    static const CScriptNum bnOne(1);
    static const valtype vchFalse(0);
    static const valtype vchTrue(1, 1);

    CScript::const_iterator pc = script.begin();
    CScript::const_iterator pend = script.end();
    CScript::const_iterator pbegincodehash = script.begin();
    opertaionCodetype opcode;
    valtype vchPushValue;
    std::vector<bool> vfExec;
    std::vector<valtype> altstack;
    set_error(serror, SCRIPT_ERROR_UNKNOWN_ERROR);
    if (script.size() > MAX_SCRIPT_SIZE)
        return set_error(serror, SCRIPT_ERROR_SCRIPT_SIZE);
    int nOpCount = 0;
    bool fRequireMinimal = (flags & SCRIPT_VERIFY_MINIMALDATA) != 0;

    try
    {
        while (pc < pend)
        {
            bool fExec = !count(vfExec.begin(), vfExec.end(), false);

            if (!script.GetOperator(pc, opcode, vchPushValue))
                return set_error(serror, SCRIPT_ERROR_BAD_OPCODE);
            if (vchPushValue.size() > MAX_SCRIPT_ELEMENT_SIZE)
                return set_error(serror, SCRIPT_ERROR_PUSH_SIZE);

            if (opcode > OPERATION_16 && ++nOpCount > MAX_OPERATIONS_PER_SCRIPT)
                return set_error(serror, SCRIPT_ERROR_OPERATION_COUNT);

            if (opcode == OPERATION_CAT ||
                opcode == OPERATION_SUBSTR ||
                opcode == OPERATION_LEFT ||
                opcode == OPERATION_RIGHT ||
                opcode == OPERATION_INVERT ||
                opcode == OPERATION_AND ||
                opcode == OPERATION_OR ||
                opcode == OPERATION_XOR ||
                opcode == OPERATION_2MUL ||
                opcode == OPERATION_2DIV ||
                opcode == OPERATION_MUL ||
                opcode == OPERATION_DIV ||
                opcode == OPERATION_MOD ||
                opcode == OPERATION_LSHIFT ||
                opcode == OPERATION_RSHIFT)
                return set_error(serror, SCRIPT_ERROR_DISABLED_OPCODE);

            if (fExec && 0 <= opcode && opcode <= OPERATION_PUSHDATA4) {
                if (fRequireMinimal && !CheckMinimalPush(vchPushValue, opcode)) {
                    return set_error(serror, SCRIPT_ERROR_MINIMALDATA);
                }
                stack.push_back(vchPushValue);
            } else if (fExec || (OPERATION_IF <= opcode && opcode <= OPERATION_ENDIF))
            switch (opcode)
            {
                case OPERATION_1NEGATE:
                case OPERATION_1:
                case OPERATION_2:
                case OPERATION_3:
                case OPERATION_4:
                case OPERATION_5:
                case OPERATION_6:
                case OPERATION_7:
                case OPERATION_8:
                case OPERATION_9:
                case OPERATION_10:
                case OPERATION_11:
                case OPERATION_12:
                case OPERATION_13:
                case OPERATION_14:
                case OPERATION_15:
                case OPERATION_16:
                {
                    CScriptNum bn((int)opcode - (int)(OPERATION_1 - 1));
                    stack.push_back(bn.getvch());
                }
                break;

                case OPERATION_NOP:
                    break;

                case OPERATION_CHECKLOCKTIMEVERIFY:
                {
                    if (!(flags & SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY)) {
                        if (flags & SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS) {
                            return set_error(serror, SCRIPT_ERROR_DISCOURAGE_UPGRADABLE_NOPS);
                        }
                        break;
                    }

                    if (stack.size() < 1)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);

                    const CScriptNum nLockTime(stacktop(-1), fRequireMinimal, 5);

                    if (nLockTime < 0)
                        return set_error(serror, SCRIPT_ERROR_NEGATIVE_LOCKTIME);

                    if (!checker.CheckLockTime(nLockTime))
                        return set_error(serror, SCRIPT_ERROR_UNSATISFIED_LOCKTIME);

                    break;
                }

                case OPERATION_CHECKSEQUENCEVERIFY:
                {
                    if (!(flags & SCRIPT_VERIFY_CHECKSEQUENCEVERIFY)) {
                        if (flags & SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS) {
                            return set_error(serror, SCRIPT_ERROR_DISCOURAGE_UPGRADABLE_NOPS);
                        }
                        break;
                    }

                    if (stack.size() < 1)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);

                    const CScriptNum nSeq(stacktop(-1), fRequireMinimal, 5);

                    if (nSeq < 0)
                        return set_error(serror, SCRIPT_ERROR_NEGATIVE_LOCKTIME);

                    if ((nSeq & ChkTxIn::SEQUENCE_LOCKTIME_DISABLE_FLAG) != 0)
                        break;

                    if (!checker.CheckSeq(nSeq))
                        return set_error(serror, SCRIPT_ERROR_UNSATISFIED_LOCKTIME);

                    break;
                }

                case OPERATION_NOP1: case OPERATION_NOP4: case OPERATION_NOP5:
                case OPERATION_NOP6: case OPERATION_NOP7: case OPERATION_NOP8: case OPERATION_NOP9: case OPERATION_NOP10:
                {
                    if (flags & SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS)
                        return set_error(serror, SCRIPT_ERROR_DISCOURAGE_UPGRADABLE_NOPS);
                }
                break;

                case OPERATION_IF:
                case OPERATION_NOTIF:
                {
                    bool fValue = false;
                    if (fExec)
                    {
                        if (stack.size() < 1)
                            return set_error(serror, SCRIPT_ERROR_UNBALANCED_CONDITIONAL);
                        valtype& vch = stacktop(-1);
                        if (sigversion == SIGVERSION_WITNESS_V0 && (flags & SCRIPT_VERIFY_MINIMALIF)) {
                            if (vch.size() > 1)
                                return set_error(serror, SCRIPT_ERROR_MINIMALIF);
                            if (vch.size() == 1 && vch[0] != 1)
                                return set_error(serror, SCRIPT_ERROR_MINIMALIF);
                        }
                        fValue = CastToBool(vch);
                        if (opcode == OPERATION_NOTIF)
                            fValue = !fValue;
                        popstack(stack);
                    }
                    vfExec.push_back(fValue);
                }
                break;

                case OPERATION_ELSE:
                {
                    if (vfExec.empty())
                        return set_error(serror, SCRIPT_ERROR_UNBALANCED_CONDITIONAL);
                    vfExec.back() = !vfExec.back();
                }
                break;

                case OPERATION_ENDIF:
                {
                    if (vfExec.empty())
                        return set_error(serror, SCRIPT_ERROR_UNBALANCED_CONDITIONAL);
                    vfExec.pop_back();
                }
                break;

                case OPERATION_VERIFY:
                {
                    if (stack.size() < 1)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    bool fValue = CastToBool(stacktop(-1));
                    if (fValue)
                        popstack(stack);
                    else
                        return set_error(serror, SCRIPT_ERROR_VERIFY);
                }
                break;

                case OPERATION_RETURN:
                {
                    return set_error(serror, SCRIPT_ERROR_OPERATION_RETURN);
                }
                break;

                case OPERATION_TOALTSTACK:
                {
                    if (stack.size() < 1)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    altstack.push_back(stacktop(-1));
                    popstack(stack);
                }
                break;

                case OPERATION_FROMALTSTACK:
                {
                    if (altstack.size() < 1)
                        return set_error(serror, SCRIPT_ERROR_INVALID_ALTSTACK_OPERATION);
                    stack.push_back(altstacktop(-1));
                    popstack(altstack);
                }
                break;

                case OPERATION_2DROP:
                {
                    if (stack.size() < 2)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    popstack(stack);
                    popstack(stack);
                }
                break;

                case OPERATION_2DUP:
                {
                    if (stack.size() < 2)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    valtype vch1 = stacktop(-2);
                    valtype vch2 = stacktop(-1);
                    stack.push_back(vch1);
                    stack.push_back(vch2);
                }
                break;

                case OPERATION_3DUP:
                {
                    if (stack.size() < 3)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    valtype vch1 = stacktop(-3);
                    valtype vch2 = stacktop(-2);
                    valtype vch3 = stacktop(-1);
                    stack.push_back(vch1);
                    stack.push_back(vch2);
                    stack.push_back(vch3);
                }
                break;

                case OPERATION_2OVER:
                {
                    if (stack.size() < 4)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    valtype vch1 = stacktop(-4);
                    valtype vch2 = stacktop(-3);
                    stack.push_back(vch1);
                    stack.push_back(vch2);
                }
                break;

                case OPERATION_2ROT:
                {
                    if (stack.size() < 6)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    valtype vch1 = stacktop(-6);
                    valtype vch2 = stacktop(-5);
                    stack.erase(stack.end()-6, stack.end()-4);
                    stack.push_back(vch1);
                    stack.push_back(vch2);
                }
                break;

                case OPERATION_2SWAP:
                {
                    if (stack.size() < 4)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    swap(stacktop(-4), stacktop(-2));
                    swap(stacktop(-3), stacktop(-1));
                }
                break;

                case OPERATION_IFDUP:
                {
                    if (stack.size() < 1)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    valtype vch = stacktop(-1);
                    if (CastToBool(vch))
                        stack.push_back(vch);
                }
                break;

                case OPERATION_DEPTH:
                {
                    CScriptNum bn(stack.size());
                    stack.push_back(bn.getvch());
                }
                break;

                case OPERATION_DROP:
                {
                    if (stack.size() < 1)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    popstack(stack);
                }
                break;

                case OPERATION_DUP:
                {
                    if (stack.size() < 1)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    valtype vch = stacktop(-1);
                    stack.push_back(vch);
                }
                break;

                case OPERATION_NIP:
                {
                    if (stack.size() < 2)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    stack.erase(stack.end() - 2);
                }
                break;

                case OPERATION_OVER:
                {
                    if (stack.size() < 2)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    valtype vch = stacktop(-2);
                    stack.push_back(vch);
                }
                break;

                case OPERATION_PICK:
                case OPERATION_ROLL:
                {
                    if (stack.size() < 2)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    int n = CScriptNum(stacktop(-1), fRequireMinimal).getint();
                    popstack(stack);
                    if (n < 0 || n >= (int)stack.size())
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    valtype vch = stacktop(-n-1);
                    if (opcode == OPERATION_ROLL)
                        stack.erase(stack.end()-n-1);
                    stack.push_back(vch);
                }
                break;

                case OPERATION_ROT:
                {
                    if (stack.size() < 3)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    swap(stacktop(-3), stacktop(-2));
                    swap(stacktop(-2), stacktop(-1));
                }
                break;

                case OPERATION_SWAP:
                {
                    if (stack.size() < 2)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    swap(stacktop(-2), stacktop(-1));
                }
                break;

                case OPERATION_TUCK:
                {
                    if (stack.size() < 2)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    valtype vch = stacktop(-1);
                    stack.insert(stack.end()-2, vch);
                }
                break;

                case OPERATION_SIZE:
                {
                    if (stack.size() < 1)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    CScriptNum bn(stacktop(-1).size());
                    stack.push_back(bn.getvch());
                }
                break;

                case OPERATION_EQUAL:
                case OPERATION_EQUALVERIFY:
                {
                    if (stack.size() < 2)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    valtype& vch1 = stacktop(-2);
                    valtype& vch2 = stacktop(-1);
                    bool fEqual = (vch1 == vch2);
                    popstack(stack);
                    popstack(stack);
                    stack.push_back(fEqual ? vchTrue : vchFalse);
                    if (opcode == OPERATION_EQUALVERIFY)
                    {
                        if (fEqual)
                            popstack(stack);
                        else
                            return set_error(serror, SCRIPT_ERROR_EQUALVERIFY);
                    }
                }
                break;

                case OPERATION_1ADD:
                case OPERATION_1SUB:
                case OPERATION_NEGATE:
                case OPERATION_ABS:
                case OPERATION_NOT:
                case OPERATION_0NOTEQUAL:
                {
                    if (stack.size() < 1)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    CScriptNum bn(stacktop(-1), fRequireMinimal);
                    switch (opcode)
                    {
                    case OPERATION_1ADD:       bn += bnOne; break;
                    case OPERATION_1SUB:       bn -= bnOne; break;
                    case OPERATION_NEGATE:     bn = -bn; break;
                    case OPERATION_ABS:        if (bn < bnZero) bn = -bn; break;
                    case OPERATION_NOT:        bn = (bn == bnZero); break;
                    case OPERATION_0NOTEQUAL:  bn = (bn != bnZero); break;
                    default:            assert(!"invalid opcode"); break;
                    }
                    popstack(stack);
                    stack.push_back(bn.getvch());
                }
                break;

                case OPERATION_ADD:
                case OPERATION_SUB:
                case OPERATION_BOOLAND:
                case OPERATION_BOOLOR:
                case OPERATION_NUMEQUAL:
                case OPERATION_NUMEQUALVERIFY:
                case OPERATION_NUMNOTEQUAL:
                case OPERATION_LESSTHAN:
                case OPERATION_GREATERTHAN:
                case OPERATION_LESSTHANOREQUAL:
                case OPERATION_GREATERTHANOREQUAL:
                case OPERATION_MIN:
                case OPERATION_MAX:
                {
                    if (stack.size() < 2)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    CScriptNum bn1(stacktop(-2), fRequireMinimal);
                    CScriptNum bn2(stacktop(-1), fRequireMinimal);
                    CScriptNum bn(0);
                    switch (opcode)
                    {
                    case OPERATION_ADD:
                        bn = bn1 + bn2;
                        break;

                    case OPERATION_SUB:
                        bn = bn1 - bn2;
                        break;

                    case OPERATION_BOOLAND:             bn = (bn1 != bnZero && bn2 != bnZero); break;
                    case OPERATION_BOOLOR:              bn = (bn1 != bnZero || bn2 != bnZero); break;
                    case OPERATION_NUMEQUAL:            bn = (bn1 == bn2); break;
                    case OPERATION_NUMEQUALVERIFY:      bn = (bn1 == bn2); break;
                    case OPERATION_NUMNOTEQUAL:         bn = (bn1 != bn2); break;
                    case OPERATION_LESSTHAN:            bn = (bn1 < bn2); break;
                    case OPERATION_GREATERTHAN:         bn = (bn1 > bn2); break;
                    case OPERATION_LESSTHANOREQUAL:     bn = (bn1 <= bn2); break;
                    case OPERATION_GREATERTHANOREQUAL:  bn = (bn1 >= bn2); break;
                    case OPERATION_MIN:                 bn = (bn1 < bn2 ? bn1 : bn2); break;
                    case OPERATION_MAX:                 bn = (bn1 > bn2 ? bn1 : bn2); break;
                    default:                     assert(!"invalid opcode"); break;
                    }
                    popstack(stack);
                    popstack(stack);
                    stack.push_back(bn.getvch());

                    if (opcode == OPERATION_NUMEQUALVERIFY)
                    {
                        if (CastToBool(stacktop(-1)))
                            popstack(stack);
                        else
                            return set_error(serror, SCRIPT_ERROR_NUMEQUALVERIFY);
                    }
                }
                break;

                case OPERATION_WITHIN:
                {
                    if (stack.size() < 3)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    CScriptNum bn1(stacktop(-3), fRequireMinimal);
                    CScriptNum bn2(stacktop(-2), fRequireMinimal);
                    CScriptNum bn3(stacktop(-1), fRequireMinimal);
                    bool fValue = (bn2 <= bn1 && bn1 < bn3);
                    popstack(stack);
                    popstack(stack);
                    popstack(stack);
                    stack.push_back(fValue ? vchTrue : vchFalse);
                }
                break;

                case OPERATION_RIPEMD160:
                case OPERATION_SHA1:
                case OPERATION_SHA256:
                case OPERATION_HASH160:
                case OPERATION_HASH256:
                {
                    if (stack.size() < 1)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    valtype& vch = stacktop(-1);
                    valtype vchHash((opcode == OPERATION_RIPEMD160 || opcode == OPERATION_SHA1 || opcode == OPERATION_HASH160) ? 20 : 32);
                    if (opcode == OPERATION_RIPEMD160)
                        CRIPEMD160().Write(vch.data(), vch.size()).Finalize(vchHash.data());
                    else if (opcode == OPERATION_SHA1)
                        CSHA1().Write(vch.data(), vch.size()).Finalize(vchHash.data());
                    else if (opcode == OPERATION_SHA256)
                        CSHA256().Write(vch.data(), vch.size()).Finalize(vchHash.data());
                    else if (opcode == OPERATION_HASH160)
                        CHash160().Write(vch.data(), vch.size()).Finalize(vchHash.data());
                    else if (opcode == OPERATION_HASH256)
                        CHash256().Write(vch.data(), vch.size()).Finalize(vchHash.data());
                    popstack(stack);
                    stack.push_back(vchHash);
                }
                break;

                case OPERATION_CODESEPARATOR:
                {
                    pbegincodehash = pc;
                }
                break;

                case OPERATION_CHECKSIG:
                case OPERATION_CHECKSIGVERIFY:
                {
                    if (stack.size() < 2)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);

                    valtype& vchSig    = stacktop(-2);
                    valtype& vchPubKey = stacktop(-1);

                    CScript scriptCode(pbegincodehash, pend);


                    if (!CheckSignatureEncoding(vchSig, flags, serror) || !CheckPubKeyEncoding(vchPubKey, flags, sigversion, serror)) {
                        return false;
                    }
                    bool fSuccess = checker.CheckSig(vchSig, vchPubKey, scriptCode, sigversion);

                    if (!fSuccess && (flags & SCRIPT_VERIFY_NULLFAIL) && vchSig.size())
                        return set_error(serror, SCRIPT_ERROR_SIG_NULLFAIL);

                    popstack(stack);
                    popstack(stack);
                    stack.push_back(fSuccess ? vchTrue : vchFalse);
                    if (opcode == OPERATION_CHECKSIGVERIFY)
                    {
                        if (fSuccess)
                            popstack(stack);
                        else
                            return set_error(serror, SCRIPT_ERROR_CHECKSIGVERIFY);
                    }
                }
                break;

                case OPERATION_CHECKMULTISIG:
                case OPERATION_CHECKMULTISIGVERIFY:
                {

                    int i = 1;
                    if ((int)stack.size() < i)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);

                    int nKeysCount = CScriptNum(stacktop(-i), fRequireMinimal).getint();
                    if (nKeysCount < 0 || nKeysCount > MAX_PUBLIC_KEYS_PER_MULTISIG)
                        return set_error(serror, SCRIPT_ERROR_PUBKEY_COUNT);
                    nOpCount += nKeysCount;
                    if (nOpCount > MAX_OPERATIONS_PER_SCRIPT)
                        return set_error(serror, SCRIPT_ERROR_OPERATION_COUNT);
                    int ikey = ++i;
                    int ikey2 = nKeysCount + 2;
                    i += nKeysCount;
                    if ((int)stack.size() < i)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);

                    int nSigsCount = CScriptNum(stacktop(-i), fRequireMinimal).getint();
                    if (nSigsCount < 0 || nSigsCount > nKeysCount)
                        return set_error(serror, SCRIPT_ERROR_SIG_COUNT);
                    int isig = ++i;
                    i += nSigsCount;
                    if ((int)stack.size() < i)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    CScript scriptCode(pbegincodehash, pend);

                    for (int k = 0; k < nSigsCount; k++)
                    {
                        valtype& vchSig = stacktop(-isig-k);
                    }

                    bool fSuccess = true;
                    while (fSuccess && nSigsCount > 0)
                    {
                        valtype& vchSig    = stacktop(-isig);
                        valtype& vchPubKey = stacktop(-ikey);

                        if (!CheckSignatureEncoding(vchSig, flags, serror) || !CheckPubKeyEncoding(vchPubKey, flags, sigversion, serror)) {
                            return false;
                        }

                        bool fOk = checker.CheckSig(vchSig, vchPubKey, scriptCode, sigversion);

                        if (fOk) {
                            isig++;
                            nSigsCount--;
                        }
                        ikey++;
                        nKeysCount--;

                        if (nSigsCount > nKeysCount)
                            fSuccess = false;
                    }

                    while (i-- > 1) {
                        if (!fSuccess && (flags & SCRIPT_VERIFY_NULLFAIL) && !ikey2 && stacktop(-1).size())
                            return set_error(serror, SCRIPT_ERROR_SIG_NULLFAIL);
                        if (ikey2 > 0)
                            ikey2--;
                        popstack(stack);
                    }

                    if (stack.size() < 1)
                        return set_error(serror, SCRIPT_ERROR_INVALID_STACK_OPERATION);
                    if ((flags & SCRIPT_VERIFY_NULLDUMMY) && stacktop(-1).size())
                        return set_error(serror, SCRIPT_ERROR_SIG_NULLDUMMY);
                    popstack(stack);

                    stack.push_back(fSuccess ? vchTrue : vchFalse);

                    if (opcode == OPERATION_CHECKMULTISIGVERIFY)
                    {
                        if (fSuccess)
                            popstack(stack);
                        else
                            return set_error(serror, SCRIPT_ERROR_CHECKMULTISIGVERIFY);
                    }
                }
                break;

                default:
                    return set_error(serror, SCRIPT_ERROR_BAD_OPCODE);
            }

            if (stack.size() + altstack.size() > MAX_STACK_SIZE)
                return set_error(serror, SCRIPT_ERROR_STACK_SIZE);
        }
    }
    catch (...)
    {
        return set_error(serror, SCRIPT_ERROR_UNKNOWN_ERROR);
    }

    if (!vfExec.empty())
        return set_error(serror, SCRIPT_ERROR_UNBALANCED_CONDITIONAL);

    return set_success(serror);
}

#define stacktop(i)  (stack.at(stack.size()+(i)))
#define altstacktop(i)  (altstack.at(altstack.size()+(i)))

bool static CheckMinimalPush(const valtype& data, opertaionCodetype opcode) {
    if (data.size() == 0) {
        return opcode == OPERATION_0;
    } else if (data.size() == 1 && data[0] >= 1 && data[0] <= 16) {
        return opcode == OPERATION_1 + (data[0] - 1);
    } else if (data.size() == 1 && data[0] == 0x81) {
        return opcode == OPERATION_1NEGATE;
    } else if (data.size() <= 75) {
        return opcode == data.size();
    } else if (data.size() <= 255) {
        return opcode == OPERATION_PUSHDATA1;
    } else if (data.size() <= 65535) {
        return opcode == OPERATION_PUSHDATA2;
    }
    return true;
}

bool static CheckPubKeyEncoding(const valtype &vchPubKey, unsigned int flags, const SigVersion &sigversion, ScriptError* serror) {
    if ((flags & SCRIPT_VERIFY_STRICTENC) != 0 && !IsCompressedOrUncompressedPubKey(vchPubKey)) {
        return set_error(serror, SCRIPT_ERROR_PUBKEYTYPE);
    }
    if ((flags & SCRIPT_VERIFY_WITNESS_PUBKEYTYPE) != 0 && sigversion == SIGVERSION_WITNESS_V0 && !IsCompressedPubKey(vchPubKey)) {
        return set_error(serror, SCRIPT_ERROR_WITNESS_PUBKEYTYPE);
    }
    return true;
}

bool CheckSignatureEncoding(const std::vector<unsigned char> &vchSig, unsigned int flags, ScriptError* serror) {
    if (vchSig.size() == 0) {
        return true;
    }
    if ((flags & (SCRIPT_VERIFY_DERSIG | SCRIPT_VERIFY_LOW_S | SCRIPT_VERIFY_STRICTENC)) != 0 && !IsValidSignatureEncoding(vchSig)) {
        return set_error(serror, SCRIPT_ERROR_SIG_DER);
    } else if ((flags & SCRIPT_VERIFY_LOW_S) != 0 && !IsLowDERSignature(vchSig, serror)) {
        return false;
    } else if ((flags & SCRIPT_VERIFY_STRICTENC) != 0) {
        if (!IsDefinedHashtypeSignature(vchSig))
            return set_error(serror, SCRIPT_ERROR_SIG_HASHTYPE);

        bool requiresForkId = !AllowsNonForkId(flags);
        bool usesForkId = UsesForkId(vchSig);
        if (requiresForkId && !usesForkId)
            return set_error(serror, SCRIPT_ERROR_SIG_HASHTYPE);
    }
    return true;
}

bool static AllowsNonForkId(unsigned int flags) {
    return flags & SCRIPT_ALLOW_NON_FORKID;
}

bool static UsesForkId(const valtype &vchSig) {
    uint32_t nHashType = GetHashType(vchSig);
    return UsesForkId(nHashType);
}

bool static UsesForkId(uint32_t nHashType) {
    return nHashType & SIGHASH_FORKID;
}

bool static IsDefinedHashtypeSignature(const valtype &vchSig) {
    if (vchSig.size() == 0) {
        return false;
    }
    unsigned char nHashType = GetHashType(vchSig) & (~(SIGHASH_ANYONECANPAY | SIGHASH_FORKID));
    if (nHashType < SIGHASH_ALL || nHashType > SIGHASH_SINGLE)
        return false;

    return true;
}

bool static IsLowDERSignature(const valtype &vchSig, ScriptError* serror) {
    if (!IsValidSignatureEncoding(vchSig)) {
        return set_error(serror, SCRIPT_ERROR_SIG_DER);
    }
    std::vector<unsigned char> vchSigCopy(vchSig.begin(), vchSig.begin() + vchSig.size() - 1);
    if (!CPublicKeyID::CheckLowS(vchSigCopy)) {
        return set_error(serror, SCRIPT_ERROR_SIG_HIGH_S);
    }
    return true;
}

uint32_t static GetHashType(const valtype &vchSig) {
    if (vchSig.size() == 0)
        return 0;
    return vchSig.back();
}

bool static IsValidSignatureEncoding(const std::vector<unsigned char> &sig) {
    if (sig.size() < 9) return false;
    if (sig.size() > 73) return false;

    if (sig[0] != 0x30) return false;

    if (sig[1] != sig.size() - 3) return false;

    unsigned int lenR = sig[3];

    if (5 + lenR >= sig.size()) return false;

    unsigned int lenS = sig[5 + lenR];

    if ((size_t)(lenR + lenS + 7) != sig.size()) return false;

    if (sig[2] != 0x02) return false;

    if (lenR == 0) return false;

    if (sig[4] & 0x80) return false;

    if (lenR > 1 && (sig[4] == 0x00) && !(sig[5] & 0x80)) return false;

    if (sig[lenR + 4] != 0x02) return false;

    if (lenS == 0) return false;

    if (sig[lenR + 6] & 0x80) return false;

    if (lenS > 1 && (sig[lenR + 6] == 0x00) && !(sig[lenR + 7] & 0x80)) return false;

    return true;
}

bool static IsCompressedPubKey(const valtype &vchPubKey) {
    if (vchPubKey.size() != 33) {
        return false;
    }
    if (vchPubKey[0] != 0x02 && vchPubKey[0] != 0x03) {
        return false;
    }
    return true;
}

bool static IsCompressedOrUncompressedPubKey(const valtype &vchPubKey) {
    if (vchPubKey.size() < 33) {
        return false;
    }
    if (vchPubKey[0] == 0x04) {
        if (vchPubKey.size() != 65) {
            return false;
        }
    } else if (vchPubKey[0] == 0x02 || vchPubKey[0] == 0x03) {
        if (vchPubKey.size() != 33) {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

static inline void popstack(std::vector<valtype>& stack)
{
    if (stack.empty())
        throw std::runtime_error("popstack(): stack empty");
    stack.pop_back();
}

bool CastToBool(const valtype& vch)
{
    for (unsigned int i = 0; i < vch.size(); i++)
    {
        if (vch[i] != 0)
        {
            if (i == vch.size()-1 && vch[i] == 0x80)
                return false;
            return true;
        }
    }
    return false;
}

namespace {

class ChkTxSignatureSerializer {
private:
    const ChkTx& txTo;
    const CScript& scriptCode;
    const unsigned int nIn;
    const bool fAnyoneCanPay;
    const bool fForkID;
    const bool fHashSingle;
    const bool fHashNone;

public:
    ChkTxSignatureSerializer(const ChkTx &txToIn, const CScript &scriptCodeIn, unsigned int nInIn, int nHashTypeIn) :
        txTo(txToIn), scriptCode(scriptCodeIn), nIn(nInIn),
        fAnyoneCanPay(!!(nHashTypeIn & SIGHASH_ANYONECANPAY)),
        fForkID(!!(nHashTypeIn & SIGHASH_FORKID)),
        fHashSingle((nHashTypeIn & 0x1f) == SIGHASH_SINGLE),
        fHashNone((nHashTypeIn & 0x1f) == SIGHASH_NONE) {}

    template<typename S>
    void SerializeScriptCode(S &s) const {
        CScript::const_iterator it = scriptCode.begin();
        CScript::const_iterator itBegin = it;
        opertaionCodetype opcode;
        unsigned int nCodeSeparators = 0;
        while (scriptCode.GetOperator(it, opcode)) {
            if (opcode == OPERATION_CODESEPARATOR)
                nCodeSeparators++;
        }
        ::WriteCompactSize(s, scriptCode.size() - nCodeSeparators);
        it = itBegin;
        while (scriptCode.GetOperator(it, opcode)) {
            if (opcode == OPERATION_CODESEPARATOR) {
                s.write((char*)&itBegin[0], it-itBegin-1);
                itBegin = it;
            }
        }
        if (itBegin != scriptCode.end())
            s.write((char*)&itBegin[0], it-itBegin);
    }

    template<typename S>
    void SerializeInput(S &s, unsigned int nInput) const {
        if (fAnyoneCanPay)
            nInput = nIn;
        ::Serialize(s, txTo.vin[nInput].prevout);
        if (nInput != nIn)
            ::Serialize(s, CScript());
        else
            SerializeScriptCode(s);
        if (nInput != nIn && (fHashSingle || fHashNone))
            ::Serialize(s, (int)0);
        else
            ::Serialize(s, txTo.vin[nInput].nSeq);
    }

    template<typename S>
    void SerializeOutput(S &s, unsigned int nOutput) const {
        if (fHashSingle && nOutput != nIn)
            ::Serialize(s, ChkTxOut());
        else
            ::Serialize(s, txTo.vout[nOutput]);
    }

    template<typename S>
    void Serialize(S &s) const {
        ::Serialize(s, txTo.nVersion);
        unsigned int nInputs = fAnyoneCanPay ? 1 : txTo.vin.size();
        ::WriteCompactSize(s, nInputs);
        for (unsigned int nInput = 0; nInput < nInputs; nInput++)
             SerializeInput(s, nInput);
        unsigned int nOutputs = fHashNone ? 0 : (fHashSingle ? nIn+1 : txTo.vout.size());
        ::WriteCompactSize(s, nOutputs);
        for (unsigned int nOutput = 0; nOutput < nOutputs; nOutput++)
             SerializeOutput(s, nOutput);
        ::Serialize(s, txTo.nLockTime);
    }
};

uint256 GetSeqHash(const ChkTx& txTo) {
    CHashWriter ss(SER_GETHASH, 0);
    for (const auto& txin : txTo.vin) {
        ss << txin.nSeq;
    }
    return ss.GetHash();
}

uint256 GetOutputsHash(const ChkTx& txTo) {
    CHashWriter ss(SER_GETHASH, 0);
    for (const auto& txout : txTo.vout) {
        ss << txout;
    }
    return ss.GetHash();
}

uint256 GetPrevoutHash(const ChkTx& txTo) {
    CHashWriter ss(SER_GETHASH, 0);
    for (const auto& txin : txTo.vin) {
        ss << txin.prevout;
    }
    return ss.GetHash();
}
}

size_t CountWitnessSigOps(const CScript& scriptSig, const CScript& scriptPubKey, const CScriptWitnesses* witness, unsigned int flags)
{
    static const CScriptWitnesses witnessEmpty;

    if ((flags & SCRIPT_VERIFY_WITNESS) == 0) {
        return 0;
    }
    assert((flags & SCRIPT_VERIFY_P2SH) != 0);

    int witnessversion;
    std::vector<unsigned char> witnessprogram;
    if (scriptPubKey.IsWitnessProgram(witnessversion, witnessprogram)) {
        return WitnessSigOps(witnessversion, witnessprogram, witness ? *witness : witnessEmpty, flags);
    }

    if (scriptPubKey.IsPayToHash() && scriptSig.IsPushingOnly()) {
        CScript::const_iterator pc = scriptSig.begin();
        std::vector<unsigned char> data;
        while (pc < scriptSig.end()) {
            opertaionCodetype opcode;
            scriptSig.GetOperator(pc, opcode, data);
        }
        CScript subscript(data.begin(), data.end());
        if (subscript.IsWitnessProgram(witnessversion, witnessprogram)) {
            return WitnessSigOps(witnessversion, witnessprogram, witness ? *witness : witnessEmpty, flags);
        }
    }

    return 0;
}

size_t static WitnessSigOps(int witversion, const std::vector<unsigned char>& witprogram, const CScriptWitnesses& witness, int flags)
{
    if (witversion == 0) {
        if (witprogram.size() == 20)
            return 1;

        if (witprogram.size() == 32 && witness.stack.size() > 0) {
            CScript subscript(witness.stack.back().begin(), witness.stack.back().end());
            return subscript.GetSignatureOperationCount(true);
        }
    }

    return 0;
}

bool VerifyScript(const CScript& scriptSig, const CScript& scriptPubKey, const CScriptWitnesses* witness, unsigned int flags, const BaseSignatureChecker& checker, ScriptError* serror)
{
    static const CScriptWitnesses emptyWitness;
    if (witness == nullptr) {
        witness = &emptyWitness;
    }
    bool hadWitness = false;

    set_error(serror, SCRIPT_ERROR_UNKNOWN_ERROR);

    if ((flags & SCRIPT_VERIFY_SIGPUSHONLY) != 0 && !scriptSig.IsPushingOnly()) {
        return set_error(serror, SCRIPT_ERROR_SIG_PUSHONLY);
    }

    std::vector<std::vector<unsigned char> > stack, stackCopy;
    if (stack.empty())
        return set_error(serror, SCRIPT_ERROR_EVAL_FALSE);
    if (CastToBool(stack.back()) == false)
        return set_error(serror, SCRIPT_ERROR_EVAL_FALSE);

    int witnessversion;
    std::vector<unsigned char> witnessprogram;
    if (flags & SCRIPT_VERIFY_WITNESS) {
        if (scriptPubKey.IsWitnessProgram(witnessversion, witnessprogram)) {
            hadWitness = true;
            if (scriptSig.size() != 0) {
                return set_error(serror, SCRIPT_ERROR_WITNESS_MALLEATED);
            }
            if (!VerifyWitnessProgram(*witness, witnessversion, witnessprogram, flags, checker, serror)) {
                return false;
            }
            stack.resize(1);
        }
    }

    if ((flags & SCRIPT_VERIFY_P2SH) && scriptPubKey.IsPayToHash())
    {
        if (!scriptSig.IsPushingOnly())
            return set_error(serror, SCRIPT_ERROR_SIG_PUSHONLY);

        swap(stack, stackCopy);

        assert(!stack.empty());

        const valtype& pubKeySerialized = stack.back();
        CScript pubKey2(pubKeySerialized.begin(), pubKeySerialized.end());
        popstack(stack);

        if (stack.empty())
            return set_error(serror, SCRIPT_ERROR_EVAL_FALSE);
        if (!CastToBool(stack.back()))
            return set_error(serror, SCRIPT_ERROR_EVAL_FALSE);

        if (flags & SCRIPT_VERIFY_WITNESS) {
            if (pubKey2.IsWitnessProgram(witnessversion, witnessprogram)) {
                hadWitness = true;
                if (scriptSig != CScript() << std::vector<unsigned char>(pubKey2.begin(), pubKey2.end())) {
                    return set_error(serror, SCRIPT_ERROR_WITNESS_MALLEATED_P2SH);
                }
                if (!VerifyWitnessProgram(*witness, witnessversion, witnessprogram, flags, checker, serror)) {
                    return false;
                }
                stack.resize(1);
            }
        }
    }

    if ((flags & SCRIPT_VERIFY_CLEANSTACK) != 0) {
        assert((flags & SCRIPT_VERIFY_P2SH) != 0);
        assert((flags & SCRIPT_VERIFY_WITNESS) != 0);
        if (stack.size() != 1) {
            return set_error(serror, SCRIPT_ERROR_CLEANSTACK);
        }
    }

    if (flags & SCRIPT_VERIFY_WITNESS) {
        assert((flags & SCRIPT_VERIFY_P2SH) != 0);
        if (!hadWitness && !witness->IsNull()) {
            return set_error(serror, SCRIPT_ERROR_WITNESS_UNEXPECTED);
        }
    }

    return set_success(serror);
}

static bool VerifyWitnessProgram(const CScriptWitnesses& witness, int witversion, const std::vector<unsigned char>& program, unsigned int flags, const BaseSignatureChecker& checker, ScriptError* serror)
{
    std::vector<std::vector<unsigned char> > stack;
    CScript scriptPubKey;

    if (witversion == 0) {
        if (program.size() == 32) {
            if (witness.stack.size() == 0) {
                return set_error(serror, SCRIPT_ERROR_WITNESS_PROGRAM_WITNESS_EMPTY);
            }
            scriptPubKey = CScript(witness.stack.back().begin(), witness.stack.back().end());
            stack = std::vector<std::vector<unsigned char> >(witness.stack.begin(), witness.stack.end() - 1);
            uint256 hashScriptPubKey;
            CSHA256().Write(&scriptPubKey[0], scriptPubKey.size()).Finalize(hashScriptPubKey.begin());
            if (memcmp(hashScriptPubKey.begin(), &program[0], 32)) {
                return set_error(serror, SCRIPT_ERROR_WITNESS_PROGRAM_MISMATCH);
            }
        } else if (program.size() == 20) {
            if (witness.stack.size() != 2) {
                return set_error(serror, SCRIPT_ERROR_WITNESS_PROGRAM_MISMATCH); // 2 items in witness
            }
            scriptPubKey << OPERATION_DUP << OPERATION_HASH160 << program << OPERATION_EQUALVERIFY << OPERATION_CHECKSIG;
            stack = witness.stack;
        } else {
            return set_error(serror, SCRIPT_ERROR_WITNESS_PROGRAM_WRONG_LENGTH);
        }
    } else if (flags & SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_WITNESS_PROGRAM) {
        return set_error(serror, SCRIPT_ERROR_DISCOURAGE_UPGRADABLE_WITNESS_PROGRAM);
    } else {
        return set_success(serror);
    }

    for (unsigned int i = 0; i < stack.size(); i++) {
        if (stack.at(i).size() > MAX_SCRIPT_ELEMENT_SIZE)
            return set_error(serror, SCRIPT_ERROR_PUSH_SIZE);
    }

    if (!EvalScript(stack, scriptPubKey, flags, checker, SIGVERSION_WITNESS_V0, serror)) {
        return false;
    }

    if (stack.size() != 1)
        return set_error(serror, SCRIPT_ERROR_EVAL_FALSE);
    if (!CastToBool(stack.back()))
        return set_error(serror, SCRIPT_ERROR_EVAL_FALSE);
    return true;
}

bool TxSignatureChecker::CheckSeq(const CScriptNum& nSeq) const
{
    const int64_t txToSeq = (int64_t)txTo->vin[nIn].nSeq;

    if (static_cast<uint32_t>(txTo->nVersion) < 2)
        return false;

    if (txToSeq & ChkTxIn::SEQUENCE_LOCKTIME_DISABLE_FLAG)
        return false;
    const uint32_t nLockTimeMask = ChkTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG | ChkTxIn::SEQUENCE_LOCKTIME_MASK;
    const int64_t txToSeqMasked = txToSeq & nLockTimeMask;
    const CScriptNum nSeqMasked = nSeq & nLockTimeMask;

    if (!(
        (txToSeqMasked <  ChkTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG && nSeqMasked <  ChkTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG) ||
        (txToSeqMasked >= ChkTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG && nSeqMasked >= ChkTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG)
    )) {
        return false;
    }

    if (nSeqMasked > txToSeqMasked)
        return false;

    return true;
}

bool TxSignatureChecker::CheckLockTime(const CScriptNum& nLockTime) const
{
    if (!(
        (txTo->nLockTime <  LOCKTIME_THRESHOLD && nLockTime <  LOCKTIME_THRESHOLD) ||
        (txTo->nLockTime >= LOCKTIME_THRESHOLD && nLockTime >= LOCKTIME_THRESHOLD)
    ))
        return false;

    if (nLockTime > (int64_t)txTo->nLockTime)
        return false;

    if (ChkTxIn::SEQUENCE_FINAL == txTo->vin[nIn].nSeq)
        return false;

    return true;
}

bool TxSignatureChecker::CheckSig(const std::vector<unsigned char>& vchSigIn, const std::vector<unsigned char>& vchPubKey, const CScript& scriptCode, SigVersion sigversion) const
{
    CPublicKeyID pubkey(vchPubKey);
    if (!pubkey.IsValid())
        return false;

    std::vector<unsigned char> vchSig(vchSigIn);
    if (vchSig.empty())
        return false;
    int nHashType = GetHashType(vchSig);
    vchSig.pop_back();

    uint256 sighash = SignatureHash(scriptCode, *txTo, nIn, nHashType, amount, sigversion, this->txdata);

    if (!VerifyingSignature(vchSig, pubkey, sighash))
        return false;

    return true;
}

bool TxSignatureChecker::VerifyingSignature(const std::vector<unsigned char>& vchSig, const CPublicKeyID& pubkey, const uint256& sighash) const
{
    return pubkey.Verify(sighash, vchSig);
}

uint256 SignatureHash(const CScript& scriptCode, const ChkTx& txTo, unsigned int nIn, int nHashType, const CAmount& amount, SigVersion sigversion, const PrecomputedTxData* cache, const int forkid)
{
    int nForkHashType = nHashType;
    if (UsesForkId(nHashType))
        nForkHashType |= forkid << 8;

    if (sigversion == SIGVERSION_WITNESS_V0 || UsesForkId(nHashType)) {
        uint256 hashPrevouts;
        uint256 hashSeq;
        uint256 hashOutputs;

        if (!(nHashType & SIGHASH_ANYONECANPAY)) {
            hashPrevouts = cache ? cache->hashPrevouts : GetPrevoutHash(txTo);
        }

        if (!(nHashType & SIGHASH_ANYONECANPAY) && (nHashType & 0x1f) != SIGHASH_SINGLE && (nHashType & 0x1f) != SIGHASH_NONE) {
            hashSeq = cache ? cache->hashSeq : GetSeqHash(txTo);
        }

        if ((nHashType & 0x1f) != SIGHASH_SINGLE && (nHashType & 0x1f) != SIGHASH_NONE) {
            hashOutputs = cache ? cache->hashOutputs : GetOutputsHash(txTo);
        } else if ((nHashType & 0x1f) == SIGHASH_SINGLE && nIn < txTo.vout.size()) {
            CHashWriter ss(SER_GETHASH, 0);
            ss << txTo.vout[nIn];
            hashOutputs = ss.GetHash();
        }

        CHashWriter ss(SER_GETHASH, 0);
        ss << txTo.nVersion;
        ss << hashPrevouts;
        ss << hashSeq;
        ss << txTo.vin[nIn].prevout;
        ss << scriptCode;
        ss << amount;
        ss << txTo.vin[nIn].nSeq;
        ss << hashOutputs;
        ss << txTo.nLockTime;
        ss << nForkHashType;

        return ss.GetHash();
    }

    static const uint256 one(uint256S("0000000000000000000000000000000000000000000000000000000000000001"));
    if (nIn >= txTo.vin.size()) {
        return one;
    }

    if ((nHashType & 0x1f) == SIGHASH_SINGLE) {
        if (nIn >= txTo.vout.size()) {
            return one;
        }
    }

    ChkTxSignatureSerializer txTmp(txTo, scriptCode, nIn, nHashType);

    CHashWriter ss(SER_GETHASH, 0);
    ss << txTmp << nForkHashType;
    return ss.GetHash();
}

PrecomputedTxData::PrecomputedTxData(const ChkTx& txTo)
{
    hashPrevouts = GetPrevoutHash(txTo);
    hashSeq = GetSeqHash(txTo);
    hashOutputs = GetOutputsHash(txTo);
}

                                                                                       