
#include "aloha/common/aloha_paths.h"

#include "aloha/common/aloha_constants.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/notreached.h"
#include "base/path_service.h"
#include "base/threading/thread_restrictions.h"

namespace aloha::path_service {
void GetDefaultUserDataDirectory(base::FilePath* result) {
  // 在 aloha.exe 所在目录下
  base::FilePath exe_path;
  if (!base::PathService::Get(base::FILE_EXE, &exe_path)) {
    NOTREACHED() << "Could not get path to executable.";
  }
  *result = exe_path.DirName();
  *result = result->Append(base::FilePath::FromASCII(aloha::kProductName))
                .Append(aloha::kUserDataDirname);
  // 检查/创建目录
  if (base::DirectoryExists(*result)) {
    return;
  }
  base::CreateDirectory(*result);
}

bool AlohaPathProvider(int key, base::FilePath* result) {
  base::FilePath cur;

  switch (key) {
    case aloha::path_service::ALOHA_USER_DATA_DIR: {
      GetDefaultUserDataDirectory(result);
      return true;
    }
    default:
      return false;
  }
}

void RegisterAlohaPathProvider() {
  base::PathService::RegisterProvider(AlohaPathProvider,
                                      aloha::path_service::PATH_START,
                                      aloha::path_service::PATH_END);
}
}  // namespace aloha::path_service
