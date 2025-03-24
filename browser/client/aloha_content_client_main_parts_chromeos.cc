// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/common/aloha_main_client.h"
#include "aloha/browser/client/aloha_content_client_main_parts_aura.h"
#include "content/public/browser/context_factory.h"
#include "content/public/common/result_codes.h"
#include "ui/aura/window.h"
#include "ui/wm/test/wm_test_helper.h"

namespace aloha {

namespace {

class AlohaContentClientMainPartsChromeOS
    : public AlohaContentClientMainPartsAura {
 public:
  AlohaContentClientMainPartsChromeOS();

  AlohaContentClientMainPartsChromeOS(
      const AlohaContentClientMainPartsChromeOS&) = delete;
  AlohaContentClientMainPartsChromeOS& operator=(
      const AlohaContentClientMainPartsChromeOS&) = delete;

  ~AlohaContentClientMainPartsChromeOS() override {}

  // content::BrowserMainParts:
  int PreMainMessageLoopRun() override;
  void PostMainMessageLoopRun() override;

 private:
  // Enable a minimal set of views::corewm to be initialized.
  std::unique_ptr<::wm::WMTestHelper> wm_test_helper_;
};

AlohaContentClientMainPartsChromeOS::AlohaContentClientMainPartsChromeOS() =
    default;
int AlohaContentClientMainPartsChromeOS::PreMainMessageLoopRun() {
  AlohaContentClientMainPartsAura::PreMainMessageLoopRun();

  // Set up basic pieces of views::corewm.
  wm_test_helper_ = std::make_unique<wm::WMTestHelper>(gfx::Size(1024, 768));
  // Ensure the X window gets mapped.
  wm_test_helper_->host()->Show();

  // Ensure Aura knows where to open new windows.
  aura::Window* root_window = wm_test_helper_->host()->window();
  AlohaMainClient::GetInstance()->OnPreMainMessageLoopRun(browser_context(),
                                                          root_window);
  return content::RESULT_CODE_NORMAL_EXIT;
}

void AlohaContentClientMainPartsChromeOS::PostMainMessageLoopRun() {
  wm_test_helper_.reset();

  AlohaContentClientMainPartsAura::PostMainMessageLoopRun();
}

}  // namespace

// static
std::unique_ptr<AlohaContentClientMainParts>
AlohaContentClientMainParts::Create() {
  return std::make_unique<AlohaContentClientMainPartsChromeOS>();
}

}  // namespace aloha
