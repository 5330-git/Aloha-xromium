// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_BROWSER_CLIENT_H_
#define ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_BROWSER_CLIENT_H_

#include <memory>

#include "aloha/browser/ui/native/widget_delegate_view.h"
#include "aloha/views_content_client/views_content_client_main_parts.h"
#include "base/memory/raw_ptr.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "content/shell/browser/shell_browser_context.h"

namespace aloha {

class ViewsContentClient;

class ViewsContentBrowserClient : public content::ContentBrowserClient {
 public:
  explicit ViewsContentBrowserClient(ViewsContentClient* views_content_client);

  ViewsContentBrowserClient(const ViewsContentBrowserClient&) = delete;
  ViewsContentBrowserClient& operator=(const ViewsContentBrowserClient&) =
      delete;

  ~ViewsContentBrowserClient() override;

  // content::ContentBrowserClient:
  std::unique_ptr<content::BrowserMainParts> CreateBrowserMainParts(
      bool is_integration_test) override;
  std::string GetUserAgent() override;
  // 扩充 WebContentsView 的功能，右键菜单需要通过 WebContentsViewDelegate
  // 实现（AlohaWebContentsViewDelegate）
  std::unique_ptr<content::WebContentsViewDelegate> GetWebContentsViewDelegate(
      content::WebContents* web_contents) override;
  // DevTools
  std::unique_ptr<content::DevToolsManagerDelegate>
  CreateDevToolsManagerDelegate() override;

 private:
  raw_ptr<ViewsContentClient> views_content_client_ = nullptr;
  raw_ptr<ViewsContentClientMainParts> views_content_client_main_parts_ = nullptr;
  // 记录 webcontent 到 webappview 的映射
  std::map<content::WebContents*, aloha::WebAppView*> web_app_views_;
};

}  // namespace aloha

#endif  // ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_BROWSER_CLIENT_H_
