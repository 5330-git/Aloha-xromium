#include "aloha/browser/ui/views/widget/widget_delegate_view.h"

#include <memory>
#include <utility>

#include "aloha/browser/ui/views/aloha_browser_content_view.h"
#include "aloha/grit/aloha_resources.h"
#include "aloha/resources/vector_icons/vector_icons.h"
#include "aloha/views/controls/tabbed_pane/tabbed_pane.h"
#include "base/functional/bind.h"
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
#include "url/gurl.h"
#include "widget_delegate_view.h"

namespace aloha {
// namespace {
base::raw_ptr<content::BrowserContext> default_browser_context_ = nullptr;
base::raw_ptr<NativeWidgetDelegateView> g_instance_ = nullptr;
// }  // namespace

void SetDefaultBrowserContext(content::BrowserContext* browser_context) {
  // 应当是单例，因此做一下检查
  DCHECK(!default_browser_context_);
  default_browser_context_ = browser_context;
}

content::BrowserContext* GetDefaultBrowserContext() {
  return default_browser_context_;
}

AlohaWidgetDelegateView::AlohaWidgetDelegateView() = default;

gfx::Size AlohaWidgetDelegateView::GetMinimumSize() const {
  return gfx::Size(700, 700);
}

std::u16string AlohaWidgetDelegateView::GetWindowTitle() const {
  return l10n_util::GetStringUTF16(IDS_ALOHA_WINDOW_TITLE);
}

AlohaWidgetDelegateView::~AlohaWidgetDelegateView() {}

IndependentWidgetDelegateView::IndependentWidgetDelegateView()
    : name_("Aloha Indepentent Window") {
  SetHasWindowSizeControls(true);
  SetLayoutManager(std::make_unique<views::FillLayout>());
}

IndependentWidgetDelegateView::~IndependentWidgetDelegateView() {}

void IndependentWidgetDelegateView::AddBrowserContentView(
    std::string name,
    std::unique_ptr<views::View> content_view) {
  name_ = name;
  SetTitle(base::UTF8ToUTF16(name));
  content_view_ = AddChildView(std::move(content_view));
}

std::unique_ptr<views::View>
IndependentWidgetDelegateView::MoveOutBrowserContentView(
    views::View* content_view) {
  return RemoveChildViewT(content_view);
}

std::unique_ptr<views::View>
IndependentWidgetDelegateView::CloseBrowserContentView(
    views::View* content_view) {
  return MoveOutBrowserContentView(content_view);
}

std::u16string IndependentWidgetDelegateView::GetWindowTitle() const {
  return base::UTF8ToUTF16(name_);
}

gfx::Size IndependentWidgetDelegateView::GetMinimumSize() const {
  return gfx::Size(1000, 700);
}

gfx::Size IndependentWidgetDelegateView::CalculatePreferredSize(
    const views::SizeBounds& /*available_size*/) const {
  gfx::Size size(1000, 800);
  return size;
}

NativeWidgetDelegateView* NativeWidgetDelegateView::instance() {
  return g_instance_;
}

NativeWidgetDelegateView::NativeWidgetDelegateView() {
  web_contents_delegate_ = std::make_unique<AlohaWebContentDelegate>(
      aloha::GetDefaultBrowserContext(), this);
  // views::Widget::InitParams params(
  //     views::Widget::InitParams::NATIVE_WIDGET_OWNS_WIDGET,
  //     views::Widget::InitParams::TYPE_WINDOW);

  g_instance_ = this;
  SetHasWindowSizeControls(true);
  SetBackground(
      views::CreateThemedSolidBackground(ui::kColorSysHeaderContainer));

  auto* layout = SetLayoutManager(std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kVertical, gfx::Insets(0)));
  auto tabbed_pane = std::make_unique<AlohaTabbedPane>(
      AlohaTabbedPane::Orientation::kVertical,
      AlohaTabbedPane::TabStripStyle::kHighlight, true);
  tabbed_pane->SetBackground(
      views::CreateThemedSolidBackground(ui::kColorSysHeaderContainer));

