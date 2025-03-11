// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/client/aloha_browser_client.h"

#include <utility>

#include "base/process/process.h"
#include "build/build_config.h"
#include "content/public/app/content_main.h"
#include "aloha/browser/client/aloha_content_main_delegate.h"

namespace aloha {

#if BUILDFLAG(IS_WIN)
AlohaBrowserClient::AlohaBrowserClient(
    HINSTANCE instance, sandbox::SandboxInterfaceInfo* sandbox_info)
    : instance_(instance), sandbox_info_(sandbox_info) {
}
#else
AlohaBrowserClient::AlohaBrowserClient(int argc, const char** argv)
    : argc_(argc), argv_(argv) {
}
#endif

AlohaBrowserClient::~AlohaBrowserClient() {
}

int AlohaBrowserClient::RunMain() {
  AlohaContentMainDelegate delegate(this);
  content::ContentMainParams params(&delegate);

#if BUILDFLAG(IS_WIN)
  params.instance = instance_;
  params.sandbox_info = sandbox_info_;
#else
  params.argc = argc_;
  params.argv = argv_;
#endif

  return content::ContentMain(std::move(params));
}

void AlohaBrowserClient::OnPreMainMessageLoopRun(
    content::BrowserContext* browser_context,
    gfx::NativeWindow window_context) {
  std::move(on_pre_main_message_loop_run_callback_)
      .Run(browser_context, window_context);
}

void AlohaBrowserClient::OnResourcesLoaded() {
  if (on_resources_loaded_callback_)
    std::move(on_resources_loaded_callback_).Run();
}

}  // namespace aloha
