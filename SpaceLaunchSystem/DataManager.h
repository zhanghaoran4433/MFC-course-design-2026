#pragma once

#include "DataTypes.h"

#include <cstddef>
#include <vector>

class CWnd;

class CDataManager
{
public:
    BOOL Initialize();
    BOOL LoadAll();
    BOOL SaveAll();

    std::vector<MissionInfo>& GetMissions();
    const std::vector<MissionInfo>& GetMissions() const;
    const std::vector<LaunchRecord>& GetRecords() const;

    MissionInfo* FindMission(const CString& missionId);
    const MissionInfo* FindMission(const CString& missionId) const;
    CheckResult* FindCheckResult(const CString& missionId);

    BOOL AddMission(const MissionInfo& mission, CString& errorMessage);
    BOOL UpdateMission(const CString& originalId,
        const MissionInfo& mission, CString& errorMessage);
    BOOL DeleteMission(const CString& missionId);
    void SetCheckResult(const CheckResult& result);
    void AddLaunchRecord(const LaunchRecord& record);
    BOOL DeleteLaunchRecord(size_t index);

    BOOL IsModified() const;
    void MarkModified();
    void MarkSaved();
    BOOL ConfirmSaveBeforeClose(CWnd* pOwner);

private:
    std::vector<MissionInfo> m_missions;
    std::vector<CheckResult> m_checkResults;
    std::vector<LaunchRecord> m_records;
    CString m_dataDirectory;
    BOOL m_isModified = FALSE;
};
