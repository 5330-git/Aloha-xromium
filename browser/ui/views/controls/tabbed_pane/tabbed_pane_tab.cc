#include "aloha/browser/ui/views/controls/tabbed_pane/tabbed_pane_tab.h"

#include "aloha/browser/ui/color/color_ids.h"
#include "aloha/browser/ui/menu/tab_menu_model.h"
#include "aloha/browser/ui/views/controls/tabbed_pane/tabbed_pane.h"
#include "aloha/resources/grit/app_icon_resources.h"
#include "aloha/resources/grit/unscaled_resources.h"
#include "aloha/resources/vector_icons/vector_icons.h"
#include "ui/accessibility/ax_action_data.h"
#include "ui/base/dragdrop/drag_drop_types.h"
#include "ui/base/dragdrop/drop_target_event.h"
#include "ui/base/dragdrop/mojom/drag_drop_types.mojom-shared.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/skia_conversions.h"
#include "ui/views/accessibility/view_accessibility.h"
#include "ui/views/animation/ink_drop.h"
#include "ui/views/controls/highlight_path_generator.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/layout_provider.h"


namespace aloha {
AlohaTabbedPaneTab::AlohaTabbedPaneTab(AlohaTabbedPane* tabbed_pane,
                                       const std::u16string& title,
                                       View* contents)
    : tabbed_pane_(tabbed_pane), contents_(contents) {
  // 让子控件的鼠标进入/退出事件可以传递给父控件(this)
  SetNotifyEnterExitOnChild(true);

  // Calculate the size while the font list is bold.
  auto title_label = std::make_unique<views::Label>(
      title, views::style::CONTEXT_LABEL, views::style::STYLE_TAB_ACTIVE);
  title_ = title_label.get();
  UpdatePreferredTitleWidth();

  if (tabbed_pane_->GetOrientation() ==
      AlohaTabbedPane::Orientation::kVertical) {
    title_label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);

    // 添加 Padding
    constexpr auto kTabPadding = gfx::Insets::TLBR(0, 10, 0, 5);
    SetBorder(views::CreateEmptyBorder(kTabPadding));
  } else {
    constexpr auto kBorderThickness = gfx::Insets(2);
    SetBorder(views::CreateEmptyBorder(kBorderThickness));
  }

  SetState(State::kInactive);
  views::BoxLayout* layout =
      SetLayoutManager(std::make_unique<views::BoxLayout>());

  favicon_ = AddChildView(std::make_unique<views::ImageView>(
      ui::ImageModel::FromResourceId(IDR_ALOHA_ICON)));
  favicon_->SetImageSize(gfx::Size(18, 18));

  AddChildView(std::move(title_label));

  // close button defination
  close_button_ = AddChildView(std::make_unique<views::ImageButton>());
  int icon_size = 16;
  int button_size = 32;
  close_button_->SetImageModel(
      views::ImageButton::ButtonState::STATE_DISABLED,
      ui::ImageModel::FromVectorIcon(aloha::kEcheCloseIcon, ui::kColorIcon,
                                     icon_size));
  close_button_->SetImageModel(
      views::ImageButton::ButtonState::STATE_HOVERED,
      ui::ImageModel::FromVectorIcon(aloha::kEcheCloseIcon, ui::kColorIcon,
                                     icon_size));
  close_button_->SetImageModel(
      views::ImageButton::ButtonState::STATE_NORMAL,
      ui::ImageModel::FromVectorIcon(aloha::kEcheCloseIcon, ui::kColorIcon,
                                     icon_size));
  close_button_->SetImageModel(
      views::ImageButton::ButtonState::STATE_PRESSED,
      ui::ImageModel::FromVectorIcon(aloha::kEcheCloseIcon, ui::kColorIcon,
                                     icon_size));

