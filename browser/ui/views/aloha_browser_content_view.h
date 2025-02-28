#ifndef _ALOHA_BROWSER_UI_VIEWS_ALOHA_BROWSER_CONTENT_VIEW_H_
#define _ALOHA_BROWSER_UI_VIEWS_ALOHA_BROWSER_CONTENT_VIEW_H_ 

#include <memory>

#include "aloha/browser/ui/views/widget/widget_delegate_view.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/box_layout_view.h"

namespace aloha {

class AlohaBrowserContentView : public views::View,
                                public content::WebContentsObserver {
 public:
  static constexpr char kAlohaHomeURL[] =
      "file:///D:/codes/build-chromium/chromium/src/aloha/resources/browser/"
      "aloha-app-main/dist/index.html";
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

  // AlohaBrowserContentView
  virtual void Init();
  // virtual void PaintTopBarView();

  virtual void ShowInIndependentWindow();
  virtual void MoveBackToTabFromIndependentWindow();
  virtual void ReloadContents();
  virtual void NavigateForward();
  virtual void NavigateBack();
  virtual void NavigateHome();
  virtual void CopyLink();
  virtual void OpenDevtoolsAndInspect();
  virtual void ShowSettingsMenu();
  virtual void Close();


 protected:
  // 用于定义整个Views各个部分视图的容器
  virtual std::unique_ptr<views::View> GetContentContainerView();

  // AlohaBrowserContentView 的生命周期由 AlohaWidgetDelegateView
  // 持有，因此这里可以使用 raw_ptr AlohaBrowserContentView
  // 可能在多个窗口（Widget）中使用
  base::raw_ptr<AlohaWidgetDelegateView> owned_widget_delegate_view_;
  base::raw_ptr<AlohaWidgetDelegateView> origin_widget_delegate_view_;

  base::raw_ptr<views::Button> open_view_in_new_window_btn_;
  base::raw_ptr<views::Button> settings_btn_;

  std::unique_ptr<views::MenuRunner> menu_runner_;
  std::unique_ptr<SettingMenuModel> setting_menu_model_;

  base::raw_ptr<DevToolsFrontend> devtools_frontend_ = nullptr;
  // TODO(yeyun.anton): 定制Devtools 的界面
  base::raw_ptr<views::View> devtools_view_ = nullptr;
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
