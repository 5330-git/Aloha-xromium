// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "aloha/browser/browser_main_parts.h"
#include "aloha/browser/ui/aura/aura_context.h"
#include "aloha/browser/ui/aura/content_window.h"
#include "base/memory/weak_ptr.h"
#include "base/types/pass_key.h"
#include "browser_main_parts.h"
#include "content/public/browser/browser_thread.h"
// #include "content/public/browser/devtools_manager_delegate.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "ui/aura/client/default_capture_client.h"
#include "ui/aura/client/window_parenting_client.h"
#include "ui/aura/window_event_dispatcher.h"
#include "ui/gfx/native_widget_types.h"
namespace aloha {

namespace {
class AlohaWindowParentingClient : public aura::client::WindowParentingClient {
 public:
  explicit AlohaWindowParentingClient(aura::Window* window) : window_(window) {
    aura::client::SetWindowParentingClient(window_, this);
  }

  AlohaWindowParentingClient(const AlohaWindowParentingClient&) = delete;
  AlohaWindowParentingClient& operator=(const AlohaWindowParentingClient&) =
      delete;

  ~AlohaWindowParentingClient() override {
    aura::client::SetWindowParentingClient(window_, nullptr);
  }

  // Overridden from aura::client::WindowParentingClient:
  aura::Window* GetDefaultParent(aura::Window* window,
                                 const gfx::Rect& bounds,
                                 const int64_t display_id) override {
    if (!capture_client_) {
      capture_client_ = std::make_unique<aura::client::DefaultCaptureClient>(
          window_->GetRootWindow());
    }
    return window_;
  }

 private:
  raw_ptr<aura::Window> window_;

  std::unique_ptr<aura::client::DefaultCaptureClient> capture_client_;
};
}  // namespace

class BrowserMainPartsAura : public BrowserMainParts {
 public:
  using PassKey = base::PassKey<BrowserMainParts>;
  explicit BrowserMainPartsAura(PassKey) {}
  BrowserMainPartsAura(const BrowserMainPartsAura&) = delete;
  const BrowserMainPartsAura& operator=(const BrowserMainPartsAura&) = delete;
  ~BrowserMainPartsAura() override = default;

 private:
  void InitializeUiToolkit() override {
    aura_context_ = std::make_unique<AuraContext>();
  }

  void ShutdownUiToolkit() override { aura_context_.reset(); }

  void CreateAndShowWindowForWebContents(
      std::unique_ptr<content::WebContents> web_contents,
      const std::u16string& title) override {
    aura::Window* native_view = web_contents->GetNativeView();
    auto content_window = std::make_unique<ContentWindow>(
        aura_context_.get(), std::move(web_contents));

    // 添加 parenting_client ，修复点击外链时触发的崩溃，parenting_client
    // 会在窗口关闭前销毁
    auto window_parenting_client =
        std::make_unique<AlohaWindowParentingClient>(native_view);

    ContentWindow* content_window_ptr = content_window.get();
    content_window_ptr->SetTitle(title);
    content_window_ptr->Show();
    // 窗口关闭
    content_window_ptr->SetCloseCallback(base::BindOnce(
        &BrowserMainPartsAura::OnWindowClosed, weak_factory_.GetWeakPtr(),
        std::move(content_window), std::move(window_parenting_client)));
  }

  void OnWindowClosed(
      std::unique_ptr<ContentWindow> content_window,
      std::unique_ptr<AlohaWindowParentingClient> window_parenting_client) {
    // We are dispatching a callback that originates from the content_window.
    // Deleting soon instead of now eliminates the chance of a crash in case the
    // content_window or associated objects have more work to do after this
    // callback.
    LOG(INFO) << "OnWindowClosed Called";
    window_parenting_client.reset();
    content::GetUIThreadTaskRunner({})->DeleteSoon(FROM_HERE,
                                                   std::move(content_window));

    // TODO(anton): 定义 mojom 接口通知 devtools 关闭。
    BrowserMainParts::OnWindowClosed();
  }

  std::unique_ptr<AuraContext> aura_context_;
  base::WeakPtrFactory<BrowserMainPartsAura> weak_factory_{this};
};

// static
std::unique_ptr<BrowserMainParts> BrowserMainParts::Create() {
  return std::make_unique<BrowserMainPartsAura>(
      BrowserMainPartsAura::PassKey());
}

}  // namespace aloha
