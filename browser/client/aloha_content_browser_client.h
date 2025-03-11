// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_BROWSER_CLIENT_H_
#define ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_BROWSER_CLIENT_H_

#include <memory>

#include "aloha/browser/client/aloha_content_client_main_parts.h"
#include "base/memory/raw_ptr.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "content/shell/browser/shell_browser_context.h"

namespace aloha {

class AlohaBrowserClient;

class AlohaContentBrowserClient : public content::ContentBrowserClient {
 public:
  explicit AlohaContentBrowserClient(AlohaBrowserClient* views_content_client);

  AlohaContentBrowserClient(const AlohaContentBrowserClient&) = delete;
  AlohaContentBrowserClient& operator=(const AlohaContentBrowserClient&) =
      delete;

  ~AlohaContentBrowserClient() override;

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
  raw_ptr<AlohaBrowserClient> views_content_client_ = nullptr;
  raw_ptr<AlohaContentClientMainParts> views_content_client_main_parts_ = nullptr;
};

}  // namespace aloha

#endif  // ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_BROWSER_CLIENT_H_
