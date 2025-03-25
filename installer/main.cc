// 一下实现目前仍属于实验阶段：希望基于 base::win::PEImageReader 或者
// base::win::PEImage 解析 `aloha.exe` 中的导入表，然后找出所有的依赖 DLL。
#include <memory>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/win/pe_image.h"
#include "base/win/pe_image_reader.h"

constexpr base::FilePath::CharType kPEFilePath[] =
    L"D:\\codes\\build-chromium\\chromium\\src\\out\\aloha_debug\\aloha.exe";

// 需要将 PE 文件映射到内存中，然后才能使用 base::win::PEImageReader 类来读取 PE
// 文件的信息。
std::unique_ptr<base::win::PEImage> MapPEImage(const base::FilePath& path) {
  HANDLE file =
      CreateFileW(path.value().c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file == INVALID_HANDLE_VALUE) {
    LOG(ERROR) << "Failed to open file: " << path;
    return nullptr;
  }

  HANDLE mapping =
      CreateFileMapping(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
  if (!mapping) {
    LOG(ERROR) << "Failed to create file mapping: " << path;
    CloseHandle(file);
    return nullptr;
  }
  // 我们需要保证整个PE文件都被映射到内存中，否则在遍历导入表时可能会出现 memory
  // access violation

  void* base = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
  if (!base) {
    LOG(ERROR) << "Failed to map file: " << path;
    CloseHandle(mapping);
    CloseHandle(file);
    return nullptr;
  }

  // 使用PEImage的构造函数加载内存映射
  auto pe_image = std::make_unique<base::win::PEImage>(base);
  return pe_image;
}

// module 是导入模块的名称，ordinal 是导入函数的序号，import_name
// 是导入函数的名称， hint 是导入函数的提示，iat 是导入地址表（Import Address
// Table）的指针，cookie 是回调函数的额外参数。
bool EnumImportsCallback(const base::win::PEImage& pe_image,
                         LPCSTR module,
                         DWORD ordinal,
                         LPCSTR import_name,
                         DWORD hint,
                         PIMAGE_THUNK_DATA iat,
                         PVOID cookie) {
  LOG(INFO) << "module: " << module << ", ordinal: " << ordinal
            << ", import_name: " << import_name << ", hint: " << hint
            << ", iat: " << iat << ", cookie: " << cookie;
  return true;
}

bool EnumExportsCallback(const base::win::PEImage& pe_image,
                         DWORD ordinal,
                         DWORD hint,
                         LPCSTR name,
                         PVOID function_addr,
                         LPCSTR forward,
                         PVOID cookie

) {
  LOG(INFO) << "ordinal: " << ordinal << ", hint: " << hint
            << ", name: " << name << ", function_addr: " << function_addr
            << ", forward: " << forward << ", cookie: " << cookie;
  return true;
}

int main() {
  logging::LoggingSettings settings;
  settings.logging_dest =
      logging::LOG_TO_SYSTEM_DEBUG_LOG | logging::LOG_TO_STDERR;
  bool success = logging::InitLogging(settings);
  CHECK(success);
  LOG(INFO) << "Hello World!";
  base::FilePath path(kPEFilePath);
  auto pe_image = MapPEImage(base::FilePath(kPEFilePath));
  CHECK(pe_image);
  // pe_image->EnumAllImports(&EnumImportsCallback, nullptr,
  //                          path.BaseName().MaybeAsASCII().c_str());
  // pe_image->EnumAllDelayImports(&EnumImportsCallback, nullptr,
  //                               path.BaseName().MaybeAsASCII().c_str());
  pe_image->EnumExports(&EnumExportsCallback, nullptr);
  LOG(INFO) << pe_image->GetNumSections();

  return 0;
}
