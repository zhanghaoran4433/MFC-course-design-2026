#include "pch.h"
#include "DataManager.h"

#include <algorithm>
#include <climits>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

namespace
{
const CString kMissionsFile = _T("missions.tsv");
const CString kChecksFile = _T("checks.tsv");
const CString kRecordsFile = _T("launch_records.tsv");

CString JoinPath(const CString& directory, const CString& fileName)
{
    CString path(directory);
    if (!path.IsEmpty() && path[path.GetLength() - 1] != _T('\\'))
    {
        path += _T('\\');
    }
    path += fileName;
    return path;
}

BOOL IsMissingFileError(DWORD errorCode)
{
    return errorCode == ERROR_FILE_NOT_FOUND || errorCode == ERROR_PATH_NOT_FOUND;
}

BOOL ReadUtf8File(const CString& path, CString& text)
{
    text.Empty();
    if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
    {
        return IsMissingFileError(GetLastError());
    }

    try
    {
        CFile file(path, CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary);
        const ULONGLONG length = file.GetLength();
        if (length > static_cast<ULONGLONG>(INT_MAX))
        {
            return FALSE;
        }

        std::string bytes(static_cast<size_t>(length), '\0');
        if (!bytes.empty())
        {
            const UINT bytesRead = file.Read(&bytes[0], static_cast<UINT>(bytes.size()));
            if (bytesRead != bytes.size())
            {
                return FALSE;
            }
        }
        file.Close();

        if (bytes.empty())
        {
            return TRUE;
        }

        const int wideLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
            bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
        if (wideLength <= 0)
        {
            return FALSE;
        }

        std::wstring wideText(static_cast<size_t>(wideLength), L'\0');
        if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes.data(),
            static_cast<int>(bytes.size()), &wideText[0], wideLength) != wideLength)
        {
            return FALSE;
        }

        text = CString(wideText.data(), wideLength);
        if (!text.IsEmpty() && text[0] == 0xFEFF)
        {
            text.Delete(0);
        }
        return TRUE;
    }
    catch (CFileException*)
    {
        return FALSE;
    }
}

BOOL WriteUtf8File(const CString& path, const CString& text)
{
    const int utf8Length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
        text.GetString(), text.GetLength(), nullptr, 0, nullptr, nullptr);
    if (utf8Length == 0 && !text.IsEmpty())
    {
        return FALSE;
    }

    std::string bytes(static_cast<size_t>(utf8Length), '\0');
    if (utf8Length > 0 && WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
        text.GetString(), text.GetLength(), &bytes[0], utf8Length,
        nullptr, nullptr) != utf8Length)
    {
        return FALSE;
    }

    try
    {
        CFile file(path, CFile::modeCreate | CFile::modeWrite |
            CFile::shareExclusive | CFile::typeBinary);
        if (!bytes.empty())
        {
            file.Write(bytes.data(), static_cast<UINT>(bytes.size()));
        }
        file.Flush();
        file.Close();
        return TRUE;
    }
    catch (CFileException*)
    {
        return FALSE;
    }
}

std::vector<CString> Split(const CString& text, TCHAR delimiter)
{
    std::vector<CString> fields;
    int start = 0;
    while (start <= text.GetLength())
    {
        const int position = text.Find(delimiter, start);
        if (position < 0)
        {
            fields.push_back(text.Mid(start));
            break;
        }
        fields.push_back(text.Mid(start, position - start));
        start = position + 1;
    }
    return fields;
}

template <typename Handler>
void ForEachTsvLine(const CString& text, Handler handler)
{
    int start = 0;
    while (start < text.GetLength())
    {
        int end = text.Find(_T('\n'), start);
        if (end < 0)
        {
            end = text.GetLength();
        }

        CString line = text.Mid(start, end - start);
        if (!line.IsEmpty() && line[line.GetLength() - 1] == _T('\r'))
        {
            line.Delete(line.GetLength() - 1);
        }
        if (!line.IsEmpty())
        {
            handler(Split(line, _T('\t')));
        }
        start = end + 1;
    }
}

BOOL ParseBoolean(const CString& text, BOOL& value)
{
    if (text == _T("0"))
    {
        value = FALSE;
        return TRUE;
    }
    if (text == _T("1"))
    {
        value = TRUE;
        return TRUE;
    }
    return FALSE;
}

