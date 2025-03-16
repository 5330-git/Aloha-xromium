// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/client/aloha_content_browser_client.h"

#include <memory>
#include <utility>
#include <vector>

#include "aloha/browser/client/aloha_content_client_main_parts.h"
#include "aloha/browser/devtools/devtools_manager_delegate.h"
#include "aloha/browser/ui/views/aloha_web_contents_view_delegate_views.h"
#include "aloha/browser/url/demo/demo_navigation_url_loader_factory.h"
#include "aloha/browser/url/demo/demo_sub_resources_url_loader_factory.h"
#include "aloha/browser/url/demo/demo_url_loader_request_interceptor.h"
#include "aloha/common/aloha_constants.h"
#include "aloha/common/aloha_main_client.h"
#include "aloha/common/aloha_paths.h"
#include "aloha/grit/aloha_resources.h"
#include "aloha_content_browser_client.h"
#include "aloha_content_client_main_parts.h"
#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "base/functional/callback_forward.h"
#include "base/functional/callback_helpers.h"
#include "base/path_service.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_child_process_host.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/file_url_loader.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/url_loader_request_interceptor.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/user_agent.h"
#include "content/shell/browser/shell_devtools_manager_delegate.h"
#include "content/shell/browser/shell_paths.h"
#include "services/cert_verifier/public/mojom/cert_verifier_service_factory.mojom.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/self_deleting_url_loader_factory.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/widget/widget.h"
#include "url/url_constants.h"

