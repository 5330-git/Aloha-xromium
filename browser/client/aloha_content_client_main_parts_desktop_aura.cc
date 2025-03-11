// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/common/result_codes.h"
#include "content/shell/browser/shell_browser_context.h"
#include "ui/display/screen.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#include "aloha/browser/client/aloha_browser_client.h"
#include "aloha/browser/client/aloha_content_client_main_parts_aura.h"

namespace aloha {

namespace {

class AlohaContentClientMainPartsDesktopAura
    : public AlohaContentClientMainPartsAura {
 public:
  explicit AlohaContentClientMainPartsDesktopAura(
      AlohaBrowserClient* views_content_client);
  AlohaContentClientMainPartsDesktopAura(
      const AlohaContentClientMainPartsDesktopAura&) = delete;
  AlohaContentClientMainPartsDesktopAura& operator=(
      const AlohaContentClientMainPartsDesktopAura&) = delete;
  ~AlohaContentClientMainPartsDesktopAura() override = default;

  // AlohaContentClientMainPartsAura:
  int PreMainMessageLoopRun() override;
  void PostMainMessageLoopRun() override;

 private:
  std::unique_ptr<display::Screen> screen_;
};

AlohaContentClientMainPartsDesktopAura::AlohaContentClientMainPartsDesktopAura(
    AlohaBrowserClient* views_content_client)
    : AlohaContentClientMainPartsAura(views_content_client) {}

int AlohaContentClientMainPartsDesktopAura::PreMainMessageLoopRun() {
  AlohaContentClientMainPartsAura::PreMainMessageLoopRun();

  screen_ = views::CreateDesktopScreen();

  views_content_client()->OnPreMainMessageLoopRun(browser_context(), nullptr);

  return content::RESULT_CODE_NORMAL_EXIT;
}

void AlohaContentClientMainPartsDesktopAura::PostMainMessageLoopRun() {
  screen_.reset();

  AlohaContentClientMainPartsAura::PostMainMessageLoopRun();
}

}  // namespace

// static
std::unique_ptr<AlohaContentClientMainParts>
AlohaContentClientMainParts::Create(AlohaBrowserClient* views_content_client) {
  return std::make_unique<AlohaContentClientMainPartsDesktopAura>(
      views_content_client);
}

}  // namespace aloha
