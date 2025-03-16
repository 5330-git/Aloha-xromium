// 参考 chrome\common\chrome_constants.cc

#ifndef ALOHA_COMMON_ALOHA_CONSTANTS_H_
#define ALOHA_COMMON_ALOHA_CONSTANTS_H_

#include "base/files/file_path.h"
namespace aloha {
// Aloha Info
extern const char kProductName[];

// Aloha Executable Related Path
extern const base::FilePath::CharType kBrowserProcessExecutablePath[];
extern const base::FilePath::CharType kHelperProcessExecutablePath[];
extern const base::FilePath::CharType kBrowserProcessExecutableName[];
extern const base::FilePath::CharType kHelperProcessExecutableName[];

// Persistent Storage Path
extern const base::FilePath::CharType kUserDataDirname[];

extern const base::FilePath::CharType kNetworkDataDirname[];
extern const base::FilePath::CharType kCacheDirname[];
extern const base::FilePath::CharType kCookieFilename[];
extern const base::FilePath::CharType kDeviceBoundSessionsFilename[];
extern const base::FilePath::CharType kTrustTokenFilename[];
extern const base::FilePath::CharType kNetworkPersistentStateFilename[];
extern const base::FilePath::CharType kTransportSecurityPersisterFilename[];
extern const base::FilePath::CharType kReportingAndNelStoreFilename[];
extern const base::FilePath::CharType kSCTAuditingPendingReportsFileName[];

namespace url {
extern const char kAlohaScheme[];
extern const char kAlohaDemoScheme[];
}

namespace switches {
extern const char kUseWebUI[];
extern const char kEnableLoggingFile[];
}  // namespace switches

namespace webapp {
extern const base::FilePath::CharType kWebAppDirName[];

namespace internal {
extern const char kAlohaHome[];
}
}  // namespace webapp
}  // namespace aloha

#endif  // ALOHA_COMMON_ALOHA_CONSTANTS_H_
