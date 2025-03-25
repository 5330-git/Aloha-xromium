#include "aloha/common/aloha_constants.h"

#include "base/files/file_path.h"

#define FPL FILE_PATH_LITERAL
namespace aloha {
const char kProductName[] = "Aloha-Xromium";
const base::FilePath::CharType kLocaleDir[] = FPL("locales/aloha");

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
const char kAlohaDemoScheme[] = "aloha-demo";

const char kAlohaAppsHost[] = "apps";
}  // namespace url

namespace switches {
const char kUseWebUI[] = "use-webui";
const char kEnableLoggingFile[] = "enable-logging-file";
const char kAlohaUserDataDir[] = "user-data-dir";
}  // namespace switches

namespace webapp {
const base::FilePath::CharType kWebAppDirName[] = FPL("aloha-webapp");

namespace internal {
const char kAlohaHome[] = "aloha-app-main";
}
}  // namespace webapp

// Aloha Native Resources
namespace resources {
const base::FilePath::CharType kAlohaResourcesPakName[] =
    FPL("aloha_resources.pak");
const base::FilePath::CharType kAloha100PercentPakName[] =
    FPL("aloha_100_percent.pak");
const base::FilePath::CharType kAloha200PercentPakName[] =
    FPL("aloha_200_percent.pak");
const base::FilePath::CharType kAlohaDefaultLocalePakName[] = 
    FPL("en-US.pak");
}  // namespace resources
}  // namespace aloha
