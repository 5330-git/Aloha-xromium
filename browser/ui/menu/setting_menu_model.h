#ifndef _ALOHA_BROWSER_UI_MENU_SETTING_MENU_MODEL_H_
#define _ALOHA_BROWSER_UI_MENU_SETTING_MENU_MODEL_H_ 
#include <unordered_map>

#include "base/containers/flat_map.h"
#include "base/functional/callback_forward.h"
#include "ui/menus/simple_menu_model.h"

namespace aloha {
// src\ui\views\examples\menu_example.cc
class SettingMenuModel : public ui::SimpleMenuModel,
                         public ui::SimpleMenuModel::Delegate {
 public:
  enum class Commands {
    COMMAND_SHOW_DEMO_1,
    COMMAND_SHOW_DEMO_2,
    COMMAND_SHOW_DEMO_3,
    COMMAND_INSPECT_THIS_PAGE,
    // COMMAND 要在上面注册
    COMMAND_UNKNOWN,
  };
  SettingMenuModel();
  ~SettingMenuModel() override;

  SettingMenuModel(const SettingMenuModel&) = delete;
  SettingMenuModel& operator=(const SettingMenuModel&) = delete;

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
}  // namespace aloha

#endif
