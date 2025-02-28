#include "aloha/browser/ui/menu/setting_menu_model.h"

#include "aloha/resources/vector_icons/vector_icons.h"
#include "base/logging.h"
#include "setting_menu_model.h"

namespace aloha {
SettingMenuModel::SettingMenuModel() : ui::SimpleMenuModel(this) {
  AddItemWithIcon(
      static_cast<int>(SettingMenuModel::Commands::COMMAND_SHOW_DEMO_1),
      u"Inspect this page",
      ui::ImageModel::FromVectorIcon(aloha::kGdSettingsIcon));
  AddItemWithIcon(
      static_cast<int>(SettingMenuModel::Commands::COMMAND_SHOW_DEMO_2),
      u"Inspect this page",
      ui::ImageModel::FromVectorIcon(aloha::kGdSettingsIcon));
  AddItemWithIcon(
      static_cast<int>(SettingMenuModel::Commands::COMMAND_SHOW_DEMO_3),
      u"Inspect this page",
      ui::ImageModel::FromVectorIcon(aloha::kGdSettingsIcon));
  AddItemWithIcon(
      static_cast<int>(SettingMenuModel::Commands::COMMAND_INSPECT_THIS_PAGE),
      u"Inspect this page",
      ui::ImageModel::FromVectorIcon(aloha::kOpenDevtoolsIcon));
}

SettingMenuModel::~SettingMenuModel() = default;

bool SettingMenuModel::IsCommandIdChecked(int command_id) const {
  return false;
}

bool SettingMenuModel::IsCommandIdEnabled(int command_id) const {
  return true;
}
void SettingMenuModel::ExecuteCommand(int command_id, int event_flags) {
  if (command_id >= 0 &&
      command_id < static_cast<int>(Commands::COMMAND_UNKNOWN)) {
    auto iter = command_callbacks_.find(static_cast<Commands>(command_id));
    if (iter != command_callbacks_.end()) {
      iter->second.Run();
    }
  }
}
}  // namespace aloha
