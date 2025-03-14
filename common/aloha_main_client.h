// 参考: ui/views/views_content_client/views_content_client.h

// Copyright 2014 The Chromium Authors
// Copyright 2025 The Aloha-Xromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_CLIENT_H_
#define ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_CLIENT_H_

#include <memory>
#include <utility>

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "build/build_config.h"
#include "components/prefs/pref_service.h"
#include "ui/gfx/native_widget_types.h"

namespace content {
class BrowserContext;
}

namespace sandbox {
struct SandboxInterfaceInfo;
}

namespace aloha {

// AlohaMainClient 是 Aloha 所有进程的入口点，通过调用 AlohaMain
// 接口启动进程任务

class AlohaMainClient {
 public:
  using OnPreMainMessageLoopRunCallback =
      base::OnceCallback<void(content::BrowserContext* browser_context,
                              gfx::NativeWindow window_context)>;
  static AlohaMainClient* GetInstance();

  AlohaMainClient(const AlohaMainClient&) = delete;
  AlohaMainClient& operator=(const AlohaMainClient&) = delete;

  ~AlohaMainClient();

  // 在 AlohaMain（content::ContentMain）之前调用
  int PreAlohaMain();

  // The task to run at the end of BrowserMainParts::PreMainMessageLoopRun().
  // Ignored if this is not the main process.
  void set_on_pre_main_message_loop_run_callback(
      OnPreMainMessageLoopRunCallback callback) {
    on_pre_main_message_loop_run_callback_ = std::move(callback);
  }

  void set_on_resources_loaded_callback(base::OnceClosure callback) {
    on_resources_loaded_callback_ = std::move(callback);
  }

  // Calls the OnPreMainMessageLoopRun callback. |browser_context| is the
  // current browser context. |window_context| is a candidate root window that
  // may be null.
  void OnPreMainMessageLoopRun(content::BrowserContext* browser_context,
                               gfx::NativeWindow window_context);

  // Calls a callback to signal resources have been loaded.
  void OnResourcesLoaded();

  // Called by AlohaContentClientMainParts to supply the quit-closure to use
  // to exit AlohaMain().
  void set_quit_closure(base::OnceClosure quit_closure) {
    quit_closure_ = std::move(quit_closure);
  }
  base::OnceClosure& quit_closure() { return quit_closure_; }

 private:
  AlohaMainClient();

  OnPreMainMessageLoopRunCallback on_pre_main_message_loop_run_callback_;
  base::OnceClosure on_resources_loaded_callback_;
  base::OnceClosure quit_closure_;

  // TEMP
  // TODO(yeyun.anton): 调研如何存储 PrefService
  std::unique_ptr<PrefService> pref_service_;
};

}  // namespace aloha

#endif  // ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_CLIENT_H_