  close_button_->SetCallback(base::BindRepeating(&AlohaTabbedPaneTab::OnClose,
                                                 base::Unretained(this)));
  const int highlight_radius =
      views::LayoutProvider::Get()->GetCornerRadiusMetric(
          views::Emphasis::kMedium, gfx::Size(button_size, button_size));
  views::InstallRoundRectHighlightPathGenerator(
      close_button_.get(), gfx::Insets(), highlight_radius);

  views::InkDrop::Get(close_button_.get())
      ->SetMode(views::InkDropHost::InkDropMode::ON);
  close_button_->SetHasInkDropActionOnClick(true);
  close_button_->SetShowInkDropWhenHotTracked(true);
  views::InkDrop::Get(close_button_.get())
      ->SetBaseColorCallback(base::BindRepeating(
          [](views::ImageButton* host) {
            return host->GetColorProvider()->GetColor(
                ui::kColorSysOnSurfaceSubtle);
          },
          close_button_.get()));
  close_button_->SetVisible(false);
  // close button defination end

  layout->SetOrientation(views::BoxLayout::Orientation::kHorizontal);
  layout->set_main_axis_alignment(views::BoxLayout::MainAxisAlignment::kStart);
  layout->set_cross_axis_alignment(
      views::BoxLayout::CrossAxisAlignment::kCenter);
  layout->set_between_child_spacing(10);

  // flex
  layout->SetFlexForView(favicon_, 0);
  layout->SetFlexForView(title_, 10);
  layout->SetFlexForView(close_button_, 0);

  // Use leaf so that name is spoken by screen reader without exposing the
  // children.
  GetViewAccessibility().SetIsLeaf(true);
  GetViewAccessibility().SetRole(ax::mojom::Role::kTab);
  UpdateAccessibleName();

  OnStateChanged();

  title_text_changed_callback_ =
      title_->AddTextChangedCallback(base::BindRepeating(
          &AlohaTabbedPaneTab::UpdateAccessibleName, base::Unretained(this)));

  // 临时实现右键 Context menu
  tab_context_menu_controller_ = std::make_unique<TabContextMenuController>();
  tab_context_menu_controller_->GetTabMenuModel()->RegisterCommandCallback(
      TabMenuModel::Commands::COMMAND_CLOSE_TAB,
      base::BindRepeating(&AlohaTabbedPaneTab::OnClose,
                          base::Unretained(this)));

  set_context_menu_controller(tab_context_menu_controller_.get());

  // 临时添加拖拽控制器
  set_drag_controller(tabbed_pane_);
}

AlohaTabbedPaneTab::~AlohaTabbedPaneTab() = default;

void AlohaTabbedPaneTab::SetSelected(bool selected) {
  contents_->SetVisible(selected);
  contents_->parent()->InvalidateLayout();
  SetState(selected ? State::kActive : State::kInactive);
#if BUILDFLAG(IS_MAC)
  SetFocusBehavior(selected ? FocusBehavior::ACCESSIBLE_ONLY
                            : FocusBehavior::NEVER);
#else
  SetFocusBehavior(selected ? FocusBehavior::ALWAYS : FocusBehavior::NEVER);
#endif
}

const std::u16string& AlohaTabbedPaneTab::GetTitleText() const {
  return title_->GetText();
}

void AlohaTabbedPaneTab::SetTitleText(const std::u16string& text) {
  title_->SetText(text);
  UpdatePreferredTitleWidth();
  PreferredSizeChanged();
}

bool AlohaTabbedPaneTab::OnMousePressed(const ui::MouseEvent& event) {
  if (GetEnabled() && event.IsOnlyLeftMouseButton()) {
    tabbed_pane_->SelectTab(this);
  }

  return true;
}

bool AlohaTabbedPaneTab::OnMouseDragged(const ui::MouseEvent& event) {
  LOG(INFO) << "OnMouseDragged";
  return true;
}

void AlohaTabbedPaneTab::OnMouseEntered(const ui::MouseEvent& event) {
  if (static_cast<uint32_t>(property_) |
      static_cast<uint32_t>(Property::kClosable)) {
    close_button_->SetVisible(true);
  }
  if (selected()) {
    SetState(State::kActive);
  } else {
    // 从inactive 状态切换到 hovered 状态
    SetState(State::kHovered);
  }
}

