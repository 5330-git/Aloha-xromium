#include "aloha/browser/ui/views/widget/widget_delegate_view.h"

#include <memory>
#include <string>
#include <utility>

#include "aloha/browser/ui/color/color_ids.h"
#include "aloha/browser/ui/views/browser_content/aloha_browser_content_view.h"
#include "aloha/browser/ui/views/controls/tabbed_pane/tabbed_pane.h"
#include "aloha/browser/ui/views/controls/tabbed_pane/tabbed_pane_tab.h"
#include "aloha/browser/ui/views/controls/tabbed_pane/tabbed_pane_tab_strip.h"
#include "aloha/common/aloha_paths.h"
#include "aloha/grit/aloha_resources.h"
#include "aloha/resources/vector_icons/vector_icons.h"
#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "base/functional/callback_forward.h"
#include "base/location.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/sequenced_task_runner.h"
#include "components/input/native_web_keyboard_event.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/web_contents_delegate.h"
#include "ui/color/color_id.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/gfx/text_constants.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/bubble/bubble_border.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/button/md_text_button.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/message_box_view.h"
#include "ui/views/controls/textarea/textarea.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/box_layout_view.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/layout_types.h"
#include "ui/views/metadata/view_factory_internal.h"
#include "ui/views/vector_icons.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "url/gurl.h"
#include "widget_delegate_view.h"

namespace aloha {
// namespace {
base::raw_ptr<content::BrowserContext> default_browser_context_ = nullptr;
base::raw_ptr<MainWidgetDelegateView> g_instance_ = nullptr;
// }  // namespace

void SetDefaultBrowserContext(content::BrowserContext* browser_context) {
  // 应当是单例，因此做一下检查
  DCHECK(!default_browser_context_);
  default_browser_context_ = browser_context;
}

content::BrowserContext* GetDefaultBrowserContext() {
  return default_browser_context_;
}

AlohaWidgetDelegateView::AlohaWidgetDelegateView() {
  SetAppIcon(ui::ImageModel::FromResourceId(IDR_ALOHA_ICON));
  SetIcon(ui::ImageModel::FromResourceId(IDR_ALOHA_ICON));
}

gfx::Size AlohaWidgetDelegateView::GetMinimumSize() const {
  return gfx::Size(100, 100);
}

std::u16string AlohaWidgetDelegateView::GetWindowTitle() const {
  return l10n_util::GetStringUTF16(IDS_ALOHA_WINDOW_TITLE);
}

void AlohaWidgetDelegateView::ShowBrowserContentViewInNewWindow(
    aloha::BrowserContentView* /*browser_content_view*/,
    base::RepeatingCallback<AlohaWidgetDelegateView*(void)> /*creator*/) {}

AlohaWidgetDelegateView::~AlohaWidgetDelegateView() {}

SimpleWidgetDelegateView* SimpleWidgetDelegateView::Create() {
  return new SimpleWidgetDelegateView();
}

SimpleWidgetDelegateView::SimpleWidgetDelegateView()
    : name_("Aloha Indepentent Window") {
  SetHasWindowSizeControls(true);
  SetLayoutManager(std::make_unique<views::FillLayout>());
}

SimpleWidgetDelegateView::~SimpleWidgetDelegateView() {}

void SimpleWidgetDelegateView::AddBrowserContentView(
    std::string name,
    std::unique_ptr<aloha::BrowserContentView> content_view) {
  name_ = name;
  SetTitle(base::UTF8ToUTF16(name));
  content_view_ = AddChildView(std::move(content_view));
}

std::unique_ptr<aloha::BrowserContentView>
SimpleWidgetDelegateView::MoveOutBrowserContentView(
    aloha::BrowserContentView* content_view) {
  if (!content_view_) {
    return nullptr;
  }
  CHECK(content_view == content_view_);
  content_view_ = nullptr;
  return RemoveChildViewT(content_view);
}

void SimpleWidgetDelegateView::CloseBrowserContentView(
    aloha::BrowserContentView* content_view) {
  MoveOutBrowserContentView(content_view);
}

std::u16string SimpleWidgetDelegateView::GetWindowTitle() const {
  return base::UTF8ToUTF16(name_);
}

gfx::Size SimpleWidgetDelegateView::GetMinimumSize() const {
  return gfx::Size(1000, 700);
}

gfx::Size SimpleWidgetDelegateView::CalculatePreferredSize(
    const views::SizeBounds& /*available_size*/) const {
  gfx::Size size(1000, 800);
  return size;
}

MainWidgetDelegateView* MainWidgetDelegateView::Create() {
  return new MainWidgetDelegateView();
}

MainWidgetDelegateView::BrowserContentViewInfo::BrowserContentViewInfo() =
    default;

MainWidgetDelegateView::BrowserContentViewInfo::~BrowserContentViewInfo() =
    default;

MainWidgetDelegateView::BrowserContentViewInfo::BrowserContentViewInfo(
    BrowserContentViewInfo&& other) {
  independent_window = std::move(other.independent_window);
  related_tab = other.related_tab;
  other.related_tab = nullptr;
}

MainWidgetDelegateView* MainWidgetDelegateView::instance() {
  return g_instance_;
}

MainWidgetDelegateView::MainWidgetDelegateView() {
  web_contents_delegate_ = std::make_unique<AlohaWebContentDelegate>(
      aloha::GetDefaultBrowserContext(), this);

  g_instance_ = this;
  SetHasWindowSizeControls(true);
  SetBackground(views::CreateThemedSolidBackground(
      theme::light::kColorLightBrowserBackground));

  auto* layout = SetLayoutManager(std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kVertical, gfx::Insets(0)));
  auto tabbed_pane = std::make_unique<AlohaTabbedPane>(
      AlohaTabbedPane::Orientation::kVertical,
      AlohaTabbedPane::TabStripStyle::kHighlight, true);

