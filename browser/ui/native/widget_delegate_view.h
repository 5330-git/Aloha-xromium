#ifndef ALOHA_BROWSER_UI_NATIVE_WIDGET_DELEGATE_VIEW_H_
#define ALOHA_BROWSER_UI_NATIVE_WIDGET_DELEGATE_VIEW_H_
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "aloha/browser/devtools/devtools_frontend.h"
#include "aloha/grit/aloha_resources.h"
#include "aloha/views/controls/tabbed_pane/tabbed_pane.h"
#include "base/functional/bind.h"
#include "base/functional/callback_forward.h"
#include "base/location.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/sequenced_task_runner.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/clipboard_types.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "setting_menu_model.h"
#include "ui/base/clipboard/clipboard_non_backed.h"
#include "ui/base/clipboard/scoped_clipboard_writer.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/mojom/menu_source_type.mojom.h"
#include "ui/base/ui_base_types.h"
#include "ui/views/background.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/menu/menu_delegate.h"
#include "ui/views/controls/menu/menu_item_view.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/controls/menu/menu_types.h"
#include "ui/views/controls/scroll_view.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane_listener.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/box_layout_view.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "url/gurl.h"

namespace aloha {
class AlohaWebContentDelegate;
class AlohaWebAppView;

// 程序启动时会由ContentBrowserMainParts创建一个 BrowserContext，很多 WebView
// 都需要靠它维护上下文， 因此需要一个全局的管理 BrowserContext 的方案
// 另外这里的 BrowserContext 应该是一个单例。因为理论上 BrowserMainParts
// 只会执行一次 另外需要注意的时BrowserContext销毁前需要确保所有指向它的
// RenderProcessesHost 都已经销毁。(具体信息看BrowserContext的析构函数中的CHECK)
content::BrowserContext* GetDefaultBrowserContext();
void SetDefaultBrowserContext(content::BrowserContext* browser_context);

// 参考 ui\views\examples\examples_window.cc 实现
// 最顶层的 View 的展示相关
class AlohaWidgetDelegateView : public views::WidgetDelegateView {
 public:
  AlohaWidgetDelegateView();
  gfx::Size GetMinimumSize() const override;
  std::u16string GetWindowTitle() const override;
  ~AlohaWidgetDelegateView() override;
  // AlohaWidgetDelegateView
  virtual void AddBrowserContentView(
      std::string name,
      std::unique_ptr<views::View> content_view) = 0;
  virtual std::unique_ptr<views::View> MoveOutBrowserContentView(
      views::View* content_view) = 0;

  virtual std::unique_ptr<views::View> CloseBrowserContentView(
      views::View* content_view) = 0;

  AlohaWebContentDelegate* web_contents_delegate() const {
    return web_contents_delegate_.get();
  }

 protected:
  // TODO(yeyun.anton) 设计统一的 WebContentsDelegate
  // 存储方案，下面的方案是临时的
  std::unique_ptr<AlohaWebContentDelegate> web_contents_delegate_;
};

// Aloha 中用于展示独立View的窗口
class IndependentWidgetDelegateView : public AlohaWidgetDelegateView {
 public:
  IndependentWidgetDelegateView();
  gfx::Size GetMinimumSize() const override;
  ~IndependentWidgetDelegateView() override;
  std::u16string GetWindowTitle() const override;
  gfx::Size CalculatePreferredSize(
      const views::SizeBounds& /*available_size*/) const override;

  void AddBrowserContentView(
      std::string name,
      std::unique_ptr<views::View> content_view) override;
  std::unique_ptr<views::View> MoveOutBrowserContentView(
      views::View* content_view) override;
  std::unique_ptr<views::View> CloseBrowserContentView(
      views::View* content_view) override;

