// 基于 ui\views\controls\tabbed_pane\tabbed_pane.h 拆分和改动

#ifndef ALOHA_UI_VIEWS_CONTROLS_TABBED_PANE_TABBED_PANE_TAB_H_
#define ALOHA_UI_VIEWS_CONTROLS_TABBED_PANE_TABBED_PANE_TAB_H_
#include "aloha/browser/ui/menu/tab_menu_model.h"
#include "aloha/browser/ui/views/controls/tabbed_pane/tabbed_pane.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/web_contents_observer.h"
#include "ui/events/event.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/label.h"
#include "ui/views/view.h"

namespace aloha {

// The tab view shown in the tab strip.
class AlohaTabbedPaneTab : public views::View,
                           public content::WebContentsObserver {
  METADATA_HEADER(AlohaTabbedPaneTab, views::View)

 public:
  enum class Property {
    kClosable = 0b1,
    kPermitOpenMenu = 0b10,
    kDraggable = 0b100,
    kSelectable = 0b1000,
  };
  enum class State {
    kInactive,
    kActive,
    kHovered,
    kDraggedOnHalfAbove,
    kDraggedOnHalfBelow,
  };

  void SetState(State state);
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
  bool OnMouseDragged(const ui::MouseEvent& event) override;
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
  // Drag: 可以参考 `ui\views\view.h` 中的注释
  bool CanDrop(const ui::OSExchangeData& data) override;
  void OnDragEntered(const ui::DropTargetEvent& event) override;
  int OnDragUpdated(const ui::DropTargetEvent& event) override;
  void OnDragExited() override;
  void OnDragDone() override;

  // AlohaTabbedPaneTab:
  void OnClose();
  void SetFavicon(const ui::ImageModel& image_model) {
    if (favicon_) {
      favicon_->SetImage(image_model);
    }
  }

  void AddProperty(Property property) { property_ = property; }
  base::WeakPtr<AlohaTabbedPaneTab> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

 private:
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
  raw_ptr<views::ImageView> favicon_ = nullptr;
  raw_ptr<views::ImageButton> close_button_ = nullptr;

  int preferred_title_width_;
  State state_ = State::kActive;
  Property property_ = Property::kClosable;
  // The content view associated with this tab.
  raw_ptr<View> contents_ = nullptr;

  base::CallbackListSubscription title_text_changed_callback_;

 private:
  // 临时
  std::unique_ptr<TabContextMenuController> tab_context_menu_controller_;
  base::WeakPtrFactory<AlohaTabbedPaneTab> weak_factory_{this};
};
}  // namespace aloha
#endif
