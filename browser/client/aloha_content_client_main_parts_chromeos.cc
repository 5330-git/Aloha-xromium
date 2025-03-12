// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/browser/context_factory.h"
#include "content/public/common/result_codes.h"
#include "content/shell/browser/shell_browser_context.h"
#include "ui/aura/window.h"
#include "aloha/browser/client/aloha_content_client.h"
#include "aloha/browser/client/views_content_client_main_parts_aura.h"
#include "ui/wm/test/wm_test_helper.h"

namespace aloha {

namespace {

class ViewsContentClientMainPartsChromeOS
    : public AlohaContentClientMainPartsAura {
 public:
  explicit ViewsContentClientMainPartsChromeOS(
      AlohaContentClient* views_content_client);

  ViewsContentClientMainPartsChromeOS(
      const ViewsContentClientMainPartsChromeOS&) = delete;
  ViewsContentClientMainPartsChromeOS& operator=(
      const ViewsContentClientMainPartsChromeOS&) = delete;

  ~ViewsContentClientMainPartsChromeOS() override {}

  // content::BrowserMainParts:
  int PreMainMessageLoopRun() override;
  void PostMainMessageLoopRun() override;

 private:
  // Enable a minimal set of views::corewm to be initialized.
  std::unique_ptr<::wm::WMTestHelper> wm_test_helper_;
};

ViewsContentClientMainPartsChromeOS::ViewsContentClientMainPartsChromeOS(
    AlohaContentClient* views_content_client)
    : AlohaContentClientMainPartsAura(views_content_client) {}

int ViewsContentClientMainPartsChromeOS::PreMainMessageLoopRun() {
  AlohaContentClientMainPartsAura::PreMainMessageLoopRun();

  // Set up basic pieces of views::corewm.
  wm_test_helper_ = std::make_unique<wm::WMTestHelper>(gfx::Size(1024, 768));
  // Ensure the X window gets mapped.
  wm_test_helper_->host()->Show();

  // Ensure Aura knows where to open new windows.
  aura::Window* root_window = wm_test_helper_->host()->window();
  views_content_client()->OnPreMainMessageLoopRun(browser_context(),
                                                  root_window);

  return content::RESULT_CODE_NORMAL_EXIT;
}

void ViewsContentClientMainPartsChromeOS::PostMainMessageLoopRun() {
  wm_test_helper_.reset();

  AlohaContentClientMainPartsAura::PostMainMessageLoopRun();
}

}  // namespace

// static
std::unique_ptr<AlohaContentClientMainParts>
AlohaContentClientMainParts::Create(AlohaContentClient* views_content_client) {
  return std::make_unique<ViewsContentClientMainPartsChromeOS>(
      views_content_client);
}

}  // namespace aloha
