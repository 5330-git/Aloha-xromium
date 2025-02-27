// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_APP_MAIN_DELEGATE_H_
#define ALOHA_APP_MAIN_DELEGATE_H_

#include <optional>

#include "content/public/app/content_main_delegate.h"

namespace content {
class ContentBrowserClient;
class ContentClient;
class ContentRendererClient;
}  // namespace content

namespace aloha {

class MainDelegate : public content::ContentMainDelegate {
 public:
  MainDelegate();
  MainDelegate(const MainDelegate&) = delete;
  MainDelegate& operator=(const MainDelegate&) = delete;
  ~MainDelegate() override;

 private:
  // content::ContentMainDelegate:
  std::optional<int> BasicStartupComplete() override;
  void PreSandboxStartup() override;
  content::ContentBrowserClient* CreateContentBrowserClient() override;
  std::optional<int> PreBrowserMain() override;
  content::ContentRendererClient* CreateContentRendererClient() override;

  std::unique_ptr<content::ContentClient> content_client_;
  std::unique_ptr<content::ContentBrowserClient> content_browser_client_;
  std::unique_ptr<content::ContentRendererClient> content_renderer_client_;
};

}  // namespace aloha

#endif  // ALOHA_APP_MAIN_DELEGATE_H_