void AlohaTabbedPaneTab::OnMouseExited(const ui::MouseEvent& event) {
  if (static_cast<uint32_t>(property_) |
      static_cast<uint32_t>(Property::kClosable)) {
    close_button_->SetVisible(false);
  }
  if (selected()) {
    SetState(State::kActive);
  } else {
    // 从hovered 状态切换到 inactive 状态
    SetState(State::kInactive);
  }
}

void AlohaTabbedPaneTab::OnGestureEvent(ui::GestureEvent* event) {
  switch (event->type()) {
    case ui::EventType::kGestureTapDown:
    case ui::EventType::kGestureTap:
      // SelectTab also sets the right tab color.
      tabbed_pane_->SelectTab(this);
      break;
    case ui::EventType::kGestureTapCancel:
      SetState(selected() ? State::kActive : State::kInactive);
      break;
    default:
      break;
  }
  event->SetHandled();
}

gfx::Size AlohaTabbedPaneTab::CalculatePreferredSize(
    const views::SizeBounds& available_size) const {
  // int min_width = preferred_title_width_ + GetInsets().width();

  auto layout_preffered_size =
      views::View::CalculatePreferredSize(available_size);
  return gfx::Size(layout_preffered_size.width(), 32);
}

bool AlohaTabbedPaneTab::HandleAccessibleAction(
    const ui::AXActionData& action_data) {
  // If the assistive tool sends kSetSelection, handle it like kDoDefault.
  // These generate a click event handled in AlohaTabbedPaneTab::OnMousePressed.
  ui::AXActionData action_data_copy(action_data);
  if (action_data.action == ax::mojom::Action::kSetSelection) {
    action_data_copy.action = ax::mojom::Action::kDoDefault;
  }
  return View::HandleAccessibleAction(action_data_copy);
}

void AlohaTabbedPaneTab::OnFocus() {
  // Do not draw focus ring in kHighlight mode.
  if (tabbed_pane_->GetStyle() != AlohaTabbedPane::TabStripStyle::kHighlight) {
    // Maintain the current Insets with CreatePaddedBorder.
    int border_size = 2;
    SetBorder(CreatePaddedBorder(
        views::CreateSolidBorder(
            border_size,
            GetColorProvider()->GetColor(ui::kColorFocusableBorderFocused)),
        GetInsets() - gfx::Insets(border_size)));
  }

  SchedulePaint();
}

void AlohaTabbedPaneTab::OnBlur() {
  // Do not draw focus ring in kHighlight mode.
  if (tabbed_pane_->GetStyle() != AlohaTabbedPane::TabStripStyle::kHighlight) {
    SetBorder(views::CreateEmptyBorder(GetInsets()));
  }
  SchedulePaint();
}

bool AlohaTabbedPaneTab::OnKeyPressed(const ui::KeyEvent& event) {
  const ui::KeyboardCode key = event.key_code();
  if (tabbed_pane_->GetOrientation() ==
      AlohaTabbedPane::Orientation::kHorizontal) {
    // Use left and right arrows to navigate tabs in horizontal orientation.
    int delta = key == ui::VKEY_RIGHT ? 1 : -1;
    if (base::i18n::IsRTL()) {
      delta = key == ui::VKEY_RIGHT ? -1 : 1;
    }
    return (key == ui::VKEY_LEFT || key == ui::VKEY_RIGHT) &&
           tabbed_pane_->MoveSelectionBy(delta);
  }
  // Use up and down arrows to navigate tabs in vertical orientation.
  return (key == ui::VKEY_UP || key == ui::VKEY_DOWN) &&
         tabbed_pane_->MoveSelectionBy(key == ui::VKEY_DOWN ? 1 : -1);
}

