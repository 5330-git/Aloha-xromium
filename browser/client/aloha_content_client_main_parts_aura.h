// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_CLIENT_MAIN_PARTS_AURA_H_
#define ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_CLIENT_MAIN_PARTS_AURA_H_

#include <memory>

#include "build/chromeos_buildflags.h"
#include "aloha/browser/client/aloha_content_client_main_parts.h"

#if !BUILDFLAG(IS_CHROMEOS_ASH)
namespace wm {
class WMState;
}
#endif

namespace aloha {

class AlohaContentClientMainPartsAura : public AlohaContentClientMainParts {
 public:
  AlohaContentClientMainPartsAura(const AlohaContentClientMainPartsAura&) =
      delete;
  AlohaContentClientMainPartsAura& operator=(
      const AlohaContentClientMainPartsAura&) = delete;

 protected:
  explicit AlohaContentClientMainPartsAura(
      AlohaContentClient* views_content_client);
  ~AlohaContentClientMainPartsAura() override;

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
