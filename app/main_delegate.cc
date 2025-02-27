// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/app/main_delegate.h"

#include <memory>

#include "aloha/browser/content_browser_client.h"
#include "aloha/common/content_client.h"
#include "aloha/renderer/content_renderer_client.h"
#include "base/base_paths.h"
#include "base/check.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/resource/resource_scale_factor.h"
#include "ui/base/ui_base_paths.h"

#if BUILDFLAG(IS_MAC)
#include "aloha/app/mac_init.h"
#endif

namespace aloha {

MainDelegate::MainDelegate() = default;

MainDelegate::~MainDelegate() = default;

std::optional<int> MainDelegate::BasicStartupComplete() {
  logging::LoggingSettings settings;
  settings.logging_dest =
      logging::LOG_TO_SYSTEM_DEBUG_LOG | logging::LOG_TO_STDERR;
  CHECK(logging::InitLogging(settings));

  content_client_ = std::make_unique<ContentClient>();
  content::SetContentClient(content_client_.get());
  return std::nullopt;
}

void MainDelegate::PreSandboxStartup() {
  base::FilePath ui_test_pak_path;
  CHECK(base::PathService::Get(ui::UI_TEST_PAK, &ui_test_pak_path));
  ui::ResourceBundle::InitSharedInstanceWithPakPath(ui_test_pak_path);

  base::FilePath content_resources_pak_file;
  CHECK(base::PathService::Get(base::DIR_ASSETS, &content_resources_pak_file));
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      content_resources_pak_file.AppendASCII("content_resources.pak"),
      ui::k100Percent);

  base::FilePath aloha_pak_file;
  CHECK(base::PathService::Get(base::DIR_ASSETS, &aloha_pak_file));
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      aloha_pak_file.AppendASCII("aloha.pak"), ui::k100Percent);

  base::FilePath ui_resource_percent100;
  CHECK(base::PathService::Get(base::DIR_ASSETS, &aloha_pak_file));
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      ui_resource_percent100.AppendASCII("ui_resources_100_percent.pak"),
      ui::k100Percent);
}

content::ContentBrowserClient* MainDelegate::CreateContentBrowserClient() {
  content_browser_client_ = std::make_unique<ContentBrowserClient>();
  // 替换成 ChromeContentBrowserClient
  // content_browser_client_ = std::make_unique<ChromeContentBrowserClient>();
  return content_browser_client_.get();
}

std::optional<int> MainDelegate::PreBrowserMain() {
#if BUILDFLAG(IS_MAC)
  MacPreBrowserMain();
#endif
  return content::ContentMainDelegate::PreBrowserMain();
}

content::ContentRendererClient* MainDelegate::CreateContentRendererClient() {
  content_renderer_client_ = std::make_unique<ContentRendererClient>();
  return content_renderer_client_.get();
}

}  // namespace aloha
