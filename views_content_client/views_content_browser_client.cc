// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/views_content_client/views_content_browser_client.h"

#include <memory>
#include <utility>

#include "aloha/browser/devtools/devtools_manager_delegate.h"
// #include "aloha/browser/ui/native/aloha_web_contents_view_delegate_views.h"
// #include "aloha/browser/ui/native/widget_delegate_view.h"
#include "aloha/browser/ui/views/aloha_web_contents_view_delegate_views.h"
#include "aloha/grit/aloha_resources.h"
#include "aloha/views_content_client/views_content_client.h"
#include "aloha/views_content_client/views_content_client_main_parts.h"
#include "base/functional/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/user_agent.h"
#include "content/shell/browser/shell_devtools_manager_delegate.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/widget/widget.h"
#include "views_content_browser_client.h"
#include "views_content_client_main_parts.h"

namespace aloha {

ViewsContentBrowserClient::ViewsContentBrowserClient(
    ViewsContentClient* views_content_client)
    : views_content_client_(views_content_client) {}

ViewsContentBrowserClient::~ViewsContentBrowserClient() {}

// 这个接口只会在启动的时候调用一次（BrowserMainLoop::Init()），理论上不应该多次调用
std::unique_ptr<content::BrowserMainParts>
ViewsContentBrowserClient::CreateBrowserMainParts(
    bool /* is_integration_test */) {
  DCHECK(!views_content_client_main_parts_);
  auto browser_main_parts =
      ViewsContentClientMainParts::Create(views_content_client_);
  views_content_client_main_parts_ = browser_main_parts.get();
  return browser_main_parts;
}

std::string ViewsContentBrowserClient::GetUserAgent() {
  return content::BuildUserAgentFromProduct("Aloha-Xromium/133.0.6877.0");
}

// return AlohaWebContentsViewDelegate
std::unique_ptr<content::WebContentsViewDelegate>
ViewsContentBrowserClient::GetWebContentsViewDelegate(
    content::WebContents* web_contents) {
  return std::make_unique<AlohaWebContentsViewDelegate>(
      web_contents);
}

std::unique_ptr<content::DevToolsManagerDelegate>
ViewsContentBrowserClient::CreateDevToolsManagerDelegate() {
  content::BrowserContext* browser_context =
      views_content_client_main_parts_->browser_context();
  return std::make_unique<aloha::DevToolsManagerDelegate>(
      browser_context,
      base::BindRepeating(
          [](content::BrowserMainParts* browser_main_parts,
             ViewsContentClient* views_content_client,
             content::BrowserContext* browser_context,
             const GURL& devtools_url) {
            static auto devtools_webview =
                std::make_unique<views::WebView>(browser_context);

            // 创建窗口
            views::Widget* devtools_widget = new views::Widget();
            views::Widget::InitParams params(
                views::Widget::InitParams::NATIVE_WIDGET_OWNS_WIDGET,
                views::Widget::InitParams::TYPE_WINDOW);
            // params.delegate = new AlohaWidgetDelegateView(
            //     std::move(views_content_client->quit_closure()),
            //     browser_context);
            devtools_webview->LoadInitialURL(devtools_url);
            // params.context = XXXX;
            params.name = base::UTF16ToUTF8(
                l10n_util::GetStringUTF16(IDS_ALOHA_DEVTOOLS_WINDOW_TITLE));
            devtools_widget->Init(std::move(params));
            devtools_widget->Show();
            devtools_webview->GetWebContents()->Focus();
            return devtools_webview->GetWebContents();
          },
          base::Unretained(views_content_client_main_parts_),
          views_content_client_));
  // return nullptr;
}

}  // namespace aloha
