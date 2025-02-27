#include "aloha/browser/ui/native/widget_delegate_view.h"

#include <memory>
#include <utility>

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
  auto webapp_view =
      std::make_unique<WebAppView>(GURL(WebAppView::kAlohaHomeURL), this);

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

// ---------------------- 迁移
bool AlohaWebContentDelegate::HandleKeyboardEvent(
    content::WebContents* source,
    const input::NativeWebKeyboardEvent& event) {
  LOG(INFO) << "event: " << event.native_key_code;
  return content::WebContentsDelegate::HandleKeyboardEvent(source, event);
}

// 新建 tab
void AlohaWebContentDelegate::WebContentsCreated(
    content::WebContents* source_contents,
    int opener_render_process_id,
    int opener_render_frame_id,
    const std::string& frame_name,
    const GURL& target_url,
    content::WebContents* new_contents) {
  // 打印信息
  LOG(INFO) << "opener_render_process_id: " << opener_render_process_id;
  LOG(INFO) << "opener_render_frame_id: " << opener_render_frame_id;
  LOG(INFO) << "frame_name: " << frame_name;
  LOG(INFO) << "target_url: " << target_url;
  LOG(INFO) << "new_contents: " << new_contents;
  LOG(INFO) << target_url.ExtractFileName();
  LOG(INFO) << target_url.HostNoBrackets();
  LOG(INFO) << target_url.HostNoBracketsPiece();

  // TODO(yeyun.anton): new_contents 没有使用到，而是通过 webview
  // 重新创建了，后续调研如何使用 new_contents
  // new_contents 由 WebContentsImpl::CreateNewWindow
  // 中创建，通过 GetCreatedWindow 可以获得其所有权（CreatedWindow对象）
  delegate_->AddBrowserContentView(
      target_url.ExtractFileName().empty() ? target_url.HostNoBrackets()
                                           : target_url.ExtractFileName(),
      std::make_unique<aloha::WebSiteView>(target_url, delegate_));

  // auto webview =
  //     std::make_unique<views::WebView>(new_contents->GetBrowserContext());
  // webview->SetWebContents(new_contents);
  // webview->GetWebContents()->SetDelegate(this);
  // delegate_->AddBrowserContentView(target_url.ExtractFileName().empty()
  //                                      ? target_url.HostNoBrackets()
  //                                      : target_url.ExtractFileName(),
  //                                  std::move(webview));
  // // webview_raw_ptr->LoadInitialURL(target_url);
  // // webview_raw_ptr->GetWebContents()->Focus();
}

// Called by WebContentsImpl::Close，想要关闭 webcontents 直接让持有者删除它即可
void AlohaWebContentDelegate::CloseContents(content::WebContents* source) {
  LOG(INFO) << "Aloha CloseContents: " << source;
}

// 鼠标悬浮时会调用这个接口
void AlohaWebContentDelegate::UpdateTargetURL(content::WebContents* source,
                                              const GURL& url) {
  LOG(INFO) << "Aloha UpdateTargetURL: " << url.spec();
}

