
#include "validation.h"
#include "../util.h"

bool CValidationState::DoS(int level, bool ret, unsigned int chRejectCodeIn, const std::string& strRejectReasonIn, bool corruptionIn, const std::string& strDebugMessageIn)
{
    LogPrintfd("level[%d], ret[%d], chRejectCodeIn[%d], strRejectReasonIn[%s], corruptionIn[%d], strDebugMessageIn[%s]",
        level, ret, chRejectCodeIn, strRejectReasonIn, corruptionIn, strDebugMessageIn);
    chRejectCode = chRejectCodeIn;
    strRejectReason = strRejectReasonIn;
    corruptionPossible = corruptionIn;
    strDebugMessage = strDebugMessageIn;
    if (mode == MODE_ERROR)
        return ret;
    nDoS += level;
    mode = MODE_INVALID;
    return ret;
}
