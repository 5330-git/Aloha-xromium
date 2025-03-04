
// 基于 ui\views\controls\tabbed_pane\tabbed_pane.h 拆分和改动

#ifndef ALOHA_UI_VIEWS_CONTROLS_TABBED_PANE_TABBED_PANE_TAB_STRIP_H_
#define ALOHA_UI_VIEWS_CONTROLS_TABBED_PANE_TABBED_PANE_TAB_STRIP_H_
#include "aloha/browser/ui/views/controls/tabbed_pane/tabbed_pane.h"
#include "base/memory/raw_ptr.h"
#include "ui/gfx/animation/animation_delegate.h"
#include "ui/views/controls/resize_area.h"
#include "ui/views/layout/box_layout_view.h"
#include "ui/views/view.h"

namespace aloha {
class AlohaTabbedPane;
// The tab strip shown above/left of the tab contents.
class AlohaTabbedPaneTabStrip : public views::View,
                                public views::ResizeAreaDelegate,
                                public gfx::AnimationDelegate {
  METADATA_HEADER(AlohaTabbedPaneTabStrip, views::View)

 public:
  // The return value of GetSelectedTabIndex() when no tab is selected.
  static constexpr size_t kNoSelectedTab = static_cast<size_t>(-1);

  AlohaTabbedPaneTabStrip(AlohaTabbedPane::Orientation orientation,
                          AlohaTabbedPane::TabStripStyle style);

  AlohaTabbedPaneTabStrip(const AlohaTabbedPaneTabStrip&) = delete;
  AlohaTabbedPaneTabStrip& operator=(const AlohaTabbedPaneTabStrip&) = delete;

  ~AlohaTabbedPaneTabStrip() override;

  // AnimationDelegate:
  void AnimationProgressed(const gfx::Animation* animation) override;
  void AnimationEnded(const gfx::Animation* animation) override;

  // Called by AlohaTabbedPaneTabStrip when the selected tab changes. This
  // function is only called if |from_tab| is not null, i.e., there was a
  // previously selected tab.
  void OnSelectedTabChanged(AlohaTabbedPaneTab* from_tab,
                            AlohaTabbedPaneTab* to_tab,
                            bool animate = true);

  AlohaTabbedPaneTab* GetSelectedTab() const;
  AlohaTabbedPaneTab* GetTabAtDeltaFromSelected(int delta) const;
  AlohaTabbedPaneTab* GetTabAtIndex(size_t index) const;
  size_t GetSelectedTabIndex() const;

  AlohaTabbedPane::Orientation GetOrientation() const;

  AlohaTabbedPane::TabStripStyle GetStyle() const;
  const views::View* GetTabsContainer() const { return tabs_container_; }
  views::View* GetTabsContainer() { return tabs_container_; }

  // views::View:
  // Size
  gfx::Size GetMinimumSize() const override;
  gfx::Size GetMaximumSize() const override;
  gfx::Size CalculatePreferredSize(
      const views::SizeBounds& available_size) const override;

 protected:
  // View:
  void OnPaintBorder(gfx::Canvas* canvas) override;

  // ResizeAreaDelegate:
  void OnResize(int resize_amount, bool done_resizing) override;

 private:
  struct Coordinates {
    int start, end;
  };

  // The orientation of the tab alignment.
  const AlohaTabbedPane::Orientation orientation_;

  // The style of the tab strip.
  const AlohaTabbedPane::TabStripStyle style_;

  // Animations for expanding and contracting the selection bar. When changing
  // selections, the selection bar first grows to encompass both the old and new
  // selections, then shrinks to encompass only the new selection. The rates of
  // expansion and contraction each follow the cubic bezier curves used in
  // gfx::Tween; see TabStrip::OnPaintBorder for details.
  std::unique_ptr<gfx::LinearAnimation> expand_animation_ =
      std::make_unique<gfx::LinearAnimation>(this);
  std::unique_ptr<gfx::LinearAnimation> contract_animation_ =
      std::make_unique<gfx::LinearAnimation>(this);

  // The x-coordinate ranges of the old selection and the new selection.
  Coordinates animating_from_;
  Coordinates animating_to_;

  // 缩放：
  base::raw_ptr<views::BoxLayoutView> tabs_container_ = nullptr;
  base::raw_ptr<views::ResizeArea> resize_strip_ = nullptr;
  int starting_width_on_resize_ = -1;
};
}  // namespace aloha

#endif