 protected:
  base::raw_ptr<views::View> content_view_;
  std::string name_;
};

// 参考 ui\views\examples\examples_window.cc 实现
// 希望以单例的形式展示 (临时方案)
// 最顶层的 View 的展示相关
class NativeWidgetDelegateView : public AlohaWidgetDelegateView,
                                 public views::TabbedPaneListener {
 public:
  NativeWidgetDelegateView();

  ~NativeWidgetDelegateView() override;

  // views::TabbedPaneListener
  void TabSelectedAt(int index) override;
  gfx::Size CalculatePreferredSize(
      const views::SizeBounds& /*available_size*/) const override;

  // AlohaWidgetDelegateView
  void AddBrowserContentView(
      std::string name,
      std::unique_ptr<views::View> content_view) override;

  std::unique_ptr<views::View> MoveOutBrowserContentView(
      views::View* content_view) override;

  std::unique_ptr<views::View> CloseBrowserContentView(
      views::View* content_view) override;

  // NativeWidgetDelegateView
  AlohaTabbedPane* GetTabbedPane() const { return tabbed_pane_; }

  // static
  // 注意在多个模块中定义可能导致全局变量的问题
  static NativeWidgetDelegateView* instance();

 private:
  base::raw_ptr<AlohaTabbedPane> tabbed_pane_ = nullptr;
  raw_ptr<views::Label> status_label_ = nullptr;

  // content_view --> tab_view
  std::unordered_map<views::View*, AlohaTabbedPaneTab*>
      views_in_independent_window_;
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

class DevtoolsView : public views::View, public content::WebContentsObserver {
 public:
  explicit DevtoolsView(const GURL& devtools_url,
                        AlohaWidgetDelegateView* root_view,
                        AlohaWidgetDelegateView* current_view);
  ~DevtoolsView() override;
  void ShowSettingsMenu() {
    LOG(INFO) << "Show Settings Menu";

    if (!menu_runner_) {
      setting_menu_model_ = std::make_unique<SettingMenuModel>();
      setting_menu_model_->RegisterCommandCallback(
          SettingMenuModel::Commands::COMMAND_INSPECT_THIS_PAGE,
          base::BindRepeating(&DevtoolsView::Inspect, base::Unretained(this)));

      menu_runner_ = std::make_unique<views::MenuRunner>(
          setting_menu_model_.get(), views::MenuRunner::HAS_MNEMONICS);
    }
    menu_runner_->RunMenuAt(settings_btn_->GetWidget(), nullptr,
                            gfx::Rect(settings_btn_->GetBoundsInScreen()),
                            views::MenuAnchorPosition::kBubbleBottomLeft,
                            ui::mojom::MenuSourceType::kNone);
  }

  void Inspect() {
    // TODO: 在右侧添加一个 WebView 用于显示 DevTools 界面
    LOG(INFO) << "Start Inspect of " << devtools_url_.spec();
    // if (devtools_frontend_) {
    //   LOG(INFO) << "DevTools already opened";
    //   CHECK(devtools_view_);
    //   return;
    // }
    // // 这个指针将和 WebAppView 生命周期一致
    // devtools_frontend_ =
    //     DevToolsFrontend::CreateAndGet(webview_->GetWebContents());
    // LOG(INFO) << "inspect:" << devtools_frontend_->frontend_url().spec();
    // auto devtools_view = std::make_unique<aloha::WebAppView>(
    //     devtools_frontend_->frontend_url(), root_widget_view_);
    // devtools_view_ = devtools_view.get();
    // devtools_frontend_->SetDevtoolsWebContents(devtools_view->web_contents());
    // // 新建窗口
    // root_widget_view_->AddBrowserContentView("Aloha devtools",
    //                                          std::move(devtools_view));
  }

  void CopyLink() {
    std::string link;
    if (webview_ && webview_->GetWebContents()) {
      link = webview_->GetWebContents()->GetURL().spec();
    } else {
      link = "Error: Invalid WebView";
    }
    // 创建用于写入剪切板的数据对象
    ui::ScopedClipboardWriter clipboard_writer(ui::ClipboardBuffer::kCopyPaste);

    clipboard_writer.WriteText(base::UTF8ToUTF16(link));
    LOG(INFO) << "Call Copy Link: " << link;
  }

 private:
  base::raw_ptr<views::WebView> webview_ = nullptr;
  base::raw_ptr<AlohaWidgetDelegateView> root_widget_view_ = nullptr;
  base::raw_ptr<AlohaWidgetDelegateView> current_widget_view_ = nullptr;
  std::unique_ptr<views::MenuRunner> menu_runner_;
  std::unique_ptr<SettingMenuModel> setting_menu_model_;
  views::MenuDelegate menu_delegate_;
  base::raw_ptr<views::ImageButton> settings_btn_ = nullptr;
  GURL devtools_url_;
  base::raw_ptr<DevToolsFrontend> devtools_frontend_ = nullptr;
  // TODO(yeyun.anton): 定制Devtools 的界面
  base::raw_ptr<views::View> devtools_view_ = nullptr;
};

// WebAppView: 即内置的 WebApp，通过 file:// 协议加载前端资源
class WebAppView : public views::View, public content::WebContentsObserver {
 public:
  WebAppView(const GURL& app_url, AlohaWidgetDelegateView* root_view);
  ~WebAppView() override;
  void OpenSelectedTabInNewWindow() {
    if (current_widget_view_ != root_widget_view_) {
      // 已经在独立窗口中打开了
      return;
    }
    std::unique_ptr<views::View> content_view =
        root_widget_view_->MoveOutBrowserContentView(this);
    views::Widget* independent_widget = new views::Widget();
    views::Widget::InitParams params(
        views::Widget::InitParams::NATIVE_WIDGET_OWNS_WIDGET,
        views::Widget::InitParams::TYPE_WINDOW);
    params.name = "Test";
    current_widget_view_ = new IndependentWidgetDelegateView();
    params.delegate = current_widget_view_;

    independent_widget->Init(std::move(params));
    // 为 root_widget_view_ 注册回调，确保其关闭前独立窗口被关闭
    root_widget_view_->RegisterWindowWillCloseCallback(
        base::BindOnce(&views::Widget::Close,
                       current_widget_view_->GetWidget()->GetWeakPtr()));

    current_widget_view_->RegisterWindowWillCloseCallback(base::BindOnce(
        &WebAppView::MoveWindowContentBackToTab, base::Unretained(this)));
    current_widget_view_->AddBrowserContentView(params.name,
                                                std::move(content_view));

    independent_widget->Show();
    open_view_in_new_window_btn_->SetEnabled(false);
  }

  void MoveWindowContentBackToTab() {
    LOG(INFO) << "MoveWindowContentBackToTab";
    std::unique_ptr<views::View> self =
        current_widget_view_->MoveOutBrowserContentView(this);
    root_widget_view_->AddBrowserContentView("", std::move(self));
    current_widget_view_ = root_widget_view_;
    open_view_in_new_window_btn_->SetEnabled(true);
  }

  void DidStartNavigation(
      content::NavigationHandle* navigation_handle) override;
  void ReloadWebView() {
    if (webview_ && webview_->GetWebContents()) {
      webview_->GetWebContents()->GetController().Reload(
          content::ReloadType::NORMAL, false);
    }
  }
  void GoBackPage() {
    LOG(INFO) << "Call GoBackPage";
    auto& navigation_controller = webview_->GetWebContents()->GetController();
    if (navigation_controller.CanGoBack()) {
      navigation_controller.GoBack();
    }
  }

  void GoForwardPage() {
    LOG(INFO) << "Call GoForwardPage";
    auto& navigation_controller = webview_->GetWebContents()->GetController();
    if (navigation_controller.CanGoForward()) {
      navigation_controller.GoForward();
    }
  }
  void ShowSettingsMenu() {
    LOG(INFO) << "Show Settings Menu";

    if (!menu_runner_) {
      setting_menu_model_ = std::make_unique<SettingMenuModel>();
      setting_menu_model_->RegisterCommandCallback(
          SettingMenuModel::Commands::COMMAND_INSPECT_THIS_PAGE,
          base::BindRepeating(&WebAppView::Inspect, base::Unretained(this)));

      menu_runner_ = std::make_unique<views::MenuRunner>(
          setting_menu_model_.get(), views::MenuRunner::HAS_MNEMONICS);
    }
    menu_runner_->RunMenuAt(settings_btn_->GetWidget(), nullptr,
                            gfx::Rect(settings_btn_->GetBoundsInScreen()),
                            views::MenuAnchorPosition::kBubbleBottomLeft,
                            ui::mojom::MenuSourceType::kNone);
  }
  void GoBackHome() {
    if (webview_ && webview_->GetWebContents()) {
      webview_->LoadInitialURL(GURL(kAlohaHomeURL));
    }
  }

  void CopyLink() {
    std::string link;
    if (webview_ && webview_->GetWebContents()) {
      link = webview_->GetWebContents()->GetURL().spec();
    } else {
      link = "Error: Invalid WebView";
    }
    // 创建用于写入剪切板的数据对象
    ui::ScopedClipboardWriter clipboard_writer(ui::ClipboardBuffer::kCopyPaste);

    clipboard_writer.WriteText(base::UTF8ToUTF16(link));
    LOG(INFO) << "Call Copy Link: " << link;
  }

  // 启动 DevTools 进行 Inspect
  void Inspect() {
    // TODO: 在右侧添加一个 WebView 用于显示 DevTools 界面
    LOG(INFO) << "Start Inspect of " << app_url_.spec();
    if (devtools_view_) {
      LOG(INFO) << "DevTools view already opened";
      return;
    }

    if (!devtools_frontend_) {
      // 这个指针将和 WebAppView 生命周期一致
      devtools_frontend_ =
          DevToolsFrontend::CreateAndGet(webview_->GetWebContents());
      LOG(INFO) << "inspect:" << devtools_frontend_->frontend_url().spec();
    }

    auto* devtools_widget = new views::Widget();
    views::Widget::InitParams params(
        views::Widget::InitParams::NATIVE_WIDGET_OWNS_WIDGET,
        views::Widget::InitParams::TYPE_WINDOW);
    auto* delegate_view = new IndependentWidgetDelegateView();
    params.delegate = delegate_view;

    auto devtools_view = std::make_unique<aloha::DevtoolsView>(
        devtools_frontend_->frontend_url(), root_widget_view_, delegate_view);
    devtools_view_ = devtools_view.get();
    devtools_frontend_->SetDevtoolsWebContents(devtools_view->web_contents());
    // 新建窗口
    delegate_view->AddBrowserContentView("Aloha devtools",
                                         std::move(devtools_view));

    devtools_widget->Init(std::move(params));
    delegate_view
        ->RegisterWindowWillCloseCallback(base::BindOnce(
            [](WebAppView* self) {
              // 窗口关闭，清空 devtools_frontend_
              self->devtools_view_ = nullptr;
            },
            base::Unretained(this)));
        // 为 root_widget_view_ 注册回调，确保其关闭前独立窗口被关闭
        root_widget_view_->RegisterWindowWillCloseCallback(base::BindOnce(
            &views::Widget::Close, delegate_view->GetWidget()->GetWeakPtr()));
    devtools_widget->Show();
  }

  static constexpr char kAlohaHomeURL[] =
      "file:///D:/codes/build-chromium/chromium/src/aloha/resources/browser/"
      "aloha-app-main/dist/index.html";

 private:
  GURL app_url_;
  base::raw_ptr<AlohaWidgetDelegateView> root_widget_view_ = nullptr;
  base::raw_ptr<AlohaWidgetDelegateView> current_widget_view_ = nullptr;
  base::raw_ptr<views::Label> label_;
  // WebView 作为ChildView 被添加到 this 中，因此这里可以直接使用 raw_ptr 持有
  base::raw_ptr<views::WebView> webview_ = nullptr;
  base::raw_ptr<views::View> top_bar_view_ = nullptr;
  base::raw_ptr<views::ImageButton> open_view_in_new_window_btn_ = nullptr;
  base::raw_ptr<views::ImageButton> settings_btn_ = nullptr;
  base::raw_ptr<DevToolsFrontend> devtools_frontend_ = nullptr;
  // TODO(yeyun.anton): 定制Devtools 的界面
  base::raw_ptr<views::View> devtools_view_ = nullptr;

  std::unique_ptr<views::MenuRunner> menu_runner_;
  std::unique_ptr<SettingMenuModel> setting_menu_model_;
  views::MenuDelegate menu_delegate_;
};

// WebSiteView: 即第三方的 Web 页面，通过 http:// 或者 https:// 协议加载前端资源
class WebSiteView : public views::View {
 public:
  WebSiteView(const GURL& app_url, AlohaWidgetDelegateView* root_view);
  void ReloadWebView() {
    if (webview_ && webview_->GetWebContents()) {
      webview_->GetWebContents()->GetController().Reload(
          content::ReloadType::NORMAL, false);
    }
  }

  void CloseWebView() {
    LOG(INFO) << "Call Close WebView";
    root_widget_view_->CloseBrowserContentView(this);
  }

 private:
  GURL app_url_;
  base::raw_ptr<AlohaWidgetDelegateView> root_widget_view_;
  // WebView 作为ChildView 被添加到 this 中，因此这里可以直接使用 raw_ptr 持有
  base::raw_ptr<views::WebView> webview_ = nullptr;
  base::raw_ptr<views::View> top_bar_view_ = nullptr;
};

}  // namespace aloha
#endif