BOOL ParseDouble(const CString& text, double& value)
{
    std::wistringstream stream(std::wstring(text.GetString()));
    stream.imbue(std::locale::classic());
    stream >> value;
    stream >> std::ws;
    return !stream.fail() && stream.eof();
}

CString FormatDouble(double value)
{
    std::wostringstream stream;
    stream.imbue(std::locale::classic());
    stream.precision(15);
    stream << value;
    return CString(stream.str().c_str());
}

BOOL IsValidField(const CString& value)
{
    return value.FindOneOf(_T("\t\r\n")) < 0;
}

BOOL LoadMissions(const CString& path, std::vector<MissionInfo>& missions)
{
    CString text;
    if (!ReadUtf8File(path, text))
    {
        return FALSE;
    }

    ForEachTsvLine(text, [&missions](const std::vector<CString>& fields)
        {
            if (fields.size() != 7)
            {
                return;
            }
            MissionInfo mission;
            MissionStatus status;
            if (!StringToMissionStatus(fields[6], status))
            {
                return;
            }
            mission.missionId = fields[0];
            mission.missionName = fields[1];
            mission.rocketName = fields[2];
            mission.payloadName = fields[3];
            mission.destination = fields[4];
            mission.launchTime = fields[5];
            mission.status = status;
            missions.push_back(mission);
        });
    return TRUE;
}

BOOL LoadChecks(const CString& path, std::vector<CheckResult>& checks)
{
    CString text;
    if (!ReadUtf8File(path, text))
    {
        return FALSE;
    }

    ForEachTsvLine(text, [&checks](const std::vector<CString>& fields)
        {
            if (fields.size() != 7)
            {
                return;
            }
            CheckResult result;
            if (!ParseBoolean(fields[1], result.propulsionReady) ||
                !ParseBoolean(fields[2], result.navigationReady) ||
                !ParseBoolean(fields[3], result.communicationReady) ||
                !ParseBoolean(fields[4], result.powerReady) ||
                !ParseBoolean(fields[5], result.weatherReady))
            {
                return;
            }
            result.missionId = fields[0];
            result.remarks = fields[6];
            checks.push_back(result);
        });
    return TRUE;
}

BOOL LoadRecords(const CString& path, std::vector<LaunchRecord>& records)
{
    CString text;
    if (!ReadUtf8File(path, text))
    {
        return FALSE;
    }

    ForEachTsvLine(text, [&records](const std::vector<CString>& fields)
        {
            if (fields.size() != 6)
            {
                return;
            }
            LaunchRecord record;
            if (!ParseDouble(fields[4], record.maxHeightKm) ||
                (fields[3] != _T("\u6210\u529f") &&
                    fields[3] != _T("\u5df2\u4e2d\u6b62")))
            {
                return;
            }
            record.missionId = fields[0];
            record.missionName = fields[1];
            record.launchTime = fields[2];
            record.result = fields[3];
            record.description = fields[5];
            records.push_back(record);
        });
    return TRUE;
}

BOOL BuildMissionsText(const std::vector<MissionInfo>& missions, CString& text)
{
    text.Empty();
    for (const MissionInfo& mission : missions)
    {
        const CString status = MissionStatusToString(mission.status);
        if (status.IsEmpty() || !IsValidField(mission.missionId) ||
            !IsValidField(mission.missionName) || !IsValidField(mission.rocketName) ||
            !IsValidField(mission.payloadName) || !IsValidField(mission.destination) ||
            !IsValidField(mission.launchTime))
        {
            return FALSE;
        }
        text += mission.missionId + _T('\t') + mission.missionName + _T('\t') +
            mission.rocketName + _T('\t') + mission.payloadName + _T('\t') +
            mission.destination + _T('\t') + mission.launchTime + _T('\t') +
            status + _T("\r\n");
    }
    return TRUE;
}

BOOL BuildChecksText(const std::vector<CheckResult>& checks, CString& text)
{
    text.Empty();
    for (const CheckResult& result : checks)
    {
        if (!IsValidField(result.missionId) || !IsValidField(result.remarks))
        {
            return FALSE;
        }
        text += result.missionId + _T('\t') +
            (result.propulsionReady ? _T("1") : _T("0")) + _T('\t') +
            (result.navigationReady ? _T("1") : _T("0")) + _T('\t') +
            (result.communicationReady ? _T("1") : _T("0")) + _T('\t') +
            (result.powerReady ? _T("1") : _T("0")) + _T('\t') +
            (result.weatherReady ? _T("1") : _T("0")) + _T('\t') +
            result.remarks + _T("\r\n");
    }
    return TRUE;
}

