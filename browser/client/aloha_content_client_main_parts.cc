// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/client/aloha_content_client_main_parts.h"

#include <utility>

#include "aloha/browser/devtools/devtools_server.h"
#include "base/run_loop.h"
#include "build/build_config.h"
#include "content/public/common/result_codes.h"
#include "content/shell/browser/shell_browser_context.h"
#include "ui/base/ime/init/input_method_initializer.h"
#include "ui/views/test/desktop_test_views_delegate.h"
#include "aloha/browser/client/aloha_browser_client.h"

namespace aloha {

AlohaContentClientMainParts::AlohaContentClientMainParts(
    AlohaBrowserClient* views_content_client)
    : views_content_client_(views_content_client) {}

AlohaContentClientMainParts::~AlohaContentClientMainParts() {
}

#if !BUILDFLAG(IS_APPLE)
void AlohaContentClientMainParts::PreBrowserMain() {}
#endif

int AlohaContentClientMainParts::PreMainMessageLoopRun() {
  ui::InitializeInputMethodForTesting();
  browser_context_ = std::make_unique<content::ShellBrowserContext>(false);
  // DevTools
  devtools::StartHttpHandler(browser_context_.get());

  views_delegate_ = std::make_unique<views::DesktopTestViewsDelegate>();
  run_loop_ = std::make_unique<base::RunLoop>();
  views_content_client()->set_quit_closure(run_loop_->QuitClosure());
  return content::RESULT_CODE_NORMAL_EXIT;
}

void AlohaContentClientMainParts::WillRunMainMessageLoop(
    std::unique_ptr<base::RunLoop>& run_loop) {
  run_loop = std::move(run_loop_);
}

void AlohaContentClientMainParts::PostMainMessageLoopRun() {
  browser_context_.reset();
  views_delegate_.reset();
}

}  // namespace aloha