DevtoolsView::DevtoolsView(const GURL& devtools_url,
                           AlohaWidgetDelegateView* root_widget_view,
                           AlohaWidgetDelegateView* current_widget_view)
    : root_widget_view_(root_widget_view),
      current_widget_view_(current_widget_view),
      devtools_url_(devtools_url) {
  // 设置布局管理器为垂直方向的 BoxLayout
  auto box_layout = std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kVertical);
  box_layout->set_main_axis_alignment(
      views::BoxLayout::MainAxisAlignment::kStart);
  box_layout->set_cross_axis_alignment(
      views::BoxLayout::CrossAxisAlignment::kStretch);

  // 将布局管理器设置到当前视图
  SetLayoutManager(std::move(box_layout));

  base::raw_ptr<views::BoxLayoutView> tools_button_views = nullptr;
  base::raw_ptr<views::BoxLayoutView> tools_button_left_views = nullptr;
  base::raw_ptr<views::BoxLayoutView> tools_button_right_views = nullptr;

  auto top_bar_view =
      views::Builder<views::BoxLayoutView>()

          .SetOrientation(views::BoxLayout::Orientation::kVertical)
          .AddChildren(

              views::Builder<views::BoxLayoutView>()
                  .CopyAddressTo(&tools_button_views)
                  .SetOrientation(views::BoxLayout::Orientation::kHorizontal)
                  .SetBetweenChildSpacing(10)
                  .SetInsideBorderInsets(gfx::Insets::TLBR(6, 6, 6, 6))
                  .SetMainAxisAlignment(
                      views::BoxLayout::MainAxisAlignment::kStretch)
                  .SetCrossAxisAlignment(
                      views::BoxLayout::CrossAxisAlignment::kCenter)
                  .AddChildren(
                      views::Builder<views::BoxLayoutView>()
                          .SetBetweenChildSpacing(10)
                          .SetOrientation(
                              views::BoxLayout::Orientation::kHorizontal)
                          .SetMainAxisAlignment(
                              views::BoxLayout::MainAxisAlignment::kStart)
                          .CopyAddressTo(&tools_button_left_views),
                      views::Builder<views::BoxLayoutView>()
                          .SetBetweenChildSpacing(10)
                          .SetOrientation(
                              views::BoxLayout::Orientation::kHorizontal)
                          .SetMainAxisAlignment(
                              views::BoxLayout::MainAxisAlignment::kEnd)
                          .CopyAddressTo(&tools_button_right_views)))
          .Build();
  tools_button_right_views->AddChildView(views::ImageButton::CreateIconButton(
      base::BindRepeating(&DevtoolsView::CopyLink, base::Unretained(this)),
      aloha::kLinkIcon, u"copy current link"));

  auto settings_btn = views::ImageButton::CreateIconButton(
      base::BindRepeating(&DevtoolsView::ShowSettingsMenu,
                          base::Unretained(this)),
      aloha::kGdSettingsIcon, u"setting");
  settings_btn_ =
      tools_button_right_views->AddChildView(std::move(settings_btn));

  tools_button_views->SetBackground(
      views::CreateThemedSolidBackground(ui::kColorSysHeaderContainer));
  tools_button_views->SetPreferredSize(gfx::Size(0, 40));  // 固定高度为 30
  // 设置flex 让子元素铺满空间
  tools_button_views->SetDefaultFlex(1);

  // 创建 WebView
  auto webview =
      std::make_unique<views::WebView>(aloha::GetDefaultBrowserContext());
  webview->GetWebContents()->SetDelegate(
      root_widget_view_->web_contents_delegate());
  Observe(webview->GetWebContents());
  webview->LoadInitialURL(devtools_url_);
  webview->GetWebContents()->Focus();

  // 添加子视图
  AddChildView(std::move(top_bar_view));
  webview_ = AddChildView(std::move(webview));

  // 设置 WebView 的弹性因子，使其占据剩余空间
  auto* current_layout = static_cast<views::BoxLayout*>(GetLayoutManager());
  current_layout->SetFlexForView(webview_, 1);
}

DevtoolsView::~DevtoolsView() = default;

