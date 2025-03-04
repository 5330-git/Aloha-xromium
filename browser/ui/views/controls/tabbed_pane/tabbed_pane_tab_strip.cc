#include "aloha/browser/ui/views/controls/tabbed_pane/tabbed_pane_tab_strip.h"

#include <memory>

#include "aloha/browser/ui/color/color_ids.h"
#include "aloha/browser/ui/views/controls/tabbed_pane/tabbed_pane_tab.h"
#include "tabbed_pane_tab_strip.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/views/accessibility/view_accessibility.h"
#include "ui/views/background.h"
#include "ui/views/controls/resize_area.h"
#include "ui/views/controls/resize_area_delegate.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/metadata/view_factory_internal.h"
#include "ui/views/view.h"

namespace aloha {
// static
constexpr size_t AlohaTabbedPaneTabStrip::kNoSelectedTab;

// TODO(yeyun.anton): 迁移
class AlohaResizeStrip : public views::ResizeArea {
  METADATA_HEADER(AlohaResizeStrip, views::View)
 public:
  enum class State { kInactivate, kHovered };
  explicit AlohaResizeStrip(views::ResizeAreaDelegate* delegate)
      : ResizeArea(delegate) {
    SetVisible(true);
    SetBackground(views::CreateThemedSolidBackground(
        theme::light::kColorLightBrowserBackground));
  }

  void OnMouseEntered(const ui::MouseEvent& event) override {
    views::View::OnMouseEntered(event);
    state_ = State::kHovered;
    SchedulePaint();
  }

  void OnMouseExited(const ui::MouseEvent& event) override {
    views::View::OnMouseExited(event);
    state_ = State::kInactivate;
    SchedulePaint();
  }

  void OnPaint(gfx::Canvas* canvas) override {
    views::View::OnPaint(canvas);
    if (state_ == State::kHovered) {
      auto highlight_strip_bounds =
          gfx::Rect(5, 0, 2, GetLocalBounds().height());
      canvas->FillRect(highlight_strip_bounds,
                       GetColorProvider()->GetColor(
                           theme::light::kColorLightTabbedPaneTabSepartor));
    }
  }