  tabbed_pane_ = AddChildView(std::move(tabbed_pane));

  tabbed_pane_->set_listener(this);
  layout->SetFlexForView(tabbed_pane_, 1);

  // 添加 tab 以测试
  base::FilePath aloha_home_path;
  path_service::GetWebAppPath(&aloha_home_path,
                              AlohaBrowserContentView::kAlohaHome);
  auto webapp_view_with_file_scheme = std::make_unique<WebAppContentView>(
      GURL("file://" + aloha_home_path.MaybeAsASCII()), this);
  webapp_view_with_file_scheme->Init();
  AddBrowserContentView("Aloha Main" + std::string("with file scheme"),
                        std::move(webapp_view_with_file_scheme));

  auto webapp_view_with_url_disposed_by_interceptor =
      std::make_unique<WebAppContentView>(
          GURL(AlohaBrowserContentView::
                   kURLInterceotedByDemoURLLoaderRequestInterceptor),
          this);
  webapp_view_with_url_disposed_by_interceptor->Init();
  AddBrowserContentView(
      "Interceptor Demo",
      std::move(webapp_view_with_url_disposed_by_interceptor));
  auto webapp_view_with_url_disposed_by_navigation_url_loader =
      std::make_unique<WebAppContentView>(
          GURL(AlohaBrowserContentView::
                   kURLInterceotedByDemoNavigationURLLoaderFactory),
          this);
  webapp_view_with_url_disposed_by_navigation_url_loader->Init();
  AddBrowserContentView(
      "URL Navigation Factory Demo" ,
      std::move(webapp_view_with_url_disposed_by_navigation_url_loader));
}

void MainWidgetDelegateView::AddBrowserContentView(
    std::string name,
    std::unique_ptr<aloha::BrowserContentView> content_view) {
  // 检查是否是被移出去的窗口
  auto tab_view_iter = views_in_independent_window_.find(content_view.get());
  if (tab_view_iter != views_in_independent_window_.end()) {
    auto tab_view = tab_view_iter->second;
    auto* empty_view = tab_view->contents();
    tabbed_pane_->GetContentsContainer()->RemoveChildViewT(empty_view);
    tabbed_pane_->GetContentsContainer()->AddChildView(std::move(content_view));

    tab_view->SetContents(tab_view_iter->first);
    return;
  }
  content_view->SetCanOpenInNewWidget(true);
  tabbed_pane_->AddTab(base::UTF8ToUTF16(name), std::move(content_view));
  size_t count = tabbed_pane_->GetTabCount();
  tabbed_pane_->SelectTabAt(count - 1);

  BrowserContentViewInfo info;
  info.independent_window = nullptr;
  info.related_tab = tabbed_pane_->GetTabAt(count - 1);
  browser_content_views.emplace(info.related_tab->contents(), std::move(info));
}

