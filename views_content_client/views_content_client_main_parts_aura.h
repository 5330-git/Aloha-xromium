// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_CLIENT_MAIN_PARTS_AURA_H_
#define ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_CLIENT_MAIN_PARTS_AURA_H_

#include <memory>

#include "build/chromeos_buildflags.h"
#include "aloha/views_content_client/views_content_client_main_parts.h"

#if !BUILDFLAG(IS_CHROMEOS_ASH)
namespace wm {
class WMState;
}
#endif

namespace aloha {

class ViewsContentClientMainPartsAura : public ViewsContentClientMainParts {
 public:
  ViewsContentClientMainPartsAura(const ViewsContentClientMainPartsAura&) =
      delete;
  ViewsContentClientMainPartsAura& operator=(
      const ViewsContentClientMainPartsAura&) = delete;

 protected:
  explicit ViewsContentClientMainPartsAura(
      ViewsContentClient* views_content_client);
  ~ViewsContentClientMainPartsAura() override;

  // content::BrowserMainParts:
  void ToolkitInitialized() override;
  void PostMainMessageLoopRun() override;

 private:
#if !BUILDFLAG(IS_CHROMEOS_ASH)
  std::unique_ptr<::wm::WMState> wm_state_;
#endif
};

}  // namespace aloha

#endif  // ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_CLIENT_MAIN_PARTS_AURA_H_