  gfx::Size CalculatePreferredSize(
      const views::SizeBounds& available_size) const override {
    return gfx::Size(8, 0);
  }
  State state_;
};

BEGIN_METADATA(AlohaResizeStrip)
END_METADATA

// TODO(yeyun.anton)： 支持 kHorizental 模式
AlohaTabbedPaneTabStrip::AlohaTabbedPaneTabStrip(
    AlohaTabbedPane::Orientation orientation,
    AlohaTabbedPane::TabStripStyle style)
    : orientation_(orientation), style_(style) {
  // 允许子控件的事件可以分发到父控件
  SetNotifyEnterExitOnChild(true);

  std::unique_ptr<views::BoxLayout> layout =
      std::make_unique<views::BoxLayout>();
  layout->SetOrientation(views::BoxLayout::Orientation::kHorizontal);
  layout->set_main_axis_alignment(views::BoxLayout::MainAxisAlignment::kStart);

  if (orientation == AlohaTabbedPane::Orientation::kHorizontal) {
    layout = std::make_unique<views::BoxLayout>(
        views::BoxLayout::Orientation::kHorizontal);
    layout->set_main_axis_alignment(
        views::BoxLayout::MainAxisAlignment::kCenter);
    layout->set_cross_axis_alignment(
        views::BoxLayout::CrossAxisAlignment::kStretch);
    layout->SetDefaultFlex(1);
    SetLayoutManager(std::move(layout));
  } else {
    constexpr auto kEdgePadding = gfx::Insets::TLBR(5, 5, 0, 0);
    constexpr int kTabSpacing = 5;
    auto tabs_container =
        views::Builder<views::BoxLayoutView>()
            .SetOrientation(views::BoxLayout::Orientation::kVertical)
            .SetInsideBorderInsets(kEdgePadding)
            .SetBetweenChildSpacing(kTabSpacing)
            .SetMainAxisAlignment(views::BoxLayout::MainAxisAlignment::kStart)
            .SetCrossAxisAlignment(views::BoxLayout::CrossAxisAlignment::kStart)
            .Build();
    tabs_container_ = AddChildView(std::move(tabs_container));
    // resize_bar
    resize_strip_ = AddChildView(std::make_unique<AlohaResizeStrip>(this));
    auto* box_layout = SetLayoutManager(std::move(layout));
    box_layout->SetFlexForView(tabs_container_, 10);
    box_layout->SetFlexForView(resize_strip_, 0);
  }

  GetViewAccessibility().SetRole(ax::mojom::Role::kNone);

  // These durations are taken from the Paper Tabs source:
  // https://github.com/PolymerElements/paper-tabs/blob/master/paper-tabs.html
  // See |selectionBar.expand| and |selectionBar.contract|.
  expand_animation_->SetDuration(base::Milliseconds(150));
  contract_animation_->SetDuration(base::Milliseconds(180));
}

AlohaTabbedPaneTabStrip::~AlohaTabbedPaneTabStrip() = default;

void AlohaTabbedPaneTabStrip::AnimationProgressed(
    const gfx::Animation* animation) {
  SchedulePaint();
}

void AlohaTabbedPaneTabStrip::AnimationEnded(const gfx::Animation* animation) {
  if (animation == expand_animation_.get()) {
    contract_animation_->Start();
  }
}

void AlohaTabbedPaneTabStrip::OnSelectedTabChanged(AlohaTabbedPaneTab* from_tab,
                                                   AlohaTabbedPaneTab* to_tab,
                                                   bool animate) {
  DCHECK(!from_tab->selected());
  DCHECK(to_tab->selected());
  if (!animate || !GetWidget()) {
    return;
  }

  if (GetOrientation() == AlohaTabbedPane::Orientation::kHorizontal) {
    animating_from_ = {from_tab->GetMirroredX(),
                       from_tab->GetMirroredX() + from_tab->width()};
    animating_to_ = {to_tab->GetMirroredX(),
                     to_tab->GetMirroredX() + to_tab->width()};
  } else {
    animating_from_ = {from_tab->bounds().y(),
                       from_tab->bounds().y() + from_tab->height()};
    animating_to_ = {to_tab->bounds().y(),
                     to_tab->bounds().y() + to_tab->height()};
  }

  contract_animation_->Stop();
  expand_animation_->Start();
}

AlohaTabbedPaneTab* AlohaTabbedPaneTabStrip::GetSelectedTab() const {
  size_t index = GetSelectedTabIndex();
  return index == kNoSelectedTab ? nullptr : GetTabAtIndex(index);
}

AlohaTabbedPaneTab* AlohaTabbedPaneTabStrip::GetTabAtDeltaFromSelected(
    int delta) const {
  const size_t selected_tab_index = GetSelectedTabIndex();
  DCHECK_NE(kNoSelectedTab, selected_tab_index);
  const size_t num_children = tabs_container_->children().size();
  // Clamping |delta| here ensures that even a large negative |delta| will be
  // positive after the addition in the next statement.
  delta %= base::checked_cast<int>(num_children);
  delta += static_cast<int>(num_children);
  return GetTabAtIndex((selected_tab_index + static_cast<size_t>(delta)) %
                       num_children);
}

AlohaTabbedPaneTab* AlohaTabbedPaneTabStrip::GetTabAtIndex(size_t index) const {
  DCHECK_LT(index, tabs_container_->children().size());
  return static_cast<AlohaTabbedPaneTab*>(tabs_container_->children()[index]);
}

size_t AlohaTabbedPaneTabStrip::GetSelectedTabIndex() const {
  for (size_t i = 0; i < tabs_container_->children().size(); ++i) {
    if (GetTabAtIndex(i)->selected()) {
      return i;
    }
  }
  return kNoSelectedTab;
}

AlohaTabbedPane::Orientation AlohaTabbedPaneTabStrip::GetOrientation() const {
  return orientation_;
}

AlohaTabbedPane::TabStripStyle AlohaTabbedPaneTabStrip::GetStyle() const {
  return style_;
}

gfx::Size AlohaTabbedPaneTabStrip::GetMinimumSize() const {
  return gfx::Size(40, 0);
}

gfx::Size AlohaTabbedPaneTabStrip::GetMaximumSize() const {
  auto* widget = GetWidget();
  if (!widget) {
    return gfx::Size();
  }
  return gfx::Size(widget->GetSize().width() / 3, 0);
}
gfx::Size AlohaTabbedPaneTabStrip::CalculatePreferredSize(
    const views::SizeBounds& available_size) const {
  // TabStrip 长度为父控件长度，宽度不超过父控件的1/3

  return gfx::Size(193, 0);
}

void AlohaTabbedPaneTabStrip::OnPaintBorder(gfx::Canvas* canvas) {
  // Do not draw border line in kHighlight mode.
  if (GetStyle() == AlohaTabbedPane::TabStripStyle::kHighlight) {
    return;
  }

  // First, draw the unselected border across the TabStrip's entire width or
  // height, depending on the orientation of the tab alignment. The area
  // underneath or on the right of the selected tab will be overdrawn later.
  const bool is_horizontal =
      GetOrientation() == AlohaTabbedPane::Orientation::kHorizontal;
  int max_cross_axis;
  gfx::Rect rect;
  constexpr int kUnselectedBorderThickness = 1;
  if (is_horizontal) {
    max_cross_axis = tabs_container_->children().front()->bounds().bottom();
    rect = gfx::Rect(0, max_cross_axis - kUnselectedBorderThickness, width(),
                     kUnselectedBorderThickness);
  } else {
    max_cross_axis = width();
    rect = gfx::Rect(max_cross_axis - kUnselectedBorderThickness, 0,
                     kUnselectedBorderThickness, height());
  }
  canvas->FillRect(rect,
                   GetColorProvider()->GetColor(ui::kColorTabContentSeparator));

  AlohaTabbedPaneTab* tab = GetSelectedTab();
  if (!tab) {
    return;
  }

  // Now, figure out the range to draw the selection marker underneath. There
  // are three states here:
  // 1) Expand animation is running: use FAST_OUT_LINEAR_IN to grow the
  //    selection marker until it encompasses both the previously selected tab
  //    and the currently selected tab;
  // 2) Contract animation is running: use LINEAR_OUT_SLOW_IN to shrink the
  //    selection marker until it encompasses only the currently selected tab;
  // 3) No animations running: the selection marker is only under the currently
  //    selected tab.
  int min_main_axis = 0;
  int max_main_axis = 0;
  if (expand_animation_->is_animating()) {
    bool animating_leading = animating_to_.start < animating_from_.start;
    double anim_value = gfx::Tween::CalculateValue(
        gfx::Tween::FAST_OUT_LINEAR_IN, expand_animation_->GetCurrentValue());
    if (animating_leading) {
      min_main_axis = gfx::Tween::IntValueBetween(
          anim_value, animating_from_.start, animating_to_.start);
      max_main_axis = animating_from_.end;
    } else {
      min_main_axis = animating_from_.start;
      max_main_axis = gfx::Tween::IntValueBetween(
          anim_value, animating_from_.end, animating_to_.end);
    }
  } else if (contract_animation_->is_animating()) {
    bool animating_leading = animating_to_.start < animating_from_.start;
    double anim_value = gfx::Tween::CalculateValue(
        gfx::Tween::LINEAR_OUT_SLOW_IN, contract_animation_->GetCurrentValue());
    if (animating_leading) {
      min_main_axis = animating_to_.start;
      max_main_axis = gfx::Tween::IntValueBetween(
          anim_value, animating_from_.end, animating_to_.end);
    } else {
      min_main_axis = gfx::Tween::IntValueBetween(
          anim_value, animating_from_.start, animating_to_.start);
      max_main_axis = animating_to_.end;
    }
  } else if (is_horizontal) {
    min_main_axis = tab->GetMirroredX();
    max_main_axis = min_main_axis + tab->width();
  } else {
    min_main_axis = tab->bounds().y();
    max_main_axis = min_main_axis + tab->height();
  }

  DCHECK_NE(min_main_axis, max_main_axis);
  // Draw over the unselected border from above.
  constexpr int kSelectedBorderThickness = 2;
  rect = gfx::Rect(min_main_axis, max_cross_axis - kSelectedBorderThickness,
                   max_main_axis - min_main_axis, kSelectedBorderThickness);
  if (!is_horizontal) {
    rect.Transpose();
  }
  canvas->FillRect(rect,
                   GetColorProvider()->GetColor(ui::kColorTabBorderSelected));
}

// 参考实现： chrome\browser\ui\views\side_panel\side_panel.cc: SidePanel::OnResize
void AlohaTabbedPaneTabStrip::OnResize(int resize_amount, bool done_resizing) {
  LOG(INFO) << "Tabbed Strip is resizing resize_amount " << resize_amount
            << " done_resizing " << done_resizing;
  if (starting_width_on_resize_ < 0) {
    starting_width_on_resize_ = width();
  }
  int proposed_width = starting_width_on_resize_ + resize_amount;
  if (proposed_width < GetMinimumSize().width()) {
    proposed_width = GetMinimumSize().width();
  } else if (proposed_width > GetMaximumSize().width()) {
    proposed_width = GetMaximumSize().width();
  }
  if (width() != proposed_width) {
    SetPreferredSize(gfx::Size(proposed_width, 0));
  }
  if (done_resizing) {
    starting_width_on_resize_ = -1;
  }
}

BEGIN_METADATA(AlohaTabbedPaneTabStrip)
ADD_READONLY_PROPERTY_METADATA(size_t, SelectedTabIndex)
ADD_READONLY_PROPERTY_METADATA(AlohaTabbedPane::Orientation, Orientation)
ADD_READONLY_PROPERTY_METADATA(AlohaTabbedPane::TabStripStyle, Style)
END_METADATA
}  // namespace aloha
