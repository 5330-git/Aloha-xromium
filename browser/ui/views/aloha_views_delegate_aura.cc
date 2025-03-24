// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/ui/views/aloha_views_delegate.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "ui/views/buildflags.h"
#include "ui/views/widget/native_widget_aura.h"


#if BUILDFLAG(ENABLE_DESKTOP_AURA)
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"
#endif  // BUILDFLAG(ENABLE_DESKTOP_AURA)

namespace aloha {

AlohaViewsDelegate::AlohaViewsDelegate() = default;

AlohaViewsDelegate::~AlohaViewsDelegate() = default;

#if BUILDFLAG(IS_WIN)
HICON AlohaViewsDelegate::GetSmallWindowIcon() const {
  return nullptr;
}
#endif

// 以下拷贝自 ui\views\test\desktop_test_views_delegate_aura.cc
// 其它内容拷贝自 ui\views\test\test_views_delegate.h
void AlohaViewsDelegate::OnBeforeWidgetInit(
    views::Widget::InitParams* params,
    views::internal::NativeWidgetDelegate* delegate) {
#if BUILDFLAG(ENABLE_DESKTOP_AURA)
  // If we already have a native_widget, we don't have to try to come
  // up with one.
  if (params->native_widget) {
    return;
  }

  if (params->parent && params->type != views::Widget::InitParams::TYPE_MENU &&
      params->type != views::Widget::InitParams::TYPE_TOOLTIP) {
    params->native_widget = new views::NativeWidgetAura(delegate);
  } else {
    params->native_widget = new views::DesktopNativeWidgetAura(delegate);
  }
#endif
}

}  // namespace aloha
