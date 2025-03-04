// // Copyright 2012 The Chromium Authors
// // Use of this source code is governed by a BSD-style license that can be
// // found in the LICENSE file.
#include "aloha/browser/ui/views/controls/tabbed_pane/tabbed_pane.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include "aloha/browser/ui/color/color_ids.h"
#include "aloha/browser/ui/menu/tab_menu_model.h"
#include "aloha/browser/ui/views/controls/tabbed_pane/tabbed_pane_tab.h"
#include "aloha/browser/ui/views/controls/tabbed_pane/tabbed_pane_tab_strip.h"
#include "aloha/grit/aloha_resources.h"
#include "aloha/resources/vector_icons/vector_icons.h"
#include "base/check_op.h"
#include "base/functional/bind.h"
#include "base/i18n/rtl.h"
#include "base/memory/weak_ptr.h"
#include "build/build_config.h"
#include "cc/paint/paint_flags.h"
#include "third_party/skia/include/core/SkPath.h"
#include "ui/accessibility/ax_action_data.h"
#include "ui/base/default_style.h"
#include "ui/base/dragdrop/mojom/drag_drop_types.mojom-shared.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/base/mojom/menu_source_type.mojom-shared.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/color/color_id.h"
#include "ui/color/color_provider.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "ui/gfx/animation/tween.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/color_palette.h"
#include "ui/gfx/font_list.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/skia_conversions.h"
#include "ui/views/accessibility/view_accessibility.h"
#include "ui/views/animation/ink_drop.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/highlight_path_generator.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/resize_area.h"
#include "ui/views/controls/scroll_view.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane_listener.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/box_layout_view.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/flex_layout.h"
#include "ui/views/layout/layout_manager.h"
#include "ui/views/layout/layout_provider.h"
#include "ui/views/style/typography.h"
#include "ui/views/view.h"
#include "ui/views/view_class_properties.h"
#include "ui/views/widget/widget.h"

