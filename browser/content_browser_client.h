// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_BROWSER_CONTENT_BROWSER_CLIENT_H_
#define ALOHA_BROWSER_CONTENT_BROWSER_CLIENT_H_

#include <memory>

#include "base/memory/raw_ptr.h"
#include "content/public/browser/content_browser_client.h"

namespace aloha {

class BrowserMainParts;
// Call in aloha::MainDelegate
class ContentBrowserClient : public content::ContentBrowserClient {
 public:
  ContentBrowserClient();
  ContentBrowserClient(const ContentBrowserClient&) = delete;
  ContentBrowserClient& operator=(const ContentBrowserClient&) = delete;
  ~ContentBrowserClient() override;

 private:
  // content::ContentBrowserClient:
  std::unique_ptr<content::BrowserMainParts> CreateBrowserMainParts(
      bool is_integration_test) override;

  // Delegate 和窗口页面有关
  std::unique_ptr<content::WebContentsViewDelegate> GetWebContentsViewDelegate(
      content::WebContents* web_contents) override;
  std::unique_ptr<content::DevToolsManagerDelegate>
  CreateDevToolsManagerDelegate() override;
  
  void RegisterBrowserInterfaceBindersForFrame(
      content::RenderFrameHost* render_frame_host,
      mojo::BinderMapWithContext<content::RenderFrameHost*>* map) override;
  void RegisterAssociatedInterfaceBindersForRenderFrameHost(
      content::RenderFrameHost& render_frame_host,
      blink::AssociatedInterfaceRegistry& associated_registry) override;
  std::string GetUserAgent() override;
  // TODO(yeyun.anton): LOGO 设置
  std::optional<gfx::ImageSkia> GetProductLogo() override;
  // 需要添加 --use-pure-webview
  void AppendExtraCommandLineSwitches(base::CommandLine* command_line,
                                              int child_process_id) override;

  raw_ptr<BrowserMainParts, AcrossTasksDanglingUntriaged> browser_main_parts_ =
      nullptr;
};

}  // namespace aloha

#endif  // ALOHA_BROWSER_CONTENT_BROWSER_CLIENT_H_