WebAppView::WebAppView(const GURL& app_url,
                       AlohaWidgetDelegateView* root_widget_view_)
    : app_url_(app_url),
      root_widget_view_(root_widget_view_),
      current_widget_view_(root_widget_view_) {
  LOG(INFO) << "WebAppView: " << app_url.spec();

  // 设置布局管理器为垂直方向的 BoxLayout
  auto box_layout = std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kVertical);
  box_layout->set_main_axis_alignment(
      views::BoxLayout::MainAxisAlignment::kStart);
  box_layout->set_cross_axis_alignment(
      views::BoxLayout::CrossAxisAlignment::kStretch);

  // 将布局管理器设置到当前视图
  SetLayoutManager(std::move(box_layout));

  // base::raw_ptr<views::BoxLayoutView> address_view_in_top_bar_view = nullptr;
  base::raw_ptr<views::BoxLayoutView> tools_button_views = nullptr;
  base::raw_ptr<views::BoxLayoutView> tools_button_left_views = nullptr;
  base::raw_ptr<views::BoxLayoutView> tools_button_right_views = nullptr;

  auto top_bar_view =
      views::Builder<views::BoxLayoutView>()

          .SetOrientation(views::BoxLayout::Orientation::kVertical)
          .AddChildren(

              views::Builder<views::BoxLayoutView>()
                  .CopyAddressTo(&tools_button_views)
                  .SetOrientation(views::BoxLayout::Orientation::kHorizontal)
                  .SetBetweenChildSpacing(10)
                  .SetInsideBorderInsets(gfx::Insets::TLBR(6, 6, 6, 6))
                  .SetMainAxisAlignment(
                      views::BoxLayout::MainAxisAlignment::kStretch)
                  .SetCrossAxisAlignment(
                      views::BoxLayout::CrossAxisAlignment::kCenter)
                  .AddChildren(
                      views::Builder<views::BoxLayoutView>()
                          .SetBetweenChildSpacing(10)
                          .SetOrientation(
                              views::BoxLayout::Orientation::kHorizontal)
                          .SetMainAxisAlignment(
                              views::BoxLayout::MainAxisAlignment::kStart)
                          .CopyAddressTo(&tools_button_left_views),
                      views::Builder<views::BoxLayoutView>()
                          .SetBetweenChildSpacing(10)
                          .SetOrientation(
                              views::BoxLayout::Orientation::kHorizontal)
                          .SetMainAxisAlignment(
                              views::BoxLayout::MainAxisAlignment::kEnd)
                          .CopyAddressTo(&tools_button_right_views)))
          .Build();

  // 添加 icon button
  auto open_view_in_new_window_btn = views::ImageButton::CreateIconButton(
      base::BindRepeating(&WebAppView::OpenSelectedTabInNewWindow,
                          base::Unretained(this)),
      aloha::kMoveGroupToNewWindowRefreshIcon, u"open in new window");
  open_view_in_new_window_btn_ = open_view_in_new_window_btn.get();
  tools_button_right_views->AddChildView(
      std::move(open_view_in_new_window_btn));

  tools_button_right_views->AddChildView(views::ImageButton::CreateIconButton(
      base::BindRepeating(&WebAppView::CopyLink, base::Unretained(this)),
      aloha::kLinkIcon, u"copy current link"));

  tools_button_right_views->AddChildView(views::ImageButton::CreateIconButton(
      base::BindRepeating(&WebAppView::ReloadWebView, base::Unretained(this)),
      aloha::kKsvReloadIcon, u"reload webview"));

  auto settings_btn = views::ImageButton::CreateIconButton(
      base::BindRepeating(&WebAppView::ShowSettingsMenu,
                          base::Unretained(this)),
      aloha::kGdSettingsIcon, u"setting");
  settings_btn_ =
      tools_button_right_views->AddChildView(std::move(settings_btn));

  tools_button_left_views->AddChildView(views::ImageButton::CreateIconButton(
      base::BindRepeating(&WebAppView::GoBackHome, base::Unretained(this)),
      aloha::kNavigateHomeIcon, u"back to home"));
  tools_button_left_views->AddChildView(views::ImageButton::CreateIconButton(
      base::BindRepeating(&WebAppView::GoBackPage, base::Unretained(this)),
      aloha::kCaretLeftIcon, u"back"));
  tools_button_left_views->AddChildView(views::ImageButton::CreateIconButton(
      base::BindRepeating(&WebAppView::GoForwardPage, base::Unretained(this)),
      aloha::kCaretRightIcon, u"forward"));

  tools_button_views->SetBackground(
      views::CreateThemedSolidBackground(ui::kColorSysHeaderContainer));
  tools_button_views->SetPreferredSize(gfx::Size(0, 40));  // 固定高度为 30
  // 设置flex 让子元素铺满空间
  tools_button_views->SetDefaultFlex(1);

  // 创建 WebView
  auto webview =
      std::make_unique<views::WebView>(aloha::GetDefaultBrowserContext());
  webview->GetWebContents()->SetDelegate(
      root_widget_view_->web_contents_delegate());
  Observe(webview->GetWebContents());
  webview->LoadInitialURL(app_url_);
  webview->GetWebContents()->Focus();

  // 添加子视图
  top_bar_view_ = AddChildView(std::move(top_bar_view));
  webview_ = AddChildView(std::move(webview));

  // 设置 WebView 的弹性因子，使其占据剩余空间
  auto* current_layout = static_cast<views::BoxLayout*>(GetLayoutManager());
  current_layout->SetFlexForView(webview_, 1);
}

