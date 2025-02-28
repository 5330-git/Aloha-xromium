// #include "aloha/browser/ui/native/aloha_web_contents_view_delegate_views.h"
#include "aloha/browser/ui/views/aloha_web_contents_view_delegate_views.h"

#include <string>

#include "aloha/browser/devtools/devtools_frontend.h"
#include "aloha_web_contents_view_delegate_views.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/context_menu_params.h"
#include "content/public/browser/web_contents.h"

// #include
// "chrome/browser/ui/views/renderer_context_menu/render_view_context_menu_views.h"

namespace aloha {
// 临时使用，创建devtools
namespace {
// content::WebContents* CreateDevToolsWindow(
//     GURL url,
//     const std::u16string& title,
//     content::BrowserContext* browser_context) {}

}  // namespace

AlohaWebContentsViewDelegate::AlohaWebContentsViewDelegate(
    content::WebContents* web_contents)
    : web_contents_(web_contents) {}

AlohaWebContentsViewDelegate::~AlohaWebContentsViewDelegate() {}

void AlohaWebContentsViewDelegate::ShowContextMenu(
    content::RenderFrameHost& render_frame_host,
    const content::ContextMenuParams& params) {
//   LOG(INFO) << "ShowContextMenu";
//   LOG(INFO) << "web_contents_ :" << web_contents_->GetBrowserContext();
//   LOG(INFO) << params.page_url;
//   LOG(INFO) << params.frame_url;
//   // ShowMenu(BuildMenu(render_frame_host, params));
//   DevToolsFrontend* frontend = DevToolsFrontend::CreateAndGet(web_contents_);
//   LOG(INFO) << "inspect:" << frontend->frontend_url().spec();
//   frontend->SetDevtoolsWebContents(
//       create_window_func_.Run(frontend->frontend_url()));
}

// std::unique_ptr<RenderViewContextMenuBase>
// AlohaWebContentsViewDelegate::BuildMenu(
//     content::RenderFrameHost& render_frame_host,
//     const content::ContextMenuParams& params) {
//   // std::unique_ptr<RenderViewContextMenuBase> menu(
//   //     RenderViewContextMenuViews::Create(render_frame_host, params));
//   // menu->Init();
//   // return menu;
//   return nullptr;
// }
// void AlohaWebContentsViewDelegate::ShowMenu(
// std::unique_ptr<RenderViewContextMenuBase> menu) {}
}  // namespace aloha
