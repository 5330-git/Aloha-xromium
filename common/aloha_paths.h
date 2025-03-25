// 参考 chrome\common\chrome_paths_win.cc 实现
#ifndef ALOHA_COMMON_ALOHA_PATHS_H_
#define ALOHA_COMMON_ALOHA_PATHS_H_
#include "base/base_paths.h"
#include "base/files/file_path.h"
namespace aloha::path_service {

enum AlohaPathKeys {
  PATH_START = base::BasePathKey::PATH_END + 1,

  ALOHA_USER_DATA_DIR = PATH_START,
  ALOHA_WEB_APP_RESOURCES_DIR,
  // aloha_resources.pak 路径
  ALOHA_RESOURCES_PAK,
  ALOHA_100_PERCENT_PAK,
  ALOHA_200_PERCENT_PAK,
  ALOHA_DEFAULT_LOCALE_PAK,

  PATH_END,
};

void GetDefaultUserDataDirectory(base::FilePath* result);
void GetWebAppPath(base::FilePath* result, std::string app_name);

// 这个接口将被注册到 base::PathService 中，在 base::PathService::Get 中使用
// 参考 content\shell\browser\shell_paths.cc
bool AlohaPathProvider(int key, base::FilePath* result);
void RegisterAlohaPathProvider();

}  // namespace aloha::path_service

#endif
