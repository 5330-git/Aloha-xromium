// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_MAIN_DELEGATE_H_
#define ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_MAIN_DELEGATE_H_

#include <memory>

#include "base/memory/raw_ptr.h"
#include "content/public/app/content_main_delegate.h"
#include "content/shell/common/shell_content_client.h"

namespace aloha {

class AlohaContentBrowserClient;
class AlohaBrowserClient;

class AlohaContentMainDelegate : public content::ContentMainDelegate {
 public:
  explicit AlohaContentMainDelegate(AlohaBrowserClient* browser_client);

  AlohaContentMainDelegate(const AlohaContentMainDelegate&) = delete;
  AlohaContentMainDelegate& operator=(const AlohaContentMainDelegate&) = delete;

  ~AlohaContentMainDelegate() override;

  // content::ContentMainDelegate implementation
  std::optional<int> BasicStartupComplete() override;
  void PreSandboxStartup() override;
  std::optional<int> PreBrowserMain() override;
  content::ContentClient* CreateContentClient() override;
  content::ContentBrowserClient* CreateContentBrowserClient() override;

 private:
  std::unique_ptr<AlohaContentBrowserClient> content_browser_client_;
  content::ShellContentClient content_client_;
  raw_ptr<AlohaBrowserClient> browser_client_;
};

}  // namespace aloha

#endif  // ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_MAIN_DELEGATE_H_
