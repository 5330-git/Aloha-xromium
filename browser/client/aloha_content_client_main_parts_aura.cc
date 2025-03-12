// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/client/aloha_content_client_main_parts_aura.h"

#include <utility>

#include "build/chromeos_buildflags.h"

#if !BUILDFLAG(IS_CHROMEOS_ASH)
#include "ui/wm/core/wm_state.h"
#endif

namespace aloha {

AlohaContentClientMainPartsAura::AlohaContentClientMainPartsAura(
    AlohaContentClient* views_content_client)
    : AlohaContentClientMainParts(views_content_client) {}

AlohaContentClientMainPartsAura::~AlohaContentClientMainPartsAura() {
}

void AlohaContentClientMainPartsAura::ToolkitInitialized() {
  AlohaContentClientMainParts::ToolkitInitialized();

#if !BUILDFLAG(IS_CHROMEOS_ASH)
  wm_state_ = std::make_unique<::wm::WMState>();
#endif
}

void AlohaContentClientMainPartsAura::PostMainMessageLoopRun() {
  AlohaContentClientMainParts::PostMainMessageLoopRun();
}

}  // namespace aloha
