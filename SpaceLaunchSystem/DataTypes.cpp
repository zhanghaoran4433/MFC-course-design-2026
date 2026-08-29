#include "pch.h"
#include "DataTypes.h"

CString MissionStatusToString(MissionStatus status)
{
    switch (status)
    {
    case MissionStatus::Planned:
        return _T("\u8ba1\u5212\u4e2d");
    case MissionStatus::Ready:
        return _T("\u5df2\u5c31\u7eea");
    case MissionStatus::Launching:
        return _T("\u53d1\u5c04\u4e2d");
    case MissionStatus::Completed:
        return _T("\u5df2\u5b8c\u6210");
    case MissionStatus::Aborted:
        return _T("\u5df2\u4e2d\u6b62");
    default:
        return CString();
    }
}

BOOL StringToMissionStatus(const CString& text, MissionStatus& status)
{
    if (text == _T("\u8ba1\u5212\u4e2d"))
    {
        status = MissionStatus::Planned;
        return TRUE;
    }
    if (text == _T("\u5df2\u5c31\u7eea"))
    {
        status = MissionStatus::Ready;
        return TRUE;
    }
    if (text == _T("\u53d1\u5c04\u4e2d"))
    {
        status = MissionStatus::Launching;
        return TRUE;
    }
    if (text == _T("\u5df2\u5b8c\u6210"))
    {
        status = MissionStatus::Completed;
        return TRUE;
    }
    if (text == _T("\u5df2\u4e2d\u6b62"))
    {
        status = MissionStatus::Aborted;
        return TRUE;
    }
    return FALSE;
}
