#include "Logger.h"
#include <atltime.h>
#include <windows.h>
#include <processthreadsapi.h>
#include <share.h>

const CString strDecorator = _T("********************************************************************************\n");

BOOL      CLogger::m_bDestroyed = FALSE;  
CLogger*  CLogger::m_pInstance = NULL;

CLogger::CLogger(CString pszLogPrefixArg)
  : m_nCurLogLevel(LogLevel::Developer)
  , m_nCurLogPeriod(LogPeriod::OneFilePerOneHour)
  , m_strLogDir(_T(""))
  , m_strLogPrefix(pszLogPrefixArg)
  , m_strLogSuffix(_T(""))
  , m_strApplicationName(_T(""))
  , m_strApplicationVersion(_T(""))
  , m_strApplicationReleaseDate(_T(""))
{
}


CLogger::~CLogger(void)
{
  m_bDestroyed = TRUE;
}

CLogger* CLogger::GetInstance(CString pszLogPrefixArg)
{
  if (m_bDestroyed)
  {
    m_pInstance = new CLogger(pszLogPrefixArg);
    //new(m_pInstance) CLogger(pszLogPrefixArg);
    atexit(KillLogger);
    m_bDestroyed = FALSE;
  }
  else if (NULL == m_pInstance)
  {
    Create(pszLogPrefixArg);
  }

  return m_pInstance;
}

VOID CLogger::SetLogLevel(LogLevel logLevelArg)
{
  m_nCurLogLevel = logLevelArg;
}

LogLevel CLogger::GetLogLevel()
{
  return m_nCurLogLevel;
}

VOID CLogger::SetLogPeriod(LogPeriod logPeriodArg)
{
  m_nCurLogPeriod = logPeriodArg;
}

LogPeriod CLogger::GetLogPeriod()
{
  return  m_nCurLogPeriod;
}

void CLogger::SetLogDir(LPCTSTR pszLogDirArg)
{
  m_strLogDir = pszLogDirArg;
}

LPCTSTR CLogger::GetLogDir()
{
  return m_strLogDir;
}

void CLogger::SetLogFilePrefix(LPCTSTR pszLogFilePrefixArg)
{
  m_strLogPrefix = pszLogFilePrefixArg;
}

LPCTSTR CLogger::GetLogFilePrefix()
{
  return m_strLogPrefix;
}

void CLogger::SetLogFileSuffix(LPCTSTR pszLogFileSuffixArg)
{
  m_strLogSuffix = pszLogFileSuffixArg;
}

LPCTSTR CLogger::GetLogFileSuffix()
{
  return m_strLogSuffix;
}

void CLogger::SetApplicationName(LPCTSTR pszAppNameArg)
{
  m_strApplicationName = pszAppNameArg;
}

LPCTSTR CLogger::GetApplicationName()
{
  return m_strApplicationName;
}

void CLogger::SetApplicationVersion(LPCTSTR pszAppVersionArg)
{
  m_strApplicationVersion = pszAppVersionArg;
}

LPCTSTR CLogger::GetApplicationVersion()
{
  return m_strApplicationVersion;
}

void CLogger::SetApplicationReleaseDate(LPCTSTR pszAppReleaseDateArg)
{
  m_strApplicationReleaseDate = pszAppReleaseDateArg;
}

LPCTSTR CLogger::GetApplicationReleaseDate()
{
  return m_strApplicationReleaseDate;
}

CString CLogger::GetApplicationInfo()
{
  CString strApplicaionInfo;

  strApplicaionInfo.Format(_T("ApplicationName:%s\tVersion:%s\t ReleaseDate:%s\n"),
    GetApplicationName(),
    GetApplicationVersion(),
    GetApplicationReleaseDate());

  strApplicaionInfo = strDecorator + strApplicaionInfo + strDecorator;

  return strApplicaionInfo;
}

CString CLogger::GetLogLevelToString(LogLevel logLevelArg)
{
  CString strLogLevelName;

  switch (logLevelArg)
  {
  case LogLevel::NoLogging:
    strLogLevelName = _T("");
    break;
  case LogLevel::ErrorOnly:
    strLogLevelName = _T("E");
    break;
  case LogLevel::Simple:
    strLogLevelName = _T("S");
    break;
  case LogLevel::Detail:
    strLogLevelName = _T("D");
    break;
  case LogLevel::Developer:
    strLogLevelName = _T("Dev");
    break;
  }

  return strLogLevelName;
}

