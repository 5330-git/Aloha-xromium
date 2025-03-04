#ifndef _ALOHA_BROWSER_UI_COLOR_COLOR_IDS_H_
#define _ALOHA_BROWSER_UI_COLOR_COLOR_IDS_H_
#include "ui/color/color_id.h"
namespace aloha {
namespace theme {
namespace light {
constexpr auto kColorLightBrowserBackground = ui::kColorSysHeaderContainer;
constexpr auto kColorLightTabbedPaneBackground = kColorLightBrowserBackground;
constexpr auto kColorLightTabbedPaneTabInactivate = kColorLightTabbedPaneBackground;
constexpr auto kColorLightTabbedPaneTabHovered= ui::kColorMenuItemBackgroundAlertedTarget;
constexpr auto kColorLightTabbedPaneTabActivate = ui::kColorTableBackgroundSelectedFocused;
constexpr auto kColorLightTabbedPaneTabSepartor = ui::kColorFocusableBorderFocused;

}  // namespace light
}  // namespace theme
}  // namespace aloha
#endif
