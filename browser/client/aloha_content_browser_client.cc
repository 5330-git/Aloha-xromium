// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/client/aloha_content_browser_client.h"

#include <memory>
#include <utility>

#include "aloha/common/aloha_main_client.h"
#include "aloha/browser/client/aloha_content_client_main_parts.h"
#include "aloha/browser/devtools/devtools_manager_delegate.h"
#include "aloha/browser/ui/views/aloha_web_contents_view_delegate_views.h"
#include "aloha/grit/aloha_resources.h"
#include "aloha_content_browser_client.h"
#include "aloha_content_client_main_parts.h"
#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_child_process_host.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/user_agent.h"
#include "content/shell/browser/shell_devtools_manager_delegate.h"
#include "content/shell/browser/shell_paths.h"
#include "services/cert_verifier/public/mojom/cert_verifier_service_factory.mojom.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/widget/widget.h"

namespace aloha {

AlohaContentBrowserClient::~AlohaContentBrowserClient() {}

// 这个接口只会在启动的时候调用一次（BrowserMainLoop::Init()），理论上不应该多次调用
std::unique_ptr<content::BrowserMainParts>
AlohaContentBrowserClient::CreateBrowserMainParts(
    bool /* is_integration_test */) {
  DCHECK(!views_content_client_main_parts_);
  auto browser_main_parts =
      AlohaContentClientMainParts::Create();
  views_content_client_main_parts_ = browser_main_parts.get();
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
      views_content_client_main_parts_->browser_context();
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
          base::Unretained(views_content_client_main_parts_),
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
  // 访问 USER DATA DIR 路径，目前这个值是通过 content::ShellBrowserContext
  // 写入的，后续需要定义我们自己的
  // TODO(yeyun.anton): 将字符串迁移出来
  base::FilePath user_data_dir;
  base::PathService::Get(content::SHELL_DIR_USER_DATA, &user_data_dir);
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
      user_data_dir.Append(FILE_PATH_LITERAL("Cache"));
  network_context_params->file_paths->data_directory =
      user_data_dir.Append(FILE_PATH_LITERAL("Network"));
  network_context_params->file_paths->unsandboxed_data_path = user_data_dir;
  network_context_params->file_paths->cookie_database_name =
      base::FilePath(FILE_PATH_LITERAL("Cookies"));
  network_context_params->file_paths->device_bound_sessions_database_name =
      base::FilePath(FILE_PATH_LITERAL("Device Bound Sessions"));
  network_context_params->file_paths->trust_token_database_name =
      base::FilePath(FILE_PATH_LITERAL("Trust Tokens"));
  network_context_params->file_paths->http_server_properties_file_name =
      base::FilePath(FILE_PATH_LITERAL("Network Persistent State"));
  network_context_params->file_paths->transport_security_persister_file_name =
      base::FilePath(FILE_PATH_LITERAL("TransportSecurity"));
  network_context_params->file_paths->reporting_and_nel_store_database_name =
      base::FilePath(FILE_PATH_LITERAL("Reporting and NEL"));
  network_context_params->file_paths->sct_auditing_pending_reports_file_name =
      base::FilePath(FILE_PATH_LITERAL("SCT Auditing Pending Reports"));
  network_context_params->file_paths->trigger_migration = true;
  // FOR TEST
    network_context_params->cookie_manager_params->allow_file_scheme_cookies = true;
  network_context_params->restore_old_session_cookies = false;
  network_context_params->persist_session_cookies = true;
  network_context_params->enable_encrypted_cookies = true;

  // network_context_params->enable_locking_cookie_database = true;

  // network_context_params->initial_ssl_config;

  // Definetions End
}

}  // namespace aloha
