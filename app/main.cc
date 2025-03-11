// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/app/main_delegate.h"
// #include "aloha/browser/ui/native/widget_delegate_view.h"
#include "aloha/browser/ui/views/widget/widget_delegate_view.h"
#include "aloha/grit/aloha_resources.h"
#include "aloha/browser/client/aloha_browser_client.h"
#include "base/base_paths.h"
#include "base/command_line.h"
#include "base/functional/bind.h"
#include "base/no_destructor.h"
#include "base/path_service.h"
#include "base/process/process_handle.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "content/public/app/content_main.h"
#include "content/public/browser/browser_context.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/models/image_model.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/resource/resource_scale_factor.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "url/url_util.h"

#if BUILDFLAG(IS_WIN)
#include <windows.h>

#include "content/public/app/sandbox_helper_win.h"
#include "sandbox/win/src/sandbox_types.h"
#endif  // BUILDFLAG(IS_WIN)

#if BUILDFLAG(IS_MAC)
#include "base/files/file_path.h"
#include "sandbox/mac/seatbelt_exec.h"
#endif  // BUILDFLAG(IS_MAC)

namespace {
// Called after: AlohaContentMainDelegate::PreSandboxStartup()
void OnResourcesLoaded() {
  base::FilePath aloha_pak_file;
  CHECK(base::PathService::Get(base::DIR_ASSETS, &aloha_pak_file));
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      aloha_pak_file.AppendASCII("aloha.pak"), ui::k100Percent);

  base::FilePath ui_resource_percent100;
  CHECK(base::PathService::Get(base::DIR_ASSETS, &aloha_pak_file));
  ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
      ui_resource_percent100.AppendASCII("ui_resources_100_percent.pak"),
      ui::k100Percent);
}
// View / Widget 是自动管理内存的，不需要手动释放，否则会触发 HEAP_CORRUPTION

void CreateAndShowMainWindow(aloha::AlohaBrowserClient* views_content_client,
                             content::BrowserContext* browser_context,
                             gfx::NativeWindow window_context) {
  // 窗口已存在
  if (aloha::MainWidgetDelegateView::instance()) {
    aloha::MainWidgetDelegateView::instance()->GetWidget()->Activate();
    return;
  }

  // 创建窗口
  views::Widget* aloha_main_widget = new views::Widget();
  views::Widget::InitParams params(
      views::Widget::InitParams::NATIVE_WIDGET_OWNS_WIDGET,
      views::Widget::InitParams::TYPE_WINDOW);
  aloha::SetDefaultBrowserContext(browser_context);
  params.delegate = new aloha::MainWidgetDelegateView();
  params.delegate->RegisterWindowClosingCallback(
      std::move(views_content_client->quit_closure()));

  params.context = window_context;
  params.name =
      base::UTF16ToUTF8(l10n_util::GetStringUTF16(IDS_ALOHA_WEBSHELL_TITLE));
  // 移除系统的默认样式，以添加我们自己的窗口样式
  // params.remove_standard_frame = true;

  aloha_main_widget->Init(std::move(params));
  aloha_main_widget->Show();
  // These lines serve no purpose other than to introduce an explicit content
  // dependency. If the main executable doesn't have this dependency, the linker
  // has more flexibility to reorder library dependencies in a shared component
  // build. On linux, this can cause libc to appear before libcontent in the
  // dlsym search path, which breaks (usually valid) assumptions made in
  // sandbox::InitLibcUrandomOverrides(). See http://crbug.com/374712.
  if (!browser_context) {
    browser_context->SaveSessionState();
    NOTREACHED();
  }
}
}  // namespace

#if BUILDFLAG(IS_WIN)
int wWinMain(HINSTANCE instance, HINSTANCE, wchar_t*, int) {
  base::CommandLine::Init(0, nullptr);

  sandbox::SandboxInterfaceInfo sandbox_info{};
  content::InitializeSandboxInfo(&sandbox_info);

  // 转为基于 View 构建界面
  if (base::CommandLine::ForCurrentProcess()->HasSwitch("use-webui")) {
    // 纯 web方式参考 ui\webui\examples\app\main.cc
    aloha::MainDelegate delegate;
    content::ContentMainParams params(&delegate);
    params.instance = instance;
    params.sandbox_info = &sandbox_info;
    return content::ContentMain(std::move(params));

  } else {
    // 实现方式参考 ui\views\examples\examples_with_content_main.cc
    aloha::AlohaBrowserClient aloha_views_content_client(instance,
                                                         &sandbox_info);
    // 加载 aloha资源

    aloha_views_content_client.set_on_resources_loaded_callback(
        base::BindOnce(&OnResourcesLoaded));
    // 设置预启动回调
    aloha_views_content_client.set_on_pre_main_message_loop_run_callback(
        base::BindOnce(&CreateAndShowMainWindow,
                       base::Unretained(&aloha_views_content_client)));
    // 启动消息循环
    return aloha_views_content_client.RunMain();
  }
}
#elif BUILDFLAG(IS_MAC)
int main(int argc, const char** argv) {
  base::CommandLine::Init(argc, argv);
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  sandbox::SeatbeltExecServer::CreateFromArgumentsResult seatbelt =
      sandbox::SeatbeltExecServer::CreateFromArguments(
          command_line->GetProgram().value().c_str(), argc,
          const_cast<char**>(argv));
  if (seatbelt.sandbox_required) {
    CHECK(seatbelt.server->InitializeSandbox());
  }

  aloha::MainDelegate delegate;
  content::ContentMainParams params(&delegate);
  return content::ContentMain(std::move(params));
}
#else
int main(int argc, const char** argv) {
  base::CommandLine::Init(argc, argv);

  aloha::MainDelegate delegate;
  content::ContentMainParams params(&delegate);
  return content::ContentMain(std::move(params));
}
#endif  // BUILDFLAG(IS_WIN)
