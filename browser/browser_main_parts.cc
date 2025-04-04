// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/browser_main_parts.h"

#include <memory>
#include <tuple>

#include "aloha/browser/browser_context.h"
#include "aloha/browser/devtools/devtools_frontend.h"
#include "aloha/browser/devtools/devtools_manager_delegate.h"
#include "aloha/browser/devtools/devtools_server.h"
#include "aloha/browser/webui_controller_factory.h"
#include "aloha/common/aloha_constants.h"
#include "aloha/common/aloha_paths.h"
#include "aloha/resources/grit/unscaled_resources.h"
#include "base/command_line.h"
#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/run_loop.h"
#include "browser_main_parts.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "ui/aura/client/default_capture_client.h"
#include "ui/base/l10n/l10n_util.h"

namespace aloha {

namespace {

class WebContentsViewDelegate : public content::WebContentsViewDelegate {
 public:
  using CreateContentWindowFunc =
      base::RepeatingCallback<content::WebContents*(const GURL&)>;

  WebContentsViewDelegate(content::WebContents* web_contents,
                          CreateContentWindowFunc create_window_func)
      : web_contents_(web_contents),
        create_window_func_(std::move(create_window_func)) {}
  WebContentsViewDelegate(const WebContentsViewDelegate&) = delete;
  WebContentsViewDelegate& operator=(const WebContentsViewDelegate&) = delete;
  ~WebContentsViewDelegate() override = default;

 protected:
  // content::WebContentsViewDelegate:
  void ShowContextMenu(content::RenderFrameHost& render_frame_host,
                       const content::ContextMenuParams& params) override {
    DevToolsFrontend* frontend = DevToolsFrontend::CreateAndGet(web_contents_);
    LOG(INFO) << "inspect:" << frontend->frontend_url().spec();
    frontend->SetDevtoolsWebContents(
        create_window_func_.Run(frontend->frontend_url()));
  }

 private:
  // 主端 host webui 的对象
  const raw_ptr<content::WebContents> web_contents_;
  CreateContentWindowFunc create_window_func_;
};

}  // namespace

BrowserMainParts::BrowserMainParts() {
  web_contents_delegate_ = std::make_unique<WebContentDelegate>(this);
}

BrowserMainParts::~BrowserMainParts() = default;

std::unique_ptr<content::WebContentsViewDelegate>
BrowserMainParts::CreateWebContentsViewDelegate(
    content::WebContents* web_contents) {
  return std::make_unique<WebContentsViewDelegate>(
      web_contents,
      base::BindRepeating(
          [](BrowserMainParts* browser_main_parts, const GURL& url) {
            return browser_main_parts->CreateAndShowWindow(
                url,
                l10n_util::GetStringUTF16(IDS_ALOHA_DEVTOOLS_WINDOW_TITLE));
          },
          base::Unretained(this)));
}

std::unique_ptr<aloha::DevToolsManagerDelegate>
BrowserMainParts::CreateDevToolsManagerDelegate() {
  return std::make_unique<aloha::DevToolsManagerDelegate>(
      browser_context_.get(),
      base::BindRepeating(
          [](BrowserMainParts* browser_main_parts,
             content::BrowserContext* browser_context, const GURL& url) {
            return browser_main_parts->CreateAndShowWindow(
                url,
                l10n_util::GetStringUTF16(IDS_ALOHA_DEVTOOLS_WINDOW_TITLE));
          },
          base::Unretained(this)));
}

void BrowserMainParts::InitializeUiToolkit() {}

void BrowserMainParts::ShutdownUiToolkit() {}

int BrowserMainParts::PreMainMessageLoopRun() {
  std::ignore = temp_dir_.CreateUniqueTempDir();

  browser_context_ = std::make_unique<BrowserContext>(temp_dir_.GetPath());

  devtools::StartHttpHandler(browser_context_.get());

  web_ui_controller_factory_ = std::make_unique<WebUIControllerFactory>();
  content::WebUIControllerFactory::RegisterFactory(
      web_ui_controller_factory_.get());

  InitializeUiToolkit();

  if (base::CommandLine::ForCurrentProcess()->HasSwitch("webshell")) {
    CreateAndShowWindow(GURL("chrome://browser"),
                        l10n_util::GetStringUTF16(IDS_ALOHA_WEBSHELL_TITLE));
    return 0;
  }
  LOG(INFO) << "Going to CreatAdnShowWindows from 'file://'";
  base::FilePath aloha_home_path;
  aloha::path_service::GetWebAppPath(&aloha_home_path,
                                     webapp::internal::kAlohaHome);

  GURL url = GURL("file://" + aloha_home_path.MaybeAsASCII());
  CreateAndShowWindow(url, l10n_util::GetStringUTF16(IDS_ALOHA_WINDOW_TITLE));

  return 0;
}

content::WebContents* BrowserMainParts::CreateAndShowWindow(
    GURL url,
    const std::u16string& title) {
  // 主窗口和 devtools 窗口的创建都会走这条路径
  // aloha::BrowserMainParts::CreateWebContentsViewDelegate 回调过来
  ++content_windows_outstanding_;

  content::WebContents::CreateParams params(browser_context_.get());
  std::unique_ptr<content::WebContents> web_contents =
      content::WebContents::Create(params);
  content::NavigationController::LoadURLParams url_params(url);
  web_contents->GetController().LoadURLWithParams(url_params);
  // 添加 WebContentsDelegate 处理新页面的创建
  web_contents->SetDelegate(this->GetWebContentDelegate());

  auto* web_contents_ptr = web_contents.get();
  CreateAndShowWindowForWebContents(std::move(web_contents), title);

  return web_contents_ptr;
}

void BrowserMainParts::WillRunMainMessageLoop(
    std::unique_ptr<base::RunLoop>& run_loop) {
  quit_run_loop_ = run_loop->QuitClosure();
}

void BrowserMainParts::PostMainMessageLoopRun() {
  devtools::StopHttpHandler();
  browser_context_.reset();
}

void BrowserMainParts::OnWindowClosed() {
  --content_windows_outstanding_;
  auto task_runner = content::GetUIThreadTaskRunner({});
  if (content_windows_outstanding_ == 0) {
    task_runner->PostTask(FROM_HERE,
                          base::BindOnce(&BrowserMainParts::QuitMessageLoop,
                                         weak_factory_.GetWeakPtr()));
  }
}

void BrowserMainParts::QuitMessageLoop() {
  ShutdownUiToolkit();
  web_ui_controller_factory_.reset();
  quit_run_loop_.Run();
}
void BrowserMainParts::WebContentDelegate::WebContentsCreated(
    content::WebContents* source_contents,
    int opener_render_process_id,
    int opener_render_frame_id,
    const std::string& frame_name,
    const GURL& target_url,
    content::WebContents* new_contents) {
  browser_main_parts_->CreateAndShowWindow(
      // GURL("aloha://main"),
      target_url, l10n_util::GetStringUTF16(IDS_ALOHA_WINDOW_TITLE));
  // 打印信息
  LOG(INFO) << "opener_render_process_id: " << opener_render_process_id;
  LOG(INFO) << "opener_render_frame_id: " << opener_render_frame_id;
  LOG(INFO) << "frame_name: " << frame_name;
  LOG(INFO) << "target_url: " << target_url;
  LOG(INFO) << "new_contents: " << new_contents;
  LOG(INFO) << target_url.ExtractFileName();
  LOG(INFO) << target_url.HostNoBrackets();
  LOG(INFO) << target_url.HostNoBracketsPiece();
}

}  // namespace aloha