BOOL CLogger::LogMessage(LogLevel logLevelArg, LPCTSTR pszFormat, ...)
{
  bool      bRet = FALSE;

  //  로그의 내용이 설정된 log 레벨보다 낮은 경우에는 Trace만 보여주고 
  //  아무런 작업도 하지 않음
  if (m_nCurLogLevel < logLevelArg)
  {
#ifdef _DEBUG
    va_list   ap;
    va_start(ap, pszFormat);

    //  set log level
    CString strLogLevelName;
    strLogLevelName = GetLogLevelToString(logLevelArg);

    //  get current time
    CString strTime;
    if (LogPeriod::OneFilePerOneDay == GetLogPeriod())
      strTime = CTime::GetCurrentTime().Format("%Y%m%d");
    else
      strTime = CTime::GetCurrentTime().Format("%Y%m%d%H");

    //  set log message
    CString strLogMsg;
    strLogMsg.Format(_T("%s [%s] [P=%d,T=%d] %s\n"),
                     CTime::GetCurrentTime().Format(_T("%Y/%m/%d %H:%M:%S")),
                     strLogLevelName,
                     GetCurrentProcessId(),
                     GetCurrentThreadId(),
                     pszFormat);

    CStringA pszLogMsg(strLogMsg);
    pszLogMsg.FormatV((CStringA)strLogMsg, ap);

    bRet = true;

    va_end(ap);

    OutputDebugString((CString)pszLogMsg);
#endif // _DEBUG

    return bRet;
  }

  va_list   ap;
  va_start(ap, pszFormat);

  //  set log level
  CString strLogLevelName;
  strLogLevelName = GetLogLevelToString(logLevelArg);

  //  get current time
  CString strTime;
  if (LogPeriod::OneFilePerOneDay == GetLogPeriod())
  {
    strTime = CTime::GetCurrentTime().Format("%Y%m%d");
  }
  else
  {
    strTime = CTime::GetCurrentTime().Format("%Y%m%d%H");
  }
  

  //  set log file name
  CString strLogFilePath;
  if (0 < m_strLogSuffix.GetLength())
  {
    strLogFilePath.Format(_T("%s\\%s-%s-%s.log"),
      m_strLogDir,
      m_strLogPrefix,
      strTime,
      m_strLogSuffix);
  }
  else
  {
    strLogFilePath.Format(_T("%s\\%s-%s.log"),
      m_strLogDir,
      m_strLogPrefix,
      strTime);
  }
  
  //  thread-safe 하도록 mutex를 걸어준다
  CSingleLock SingleLock(&m_mtxLogger, TRUE);
  //SingleLock.Lock();

  //  open file
  //  파일의 첫 부분에 log를 남기는 application의 정보를 남기기 위해서 아래와 같은 처리를 함
  //  파일을 처음 생성한 경우에는 application 정보를 남기고,
  //  존재하는 파일을 읽어들이는 경우에는 파일의 끝부터 로그를 기록 한다.
  errno_t fileError;
  FILE  *pLogFile;
  fileError = _tfopen_s(&pLogFile, strLogFilePath, _T("r"));
  if (ERROR_SUCCESS != fileError)
  {
    pLogFile = _tfsopen(strLogFilePath, _T("w"), _SH_DENYNO);
    if (nullptr == pLogFile)
    {
      bRet = FALSE;
      return bRet;
    }

    CStringA pszApplicationInfo(GetApplicationInfo());
    _fprintf_p(pLogFile, pszApplicationInfo);
  }
  else
  {
    fclose(pLogFile);
    pLogFile = _tfsopen(strLogFilePath, _T("a"), _SH_DENYNO);
    if (nullptr == pLogFile)
    {
      bRet = FALSE;
      return bRet;
    }
  }

  //  set log message
  CString strLogMsg;
  strLogMsg.Format(_T("%s [%s] [P=%d,T=%d] %s\n"),
    CTime::GetCurrentTime().Format(_T("%Y/%m/%d %H:%M:%S")),
    strLogLevelName,
    GetCurrentProcessId(),
    GetCurrentThreadId(),
    pszFormat);

  CStringA pszLogMsg(strLogMsg);
  _vfprintf_p(pLogFile, pszLogMsg, ap);
  pszLogMsg.FormatV((CStringA)strLogMsg, ap);

  fclose(pLogFile);

  bRet = true;

  //SingleLock.Unlock();

  va_end(ap);

  OutputDebugString((CString)pszLogMsg);

  return bRet;
}
