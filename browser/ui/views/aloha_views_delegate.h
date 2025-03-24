// 替代掉 AlohaContentClientMainParts 对 test_views_delegate 的引用
// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_BROWSER_UI_VIWES_ALOHA_VIEWS_DELEGATE_H_
#define ALOHA_BROWSER_UI_VIWES_ALOHA_VIEWS_DELEGATE_H_

#include <memory>

#include "base/memory/raw_ptr.h"
#include "build/build_config.h"
#include "ui/views/layout/layout_provider.h"
#include "ui/views/views_delegate.h"

namespace aloha {

class AlohaViewsDelegate : public views::ViewsDelegate {
 public:
  AlohaViewsDelegate();

  AlohaViewsDelegate(const AlohaViewsDelegate&) = delete;
  AlohaViewsDelegate& operator=(const AlohaViewsDelegate&) = delete;

  ~AlohaViewsDelegate() override;

  // If set to |true|, forces widgets that do not provide a native widget to use
  // DesktopNativeWidgetAura instead of whatever the default native widget would
  // be. This has no effect on ChromeOS.
  void set_use_desktop_native_widgets(bool desktop) {
    use_desktop_native_widgets_ = desktop;
  }

  void set_use_transparent_windows(bool transparent) {
    use_transparent_windows_ = transparent;
  }

// When running on ChromeOS, NativeWidgetAura requires the parent and/or context
// to be non-null. Some test views provide neither, so we do it here. Normally
// this is done by the browser-specific ViewsDelegate.
#if BUILDFLAG(IS_CHROMEOS)
  void set_context(gfx::NativeWindow context) { context_ = context; }
#endif

#if BUILDFLAG(IS_MAC)
  // Allows tests to provide a ContextFactory via the ViewsDelegate interface.
  void set_context_factory(ui::ContextFactory* context_factory) {
    context_factory_ = context_factory;
  }
#endif

  // For convenience, we create a layout provider by default, but embedders
  // that use their own layout provider subclasses may need to set those classes
  // as the layout providers for their tests.
  void set_layout_provider(std::unique_ptr<views::LayoutProvider> layout_provider) {
    layout_provider_.swap(layout_provider);
  }

  // ViewsDelegate:
#if BUILDFLAG(IS_WIN)
  HICON GetSmallWindowIcon() const override;
#endif
  void OnBeforeWidgetInit(views::Widget::InitParams* params,
                          views::internal::NativeWidgetDelegate* delegate) override;
#if BUILDFLAG(IS_MAC)
  ui::ContextFactory* GetContextFactory() override;
#endif

 private:
#if BUILDFLAG(IS_MAC)
  raw_ptr<ui::ContextFactory> context_factory_ = nullptr;
#endif
  bool use_desktop_native_widgets_ = false;
  bool use_transparent_windows_ = false;
  std::unique_ptr<views::LayoutProvider> layout_provider_ =
      std::make_unique<views::LayoutProvider>();
#if BUILDFLAG(IS_CHROMEOS)
  gfx::NativeWindow context_ = gfx::NativeWindow();
#endif
};

}  // namespace views

#endif  // UI_VIEWS_TEST_TEST_VIEWS_DELEGATE_H_
