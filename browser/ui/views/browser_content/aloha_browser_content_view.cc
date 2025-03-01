#include "aloha/browser/ui/views/browser_content/aloha_browser_content_view.h"

#include <memory>
#include <utility>

#include "aloha/resources/vector_icons/vector_icons.h"
#include "aloha_browser_content_view.h"
#include "base/functional/bind.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/utf_string_conversions.h"
#include "components/input/native_web_keyboard_event.h"
#include "content/public/browser/navigation_handle.h"
#include "ui/color/color_id.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/textarea/textarea.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/metadata/view_factory_internal.h"
// #include "widget/widget_delegate_view.h"

namespace aloha {

AlohaBrowserContentView::AlohaBrowserContentView(
    const GURL& init_url,
    AlohaWidgetDelegateView* origin_widget_delegate_view)
    : origin_widget_delegate_view_(origin_widget_delegate_view),
      init_url_(init_url) {
  // 将布局管理器设置到当前视图
  SetLayoutManager(std::make_unique<views::FillLayout>());

  AddChildView(GetContentContainerView());

  // 初始化 webview
  sub_views_.webview->SetBrowserContext(GetDefaultBrowserContext());
  sub_views_.webview->GetWebContents()->SetDelegate(
      origin_widget_delegate_view->web_contents_delegate());
  Observe(sub_views_.webview->GetWebContents());
  sub_views_.webview->LoadInitialURL(init_url_);
  sub_views_.webview->GetWebContents()->Focus();
}

AlohaBrowserContentView::~AlohaBrowserContentView() {
  CloseChildWidgets();
}

void AlohaBrowserContentView::Init() {
  sub_views_.top_bar_right_view->AddChildView(
      views::ImageButton::CreateIconButton(
          base::BindRepeating(&AlohaBrowserContentView::CopyLink,
                              base::Unretained(this)),
          aloha::kLinkIcon, u"copy current link"));

  sub_views_.top_bar_right_view->AddChildView(
      views::ImageButton::CreateIconButton(
          base::BindRepeating(&AlohaBrowserContentView::ReloadContents,
                              base::Unretained(this)),
          aloha::kKsvReloadIcon, u"reload webview"));

  auto settings_btn = views::ImageButton::CreateIconButton(
      base::BindRepeating(&AlohaBrowserContentView::ShowSettingsMenu,
                          base::Unretained(this)),
      aloha::kGdSettingsIcon, u"setting");
  settings_btn_ =
      sub_views_.top_bar_right_view->AddChildView(std::move(settings_btn));
}

std::unique_ptr<views::View>
AlohaBrowserContentView::GetContentContainerView() {
  auto container =
      views::Builder<views::BoxLayoutView>()

          .SetOrientation(views::BoxLayout::Orientation::kVertical)
          .SetMainAxisAlignment(views::BoxLayout::MainAxisAlignment::kStretch)

          .AddChild(
              views::Builder<views::BoxLayoutView>()
                  .CopyAddressTo(&sub_views_.top_bar_container)
                  .SetOrientation(views::BoxLayout::Orientation::kHorizontal)
                  .SetBetweenChildSpacing(10)
                  .SetInsideBorderInsets(gfx::Insets::TLBR(6, 6, 6, 6))
                  .SetMainAxisAlignment(
                      views::BoxLayout::MainAxisAlignment::kStretch)
                  .SetCrossAxisAlignment(
                      views::BoxLayout::CrossAxisAlignment::kCenter)
                  .SetDefaultFlex(1)
                  .SetBackground(views::CreateThemedSolidBackground(
                      ui::kColorSysHeaderContainer))
                  .AddChildren(
                      views::Builder<views::BoxLayoutView>()
                          .SetBetweenChildSpacing(10)
                          //   .SetBackground(views::CreateThemedSolidBackground(
                          //       ui::kColorSysWhite))
                          .SetOrientation(
                              views::BoxLayout::Orientation::kHorizontal)
                          .SetMainAxisAlignment(
                              views::BoxLayout::MainAxisAlignment::kStart)
                          .CopyAddressTo(&sub_views_.top_bar_left_view),
                      views::Builder<views::BoxLayoutView>()
                          .SetBetweenChildSpacing(10)
                          //   .SetBackground(views::CreateThemedSolidBackground(
                          //       ui::kColorRefPrimary20))
                          .SetOrientation(
                              views::BoxLayout::Orientation::kHorizontal)
                          .SetMainAxisAlignment(
                              views::BoxLayout::MainAxisAlignment::kCenter)
                          .CopyAddressTo(&sub_views_.top_bar_middle_view),
                      views::Builder<views::BoxLayoutView>()
                          .SetBetweenChildSpacing(10)
                          //   .SetBackground(views::CreateThemedSolidBackground(
                          //       ui::kColorSysWhite))
                          .SetOrientation(
                              views::BoxLayout::Orientation::kHorizontal)
                          .SetMainAxisAlignment(
                              views::BoxLayout::MainAxisAlignment::kEnd)
                          .CopyAddressTo(&sub_views_.top_bar_right_view))

                  )
          .AddChild(views::Builder<views::WebView>().CopyAddressTo(
              &sub_views_.webview))
          .Build();

  // 让  webview 占满剩余的空间
  static_cast<views::BoxLayout*>(container->GetLayoutManager())
      ->SetFlexForView(sub_views_.webview, 1);
  return container;
}

void AlohaBrowserContentView::ReloadContents() {
  if (sub_views_.webview) {
    sub_views_.webview->GetWebContents()->GetController().Reload(
        content::ReloadType::NORMAL, true);
  }
}

void AlohaBrowserContentView::NavigateForward() {
  if (sub_views_.webview) {
    auto& navigation_controller =
        sub_views_.webview->GetWebContents()->GetController();
    if (navigation_controller.CanGoForward()) {
      navigation_controller.GoForward();
    }
  }
}

void AlohaBrowserContentView::NavigateBack() {
  if (sub_views_.webview) {
    auto& navigation_controller =
        sub_views_.webview->GetWebContents()->GetController();
    if (navigation_controller.CanGoBack()) {
      navigation_controller.GoBack();
    }
  }
}

void AlohaBrowserContentView::NavigateHome() {
  if (sub_views_.webview) {
    sub_views_.webview->GetWebContents()->GetController().LoadURL(
        init_url_, content::Referrer(), ui::PAGE_TRANSITION_LINK,
        std::string());
  }
}

void AlohaBrowserContentView::CopyLink() {
  std::string link;
  if (sub_views_.webview && sub_views_.webview->GetWebContents()) {
    link = sub_views_.webview->GetWebContents()->GetURL().spec();
  } else {
    link = "Error: Invalid WebView";
  }
  // 创建用于写入剪切板的数据对象
  ui::ScopedClipboardWriter clipboard_writer(ui::ClipboardBuffer::kCopyPaste);

  clipboard_writer.WriteText(base::UTF8ToUTF16(link));
  LOG(INFO) << "Call Copy Link: " << link;
}

void AlohaBrowserContentView::OpenDevtoolsAndInspect() {
  // TODO: 在右侧添加一个 WebView 用于显示 DevTools 界面
  LOG(INFO) << "Start Inspect of "
            << sub_views_.webview->GetWebContents()->GetURL().spec();
  if (devtools_view_) {
    LOG(INFO) << "DevTools view already opened";
    devtools_view_->GetWidget()->Activate();
    return;
  }

  if (!devtools_frontend_) {
    // 这个指针将和 WebAppView 生命周期一致
    devtools_frontend_ =
        DevToolsFrontend::CreateAndGet(sub_views_.webview->GetWebContents());
    LOG(INFO) << "inspect:" << devtools_frontend_->frontend_url().spec();
  }

  views::Widget* devtools_widget = new views::Widget();
  views::Widget::InitParams params(
      views::Widget::InitParams::NATIVE_WIDGET_OWNS_WIDGET,
      views::Widget::InitParams::TYPE_WINDOW);
  auto* delegate_view = new SimpleWidgetDelegateView();
  params.delegate = delegate_view;

  auto devtools_view = std::make_unique<AlohaBrowserContentView>(
      devtools_frontend_->frontend_url(), delegate_view);
  devtools_view->Init();
  devtools_view_ = devtools_view.get();
  devtools_frontend_->SetDevtoolsWebContents(devtools_view->web_contents());
  // 新建窗口
  delegate_view->AddBrowserContentView("Aloha devtools",
                                       std::move(devtools_view));

  devtools_widget->Init(std::move(params));
  delegate_view->RegisterWindowWillCloseCallback(base::BindOnce(
      [](AlohaWidgetDelegateView* delegate_view,
         AlohaBrowserContentView* self) {
        // 窗口关闭，清空 devtools_frontend_
        self->devtools_view_->CloseChildWidgets();
        self->devtools_view_ = nullptr;
        delegate_view->WindowClosing();
      },
      base::Unretained(delegate_view), base::Unretained(this)));

  devtools_widget->Show();
  AddChildWidget(devtools_widget->GetWeakPtr());
  // TODO: 调研 Chromium 内部提供的监听View在窗口间移动的方案
  // AddedToWidget();
  // GetWidget()
}

void AlohaBrowserContentView::ShowSettingsMenu() {
  LOG(INFO) << "Show Settings Menu";

  if (!menu_runner_) {
    setting_menu_model_ = std::make_unique<SettingMenuModel>();
    setting_menu_model_->RegisterCommandCallback(
        SettingMenuModel::Commands::COMMAND_INSPECT_THIS_PAGE,
        base::BindRepeating(&AlohaBrowserContentView::OpenDevtoolsAndInspect,
                            base::Unretained(this)));

    menu_runner_ = std::make_unique<views::MenuRunner>(
        setting_menu_model_.get(), views::MenuRunner::HAS_MNEMONICS);
  }
  menu_runner_->RunMenuAt(settings_btn_->GetWidget(), nullptr,
                          gfx::Rect(settings_btn_->GetBoundsInScreen()),
                          views::MenuAnchorPosition::kBubbleBottomLeft,
                          ui::mojom::MenuSourceType::kNone);
}

void AlohaBrowserContentView::Close() {
  // 如果处于独立窗口需要关闭独立窗口将 BrowserContentView 归还给主端窗口
  if (!CanOpenInNewWidget()) {
    auto* owned_widget_delegate_view = GetWidget();
    if (owned_widget_delegate_view) {
      owned_widget_delegate_view->Close();
    }
  }
  origin_widget_delegate_view_->CloseBrowserContentView(this);
}

void AlohaBrowserContentView::SetCanOpenInNewWidget(bool can_open_in_new_tab) {
  can_open_in_new_tab_ = can_open_in_new_tab;
  open_view_in_new_window_btn_->SetVisible(can_open_in_new_tab);
}

bool AlohaBrowserContentView::CanOpenInNewWidget() {
  return can_open_in_new_tab_;
}

void AlohaBrowserContentView::AddChildWidget(
    base::WeakPtr<views::Widget> widget) {
  child_widgets_.push_back(std::move(widget));
}

void AlohaBrowserContentView::CloseChildWidgets() {
  for (auto& widget : child_widgets_) {
    if (widget) {
      widget->CloseWithReason(views::Widget::ClosedReason::kCloseButtonClicked);
    }
  }
}

WebAppContentView::WebAppContentView(const GURL& init_url,
                                     AlohaWidgetDelegateView* delegate)
    : AlohaBrowserContentView(init_url, delegate) {}

WebAppContentView::~WebAppContentView() = default;

AlohaWidgetDelegateView* CreateAlohaWidgetDelegateView() {
  return new SimpleWidgetDelegateView();
}

void WebAppContentView::Init() {
  // 添加 icon button
  auto open_view_in_new_window_btn = views::ImageButton::CreateIconButton(
      base::BindRepeating(
          &AlohaWidgetDelegateView::ShowBrowserContentViewInNewWindow,
          base::Unretained(origin_widget_delegate_view_),
          base::Unretained(this),
          base::BindRepeating([]() -> AlohaWidgetDelegateView* {
            return new SimpleWidgetDelegateView();
          })),
      aloha::kMoveGroupToNewWindowRefreshIcon, u"open in new window");
  open_view_in_new_window_btn_ = open_view_in_new_window_btn.get();
  sub_views_.top_bar_right_view->AddChildView(
      std::move(open_view_in_new_window_btn));

  sub_views_.top_bar_right_view->AddChildView(
      views::ImageButton::CreateIconButton(
          base::BindRepeating(&AlohaBrowserContentView::CopyLink,
                              base::Unretained(this)),
          aloha::kLinkIcon, u"copy current link"));

  sub_views_.top_bar_right_view->AddChildView(
      views::ImageButton::CreateIconButton(
          base::BindRepeating(&AlohaBrowserContentView::ReloadContents,
                              base::Unretained(this)),
          aloha::kKsvReloadIcon, u"reload webview"));

  auto settings_btn = views::ImageButton::CreateIconButton(
      base::BindRepeating(&AlohaBrowserContentView::ShowSettingsMenu,
                          base::Unretained(this)),
      aloha::kGdSettingsIcon, u"setting");
  settings_btn_ =
      sub_views_.top_bar_right_view->AddChildView(std::move(settings_btn));

  sub_views_.top_bar_left_view->AddChildView(
      views::ImageButton::CreateIconButton(
          base::BindRepeating(&AlohaBrowserContentView::NavigateHome,
                              base::Unretained(this)),
          aloha::kNavigateHomeIcon, u"back to home"));
  sub_views_.top_bar_left_view->AddChildView(
      views::ImageButton::CreateIconButton(
          base::BindRepeating(&AlohaBrowserContentView::NavigateBack,
                              base::Unretained(this)),
          aloha::kCaretLeftIcon, u"back"));
  sub_views_.top_bar_left_view->AddChildView(
      views::ImageButton::CreateIconButton(
          base::BindRepeating(&AlohaBrowserContentView::NavigateForward,
                              base::Unretained(this)),
          aloha::kCaretRightIcon, u"forward"));
}

WebSiteContentView::WebSiteContentView(const GURL& init_url,
                                       AlohaWidgetDelegateView* delegate)
    : AlohaBrowserContentView(init_url, delegate) {}

WebSiteContentView::~WebSiteContentView() = default;

void WebSiteContentView::Init() {
  auto open_view_in_new_window_btn = views::ImageButton::CreateIconButton(
      base::BindRepeating(
          &AlohaWidgetDelegateView::ShowBrowserContentViewInNewWindow,
          base::Unretained(origin_widget_delegate_view_),
          base::Unretained(this),
          base::BindRepeating([]() -> AlohaWidgetDelegateView* {
            return new SimpleWidgetDelegateView();
          })),
      aloha::kMoveGroupToNewWindowRefreshIcon, u"open in new window");
  open_view_in_new_window_btn_ = open_view_in_new_window_btn.get();
  sub_views_.top_bar_right_view->AddChildView(
      std::move(open_view_in_new_window_btn));

  sub_views_.top_bar_right_view->AddChildView(
      views::ImageButton::CreateIconButton(
          base::BindRepeating(&AlohaBrowserContentView::CopyLink,
                              base::Unretained(this)),
          aloha::kLinkIcon, u"copy current link"));

  sub_views_.top_bar_right_view->AddChildView(
      views::ImageButton::CreateIconButton(
          base::BindRepeating(&AlohaBrowserContentView::ReloadContents,
                              base::Unretained(this)),
          aloha::kKsvReloadIcon, u"reload webview"));

  auto settings_btn = views::ImageButton::CreateIconButton(
      base::BindRepeating(&AlohaBrowserContentView::ShowSettingsMenu,
                          base::Unretained(this)),
      aloha::kGdSettingsIcon, u"setting");
  settings_btn_ =
      sub_views_.top_bar_right_view->AddChildView(std::move(settings_btn));

  auto close_btn = views::ImageButton::CreateIconButton(
      base::BindRepeating(&AlohaBrowserContentView::Close,
                          base::Unretained(this)),
      aloha::kEcheCloseIcon, u"close");
  sub_views_.top_bar_right_view->AddChildView(std::move(close_btn));

  sub_views_.top_bar_left_view->AddChildView(
      views::ImageButton::CreateIconButton(
          base::BindRepeating(&AlohaBrowserContentView::NavigateHome,
                              base::Unretained(this)),
          aloha::kNavigateHomeIcon, u"back to home"));
  sub_views_.top_bar_left_view->AddChildView(
      views::ImageButton::CreateIconButton(
          base::BindRepeating(&AlohaBrowserContentView::NavigateBack,
                              base::Unretained(this)),
          aloha::kCaretLeftIcon, u"back"));
  sub_views_.top_bar_left_view->AddChildView(
      views::ImageButton::CreateIconButton(
          base::BindRepeating(&AlohaBrowserContentView::NavigateForward,
                              base::Unretained(this)),
          aloha::kCaretRightIcon, u"forward"));

  sub_views_.top_bar_middle_view->AddChildView(
      views::Builder<views::Textarea>()
          .SetText(base::UTF8ToUTF16(init_url_.spec()))
          .SetPlaceholderText(u"Please input url")
          .Build());
  sub_views_.top_bar_middle_view->SetLayoutManager(
      std::make_unique<views::FillLayout>());
  // 尽可能调大 flex 以占满剩余的空间
  sub_views_.top_bar_container->SetFlexForView(sub_views_.top_bar_middle_view,
                                               100);
}

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
  auto created_view =
      std::make_unique<aloha::WebSiteContentView>(target_url, delegate_);
  created_view->Init();

  delegate_->AddBrowserContentView(target_url.ExtractFileName().empty()
                                       ? target_url.HostNoBrackets()
                                       : target_url.ExtractFileName(),
                                   std::move(created_view));
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
}  // namespace aloha
