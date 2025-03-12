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

class AlohaContentClient;

class AlohaContentBrowserClient : public content::ContentBrowserClient {
 public:
  explicit AlohaContentBrowserClient(AlohaContentClient* views_content_client);

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

  // 为子进程设置一些属性
  // 当前还未明确这个接口的意义
  void BrowserChildProcessHostCreated(
      content::BrowserChildProcessHost* host) override;

  // DevTools
  // 目前 Aloha 的 Devtools 是直接套用 webui_example 中的 DevToolsFrontend
  // 启动服务的 而对于 DevTools 的展示页面是直接在 AlohaBrowserContentView
  // 中创建的，并没有直接用到 webui_example 中的 DevToolsManagerDelegate
  // 为了代码的一致性后续应当研究一下如何使用和管理这里的
  // DevToolsManagerDelegate
  // TODO(yeyun.anton)
  std::unique_ptr<content::DevToolsManagerDelegate>
  CreateDevToolsManagerDelegate() override;

  // NetworkContext 相关，包括网路数据的落盘
  // 被：StoragePartitionImpl::InitNetworkContext() 调用
  void ConfigureNetworkContextParams(
    content::BrowserContext* context,
    bool in_memory,
    const base::FilePath& relative_partition_path,
    network::mojom::NetworkContextParams* network_context_params,
    cert_verifier::mojom::CertVerifierCreationParams*
        cert_verifier_creation_params) override;

 private:
  raw_ptr<AlohaContentClient> views_content_client_ = nullptr;
  raw_ptr<AlohaContentClientMainParts> views_content_client_main_parts_ =
      nullptr;
};

}  // namespace aloha

#endif  // ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_BROWSER_CLIENT_H_
