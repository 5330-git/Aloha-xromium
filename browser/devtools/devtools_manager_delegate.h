// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_BROWSER_DEVTOOLS_DEVTOOLS_MANAGER_DELEGATE_H_
#define ALOHA_BROWSER_DEVTOOLS_DEVTOOLS_MANAGER_DELEGATE_H_

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/devtools_manager_delegate.h"
#include "url/gurl.h"

namespace content {
class BrowserContext;
class WebContents;
}  // namespace content

namespace aloha {

class DevToolsManagerDelegate : public content::DevToolsManagerDelegate {
 public:
  using CreateContentWindowFunc =
      base::RepeatingCallback<content::WebContents*(content::BrowserContext*,
                                                    const GURL&)>;

  DevToolsManagerDelegate(content::BrowserContext* browser_context,
                          CreateContentWindowFunc create_content_window_func);
  DevToolsManagerDelegate(const DevToolsManagerDelegate&) = delete;
  DevToolsManagerDelegate& operator=(const DevToolsManagerDelegate&) = delete;
  ~DevToolsManagerDelegate() override;

  // DevToolsManagerDelegate:
  content::BrowserContext* GetDefaultBrowserContext() override;
  scoped_refptr<content::DevToolsAgentHost> CreateNewTarget(
      const GURL& url,
      TargetType target_type) override;
  std::string GetDiscoveryPageHTML() override;
  bool HasBundledFrontendResources() override;
  base::WeakPtr<DevToolsManagerDelegate> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

 private:
  const raw_ptr<content::BrowserContext> browser_context_;
  CreateContentWindowFunc create_content_window_func_;
  base::WeakPtrFactory<DevToolsManagerDelegate> weak_factory_{this};
};

// 承载 DevTools 的 View 控件

}  // namespace aloha

#endif  // ALOHA_BROWSER_DEVTOOLS_DEVTOOLS_MANAGER_DELEGATE_H_
