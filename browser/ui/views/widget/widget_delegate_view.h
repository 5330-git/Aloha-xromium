#ifndef ALOHA_BROWSER_UI_VIEWS_WIDGET_WIDGET_DELEGATE_VIEW_H_
#define ALOHA_BROWSER_UI_VIEWS_WIDGET_WIDGET_DELEGATE_VIEW_H_
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "aloha/browser/devtools/devtools_frontend.h"
#include "aloha/browser/ui/menu/setting_menu_model.h"
#include "aloha/browser/ui/views/browser_content/public/browser_content_view.h"
#include "aloha/browser/ui/views/controls/tabbed_pane/tabbed_pane.h"
#include "aloha/grit/aloha_resources.h"
#include "base/functional/bind.h"
#include "base/functional/callback_forward.h"
#include "base/location.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/sequenced_task_runner.h"
#include "base/win/windows_types.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/clipboard_types.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
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

// TODO(yeyun.anton): 将 AlohaWidgetDelegateView 移动到 public 下
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
      std::unique_ptr<aloha::BrowserContentView> content_view) = 0;
  virtual std::unique_ptr<aloha::BrowserContentView> MoveOutBrowserContentView(
      aloha::BrowserContentView* content_view) = 0;

  virtual void CloseBrowserContentView(
      aloha::BrowserContentView* content_view) = 0;
  virtual void ShowBrowserContentViewInNewWindow(
      aloha::BrowserContentView* browser_content_view,
      base::RepeatingCallback<AlohaWidgetDelegateView*(void)>
          widget_delegate_view_creator);

  AlohaWebContentDelegate* web_contents_delegate() const {
    return web_contents_delegate_.get();
  }

 protected:
  // TODO(yeyun.anton) 设计统一的 WebContentsDelegate
  // 存储方案，下面的方案是临时的
  std::unique_ptr<AlohaWebContentDelegate> web_contents_delegate_;
};

// Aloha 中用于展示独立View的窗口
class SimpleWidgetDelegateView : public AlohaWidgetDelegateView {
 public:
  SimpleWidgetDelegateView();
  static SimpleWidgetDelegateView* Create();
  gfx::Size GetMinimumSize() const override;
  ~SimpleWidgetDelegateView() override;
  std::u16string GetWindowTitle() const override;
  gfx::Size CalculatePreferredSize(
      const views::SizeBounds& /*available_size*/) const override;

  void AddBrowserContentView(
      std::string name,
      std::unique_ptr<aloha::BrowserContentView> content_view) override;
  std::unique_ptr<aloha::BrowserContentView> MoveOutBrowserContentView(
      aloha::BrowserContentView* content_view) override;
  void CloseBrowserContentView(
      aloha::BrowserContentView* content_view) override;

 protected:
  base::raw_ptr<aloha::BrowserContentView> content_view_;
  std::string name_;
};

// 参考 ui\views\examples\examples_window.cc 实现
// 希望以单例的形式展示 (临时方案)
// 最顶层的 View 的展示相关
class MainWidgetDelegateView : public AlohaWidgetDelegateView,
                               public views::TabbedPaneListener {
 public:
  struct BrowserContentViewInfo {
    // 独立窗口的widget
    std::unique_ptr<views::Widget> independent_window = nullptr;
    // 指向主窗口的 TabView
    base::raw_ptr<AlohaTabbedPaneTab> related_tab = nullptr;
    BrowserContentViewInfo();
    ~BrowserContentViewInfo();
    BrowserContentViewInfo(BrowserContentViewInfo&& other);
    BrowserContentViewInfo(const BrowserContentViewInfo& other) = delete;
    BrowserContentViewInfo& operator=(const BrowserContentViewInfo& other) =
        delete;
    BrowserContentViewInfo& operator=(BrowserContentViewInfo&& other) = default;
  };

  MainWidgetDelegateView();
  static MainWidgetDelegateView* Create();

  ~MainWidgetDelegateView() override;

  // views::TabbedPaneListener
  void TabSelectedAt(int index) override;
  gfx::Size CalculatePreferredSize(
      const views::SizeBounds& /*available_size*/) const override;

  // AlohaWidgetDelegateView
  void AddBrowserContentView(
      std::string name,
      std::unique_ptr<aloha::BrowserContentView> content_view) override;

  std::unique_ptr<aloha::BrowserContentView> MoveOutBrowserContentView(
      aloha::BrowserContentView* content_view) override;

  void CloseBrowserContentView(
      aloha::BrowserContentView* content_view) override;

  // MainWidgetDelegateView
  AlohaTabbedPane* GetTabbedPane() const { return tabbed_pane_; }

  void ShowBrowserContentViewInNewWindow(
      aloha::BrowserContentView* browser_content_view,
      base::RepeatingCallback<AlohaWidgetDelegateView*(void)>
          widget_delegate_view_creator) override;

  // static
  // 注意在多个模块中定义可能导致全局变量的问题
  static MainWidgetDelegateView* instance();

 private:
  base::raw_ptr<AlohaTabbedPane> tabbed_pane_ = nullptr;
  raw_ptr<views::Label> status_label_ = nullptr;

  // content_view --> tab_view
  std::unordered_map<views::View*, AlohaTabbedPaneTab*>
      views_in_independent_window_;

  std::unordered_map<views::View*, BrowserContentViewInfo>
      browser_content_views;
};

}  // namespace aloha
#endif