namespace aloha {

AlohaTabbedPane::AlohaTabbedPane(AlohaTabbedPane::Orientation orientation,
                                 AlohaTabbedPane::TabStripStyle style,
                                 bool scrollable) {
  DCHECK(orientation != AlohaTabbedPane::Orientation::kHorizontal ||
         style != AlohaTabbedPane::TabStripStyle::kHighlight);
  SetBackground(
      views::CreateThemedSolidBackground(ui::kColorSysHeaderContainer));
  if (orientation == AlohaTabbedPane::Orientation::kHorizontal) {
    SetOrientation(views::LayoutOrientation::kVertical);
  }

  auto tab_strip =
      std::make_unique<AlohaTabbedPaneTabStrip>(orientation, style);

  if (scrollable) {
    scroll_view_ = AddChildView(std::make_unique<views::ScrollView>(
        views::ScrollView::ScrollWithLayers::kEnabled));
    scroll_view_->SetBackgroundThemeColorId(ui::kColorSysHeaderContainer);
    tab_strip_ = tab_strip.get();
    scroll_view_->SetContents(std::move(tab_strip));
    scroll_view_->ClipHeightTo(0, 0);
  } else {
    tab_strip_ = AddChildView(std::move(tab_strip));
  }
  contents_container_ = AddChildView(std::make_unique<View>());
  contents_container_->SetProperty(
      views::kFlexBehaviorKey,
      views::FlexSpecification(views::MinimumFlexSizeRule::kScaleToZero,
                               views::MaximumFlexSizeRule::kUnbounded));
  contents_container_->SetLayoutManager(std::make_unique<views::FillLayout>());

  // Support navigating tabs by Ctrl+AlohaTabbedPaneTab and
  // Ctrl+Shift+AlohaTabbedPaneTab.
  AddAccelerator(
      ui::Accelerator(ui::VKEY_TAB, ui::EF_CONTROL_DOWN | ui::EF_SHIFT_DOWN));
  AddAccelerator(ui::Accelerator(ui::VKEY_TAB, ui::EF_CONTROL_DOWN));
  GetViewAccessibility().SetRole(ax::mojom::Role::kTabList);
  UpdateAccessibleName();
}

AlohaTabbedPane::~AlohaTabbedPane() = default;

size_t AlohaTabbedPane::GetSelectedTabIndex() const {
  return tab_strip_->GetSelectedTabIndex();
}

size_t AlohaTabbedPane::GetTabCount() const {
  DCHECK_EQ(tab_strip_->GetTabsContainer()->children().size(),
            GetContentsContainer()->children().size())
      << "Tab count mismatch : "
      << tab_strip_->GetTabsContainer()->children().size() << " "
      << contents_container_->children().size();
  return contents_container_->children().size();
}

void AlohaTabbedPane::AddTabInternal(size_t index,
                                     const std::u16string& title,
                                     std::unique_ptr<View> contents) {
  DCHECK_LE(index, GetTabCount());
  contents->SetVisible(false);
  contents->GetViewAccessibility().SetRole(ax::mojom::Role::kTabPanel);
  if (!title.empty()) {
    contents->GetViewAccessibility().SetName(title,
                                             ax::mojom::NameFrom::kAttribute);
  }

  tab_strip_->GetTabsContainer()->AddChildViewAt(
      std::make_unique<AlohaTabbedPaneTab>(this, title, contents.get()), index);
  contents_container_->AddChildViewAt(std::move(contents), index);
  if (!GetSelectedTab()) {
    SelectTabAt(index);
  }

  PreferredSizeChanged();
}

void AlohaTabbedPane::SelectTab(AlohaTabbedPaneTab* new_selected_tab,
                                bool animate) {
  AlohaTabbedPaneTab* old_selected_tab = tab_strip_->GetSelectedTab();
  if (old_selected_tab == new_selected_tab) {
    return;
  }

  new_selected_tab->SetSelected(true);
  if (old_selected_tab) {
    if (old_selected_tab->HasFocus()) {
      new_selected_tab->RequestFocus();
    }
    old_selected_tab->SetSelected(false);
    tab_strip_->OnSelectedTabChanged(old_selected_tab, new_selected_tab,
                                     animate);

    NotifyAccessibilityEvent(ax::mojom::Event::kSelectedChildrenChanged, true);
  }

  UpdateAccessibleName();
  tab_strip_->SchedulePaint();

  views::FocusManager* focus_manager =
      new_selected_tab->contents()->GetFocusManager();
  if (focus_manager) {
    const View* focused_view = focus_manager->GetFocusedView();
    if (focused_view && contents_container_->Contains(focused_view) &&
        !new_selected_tab->contents()->Contains(focused_view)) {
      focus_manager->SetFocusedView(new_selected_tab->contents());
    }
  }

  if (listener()) {
    listener()->TabSelectedAt(base::checked_cast<int>(
        tab_strip_->GetTabsContainer()->GetIndexOf(new_selected_tab).value()));
  }
}

void AlohaTabbedPane::SelectTabAt(size_t index, bool animate) {
  AlohaTabbedPaneTab* tab = tab_strip_->GetTabAtIndex(index);
  if (tab) {
    SelectTab(tab, animate);
  }
}

views::ScrollView* AlohaTabbedPane::GetScrollView() {
  return scroll_view_;
}

AlohaTabbedPane::Orientation AlohaTabbedPane::GetOrientation() const {
  return tab_strip_->GetOrientation();
}

AlohaTabbedPane::TabStripStyle AlohaTabbedPane::GetStyle() const {
  return tab_strip_->GetStyle();
}

AlohaTabbedPaneTab* AlohaTabbedPane::GetTabAt(size_t index) {
  return tab_strip_->GetTabAtIndex(index);
}

void AlohaTabbedPane::RemoveTabAt(size_t index) {
  // 在 ui\views\controls\tabbed_pane\tabbed_pane.h 基础上新增的接口

  DCHECK_LE(index, GetTabCount());
  // 当当前对应的tab被删除后应该将选中的 tab 移动到前一个tab
  if (index > 0) {
    SelectTabAt(index - 1, false);
  } else if (index == 0 && GetTabCount() > 1) {
    SelectTabAt(1, false);
  } else {
    // 没有 tab 主动关闭窗口
    GetWidget()->Close();
  }

  AlohaTabbedPaneTab* tab_to_be_delete = tab_strip_->GetTabAtIndex(index);
  views::View* content_of_tab = tab_to_be_delete->contents();

  // 使用 RemoveChildViewT 接口，这样所有权可以转移出来销毁
  contents_container_->RemoveChildViewT(content_of_tab);
  tab_strip_->GetTabsContainer()->RemoveChildViewT(tab_to_be_delete);

  // 刷新
  PreferredSizeChanged();
}

void AlohaTabbedPane::RemoveTab(AlohaTabbedPaneTab* tab) {
  // 在 ui\views\controls\tabbed_pane\tabbed_pane.h 基础上新增的接口
  if (!tab) {
    return;
  }
  for (size_t i = 0; i < GetTabCount(); i++) {
    if (tab == tab_strip_->GetTabAtIndex(i)) {
      RemoveTabAt(i);
      break;
    }
  }
}

// DragController
void AlohaTabbedPane::WriteDragDataForView(views::View* sender,
                                           const gfx::Point& press_pt,
                                           ui::OSExchangeData* data) {
  // TODO(yeyun.anton) 搜索到对应的tab，并将URL写入到剪切板中
  LOG(INFO) << "WriteDragDataForView";
}

int AlohaTabbedPane::GetDragOperationsForView(views::View* sender,
                                              const gfx::Point& p) {
  LOG(INFO) << "GetDragOperationsForView";
  return static_cast<int>(ui::mojom::DragOperation::kMove);
}

bool AlohaTabbedPane::CanStartDragForView(views::View* sender,
                                          const gfx::Point& press_pt,
                                          const gfx::Point& p) {
  // TODO(yeyun.anton) 判断 sender 是不是当前TabbedPane 管理的标签
  for (size_t i = 0; i < GetTabCount(); i++) {
    if (tab_strip_->GetTabAtIndex(i) == sender) {
      return true;
    }
  }
  LOG(INFO) << "CanStartDragForView";
  return false;
}

void AlohaTabbedPane::OnWillStartDragForView(views::View* dragged_view) {
  LOG(INFO) << "OnWillStartDragForView";
}

void AlohaTabbedPane::SetDropTarget(
    base::WeakPtr<AlohaTabbedPaneTab> target_tab,
    bool drop_before) {
  drop_target_ = target_tab;
  drop_before_ = drop_before;
}

void AlohaTabbedPane::HandleDrop(AlohaTabbedPaneTab* dropped_tab) {
  LOG(INFO) << "HandleDrop";
  // 重新排序(临时方案)
  // 需要分 dropped_tab 是在 drop_target_ 之前还是之后
  size_t dropped_tab_original_index = GetTabCount() - 1;
  if (drop_target_) {
    for (size_t i = 0; i < GetTabCount(); i++) {
      if (tab_strip_->GetTabAtIndex(i) == drop_target_.get()) {
        if (dropped_tab_original_index < i) {
          if (drop_before_) {
            tab_strip_->GetTabsContainer()->ReorderChildView(dropped_tab,
                                                             i - 1);
          } else {
            tab_strip_->GetTabsContainer()->ReorderChildView(dropped_tab, i);
          }
        } else {
          if (drop_before_) {
            tab_strip_->GetTabsContainer()->ReorderChildView(dropped_tab, i);
          } else {
            tab_strip_->GetTabsContainer()->ReorderChildView(dropped_tab,
                                                             i + 1);
          }
        }

        tab_strip_->SchedulePaint();
        break;
      } else if (tab_strip_->GetTabAtIndex(i) == dropped_tab) {
        dropped_tab_original_index = i;
      }
    }
    // 恢复 drop_target_ 的状态
    drop_target_->SetState(AlohaTabbedPaneTab::State::kInactive);
  }
}

AlohaTabbedPaneTab* AlohaTabbedPane::GetSelectedTab() {
  return tab_strip_->GetSelectedTab();
}

views::View* AlohaTabbedPane::GetSelectedTabContentView() {
  return GetSelectedTab() ? GetSelectedTab()->contents() : nullptr;
}

bool AlohaTabbedPane::MoveSelectionBy(int delta) {
  if (contents_container_->children().size() <= 1) {
    return false;
  }
  SelectTab(tab_strip_->GetTabAtDeltaFromSelected(delta));
  return true;
}

gfx::Size AlohaTabbedPane::CalculatePreferredSize(
    const views::SizeBounds& available_size) const {
  // In horizontal mode, use the preferred size as determined by the largest
  // child or the minimum size necessary to display the tab titles, whichever is
  // larger.
  if (GetOrientation() == AlohaTabbedPane::Orientation::kHorizontal) {
    return FlexLayoutView::CalculatePreferredSize(available_size);
  }

  // In vertical mode, Tabstrips don't require any minimum space along their
  // main axis, and can shrink all the way to zero size.
  const gfx::Size size =
      GetLayoutManager()->GetPreferredSize(this, available_size);
  return gfx::Size(size.width(),
                   contents_container_->GetHeightForWidth(size.width()));
}

bool AlohaTabbedPane::AcceleratorPressed(const ui::Accelerator& accelerator) {
  // Handle Ctrl+AlohaTabbedPaneTab and Ctrl+Shift+AlohaTabbedPaneTab navigation
  // of pages.
  DCHECK(accelerator.key_code() == ui::VKEY_TAB && accelerator.IsCtrlDown());
  return MoveSelectionBy(accelerator.IsShiftDown() ? -1 : 1);
}

void AlohaTabbedPane::UpdateAccessibleName() {
  const AlohaTabbedPaneTab* const selected_tab = GetSelectedTab();

  if (selected_tab) {
    GetViewAccessibility().SetName(selected_tab->GetTitleText());
  } else {
    GetViewAccessibility().RemoveName();
  }
}

BEGIN_METADATA(AlohaTabbedPane)
END_METADATA

}  // namespace aloha

DEFINE_ENUM_CONVERTERS(aloha::AlohaTabbedPane::Orientation,
                       {aloha::AlohaTabbedPane::Orientation::kHorizontal,
                        u"HORIZONTAL"},
                       {aloha::AlohaTabbedPane::Orientation::kVertical,
                        u"VERTICAL"})

DEFINE_ENUM_CONVERTERS(aloha::AlohaTabbedPane::TabStripStyle,
                       {aloha::AlohaTabbedPane::TabStripStyle::kBorder,
                        u"BORDER"},
                       {aloha::AlohaTabbedPane::TabStripStyle::kHighlight,
                        u"HIGHLIGHT"})
