
#include "script_error.h"

const char* ScriptErrorString(const ScriptError serror)
{
    switch (serror)
    {
        case SCRIPT_ERROR_OK:
            return "No error";
        case SCRIPT_ERROR_EVAL_FALSE:
            return "Script evaluated without error but finished with a false/empty top stack element";
        case SCRIPT_ERROR_VERIFY:
            return "Script failed an OPERATION_VERIFY operation";
        case SCRIPT_ERROR_EQUALVERIFY:
            return "Script failed an OPERATION_EQUALVERIFY operation";
        case SCRIPT_ERROR_CHECKMULTISIGVERIFY:
            return "Script failed an OPERATION_CHECKMULTISIGVERIFY operation";
        case SCRIPT_ERROR_CHECKSIGVERIFY:
            return "Script failed an OPERATION_CHECKSIGVERIFY operation";
        case SCRIPT_ERROR_NUMEQUALVERIFY:
            return "Script failed an OPERATION_NUMEQUALVERIFY operation";
        case SCRIPT_ERROR_SCRIPT_SIZE:
            return "Script is too big";
        case SCRIPT_ERROR_PUSH_SIZE:
            return "Push value size limit exceeded";
        case SCRIPT_ERROR_OPERATION_COUNT:
            return "Operation limit exceeded";
        case SCRIPT_ERROR_STACK_SIZE:
            return "Stack size limit exceeded";
        case SCRIPT_ERROR_SIG_COUNT:
            return "Signature count negative or greater than pubkey count";
        case SCRIPT_ERROR_PUBKEY_COUNT:
            return "Pubkey count negative or limit exceeded";
        case SCRIPT_ERROR_BAD_OPCODE:
            return "Opcode missing or not understood";
        case SCRIPT_ERROR_DISABLED_OPCODE:
            return "Attempted to use a disabled opcode";
        case SCRIPT_ERROR_INVALID_STACK_OPERATION:
            return "Operation not valid with the current stack size";
        case SCRIPT_ERROR_INVALID_ALTSTACK_OPERATION:
            return "Operation not valid with the current altstack size";
        case SCRIPT_ERROR_OPERATION_RETURN:
            return "OPERATION_RETURN was encountered";
        case SCRIPT_ERROR_UNBALANCED_CONDITIONAL:
            return "Invalid OPERATION_IF construction";
        case SCRIPT_ERROR_NEGATIVE_LOCKTIME:
            return "Negative locktime";
        case SCRIPT_ERROR_UNSATISFIED_LOCKTIME:
            return "Locktime requirement not satisfied";
        case SCRIPT_ERROR_SIG_HASHTYPE:
            return "Signature hash type missing or not understood";
        case SCRIPT_ERROR_SIG_DER:
            return "Non-canonical DER signature";
        case SCRIPT_ERROR_MINIMALDATA:
            return "Data push larger than necessary";
        case SCRIPT_ERROR_SIG_PUSHONLY:
            return "Only non-push operators allowed in signatures";
        case SCRIPT_ERROR_SIG_HIGH_S:
            return "Non-canonical signature: S value is unnecessarily high";
        case SCRIPT_ERROR_SIG_NULLDUMMY:
            return "Dummy CHECKMULTISIG argument must be zero";
        case SCRIPT_ERROR_MINIMALIF:
            return "OPERATION_IF/NOTIF argument must be minimal";
        case SCRIPT_ERROR_SIG_NULLFAIL:
            return "Signature must be zero for failed CHECK(MULTI)SIG operation";
        case SCRIPT_ERROR_DISCOURAGE_UPGRADABLE_NOPS:
            return "NOPx reserved for soft-fork upgrades";
        case SCRIPT_ERROR_DISCOURAGE_UPGRADABLE_WITNESS_PROGRAM:
            return "Witness version reserved for soft-fork upgrades";
        case SCRIPT_ERROR_PUBKEYTYPE:
            return "Public key is neither compressed or uncompressed";
        case SCRIPT_ERROR_WITNESS_PROGRAM_WRONG_LENGTH:
            return "Witness program has incorrect length";
        case SCRIPT_ERROR_WITNESS_PROGRAM_WITNESS_EMPTY:
            return "Witness program was passed an empty witness";
        case SCRIPT_ERROR_WITNESS_PROGRAM_MISMATCH:
            return "Witness program hash mismatch";
        case SCRIPT_ERROR_WITNESS_MALLEATED:
            return "Witness requires empty scriptSig";
        case SCRIPT_ERROR_WITNESS_MALLEATED_P2SH:
            return "Witness requires only-redeemscript scriptSig";
        case SCRIPT_ERROR_WITNESS_UNEXPECTED:
            return "Witness provided for non-witness script";
        case SCRIPT_ERROR_WITNESS_PUBKEYTYPE:
            return "Using non-compressed keys in segwit";
        case SCRIPT_ERROR_MUST_USE_FORKID:
            return "Signature must use SIGHASH_FORKID";
        case SCRIPT_ERROR_UNKNOWN_ERROR:
        case SCRIPT_ERROR_ERROR_COUNT:
        default: break;
    }
    return "unknown error";
}
                                                                                                                                                            