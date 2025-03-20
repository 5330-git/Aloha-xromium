#ifndef _ALOHA_BROWSER_UI_VIEWS_ALOHA_BROWSER_CONTENT_VIEW_H_
#define _ALOHA_BROWSER_UI_VIEWS_ALOHA_BROWSER_CONTENT_VIEW_H_

#include <memory>

#include "aloha/browser/ui/views/browser_content/public/browser_content_view.h"
#include "aloha/browser/ui/views/widget/widget_delegate_view.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "ui/base/models/image_model.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/box_layout_view.h"
#include "ui/views/widget/widget.h"

namespace aloha {

class AlohaBrowserContentView : public BrowserContentView {
 public:
  static constexpr char kURLInterceotedByDemoURLLoaderRequestInterceptor[] =
      "aloha-demo://demo_url_loader_request_interceptor";
  static constexpr char kURLInterceotedByDemoNavigationURLLoaderFactory[] =
      "aloha-demo://demo_navigation_url_loader_factory";
  static constexpr char kURLInterceotedByDemoSubResourcesURLLoaderFactory[] =
      "aloha-demo://demo_sub_resources_url_loader_factory";
  static constexpr char kAlohaHome[] = "aloha-app-main";
  static constexpr char kAlohaAppMainURL[] =
      "aloha://apps/aloha-app-main/index.html";
  struct SubViews {
    base::raw_ptr<views::WebView> webview = nullptr;
    base::raw_ptr<views::BoxLayoutView> top_bar_container = nullptr;
    base::raw_ptr<views::BoxLayoutView> top_bar_left_view = nullptr;
    base::raw_ptr<views::BoxLayoutView> top_bar_middle_view = nullptr;
    base::raw_ptr<views::BoxLayoutView> top_bar_right_view = nullptr;
  };

  AlohaBrowserContentView(const GURL& init_url,
                          AlohaWidgetDelegateView* origin_widget_delegate_view);
  ~AlohaBrowserContentView() override;

  // BrowserContentView
  void Init() override;
  void ReloadContents() override;
  void NavigateForward() override;
  void NavigateBack() override;
  void NavigateHome() override;
  void CopyLink() override;
  void OpenDevtoolsAndInspect() override;
  void ShowSettingsMenu() override;
  void Close() override;
  void SetCanOpenInNewWidget(bool can_open_in_new_tab) override;
  bool CanOpenInNewWidget() override;
  void AddChildWidget(base::WeakPtr<views::Widget> widget) override;
  void CloseChildWidgets() override;
  ui::ImageModel GetFavicon() override;

  // WebContentsObserver
  void DidUpdateFaviconURL(
      content::RenderFrameHost* render_frame_host,
      const std::vector<blink::mojom::FaviconURLPtr>& candidates) override;

 protected:
  // 用于定义整个Views各个部分视图的容器
  std::unique_ptr<views::View> GetContentContainerView() override;

  base::raw_ptr<AlohaWidgetDelegateView> origin_widget_delegate_view_;

  bool can_open_in_new_tab_ = false;
  std::vector<base::WeakPtr<views::Widget>> child_widgets_;

  base::raw_ptr<views::Button> open_view_in_new_window_btn_;
  base::raw_ptr<views::Button> settings_btn_;

  std::unique_ptr<views::MenuRunner> menu_runner_;
  std::unique_ptr<SettingMenuModel> setting_menu_model_;

  base::raw_ptr<DevToolsFrontend> devtools_frontend_ = nullptr;
  // TODO(yeyun.anton): 定制Devtools 的界面
  base::raw_ptr<BrowserContentView> devtools_view_ = nullptr;
  SubViews sub_views_;
  const GURL init_url_;
};

// 未来这个类将专门用于加载自定义协议的内容
class WebAppContentView : public AlohaBrowserContentView {
 public:
  WebAppContentView(const GURL& init_url,
                    AlohaWidgetDelegateView* origin_widget_delegate_view);
  ~WebAppContentView() override;
  void Init() override;
  // void OpenDevtoolsAndInspect() override;
};

// 未来这个类将专门用于加载网站内容
class WebSiteContentView : public AlohaBrowserContentView {
 public:
  WebSiteContentView(const GURL& init_url,
                     AlohaWidgetDelegateView* origin_widget_delegate_view);
  ~WebSiteContentView() override;
  void Init() override;
};

// TODO(yeyun.anton): 迁移到单独的文件中
class AlohaWebContentDelegate : public content::WebContentsDelegate {
 public:
  explicit AlohaWebContentDelegate(content::BrowserContext* browser_context,
                                   AlohaWidgetDelegateView* delegate)
      : browser_context_(browser_context), delegate_(delegate) {}
  bool HandleKeyboardEvent(content::WebContents* source,
                           const input::NativeWebKeyboardEvent& event) override;

  // 新建标签页时会调用这个接口
  void WebContentsCreated(content::WebContents* source_contents,
                          int opener_render_process_id,
                          int opener_render_frame_id,
                          const std::string& frame_name,
                          const GURL& target_url,
                          content::WebContents* new_contents) override;
  void CloseContents(content::WebContents* source) override;
  void UpdateTargetURL(content::WebContents* source, const GURL& url) override;

 private:
  base::raw_ptr<content::BrowserContext> browser_context_;
  // AlohaWebContentDelegate 由 AlohaWidgetDelegateView 持有，因此这里可以使用
  // raw_ptr
  base::raw_ptr<AlohaWidgetDelegateView> delegate_;
};
}  // namespace aloha

#endif