void AlohaTabbedPaneTab::OnThemeChanged() {
  View::OnThemeChanged();
  UpdateTitleColor();
}

bool AlohaTabbedPaneTab::CanDrop(const ui::OSExchangeData& data) {
  LOG(INFO) << "CanDrop";
  return true;
}

void AlohaTabbedPaneTab::OnDragEntered(const ui::DropTargetEvent& event) {
  LOG(INFO) << "OnDragEntered";
  if (selected()) {
    SetState(State::kActive);
  } else {
    // 从inactive 状态切换到 hovered 状态
    SetState(State::kHovered);
  }
}

int AlohaTabbedPaneTab::OnDragUpdated(const ui::DropTargetEvent& event) {
  // TODO(yeyun.anton):
  // 判断鼠标位于中心线的上方还是下方，并依此绘制边界线，便于用户感知拖拽的tab的释放位置
  int tab_height = GetBoundsInScreen().height();
  if (event.location().y() <= tab_height / 3) {
    SetState(State::kDraggedOnHalfAbove);
    tabbed_pane_->SetDropTarget(GetWeakPtr(), true);
    LOG(INFO) << "Will Drop Before " << GetTitleText();
  } else if (event.location().y() >= tab_height * 2 / 3) {
    SetState(State::kDraggedOnHalfBelow);
    tabbed_pane_->SetDropTarget(GetWeakPtr(), false);
    LOG(INFO) << "Will Drop After " << GetTitleText();
  } else {
    SetState(State::kHovered);
    tabbed_pane_->SetDropTarget(GetWeakPtr(), true);
    LOG(INFO) << "Will Drop Before" << GetTitleText();
  }

  return static_cast<int>(ui::mojom::DragOperation::kMove);
}

void AlohaTabbedPaneTab::OnDragExited() {
  LOG(INFO) << "OnDragExited " << this;
  if (selected()) {
    SetState(State::kActive);
  } else {
    // 从hovered 状态切换到 inactive 状态
    SetState(State::kInactive);
  }
}

void AlohaTabbedPaneTab::OnDragDone() {
  LOG(INFO) << "OnDragDone" << this;
  tabbed_pane_->HandleDrop(this);
}

void AlohaTabbedPaneTab::OnClose() {
  LOG(INFO) << "OnClose";
  tabbed_pane_->RemoveTab(this);
}

void AlohaTabbedPaneTab::SetState(State state) {
  if (state == state_) {
    return;
  }
  state_ = state;
  OnStateChanged();
  SchedulePaint();
}

void AlohaTabbedPaneTab::OnStateChanged() {
  // Update colors that depend on state if present in a Widget hierarchy.
  if (GetWidget()) {
    UpdateTitleColor();
  }

  // AlohaTabbedPaneTab design spec dictates special handling of font weight for
  // the windows platform when dealing with border style tabs.
  if (tabbed_pane_->GetStyle() == AlohaTabbedPane::TabStripStyle::kHighlight) {
    // Notify assistive tools to update this tab's selected status. The way
    // ChromeOS accessibility is implemented right now, firing almost any event
    // will work, we just need to trigger its state to be refreshed.
    if (state_ == State::kInactive) {
      // TODO(crbug.com/325137417): This view doesn't set the AXCheckedState, it
      // only sets the kSelected attribute. Investigate why this is and whether
      // we should fire another type of event automatically from the
      // accessibility cache.
      NotifyAccessibilityEvent(ax::mojom::Event::kCheckedStateChanged, true);
    }

    // Style the tab text according to the spec for highlight style tabs. We no
    // longer have windows specific bolding of text in this case.
    int font_size_delta = 1;
    gfx::Font::Weight font_weight = (state_ == State::kActive)
                                        ? font_weight = gfx::Font::Weight::BOLD
                                        : gfx::Font::Weight::MEDIUM;
    ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
    title_->SetFontList(
        rb.GetFontListForDetails(ui::ResourceBundle::FontDetails(
            std::string(), font_size_delta, font_weight)));
  } else {
    title_->SetTextStyle(views::style::STYLE_BODY_3_EMPHASIS);
  }

  UpdateAccessibleSelection();
}

