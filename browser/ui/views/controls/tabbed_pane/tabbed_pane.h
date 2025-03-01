// // 基于 ui\views\controls\tabbed_pane\tabbed_pane.h 进行改动

#ifndef ALOHA_UI_VIEWS_CONTROLS_TABBED_PANE_TABBED_PANE_H_
#define ALOHA_UI_VIEWS_CONTROLS_TABBED_PANE_TABBED_PANE_H_

#include <memory>
#include <string>
#include <utility>

#include "base/callback_list.h"
#include "base/memory/raw_ptr.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/gfx/animation/animation_delegate.h"
#include "ui/gfx/animation/linear_animation.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane_listener.h"
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
class AlohaTabbedPane : public views::FlexLayoutView {
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
  views::View* GetContents() { return contents_; }

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
  raw_ptr<View> contents_ = nullptr;

  // The scroll view containing the tab strip, if |scrollable| is specified on
  // creation.
  raw_ptr<views::ScrollView> scroll_view_ = nullptr;
};

// The tab view shown in the tab strip.
class AlohaTabbedPaneTab : public views::View {
  METADATA_HEADER(AlohaTabbedPaneTab, views::View)

 public:
  AlohaTabbedPaneTab(AlohaTabbedPane* tabbed_pane,
                     const std::u16string& title,
                     View* contents);

  AlohaTabbedPaneTab(const AlohaTabbedPaneTab&) = delete;
  AlohaTabbedPaneTab& operator=(const AlohaTabbedPaneTab&) = delete;

  ~AlohaTabbedPaneTab() override;

  views::View* contents() const { return contents_; }

  void SetContents(View* contents) { contents_ = contents; }

  bool selected() const { return contents_->GetVisible(); }
  void SetSelected(bool selected);

  const std::u16string& GetTitleText() const;
  void SetTitleText(const std::u16string& text);

  // Overridden from View:
  bool OnMousePressed(const ui::MouseEvent& event) override;
  void OnMouseEntered(const ui::MouseEvent& event) override;
  void OnMouseExited(const ui::MouseEvent& event) override;
  void OnGestureEvent(ui::GestureEvent* event) override;
  gfx::Size CalculatePreferredSize(
      const views::SizeBounds& available_size) const override;
  bool HandleAccessibleAction(const ui::AXActionData& action_data) override;
  void OnFocus() override;
  void OnBlur() override;
  bool OnKeyPressed(const ui::KeyEvent& event) override;
  void OnThemeChanged() override;

 private:
  enum class State {
    kInactive,
    kActive,
    kHovered,
  };

  void SetState(State state);

  // Called whenever |state_| changes.
  void OnStateChanged();

  // views::View:
  void OnPaint(gfx::Canvas* canvas) override;

  void UpdatePreferredTitleWidth();
  void UpdateTitleColor();

  void UpdateAccessibleName();
  void UpdateAccessibleSelection();

  raw_ptr<AlohaTabbedPane> tabbed_pane_;
  raw_ptr<views::Label> title_ = nullptr;
  int preferred_title_width_;
  State state_ = State::kActive;
  // The content view associated with this tab.
  raw_ptr<View> contents_;

  base::CallbackListSubscription title_text_changed_callback_;
};

// The tab strip shown above/left of the tab contents.
class AlohaTabbedPaneTabStrip : public views::View,
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

 protected:
  // View:
  void OnPaintBorder(gfx::Canvas* canvas) override;

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
