#pragma once

#include <afxstr.h>

enum class MissionStatus
{
    Planned,
    Ready,
    Launching,
    Completed,
    Aborted
};

enum class LaunchState
{
    Idle,
    Countdown,
    Ascending,
    Paused,
    Completed,
    Aborted
};

struct MissionInfo
{
    CString missionId;
    CString missionName;
    CString rocketName;
    CString payloadName;
    CString destination;
    CString launchTime;
    MissionStatus status = MissionStatus::Planned;
};

struct CheckResult
{
    CString missionId;
    BOOL propulsionReady = FALSE;
    BOOL navigationReady = FALSE;
    BOOL communicationReady = FALSE;
    BOOL powerReady = FALSE;
    BOOL weatherReady = FALSE;
    CString remarks;
};

struct LaunchRecord
{
    CString missionId;
    CString missionName;
    CString launchTime;
    CString result;
    double maxHeightKm = 0.0;
    CString description;
};

CString MissionStatusToString(MissionStatus status);
BOOL StringToMissionStatus(const CString& text, MissionStatus& status);
