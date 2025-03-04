#include "aloha/browser/ui/menu/tab_menu_model.h"

#include "aloha/resources/vector_icons/vector_icons.h"
#include "base/logging.h"
#include "setting_menu_model.h"
#include "ui/color/color_id.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/controls/menu/menu_types.h"

namespace aloha {
TabMenuModel::TabMenuModel() : ui::SimpleMenuModel(this) {
  int icon_size = 18;
  AddItemWithIcon(static_cast<int>(Commands::COMMAND_COPY_LINK), u"Copy link",
                  ui::ImageModel::FromVectorIcon(aloha::kLinkIcon,
                                                 ui::kColorIcon, icon_size));
  AddItemWithIcon(static_cast<int>(Commands::COMMAND_PIN_TAB), u"Pin tab",
                  ui::ImageModel::FromVectorIcon(aloha::kPinIcon,
                                                 ui::kColorIcon, icon_size));
  AddItemWithIcon(static_cast<int>(Commands::COMMAND_MOVE_TO_GROUP),
                  u"Move to group",
                  ui::ImageModel::FromVectorIcon(aloha::kMoveToGroupIcon,
                                                 ui::kColorIcon, icon_size));
  AddItemWithIcon(static_cast<int>(Commands::COMMAND_CLOSE_TAB), u"Close tab",
                  ui::ImageModel::FromVectorIcon(aloha::kEcheCloseIcon,
                                                 ui::kColorIcon, icon_size));
}

TabMenuModel::~TabMenuModel() = default;

bool TabMenuModel::IsCommandIdChecked(int command_id) const {
  return false;
}

bool TabMenuModel::IsCommandIdEnabled(int command_id) const {
  return true;
}
void TabMenuModel::ExecuteCommand(int command_id, int event_flags) {
  if (command_id >= 0 &&
      command_id < static_cast<int>(Commands::COMMAND_UNKNOWN)) {
    auto iter = command_callbacks_.find(static_cast<Commands>(command_id));
    if (iter != command_callbacks_.end()) {
      iter->second.Run();
    }
  }
}

TabContextMenuController::TabContextMenuController()
    : tab_menu_model_(std::make_unique<TabMenuModel>()),
      menu_runner_(std::make_unique<views::MenuRunner>(
          tab_menu_model_.get(),
          views::MenuRunner::HAS_MNEMONICS)) {}

TabContextMenuController::~TabContextMenuController() = default;

// 参考 chrome\browser\ui\views\tabs\browser_tab_strip_controller.cc
void TabContextMenuController::ShowContextMenuForViewImpl(
    views::View* source,
    const gfx::Point& point,
    ui::mojom::MenuSourceType source_type) {
  menu_runner_->RunMenuAt(source->GetWidget(), nullptr,
                          gfx::Rect(point, gfx::Size()),
                          views::MenuAnchorPosition::kTopLeft, source_type);
}
}  // namespace aloha
