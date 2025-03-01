#ifndef XX121_
#define XX121_

#include <memory>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/box_layout_view.h"
#include "ui/views/widget/widget.h"

namespace aloha {

class BrowserContentView : public views::View,
                           public content::WebContentsObserver {
  // 动态反射机制
  METADATA_HEADER(BrowserContentView, views::View)
  // AlohaBrowserContentView
 public:
  virtual void Init() = 0;
  virtual void ReloadContents() = 0;
  virtual void NavigateForward() = 0;
  virtual void NavigateBack() = 0;
  virtual void NavigateHome() = 0;
  virtual void CopyLink() = 0;
  virtual void OpenDevtoolsAndInspect() = 0;
  virtual void ShowSettingsMenu() = 0;
  virtual void Close() = 0;
  virtual void SetCanOpenInNewWidget(bool can_open_in_new_tab) = 0;
  virtual bool CanOpenInNewWidget() = 0;
  // 纳入窗口关闭链，在 BrowserContentView 关闭时会触发关联的窗口关闭
  virtual void AddChildWidget(base::WeakPtr<views::Widget> widget) = 0;
  virtual void CloseChildWidgets() = 0;

 protected:
  // 用于定义整个Views各个部分视图的容器
  virtual std::unique_ptr<views::View> GetContentContainerView() = 0;
};
}  // namespace aloha
#endif