void AlohaTabbedPaneTab::OnPaint(gfx::Canvas* canvas) {
  View::OnPaint(canvas);
  // Paints the active tab for the vertical highlighted tabbed pane.
  // if (!selected() ||
  if (tabbed_pane_->GetOrientation() !=
          AlohaTabbedPane::Orientation::kVertical ||
      tabbed_pane_->GetStyle() != AlohaTabbedPane::TabStripStyle::kHighlight) {
    return;
  }
  constexpr SkScalar kRadius = SkIntToScalar(10);
  constexpr SkScalar kDefaultRadii[8] = {
      kRadius, kRadius, kRadius, kRadius, kRadius, kRadius, kRadius, kRadius,
  };
  constexpr int kBorderWidth = 2;
  SkPath path;
  gfx::Rect bounds = GetLocalBounds();

  path.addRoundRect(gfx::RectToSkRect(bounds), kDefaultRadii);
  cc::PaintFlags fill_flags;
  fill_flags.setAntiAlias(true);

  // 根据状态设置颜色
  // TODO: 检查 Focus 下的状态
  fill_flags.setColor(GetColorProvider()->GetColor(
      selected() ? theme::light::kColorLightTabbedPaneTabActivate
      : state_ == State::kHovered
          ? theme::light::kColorLightTabbedPaneTabHovered
          : theme::light::kColorLightTabbedPaneTabInactivate));
  canvas->DrawPath(path, fill_flags);
  // 拖拽时鼠标进入时，根据位置绘制边框
  if (state_ == State::kDraggedOnHalfAbove) {
    // 为上半部分绘制边框
    gfx::Rect top_border = gfx::Rect(0, 0, bounds.width(), kBorderWidth);
    canvas->FillRect(top_border,
                     GetColorProvider()->GetColor(
                         theme::light::kColorLightTabbedPaneTabSepartor));
  } else if (state_ == State::kDraggedOnHalfBelow) {
    // 为下半部分绘制边框
    gfx::Rect bottom_border = gfx::Rect(0, bounds.height() - kBorderWidth,
                                        bounds.width(), kBorderWidth);
    canvas->FillRect(bottom_border,
                     GetColorProvider()->GetColor(
                         theme::light::kColorLightTabbedPaneTabSepartor));
  }
}

void AlohaTabbedPaneTab::UpdatePreferredTitleWidth() {
  // Active and inactive states use different font sizes. Find the largest size
  // and reserve that amount of space.
  const State old_state = state_;
  SetState(State::kActive);
  preferred_title_width_ = title_->GetPreferredSize({}).width();
  SetState(State::kInactive);
  preferred_title_width_ =
      std::max(preferred_title_width_, title_->GetPreferredSize({}).width());
  SetState(old_state);
}

void AlohaTabbedPaneTab::UpdateTitleColor() {
  DCHECK(GetWidget());
  const SkColor font_color = GetColorProvider()->GetColor(
      state_ == State::kActive ? ui::kColorTabForegroundSelected
                               : ui::kColorTabForeground);
  title_->SetEnabledColor(font_color);
}

void AlohaTabbedPaneTab::UpdateAccessibleName() {
  if (title_->GetText().empty()) {
    GetViewAccessibility().SetName(
        std::string(), ax::mojom::NameFrom::kAttributeExplicitlyEmpty);
  } else {
    GetViewAccessibility().SetName(title_->GetText(),
                                   ax::mojom::NameFrom::kContents);
  }
  tabbed_pane_->UpdateAccessibleName();
}

void AlohaTabbedPaneTab::UpdateAccessibleSelection() {
  GetViewAccessibility().SetIsSelected(selected());
}

BEGIN_METADATA(AlohaTabbedPaneTab)
END_METADATA
}  // namespace aloha
