#include "aloha/common/aloha_constants.h"

#include "base/files/file_path.h"


#define FPL FILE_PATH_LITERAL
namespace aloha {
const char kProductName[] = "Aloha-Xromium";

const base::FilePath::CharType kBrowserProcessExecutableName[] =
    FPL("aloha.exe");
const base::FilePath::CharType kHelperProcessExecutableName[] =
    FPL("aloha.exe");
const base::FilePath::CharType kBrowserProcessExecutablePath[] =
    FPL("aloha.exe");
const base::FilePath::CharType kHelperProcessExecutablePath[] =
    FPL("aloha.exe");

// Persistent Storage Path

const base::FilePath::CharType kUserDataDirname[] = FPL("User Data");

const base::FilePath::CharType kNetworkDataDirname[] = FPL("Network");
const base::FilePath::CharType kCacheDirname[] = FPL("Cache");
const base::FilePath::CharType kCookieFilename[] = FPL("Cookies");
const base::FilePath::CharType kDeviceBoundSessionsFilename[] =
    FPL("Device Bound Sessions");
const base::FilePath::CharType kTrustTokenFilename[] = FPL("Trust Tokens");
const base::FilePath::CharType kNetworkPersistentStateFilename[] =
    FPL("Network Persistent State");
const base::FilePath::CharType kTransportSecurityPersisterFilename[] =
    FPL("TransportSecurity");
const base::FilePath::CharType kReportingAndNelStoreFilename[] =
    FPL("Reporting and NEL");
const base::FilePath::CharType kSCTAuditingPendingReportsFileName[] =
    FPL("SCT Auditing Pending Reports");

namespace url {
const char kAlohaScheme[] = "aloha";
}

namespace  switches  {
    const char kUseWebUI[] = "use-webui";
    const char kEnableLoggingFile[] = "enable-logging-file";
}

}  // namespace aloha
