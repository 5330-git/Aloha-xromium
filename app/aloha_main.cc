#include "aloha/app/aloha_main.h"

#include "aloha/browser/ui/views/widget/widget_delegate_view.h"
#include "aloha/common/aloha_main_client.h"
#include "aloha_content_main_delegate.h"
#include "base/base_paths.h"
#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "base/path_service.h"
#include "ui/base/resource/resource_bundle.h"

namespace {
// Called after: AlohaContentMainDelegate::PreSandboxStartup()
void OnResourcesLoaded() {
  // base::FilePath aloha_pak_file;
  // CHECK(base::PathService::Get(base::DIR_ASSETS, &aloha_pak_file));
  // ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
  //     aloha_pak_file.AppendASCII("aloha.pak"), ui::k100Percent);

  // base::FilePath ui_resource_percent100;
  // CHECK(base::PathService::Get(base::DIR_ASSETS, &aloha_pak_file));
  // ui::ResourceBundle::GetSharedInstance().AddDataPackFromPath(
  //     ui_resource_percent100.AppendASCII("ui_resources_100_percent.pak"),
  //     ui::k100Percent);
}
// View / Widget 是自动管理内存的，不需要手动释放，否则会触发 HEAP_CORRUPTION

void CreateAndShowMainWindow(aloha::AlohaMainClient* views_content_client,
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

namespace aloha {
int AlohaMain(HINSTANCE instance, sandbox::SandboxInterfaceInfo* sandbox_info) {
  aloha::AlohaMainClient::GetInstance()->set_on_resources_loaded_callback(
      base::BindOnce(&OnResourcesLoaded));
  aloha::AlohaMainClient::GetInstance()
      ->set_on_pre_main_message_loop_run_callback(base::BindOnce(
          &CreateAndShowMainWindow,
          base::Unretained(aloha::AlohaMainClient::GetInstance())));
  aloha::AlohaMainClient::GetInstance()->PreAlohaMain();

  AlohaContentMainDelegate delegate;
  content::ContentMainParams params(&delegate);
  params.instance = instance;
  params.sandbox_info = sandbox_info;
  params.minimal_browser_mode = false;

  return content::ContentMain(std::move(params));
}
}  // namespace aloha
