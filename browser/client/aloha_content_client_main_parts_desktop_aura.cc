// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/common/aloha_main_client.h"
#include "aloha/browser/client/aloha_content_client_main_parts_aura.h"
#include "content/public/common/result_codes.h"
#include "ui/display/screen.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"

namespace aloha {

namespace {

class AlohaContentClientMainPartsDesktopAura
    : public AlohaContentClientMainPartsAura {
 public:
  AlohaContentClientMainPartsDesktopAura();

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

AlohaContentClientMainPartsDesktopAura::
    AlohaContentClientMainPartsDesktopAura() = default;

int AlohaContentClientMainPartsDesktopAura::PreMainMessageLoopRun() {
  AlohaContentClientMainPartsAura::PreMainMessageLoopRun();

  screen_ = views::CreateDesktopScreen();

  AlohaMainClient::GetInstance()->OnPreMainMessageLoopRun(browser_context(),
                                                          nullptr);

  return content::RESULT_CODE_NORMAL_EXIT;
}

void AlohaContentClientMainPartsDesktopAura::PostMainMessageLoopRun() {
  screen_.reset();

  AlohaContentClientMainPartsAura::PostMainMessageLoopRun();
}

}  // namespace

// static
std::unique_ptr<AlohaContentClientMainParts>
AlohaContentClientMainParts::Create() {
  return std::make_unique<AlohaContentClientMainPartsDesktopAura>();
}

}  // namespace aloha
