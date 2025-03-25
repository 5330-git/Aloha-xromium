// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/app/aloha_content_main_delegate.h"

#include <string>

#include "aloha/browser/client/aloha_content_browser_client.h"
#include "aloha/browser/client/aloha_content_client_main_parts.h"
#include "aloha/common/aloha_constants.h"
#include "aloha/common/aloha_content_client.h"
#include "aloha/common/aloha_main_client.h"
#include "aloha/common/aloha_paths.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/notreached.h"
#include "base/path_service.h"
#include "build/build_config.h"
#include "content/public/common/content_switches.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/resource/resource_scale_factor.h"
#include "ui/base/ui_base_paths.h"

#if BUILDFLAG(IS_WIN)
#include "base/logging_win.h"
#endif

namespace aloha {
namespace {

#if BUILDFLAG(IS_WIN)
// {83FAC8EE-7A0E-4dbb-A3F6-6F500D7CAB1A}
const GUID kViewsContentClientProviderName = {
    0x83fac8ee,
    0x7a0e,
    0x4dbb,
    {0xa3, 0xf6, 0x6f, 0x50, 0xd, 0x7c, 0xab, 0x1a}};
#endif

}  // namespace

AlohaContentMainDelegate::AlohaContentMainDelegate() = default;

AlohaContentMainDelegate::~AlohaContentMainDelegate() {}

std::optional<int> AlohaContentMainDelegate::BasicStartupComplete() {
  const base::CommandLine& command_line =
      *base::CommandLine::ForCurrentProcess();
  std::string process_type =
      command_line.GetSwitchValueASCII(::switches::kProcessType);

  logging::LoggingSettings settings;
  settings.logging_dest =
      logging::LOG_TO_SYSTEM_DEBUG_LOG | logging::LOG_TO_STDERR;
  // 如果增加了参数 --enable-logging-file 则会将日志输出到文件中
  if (command_line.HasSwitch(aloha::switches::kEnableLoggingFile)) {
    settings.logging_dest |= logging::LOG_TO_FILE;
    settings.log_file_path =
        base::PathService::CheckedGet(path_service::ALOHA_USER_DATA_DIR)
            .AppendASCII("aloha.log")
            .value()
            .c_str();
  }
  bool success = logging::InitLogging(settings);
  CHECK(success);
#if BUILDFLAG(IS_WIN)
  logging::LogEventProvider::Initialize(kViewsContentClientProviderName);
#endif

  return std::nullopt;
}

void AlohaContentMainDelegate::PreSandboxStartup() {
  base::FilePath aloha_resources_pak_path;
  CHECK(base::PathService::Get(path_service::ALOHA_RESOURCES_PAK,
                               &aloha_resources_pak_path));
  ui::ResourceBundle::InitSharedInstanceWithPakPath(aloha_resources_pak_path);

  // 加载带缩放尺寸的资源
  base::FilePath aloha_scale_pak_path;
  ui::ResourceScaleFactor scale_factor;
  if (ui::IsScaleFactorSupported(ui::k100Percent)) {
    CHECK(base::PathService::Get(path_service::ALOHA_100_PERCENT_PAK,
                                 &aloha_scale_pak_path));
    scale_factor = ui::k100Percent;
  } else if (ui::IsScaleFactorSupported(ui::k200Percent)) {
    CHECK(base::PathService::Get(path_service::ALOHA_200_PERCENT_PAK,
                                 &aloha_scale_pak_path));
    scale_factor = ui::k200Percent;
  } else {
    NOTREACHED() << "Unsupported scale factor";
  }
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      aloha_scale_pak_path, scale_factor);
  
  // 加载 locale 资源(测试阶段默认加入英语，未来需要根据系统语言加载)
  base::FilePath aloha_locale_pak_path;
  CHECK(base::PathService::Get(path_service::ALOHA_DEFAULT_LOCALE_PAK,
                               &aloha_locale_pak_path));
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      aloha_locale_pak_path, ui::kScaleFactorNone);

  AlohaMainClient::GetInstance()->OnResourcesLoaded();
}

std::optional<int> AlohaContentMainDelegate::PreBrowserMain() {
  std::optional<int> exit_code = content::ContentMainDelegate::PreBrowserMain();
  if (exit_code.has_value()) {
    return exit_code;
  }

  AlohaContentClientMainParts::PreBrowserMain();
  return std::nullopt;
}

content::ContentClient* AlohaContentMainDelegate::CreateContentClient() {
  AlohaContentClient::SetContentClient(&aloha_content_client_);
  return &aloha_content_client_;
}

content::ContentBrowserClient*
AlohaContentMainDelegate::CreateContentBrowserClient() {
  content_browser_client_ = std::make_unique<AlohaContentBrowserClient>();
  return content_browser_client_.get();
}

}  // namespace aloha