std::unique_ptr<aloha::BrowserContentView>
MainWidgetDelegateView::MoveOutBrowserContentView(
    aloha::BrowserContentView* content_view) {
  if (content_view) {
    // 找到对应的 Tab
    for (size_t i = 0; i < tabbed_pane_->GetTabCount(); i++) {
      auto* tab = tabbed_pane_->GetTabAt(i);
      if (tab->contents() == content_view) {
        // TODO(yeyun.anton):
        // 为被移除的区域添加内容，未来通过点击定位到被移除且独立出来的窗口上 将
        // content view 移除掉，并存储到容器中，维护一个 content 到 tab 的映射
        auto tab_view = tabbed_pane_->GetContentsContainer()->RemoveChildViewT(
            content_view);
        tab->SetContents(tabbed_pane_->GetContentsContainer()->AddChildView(
            std::make_unique<views::MessageBoxView>()));

        views_in_independent_window_.emplace(content_view, tab);
        return tab_view;
      }
    }
  }

  return nullptr;
}

void MainWidgetDelegateView::CloseBrowserContentView(
    aloha::BrowserContentView* content_view) {
  auto iter = browser_content_views.find(content_view);
  CHECK(iter != browser_content_views.end());
  // 如果时独立窗口，在此处会随着 widget
  // 的销毁而销毁，如果在主窗口中则在后面移除Tab 时销毁
  AlohaTabbedPaneTab* tab_view = iter->second.related_tab;
  browser_content_views.erase(iter);
  tabbed_pane_->RemoveTab(tab_view);
}

void MainWidgetDelegateView::ShowBrowserContentViewInNewWindow(
    aloha::BrowserContentView* browser_content_view,
    base::RepeatingCallback<AlohaWidgetDelegateView*(void)>
        widget_delegate_view_creator) {
  // 查找对应的tab
  auto browser_content_view_ownership =
      MoveOutBrowserContentView(browser_content_view);
  if (!browser_content_view_ownership) {
    return;
  }
  auto widget_delegate_view = widget_delegate_view_creator.Run();
  widget_delegate_view->AddBrowserContentView(
      "Aloha", std::move(browser_content_view_ownership));
  browser_content_view->SetCanOpenInNewWidget(false);

  // 持有独立窗口的 widget
  auto* child_widget = new views::Widget();

  views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW);
  params.ownership = views::Widget::InitParams::NATIVE_WIDGET_OWNS_WIDGET;
  params.delegate = widget_delegate_view;

  child_widget->Init(std::move(params));
  child_widget->Show();

  // 为独立窗口注册回调，让其关闭时归还 BrowserContentView
  widget_delegate_view->RegisterWindowWillCloseCallback(base::BindOnce(
      [](MainWidgetDelegateView* main_delegate, AlohaWidgetDelegateView* self,
         aloha::BrowserContentView* content_view) {
        auto ownership = self->MoveOutBrowserContentView(content_view);
        if (ownership) {
          ownership->SetCanOpenInNewWidget(true);
          main_delegate->AddBrowserContentView("", std::move(ownership));
        }
      },
      base::Unretained(this), base::Unretained(widget_delegate_view),
      browser_content_view));

  // 为主窗口关闭设置回调，确保独立窗口也关闭
  // 回调可能有滞后性，因此使用弱指针
  RegisterWindowWillCloseCallback(
      base::BindOnce(&views::Widget::CloseWithReason,
                     widget_delegate_view->GetWidget()->GetWeakPtr(),
                     views::Widget::ClosedReason::kCloseButtonClicked));
}

gfx::Size MainWidgetDelegateView::CalculatePreferredSize(
    const views::SizeBounds& /*available_size*/) const {
  gfx::Size size(1000, 800);
  for (size_t i = 0; i < tabbed_pane_->GetTabCount(); i++) {
    size.set_height(std::max(
        size.height(),
        tabbed_pane_->GetTabAt(i)->contents()->GetHeightForWidth(800)));
  }
  return size;
}

void MainWidgetDelegateView::TabSelectedAt(int index) {
  // status_label_->SetVisible(false);
}
MainWidgetDelegateView::~MainWidgetDelegateView() {}

}  // namespace aloha
