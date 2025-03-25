// Copyright 2025 The Aloha-Xromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>

#include "aloha/app/aloha_main.h"
#include "aloha/common/aloha_main_client.h"
#include "aloha/resources/grit/unscaled_resources.h"
#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/functional/bind.h"
#include "base/no_destructor.h"
#include "base/process/process_handle.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "content/public/app/content_main.h"
#include "content/public/app/sandbox_helper_win.h"
#include "content/public/browser/browser_context.h"
#include "sandbox/win/src/sandbox_types.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/models/image_model.h"
#include "ui/base/resource/resource_scale_factor.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "url/url_util.h"

int wWinMain(HINSTANCE instance, HINSTANCE, wchar_t*, int) {
  base::CommandLine::Init(0, nullptr);

  sandbox::SandboxInterfaceInfo sandbox_info{};
  content::InitializeSandboxInfo(&sandbox_info);

  // 启动消息循环
  return aloha::AlohaMain(instance, &sandbox_info);
}