namespace aloha {

AlohaContentBrowserClient::~AlohaContentBrowserClient() {}

// 这个接口只会在启动的时候调用一次（BrowserMainLoop::Init()），理论上不应该多次调用
std::unique_ptr<content::BrowserMainParts>
AlohaContentBrowserClient::CreateBrowserMainParts(
    bool /* is_integration_test */) {
  DCHECK(!aloha_content_client_main_parts_);
  auto browser_main_parts = AlohaContentClientMainParts::Create();
  aloha_content_client_main_parts_ = browser_main_parts.get();
  return browser_main_parts;
}

std::string AlohaContentBrowserClient::GetUserAgent() {
  return content::BuildUserAgentFromProduct("Aloha-Xromium/133.0.6877.0");
}

// return AlohaWebContentsViewDelegate
std::unique_ptr<content::WebContentsViewDelegate>
AlohaContentBrowserClient::GetWebContentsViewDelegate(
    content::WebContents* web_contents) {
  return std::make_unique<AlohaWebContentsViewDelegate>(web_contents);
}

std::unique_ptr<content::DevToolsManagerDelegate>
AlohaContentBrowserClient::CreateDevToolsManagerDelegate() {
  content::BrowserContext* browser_context =
      aloha_content_client_main_parts_->browser_context();
  return std::make_unique<aloha::DevToolsManagerDelegate>(
      browser_context,
      base::BindRepeating(
          [](content::BrowserMainParts* browser_main_parts,
             AlohaMainClient* views_content_client,
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
          base::Unretained(aloha_content_client_main_parts_),
          AlohaMainClient::GetInstance()));
  // return nullptr;
}

void AlohaContentBrowserClient::BrowserChildProcessHostCreated(
    content::BrowserChildProcessHost* host) {
  host->SetName(u"AlohaChildProcess");
}

void AlohaContentBrowserClient::ConfigureNetworkContextParams(
    content::BrowserContext* context,
    bool in_memory,
    const base::FilePath& relative_partition_path,
    network::mojom::NetworkContextParams* network_context_params,
    cert_verifier::mojom::CertVerifierCreationParams*
        cert_verifier_creation_params) {
  content::ContentBrowserClient::ConfigureNetworkContextParams(
      context, in_memory, relative_partition_path, network_context_params,
      cert_verifier_creation_params);
  // content::ContentBrowserClient 会做下面的工作：
  // network_context_params->user_agent = GetUserAgentBasedOnPolicy(context);
  // network_context_params->accept_language = "en-us,en";

  // 设置 Cookies 落盘数据路径：
  LOG(INFO) << "relative_partition_path:" << relative_partition_path.value();
  LOG(INFO) << "in memory:" << in_memory;
  base::FilePath user_data_dir;
  base::PathService::Get(aloha::path_service::ALOHA_USER_DATA_DIR,
                         &user_data_dir);
  if (!network_context_params->file_paths) {
    network_context_params->file_paths =
        network::mojom::NetworkContextFilePaths::New();
  }
  if (!network_context_params->cookie_manager_params) {
    network_context_params->cookie_manager_params =
        network::mojom::CookieManagerParams::New();
  }
  // Definations Start
  // 命名参考：chrome/browser/net/profile_network_context_service.cc:
  // ProfileNetworkContextService::ConfigureNetworkContextParamsInternal
  network_context_params->file_paths->http_cache_directory =
      user_data_dir.Append(aloha::kCacheDirname);
  network_context_params->file_paths->data_directory =
      user_data_dir.Append(aloha::kNetworkDataDirname);
  network_context_params->file_paths->unsandboxed_data_path = user_data_dir;
  network_context_params->file_paths->cookie_database_name =
      base::FilePath(aloha::kCookieFilename);
  network_context_params->file_paths->device_bound_sessions_database_name =
      base::FilePath(aloha::kDeviceBoundSessionsFilename);
  network_context_params->file_paths->trust_token_database_name =
      base::FilePath(aloha::kTrustTokenFilename);
  network_context_params->file_paths->http_server_properties_file_name =
      base::FilePath(aloha::kNetworkPersistentStateFilename);
  network_context_params->file_paths->transport_security_persister_file_name =
      base::FilePath(aloha::kTransportSecurityPersisterFilename);
  network_context_params->file_paths->reporting_and_nel_store_database_name =
      base::FilePath(aloha::kReportingAndNelStoreFilename);
  network_context_params->file_paths->sct_auditing_pending_reports_file_name =
      base::FilePath(aloha::kSCTAuditingPendingReportsFileName);
  network_context_params->file_paths->trigger_migration = true;
  // FOR TEST
  network_context_params->cookie_manager_params->allow_file_scheme_cookies =
      true;
  network_context_params->restore_old_session_cookies = false;
  network_context_params->persist_session_cookies = true;
  network_context_params->enable_encrypted_cookies = true;
  // TODO(yeyun.anton): 将 kAlohaScheme 放到 CookieableScheme 中
  // 参见：net::CookieManager::AllowFileSchemeCookies
  // 这会比较困难，因为内核代码并没有预留接口，除非修改内核代码，但这样会增加与内核代码的耦合度，
  // 后续需要评估是否需要在C++/JavaScript混合应用（aloha://apps）下使用
  // cookies 相关的特性
  // network_context_params->cookie_manager_params
  //     ->secure_origin_cookies_allowed_schemes = {aloha::url::kAlohaScheme};
  // network_context_params->cookie_manager_params
  //     ->matching_scheme_cookies_allowed_schemes = {aloha::url::kAlohaScheme};
  // network_context_params->cookie_manager_params
  //     ->third_party_cookies_allowed_schemes = {aloha::url::kAlohaScheme};

  // network_context_params->enable_locking_cookie_database = true;

  // network_context_params->initial_ssl_config;

  // Definetions End
}
bool AlohaContentBrowserClient::IsHandledURL(const GURL& url) {
  if (url.SchemeIs(aloha::url::kAlohaScheme)) {
    return true;
  }
  return false;
}
void AlohaContentBrowserClient::
    RegisterNonNetworkWorkerMainResourceURLLoaderFactories(
        content::BrowserContext* browser_context,
        NonNetworkURLLoaderFactoryMap* factories) {
  LOG(INFO) << "RegisterNonNetworkWorkerMainResourceURLLoaderFactories";
  content::ContentBrowserClient::
      RegisterNonNetworkWorkerMainResourceURLLoaderFactories(browser_context,
                                                             factories);
}

// TODO(yeyun.anton): 检查是否需要实现
void AlohaContentBrowserClient::RegisterNonNetworkSubresourceURLLoaderFactories(
    int render_process_id,
    int render_frame_id,
    const std::optional<::url::Origin>& request_initiator_origin,
    NonNetworkURLLoaderFactoryMap* factories) {
  LOG(INFO) << "RegisterNonNetworkSubresourceURLLoaderFactories";
  for (auto& [url, factory] : *factories) {
    LOG(INFO) << url << " : " << factory.version();
  }
  content::ContentBrowserClient::
      RegisterNonNetworkSubresourceURLLoaderFactories(
          render_process_id, render_process_id, request_initiator_origin,
          factories);
  auto demo_sub_resources_url_loader_factory =
      aloha::url::DemoSubResourcesURLLoaderFactory::Create();
  factories->insert({aloha::url::kAlohaDemoScheme,
                     std::move(demo_sub_resources_url_loader_factory)});
}

// TODO(yeyun.anton): 检查是否需要实现
bool AlohaContentBrowserClient::HasCustomSchemeHandler(
    content::BrowserContext* browser_context,
    const std::string& scheme) {
  LOG(INFO) << "HasCustomSchemeHandler";
  return content::ContentBrowserClient::HasCustomSchemeHandler(browser_context,
                                                               scheme);
}

void AlohaContentBrowserClient::GetAdditionalAllowedSchemesForFileSystem(
    std::vector<std::string>* additial_allowed_schemes) {
  LOG(INFO) << "GetAdditionalAllowedSchemesForFileSystem";
  additial_allowed_schemes->push_back(aloha::url::kAlohaScheme);
}

base::FilePath AlohaContentBrowserClient::GetFirstPartySetsDirectory() {
  base::FilePath user_data_dir;
  base::PathService::Get(aloha::path_service::ALOHA_USER_DATA_DIR,
                         &user_data_dir);
  return user_data_dir;
}
std::vector<std::unique_ptr<content::URLLoaderRequestInterceptor>>
AlohaContentBrowserClient::WillCreateURLLoaderRequestInterceptors(
    content::NavigationUIData* navigation_ui_data,
    content::FrameTreeNodeId frame_tree_node_id,
    int64_t navigation_id,
    bool force_no_https_upgrade,
    scoped_refptr<base::SequencedTaskRunner> navigation_response_task_runner) {
  std::vector<std::unique_ptr<content::URLLoaderRequestInterceptor>>
      interceptors;

  std::unique_ptr<content::URLLoaderRequestInterceptor> interceptor =
      std::make_unique<aloha::url::DemoURLLoaderRequestInterceptor>();
  interceptors.push_back(std::move(interceptor));

  return interceptors;
}

mojo::PendingRemote<network::mojom::URLLoaderFactory>
AlohaContentBrowserClient::CreateNonNetworkNavigationURLLoaderFactory(
    const std::string& scheme,
    content::FrameTreeNodeId frame_tree_node_id) {
  if (scheme == aloha::url::kAlohaDemoScheme) {
    // TODO(yeyun.anton): 创建 Aloha 协议的加载器工厂
    return aloha::url::DemoNavigationURLLoaderFactory::Create();
  }
  return {};
}

}  // namespace aloha