  tabbed_pane_ = AddChildView(std::move(tabbed_pane));

  tabbed_pane_->set_listener(this);
  layout->SetFlexForView(tabbed_pane_, 1);

  // 随意添加一个 Tab
  auto webapp_view = std::make_unique<WebAppContentView>(
      GURL(AlohaBrowserContentView::kAlohaHomeURL), this);
  webapp_view->Init();

  auto* raw_webapp_view = webapp_view.get();
  // auto* view = tabbed_pane_->AddTab(
  //     base::UTF8ToUTF16(std::string("Aloha Home")), std::move(webapp_view));
  AddBrowserContentView("Aloha Main", std::move(webapp_view));
  LOG(INFO) << raw_webapp_view;
  // LOG(INFO) << view;
}

void NativeWidgetDelegateView::AddBrowserContentView(
    std::string name,
    std::unique_ptr<views::View> content_view) {
  // 检查是否是被移出去的窗口
  auto tab_view_iter = views_in_independent_window_.find(content_view.get());
  if (tab_view_iter != views_in_independent_window_.end()) {
    auto tab_view = tab_view_iter->second;
    auto* empty_view = tab_view->contents();
    tabbed_pane_->GetContents()->RemoveChildViewT(empty_view);
    tabbed_pane_->GetContents()->AddChildView(std::move(content_view));

    tab_view->SetContents(tab_view_iter->first);
    return;
  }
  tabbed_pane_->AddTab(base::UTF8ToUTF16(name), std::move(content_view));
  size_t count = tabbed_pane_->GetTabCount();
  tabbed_pane_->SelectTabAt(count - 1);
}

std::unique_ptr<views::View>
NativeWidgetDelegateView::MoveOutBrowserContentView(views::View* content_view) {
  if (content_view) {
    // 找到对应的 Tab
    for (size_t i = 0; i < tabbed_pane_->GetTabCount(); i++) {
      auto* tab = tabbed_pane_->GetTabAt(i);
      if (tab->contents() == content_view) {
        // TODO(yeyun.anton):
        // 为被移除的区域添加内容，未来通过点击定位到被移除且独立出来的窗口上 将
        // content view 移除掉，并存储到容器中，维护一个 content 到 tab 的映射
        auto tab_view =
            tabbed_pane_->GetContents()->RemoveChildViewT(content_view);
        tab->SetContents(tabbed_pane_->GetContents()->AddChildView(
            std::make_unique<views::MessageBoxView>()));

        views_in_independent_window_.emplace(content_view, tab);
        return tab_view;
      }
    }
  }

  return nullptr;
}

std::unique_ptr<views::View> NativeWidgetDelegateView::CloseBrowserContentView(
    views::View* content_view) {
  auto content_view_uniptr = MoveOutBrowserContentView(content_view);
  if (content_view) {
    auto tab_view_iter = views_in_independent_window_.find(content_view);
    CHECK(tab_view_iter != views_in_independent_window_.end());
    auto tab_view = tab_view_iter->second;
    tabbed_pane_->RemoveTab(tab_view);
    return content_view_uniptr;
  }
  return nullptr;
}

gfx::Size NativeWidgetDelegateView::CalculatePreferredSize(
    const views::SizeBounds& /*available_size*/) const {
  gfx::Size size(1000, 800);
  for (size_t i = 0; i < tabbed_pane_->GetTabCount(); i++) {
    size.set_height(std::max(
        size.height(),
        tabbed_pane_->GetTabAt(i)->contents()->GetHeightForWidth(800)));
  }
  return size;
}

void NativeWidgetDelegateView::TabSelectedAt(int index) {
  // status_label_->SetVisible(false);
}
NativeWidgetDelegateView::~NativeWidgetDelegateView() {}

}  // namespace aloha
