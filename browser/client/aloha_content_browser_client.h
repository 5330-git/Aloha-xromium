// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_BROWSER_CLIENT_H_
#define ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_BROWSER_CLIENT_H_

#include <memory>
#include <vector>

#include "aloha/browser/client/aloha_content_client_main_parts.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/memory/raw_ptr.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/url_loader_request_interceptor.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "content/shell/browser/shell_browser_context.h"

namespace aloha {

class AlohaMainClient;

class AlohaContentBrowserClient : public content::ContentBrowserClient {
 public:
  AlohaContentBrowserClient() = default;

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

  bool IsHandledURL(const GURL& url) override;
  void RegisterNonNetworkWorkerMainResourceURLLoaderFactories(
      content::BrowserContext* context,
      NonNetworkURLLoaderFactoryMap* factories) override;

  // 目前确定这个接口不会负责创建Response
  void RegisterNonNetworkSubresourceURLLoaderFactories(
      int render_process_id,
      int render_frame_id,
      const std::optional<::url::Origin>& request_initiator_origin,
      NonNetworkURLLoaderFactoryMap* factories) override;

  // 目前确定这个接口不会影响通过 URLLoaderRequestInterceptor
  // 拦截处理并响应的资源请求流程
  bool HasCustomSchemeHandler(content::BrowserContext* browser_context,
                              const std::string& scheme) override;

  void GetAdditionalAllowedSchemesForFileSystem(
      std::vector<std::string>* additial_allowed_schemes) override;

  // 解决日志中的问题："Empty path. Failed initializing First-Party Sets
  // database."
  base::FilePath GetFirstPartySetsDirectory() override;

  // 实现 aloha:// 协议的核心接口，通过注册的
  // content::URLLoaderRequestInterceptor 拦截处理并响应 aloha:// 协议的请求
  std::vector<std::unique_ptr<content::URLLoaderRequestInterceptor>>
  WillCreateURLLoaderRequestInterceptors(
      content::NavigationUIData* navigation_ui_data,
      content::FrameTreeNodeId frame_tree_node_id,
      int64_t navigation_id,
      bool force_no_https_upgrade,
      scoped_refptr<base::SequencedTaskRunner> navigation_response_task_runner)
      override;

  // TODO(yeyun.anton): 实现这个接口
  //   base::FilePath GetLoggingFileName(const base::CommandLine& command_line)
  //   override;

 private:
  raw_ptr<AlohaContentClientMainParts> aloha_content_client_main_parts_ =
      nullptr;
};

}  // namespace aloha

#endif  // ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_BROWSER_CLIENT_H_
