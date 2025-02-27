// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_BROWSER_DEVTOOLS_DEVTOOLS_FRONTEND_H_
#define ALOHA_BROWSER_DEVTOOLS_DEVTOOLS_FRONTEND_H_

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/web_contents_observer.h"
#include "url/gurl.h"

namespace content {
class WebContents;
}

namespace aloha {

class DevToolsFrontend {
 public:
  DevToolsFrontend(const DevToolsFrontend&) = delete;
  DevToolsFrontend& operator=(const DevToolsFrontend&) = delete;
  ~DevToolsFrontend();

  static DevToolsFrontend* CreateAndGet(
      content::WebContents* inspected_contents);

  const GURL& frontend_url() { return frontend_url_; }

  void SetDevtoolsWebContents(content::WebContents* devtools_contents);

 private:
  class AgentHostClient;
  class Pointer;
  explicit DevToolsFrontend(content::WebContents* inspected_contents);

  const GURL frontend_url_;
  const base::raw_ptr<content::WebContents> inspected_contents_;
  std::unique_ptr<AgentHostClient> agent_host_client_;
};

}  // namespace aloha

#endif  // ALOHA_BROWSER_DEVTOOLS_DEVTOOLS_FRONTEND_H_
