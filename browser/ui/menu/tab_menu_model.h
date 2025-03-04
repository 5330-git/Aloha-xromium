#ifndef _ALOHA_BROWSER_UI_MENU_TAB_MENU_MODEL_H_
#define _ALOHA_BROWSER_UI_MENU_TAB_MENU_MODEL_H_
#include <unordered_map>

#include "base/containers/flat_map.h"
#include "base/functional/callback_forward.h"
#include "ui/menus/simple_menu_model.h"
#include "ui/views/context_menu_controller.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/view.h"

namespace aloha {
// src\ui\views\examples\menu_example.cc
class TabMenuModel : public ui::SimpleMenuModel,
                     public ui::SimpleMenuModel::Delegate {
 public:
  enum class Commands {
    COMMAND_COPY_LINK,
    COMMAND_PIN_TAB,
    COMMAND_MOVE_TO_GROUP,
    COMMAND_CLOSE_TAB,
    // COMMAND 要在上面注册
    COMMAND_UNKNOWN,
  };
  TabMenuModel();
  ~TabMenuModel() override;

  TabMenuModel(const TabMenuModel&) = delete;
  TabMenuModel& operator=(const TabMenuModel&) = delete;

  // ui::SimpleMenuModel::Delegate:
  bool IsCommandIdChecked(int command_id) const override;
  bool IsCommandIdEnabled(int command_id) const override;
  void ExecuteCommand(int command_id, int event_flags) override;

  void RegisterCommandCallback(Commands command,
                               base::RepeatingClosure callback) {
    CHECK(command_callbacks_.find(command) == command_callbacks_.end());
    command_callbacks_[command] = callback;
  }

 private:
  std::unordered_map<Commands, base::RepeatingClosure> command_callbacks_;
};

class TabContextMenuController : public views::ContextMenuController {
 public:
  TabContextMenuController();
  ~TabContextMenuController() override;

  TabMenuModel* GetTabMenuModel() const { return tab_menu_model_.get(); }

 protected:
  void ShowContextMenuForViewImpl(
      views::View* source,
      const gfx::Point& point,
      ui::mojom::MenuSourceType source_type) override;

 private:
  std::unique_ptr<TabMenuModel> tab_menu_model_;
  std::unique_ptr<views::MenuRunner> menu_runner_;
};
}  // namespace aloha

#endif
