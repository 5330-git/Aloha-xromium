// 基于 ui\views\controls\tabbed_pane\tabbed_pane.h 拆分和改动

#ifndef ALOHA_UI_VIEWS_CONTROLS_TABBED_PANE_TABBED_PANE_H_
#define ALOHA_UI_VIEWS_CONTROLS_TABBED_PANE_TABBED_PANE_H_

#include <memory>
#include <string>
#include <utility>

#include "aloha/browser/ui/menu/tab_menu_model.h"
#include "base/callback_list.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/gfx/animation/animation_delegate.h"
#include "ui/gfx/animation/linear_animation.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/controls/resize_area_delegate.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane_listener.h"
#include "ui/views/drag_controller.h"
#include "ui/views/layout/flex_layout_view.h"
#include "ui/views/metadata/view_factory.h"
#include "ui/views/view.h"

namespace aloha {

// class Label;
class AlohaTabbedPaneTab;
// class TabbedPaneListener;
class AlohaTabbedPaneTabStrip;

// AlohaTabbedPane is a view that shows tabs. When the user clicks on a tab, the
// associated view is displayed.
// Support for horizontal-highlight and vertical-border modes is limited and
// may require additional polish.
class AlohaTabbedPane : public views::FlexLayoutView,
                        public views::DragController {
  METADATA_HEADER(AlohaTabbedPane, FlexLayoutView)

 public:
  // The orientation of the tab alignment.
  enum class Orientation {
    kHorizontal,
    kVertical,
  };

  // The style of the tab strip.
  enum class TabStripStyle {
    kBorder,     // Draw border around the selected tab.
    kHighlight,  // Highlight background and text of the selected tab.
  };

  explicit AlohaTabbedPane(Orientation orientation = Orientation::kHorizontal,
                           TabStripStyle style = TabStripStyle::kBorder,
                           bool scrollable = false);

  AlohaTabbedPane(const AlohaTabbedPane&) = delete;
  AlohaTabbedPane& operator=(const AlohaTabbedPane&) = delete;

  ~AlohaTabbedPane() override;

  views::TabbedPaneListener* listener() const { return listener_; }
  void set_listener(views::TabbedPaneListener* listener) {
    listener_ = listener;
  }

  // Returns the index of the currently selected tab, or
  // TabStrip::kNoSelectedTab if no tab is selected.
  size_t GetSelectedTabIndex() const;

  // Returns the number of tabs.
  size_t GetTabCount() const;

  // Adds a new tab at the end of this AlohaTabbedPane with the specified
  // |title|. |contents| is the view displayed when the tab is selected and is
  // owned by the AlohaTabbedPane.
  template <typename T>
  T* AddTab(const std::u16string& title, std::unique_ptr<T> contents) {
    return AddTabAtIndex(GetTabCount(), title, std::move(contents));
  }

  // Adds a new tab at |index| with |title|. |contents| is the view displayed
  // when the tab is selected and is owned by the AlohaTabbedPane. If the tabbed
  // pane is currently empty, the new tab is selected.
  template <typename T>
  T* AddTabAtIndex(size_t index,
                   const std::u16string& title,
                   std::unique_ptr<T> contents) {
    T* result = contents.get();
    AddTabInternal(index, title, std::move(contents));
    return result;
  }

  // Selects the tab at |index|, which must be valid.
  void SelectTabAt(size_t index, bool animate = true);

  // Selects |tab| (the tabstrip view, not its content) if it is valid.
  void SelectTab(AlohaTabbedPaneTab* tab, bool animate = true);

  // Gets the scroll view containing the tab strip, if it exists
  views::ScrollView* GetScrollView();

  // Gets the orientation of the tab alignment.
  Orientation GetOrientation() const;

  // Gets the style of the tab strip.
  TabStripStyle GetStyle() const;

  // Returns the tab at the given index.
  AlohaTabbedPaneTab* GetTabAt(size_t index);

  void RemoveTabAt(size_t index);

  void RemoveTab(AlohaTabbedPaneTab* tab);

  // std::unique_ptr<views::View> RemoveAndGetContentsAt(size_t index);

  AlohaTabbedPaneTabStrip* GetTabStrip() { return tab_strip_; }
  const views::View* GetContentsContainer() const { return contents_container_; }
  views::View* GetContentsContainer() { return contents_container_; }

  // DragController:
  // 将注册到tab上，这样tab上的拖拽事件就可以分发到TabPane上
  void WriteDragDataForView(views::View* sender,
                            const gfx::Point& press_pt,
                            ui::OSExchangeData* data) override;
  int GetDragOperationsForView(views::View* sender,
                               const gfx::Point& p) override;
  bool CanStartDragForView(views::View* sender,
                           const gfx::Point& press_pt,
                           const gfx::Point& p) override;
  void OnWillStartDragForView(views::View* dragged_view) override;

  // Drag 相关，自定义
  // 设置
  void SetDropTarget(base::WeakPtr<AlohaTabbedPaneTab> tab, bool before = true);
  base::WeakPtr<AlohaTabbedPaneTab> GetDropTarget();
  void HandleDrop(AlohaTabbedPaneTab* dropped_tab);


 private:
  friend class FocusTraversalTest;
  friend class AlohaTabbedPaneTab;
  friend class AlohaTabbedPaneTabStrip;

  // Adds a new tab at |index| with |title|. |contents| is the view displayed
  // when the tab is selected and is owned by the AlohaTabbedPane. If the tabbed
  // pane is currently empty, the new tab is selected.
  void AddTabInternal(size_t index,
                      const std::u16string& title,
                      std::unique_ptr<View> contents);

  // Get the AlohaTabbedPaneTab (the tabstrip view, not its content) at the
  // selected index.
  AlohaTabbedPaneTab* GetSelectedTab();

  // Returns the content View of the currently selected AlohaTabbedPaneTab.
  View* GetSelectedTabContentView();

  // Moves the selection by |delta| tabs, where negative delta means leftwards
  // and positive delta means rightwards. Returns whether the selection could be
  // moved by that amount; the only way this can fail is if there is only one
  // tab.
  bool MoveSelectionBy(int delta);

  // View:
  gfx::Size CalculatePreferredSize(
      const views::SizeBounds& available_size) const override;
  bool AcceleratorPressed(const ui::Accelerator& accelerator) override;

  void UpdateAccessibleName();

  // A listener notified when tab selection changes. Weak, not owned.
  raw_ptr<views::TabbedPaneListener> listener_ = nullptr;

  // The tab strip and contents container. The child indices of these members
  // correspond to match each AlohaTabbedPaneTab with its respective content
  // View.
  raw_ptr<AlohaTabbedPaneTabStrip> tab_strip_ = nullptr;
  raw_ptr<View> contents_container_ = nullptr;

  // The scroll view containing the tab strip, if |scrollable| is specified on
  // creation.
  raw_ptr<views::ScrollView> scroll_view_ = nullptr;

  // Tab 拖拽和释放 相关
  base::WeakPtr<AlohaTabbedPaneTab> drop_target_;
  bool drop_before_ = true;
};

BEGIN_VIEW_BUILDER(VIEWS_EXPORT, AlohaTabbedPane, views::FlexLayoutView)
VIEW_BUILDER_METHOD_ALIAS(AddTab,
                          AddTab<views::View>,
                          const std::u16string&,
                          std::unique_ptr<views::View>)
END_VIEW_BUILDER
}  // namespace aloha

DEFINE_VIEW_BUILDER(VIEWS_EXPORT, aloha::AlohaTabbedPane)

#endif  // UI_VIEWS_CONTROLS_TABBED_PANE_TABBED_PANE_H_
