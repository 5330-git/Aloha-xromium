// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/content_browser_client.h"

#include <optional>

#include "aloha/browser/browser_main_parts.h"
#include "aloha/browser/ui/web/browser.h"
#include "aloha/browser/ui/web/browser.mojom.h"
#include "aloha/browser/ui/web/webshell_guest_view.h"
#include "base/command_line.h"
#include "components/guest_view/common/guest_view.mojom.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/devtools_manager_delegate.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "content/public/common/user_agent.h"
#include "content_browser_client.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/image/image_skia.h"

namespace aloha {

ContentBrowserClient::ContentBrowserClient() = default;

ContentBrowserClient::~ContentBrowserClient() = default;

std::unique_ptr<content::BrowserMainParts>
ContentBrowserClient::CreateBrowserMainParts(bool is_integration_test) {
  auto browser_main_parts = BrowserMainParts::Create();
  browser_main_parts_ = browser_main_parts.get();
  return browser_main_parts;
}

std::unique_ptr<content::WebContentsViewDelegate>
ContentBrowserClient::GetWebContentsViewDelegate(
    content::WebContents* web_contents) {
  return browser_main_parts_->CreateWebContentsViewDelegate(web_contents);
}

std::unique_ptr<content::DevToolsManagerDelegate>
ContentBrowserClient::CreateDevToolsManagerDelegate() {
  return browser_main_parts_->CreateDevToolsManagerDelegate();
}

void ContentBrowserClient::RegisterBrowserInterfaceBindersForFrame(
    content::RenderFrameHost* render_frame_host,
    mojo::BinderMapWithContext<content::RenderFrameHost*>* map) {
  map->Add<aloha::mojom::PageHandlerFactory>(base::BindRepeating(
      [](content::RenderFrameHost* host,
         mojo::PendingReceiver<aloha::mojom::PageHandlerFactory> receiver) {
        if (host->GetParent()) {
          LOG(ERROR) << "Called for Non-Main Frame!";
          return;
        }

        auto* web_ui = host->GetWebUI();
        Browser* browser = web_ui->GetController()->GetAs<Browser>();
        if (!browser) {
          LOG(ERROR) << "Failed to Get Browser";
          return;
        }

        browser->BindInterface(std::move(receiver));
      }));
}

void ContentBrowserClient::RegisterAssociatedInterfaceBindersForRenderFrameHost(
    content::RenderFrameHost& render_frame_host,
    blink::AssociatedInterfaceRegistry& associated_registry) {
  associated_registry.AddInterface<guest_view::mojom::GuestViewHost>(
      base::BindRepeating(&WebshellGuestView::Create,
                          render_frame_host.GetGlobalId()));
}

std::string ContentBrowserClient::GetUserAgent() {
  return content::BuildUserAgentFromProduct("Aloha-Xromium/133.0.6877.0");
}

std::optional<gfx::ImageSkia> ContentBrowserClient::GetProductLogo() {
  LOG(INFO) << "GetProductLogo";
  return std::nullopt;
}

void ContentBrowserClient::AppendExtraCommandLineSwitches(base::CommandLine* command_line,
                                    int child_process_id) {
  // 检查主进程是否包含 --use-pure-webview 参数
  if (base::CommandLine::ForCurrentProcess()->HasSwitch("use-webui")) {
    // 将参数传递给子进程
    command_line->AppendSwitch("use-webui");
  }
  LOG(INFO) << "ChildProcessId:" << child_process_id;
}

}  // namespace aloha
