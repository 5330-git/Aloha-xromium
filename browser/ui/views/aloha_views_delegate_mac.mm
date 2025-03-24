// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/ui/views/aloha_views_delegate.h"

#include "ui/views/widget/native_widget_mac.h"

namespace aloha {

AlohaViewsDelegate::AlohaViewsDelegate() = default;

AlohaViewsDelegate::~AlohaViewsDelegate() = default;

void AlohaViewsDelegate::OnBeforeWidgetInit(
    views::Widget::InitParams* params,
    views::internal::NativeWidgetDelegate* delegate) {
  if (params->opacity == views::Widget::InitParams::WindowOpacity::kInferred) {
    params->opacity = use_transparent_windows_
                          ? views::Widget::InitParams::WindowOpacity::kTranslucent
                          : views::Widget::InitParams::WindowOpacity::kOpaque;
  }
  // TODO(tapted): This should return a *Desktop*NativeWidgetMac.
  if (!params->native_widget && use_desktop_native_widgets_)
    params->native_widget = new views::NativeWidgetMac(delegate);
}

ui::ContextFactory* AlohaViewsDelegate::GetContextFactory() {
  return context_factory_;
}

}  // namespace views