BOOL BuildRecordsText(const std::vector<LaunchRecord>& records, CString& text)
{
    text.Empty();
    for (const LaunchRecord& record : records)
    {
        if ((record.result != _T("\u6210\u529f") &&
            record.result != _T("\u5df2\u4e2d\u6b62")) ||
            !IsValidField(record.missionId) || !IsValidField(record.missionName) ||
            !IsValidField(record.launchTime) || !IsValidField(record.result) ||
            !IsValidField(record.description))
        {
            return FALSE;
        }
        text += record.missionId + _T('\t') + record.missionName + _T('\t') +
            record.launchTime + _T('\t') + record.result + _T('\t') +
            FormatDouble(record.maxHeightKm) + _T('\t') + record.description +
            _T("\r\n");
    }
    return TRUE;
}

void DeleteTemporaryFiles(const std::vector<CString>& paths)
{
    for (const CString& path : paths)
    {
        DeleteFile(path);
    }
}

BOOL ReplaceDataFiles(const std::vector<CString>& temporaryPaths,
    const std::vector<CString>& finalPaths)
{
    std::vector<CString> backupPaths;
    std::vector<BOOL> hadOriginal;
    backupPaths.reserve(finalPaths.size());
    hadOriginal.reserve(finalPaths.size());

    for (size_t index = 0; index < finalPaths.size(); ++index)
    {
        const CString backupPath = finalPaths[index] + _T(".bak");
        DeleteFile(backupPath);
        const BOOL exists = GetFileAttributes(finalPaths[index]) != INVALID_FILE_ATTRIBUTES;
        backupPaths.push_back(backupPath);
        hadOriginal.push_back(exists);
        if (exists && !MoveFileEx(finalPaths[index], backupPath,
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        {
            for (size_t rollback = 0; rollback < index; ++rollback)
            {
                if (hadOriginal[rollback])
                {
                    MoveFileEx(backupPaths[rollback], finalPaths[rollback],
                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
                }
            }
            DeleteTemporaryFiles(temporaryPaths);
            return FALSE;
        }
    }

    for (size_t index = 0; index < temporaryPaths.size(); ++index)
    {
        if (!MoveFileEx(temporaryPaths[index], finalPaths[index],
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        {
            for (size_t installed = 0; installed < index; ++installed)
            {
                DeleteFile(finalPaths[installed]);
            }
            for (size_t rollback = 0; rollback < finalPaths.size(); ++rollback)
            {
                if (hadOriginal[rollback])
                {
                    MoveFileEx(backupPaths[rollback], finalPaths[rollback],
                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
                }
            }
            DeleteTemporaryFiles(temporaryPaths);
            return FALSE;
        }
    }

    DeleteTemporaryFiles(backupPaths);
    return TRUE;
}
}

BOOL CDataManager::Initialize()
{
    std::vector<TCHAR> modulePath(MAX_PATH);
    DWORD length = 0;
    for (;;)
    {
        SetLastError(ERROR_SUCCESS);
        length = GetModuleFileName(nullptr, modulePath.data(),
            static_cast<DWORD>(modulePath.size()));
        if (length == 0)
        {
            return FALSE;
        }
        if (length < modulePath.size() - 1 || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            break;
        }
        modulePath.resize(modulePath.size() * 2);
    }

    CString executablePath(modulePath.data(), static_cast<int>(length));
    const int separator = executablePath.ReverseFind(_T('\\'));
    if (separator < 0)
    {
        return FALSE;
    }
    m_dataDirectory = executablePath.Left(separator + 1) + _T("data");

    if (!CreateDirectory(m_dataDirectory, nullptr) &&
        GetLastError() != ERROR_ALREADY_EXISTS)
    {
        return FALSE;
    }
    const DWORD attributes = GetFileAttributes(m_dataDirectory);
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        return FALSE;
    }
    return LoadAll();
}

BOOL CDataManager::LoadAll()
{
    if (m_dataDirectory.IsEmpty())
    {
        return FALSE;
    }

    std::vector<MissionInfo> missions;
    std::vector<CheckResult> checkResults;
    std::vector<LaunchRecord> records;
    if (!LoadMissions(JoinPath(m_dataDirectory, kMissionsFile), missions) ||
        !LoadChecks(JoinPath(m_dataDirectory, kChecksFile), checkResults) ||
        !LoadRecords(JoinPath(m_dataDirectory, kRecordsFile), records))
    {
        return FALSE;
    }

    m_missions.swap(missions);
    m_checkResults.swap(checkResults);
    m_records.swap(records);
    MarkSaved();
    return TRUE;
}

BOOL CDataManager::SaveAll()
{
    if (m_dataDirectory.IsEmpty())
    {
        return FALSE;
    }

    CString missionsText;
    CString checksText;
    CString recordsText;
    if (!BuildMissionsText(m_missions, missionsText) ||
        !BuildChecksText(m_checkResults, checksText) ||
        !BuildRecordsText(m_records, recordsText))
    {
        return FALSE;
    }

    const std::vector<CString> finalPaths = {
        JoinPath(m_dataDirectory, kMissionsFile),
        JoinPath(m_dataDirectory, kChecksFile),
        JoinPath(m_dataDirectory, kRecordsFile)
    };
    const std::vector<CString> temporaryPaths = {
        finalPaths[0] + _T(".tmp"),
        finalPaths[1] + _T(".tmp"),
        finalPaths[2] + _T(".tmp")
    };

    DeleteTemporaryFiles(temporaryPaths);
    if (!WriteUtf8File(temporaryPaths[0], missionsText) ||
        !WriteUtf8File(temporaryPaths[1], checksText) ||
        !WriteUtf8File(temporaryPaths[2], recordsText))
    {
        DeleteTemporaryFiles(temporaryPaths);
        return FALSE;
    }
    if (!ReplaceDataFiles(temporaryPaths, finalPaths))
    {
        return FALSE;
    }

    MarkSaved();
    return TRUE;
}

std::vector<MissionInfo>& CDataManager::GetMissions()
{
    return m_missions;
}

const std::vector<MissionInfo>& CDataManager::GetMissions() const
{
    return m_missions;
}

const std::vector<LaunchRecord>& CDataManager::GetRecords() const
{
    return m_records;
}

MissionInfo* CDataManager::FindMission(const CString& missionId)
{
    const auto found = std::find_if(m_missions.begin(), m_missions.end(),
        [&missionId](const MissionInfo& mission)
        {
            return mission.missionId == missionId;
        });
    return found == m_missions.end() ? nullptr : &(*found);
}

const MissionInfo* CDataManager::FindMission(const CString& missionId) const
{
    const auto found = std::find_if(m_missions.cbegin(), m_missions.cend(),
        [&missionId](const MissionInfo& mission)
        {
            return mission.missionId == missionId;
        });
    return found == m_missions.cend() ? nullptr : &(*found);
}

CheckResult* CDataManager::FindCheckResult(const CString& missionId)
{
    const auto found = std::find_if(m_checkResults.begin(), m_checkResults.end(),
        [&missionId](const CheckResult& result)
        {
            return result.missionId == missionId;
        });
    return found == m_checkResults.end() ? nullptr : &(*found);
}

BOOL CDataManager::AddMission(const MissionInfo& mission, CString& errorMessage)
{
    errorMessage.Empty();
    if (mission.missionId.IsEmpty())
    {
        errorMessage = _T("\u4efb\u52a1 ID \u4e0d\u80fd\u4e3a\u7a7a\u3002");
        return FALSE;
    }
    if (!IsValidField(mission.missionId) || !IsValidField(mission.missionName) ||
        !IsValidField(mission.rocketName) || !IsValidField(mission.payloadName) ||
        !IsValidField(mission.destination) || !IsValidField(mission.launchTime))
    {
        errorMessage = _T("\u5b57\u6bb5\u4e0d\u80fd\u5305\u542b\u5236\u8868\u7b26\u3001\u56de\u8f66\u6216\u6362\u884c\u3002");
        return FALSE;
    }
    if (FindMission(mission.missionId) != nullptr)
    {
        errorMessage = _T("\u4efb\u52a1 ID \u5df2\u5b58\u5728\u3002");
        return FALSE;
    }
    m_missions.push_back(mission);
    MarkModified();
    return TRUE;
}

BOOL CDataManager::UpdateMission(const CString& originalId,
    const MissionInfo& mission, CString& errorMessage)
{
    errorMessage.Empty();
    MissionInfo* existing = FindMission(originalId);
    if (existing == nullptr)
    {
        errorMessage = _T("\u672a\u627e\u5230\u8981\u4fee\u6539\u7684\u4efb\u52a1\u3002");
        return FALSE;
    }
    if (mission.missionId.IsEmpty())
    {
        errorMessage = _T("\u4efb\u52a1 ID \u4e0d\u80fd\u4e3a\u7a7a\u3002");
        return FALSE;
    }
    if (!IsValidField(mission.missionId) || !IsValidField(mission.missionName) ||
        !IsValidField(mission.rocketName) || !IsValidField(mission.payloadName) ||
        !IsValidField(mission.destination) || !IsValidField(mission.launchTime))
    {
        errorMessage = _T("\u5b57\u6bb5\u4e0d\u80fd\u5305\u542b\u5236\u8868\u7b26\u3001\u56de\u8f66\u6216\u6362\u884c\u3002");
        return FALSE;
    }
    if (mission.missionId != originalId && FindMission(mission.missionId) != nullptr)
    {
        errorMessage = _T("\u4efb\u52a1 ID \u5df2\u5b58\u5728\u3002");
        return FALSE;
    }

    if (mission.missionId != originalId)
    {
        CheckResult* checkResult = FindCheckResult(originalId);
        if (checkResult != nullptr)
        {
            checkResult->missionId = mission.missionId;
        }
    }
    *existing = mission;
    MarkModified();
    return TRUE;
}

BOOL CDataManager::DeleteMission(const CString& missionId)
{
    const auto originalSize = m_missions.size();
    m_missions.erase(std::remove_if(m_missions.begin(), m_missions.end(),
        [&missionId](const MissionInfo& mission)
        {
            return mission.missionId == missionId;
        }), m_missions.end());
    if (m_missions.size() == originalSize)
    {
        return FALSE;
    }

    m_checkResults.erase(std::remove_if(m_checkResults.begin(), m_checkResults.end(),
        [&missionId](const CheckResult& result)
        {
            return result.missionId == missionId;
        }), m_checkResults.end());
    MarkModified();
    return TRUE;
}

void CDataManager::SetCheckResult(const CheckResult& result)
{
    CheckResult* existing = FindCheckResult(result.missionId);
    if (existing == nullptr)
    {
        m_checkResults.push_back(result);
    }
    else
    {
        *existing = result;
    }
    MarkModified();
}

void CDataManager::AddLaunchRecord(const LaunchRecord& record)
{
    m_records.push_back(record);
    MarkModified();
}

BOOL CDataManager::DeleteLaunchRecord(size_t index)
{
    if (index >= m_records.size())
    {
        return FALSE;
    }
    m_records.erase(m_records.begin() + index);
    MarkModified();
    return TRUE;
}

BOOL CDataManager::IsModified() const
{
    return m_isModified;
}

void CDataManager::MarkModified()
{
    m_isModified = TRUE;
}

void CDataManager::MarkSaved()
{
    m_isModified = FALSE;
}

BOOL CDataManager::ConfirmSaveBeforeClose(CWnd* pOwner)
{
    if (!IsModified())
    {
        return TRUE;
    }

    const CString message = _T("\u6570\u636e\u5df2\u4fee\u6539\uff0c\u662f\u5426\u4fdd\u5b58\uff1f");
    const CString caption = _T("\u4fdd\u5b58\u63d0\u793a");
    const int choice = pOwner != nullptr
        ? pOwner->MessageBox(message, caption, MB_YESNOCANCEL | MB_ICONQUESTION)
        : AfxMessageBox(message, MB_YESNOCANCEL | MB_ICONQUESTION);
    if (choice == IDNO)
    {
        return TRUE;
    }
    if (choice != IDYES)
    {
        return FALSE;
    }
    if (SaveAll())
    {
        return TRUE;
    }

    const CString errorMessage = _T("\u4fdd\u5b58\u5931\u8d25\uff0c\u8bf7\u68c0\u67e5\u6570\u636e\u76ee\u5f55\u7684\u5199\u5165\u6743\u9650\u3002");
    if (pOwner != nullptr)
    {
        pOwner->MessageBox(errorMessage, caption, MB_OK | MB_ICONERROR);
    }
    else
    {
        AfxMessageBox(errorMessage, MB_OK | MB_ICONERROR);
    }
    return FALSE;
}