WebAppView::~WebAppView() {}

void WebAppView::DidStartNavigation(
    content::NavigationHandle* navigation_handle) {
  if (navigation_handle->IsInMainFrame()) {
    LOG(INFO) << "Starting navigation to: " << navigation_handle->GetURL();
  }
}

WebSiteView::WebSiteView(const GURL& app_url,
                         AlohaWidgetDelegateView* root_widget_view_)
    : app_url_(app_url), root_widget_view_(root_widget_view_) {
  LOG(INFO) << "WebAppView: " << app_url.spec();

  // 设置布局管理器为垂直方向的 BoxLayout
  auto box_layout = std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kVertical);
  box_layout->set_main_axis_alignment(
      views::BoxLayout::MainAxisAlignment::kStart);
  box_layout->set_cross_axis_alignment(
      views::BoxLayout::CrossAxisAlignment::kStretch);

  // 将布局管理器设置到当前视图
  SetLayoutManager(std::move(box_layout));

  // 创建顶部栏视图，使用 Builder 构建

  base::raw_ptr<views::Textarea> textarea;

  auto top_bar_view =
      views::Builder<views::BoxLayoutView>()
          .SetOrientation(views::BoxLayout::Orientation::kHorizontal)
          .SetInsideBorderInsets(gfx::Insets::TLBR(10, 6, 6, 10))
          .SetBetweenChildSpacing(20)
          .SetMainAxisAlignment(views::BoxLayout::MainAxisAlignment::kEnd)
          .SetCrossAxisAlignment(views::BoxLayout::CrossAxisAlignment::kCenter)
          .AddChildren(views::Builder<views::Textarea>()
                           .CopyAddressTo(&textarea)
                           .SetText(base::UTF8ToUTF16(app_url_.spec()))
                           .SetPlaceholderText(u"Please input url")
                           .SetPreferredSize(gfx::Size(400, 40))
                           .SetBackground(views::CreateThemedSolidBackground(
                               ui::kColorDialogBackground)))

          .Build();
  // 添加 icon button
  top_bar_view->AddChildView(views::ImageButton::CreateIconButton(
      base::BindRepeating(&WebSiteView::ReloadWebView, base::Unretained(this)),
      aloha::kKsvReloadIcon, u"reload webview"));
  top_bar_view->AddChildView(views::ImageButton::CreateIconButton(
      base::BindRepeating(&WebSiteView::CloseWebView, base::Unretained(this)),
      aloha::kEcheCloseIcon, u"close webview"));

  top_bar_view->SetBackground(
      views::CreateThemedSolidBackground(ui::kColorSysHeaderContainer));
  top_bar_view->SetPreferredSize(gfx::Size(0, 50));  // 固定高度为 30
  // 设置flex 让label 铺满剩余空间
  top_bar_view->SetFlexForView(textarea, 1);

  // 创建 WebView
  auto webview =
      std::make_unique<views::WebView>(aloha::GetDefaultBrowserContext());
  webview->GetWebContents()->SetDelegate(
      root_widget_view_->web_contents_delegate());
  webview->LoadInitialURL(app_url_);
  webview->GetWebContents()->Focus();

  // 添加子视图
  top_bar_view_ = AddChildView(std::move(top_bar_view));
  webview_ = AddChildView(std::move(webview));

  // 设置 WebView 的弹性因子，使其占据剩余空间
  auto* current_layout = static_cast<views::BoxLayout*>(GetLayoutManager());
  current_layout->SetFlexForView(webview_, 1);
}

}  // namespace aloha
