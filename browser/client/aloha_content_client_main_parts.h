// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_CLIENT_MAIN_PARTS_H_
#define ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_CLIENT_MAIN_PARTS_H_

#include <memory>

#include "aloha/browser/profile/aloha_browser_profile.h"
#include "base/memory/raw_ptr.h"
#include "build/build_config.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/browser_main_parts.h"

#if BUILDFLAG(IS_APPLE)
#include "ui/display/screen.h"
#endif

namespace base {
class RunLoop;
}

namespace views {
class TestViewsDelegate;
}

namespace aloha {

class AlohaMainClient;

class AlohaContentClientMainParts : public content::BrowserMainParts {
 public:
  // Platform-specific create function.
  static std::unique_ptr<AlohaContentClientMainParts> Create();

  static void PreBrowserMain();

  AlohaContentClientMainParts(const AlohaContentClientMainParts&) = delete;
  AlohaContentClientMainParts& operator=(const AlohaContentClientMainParts&) =
      delete;

  ~AlohaContentClientMainParts() override;

  // content::BrowserMainParts:
  int PreMainMessageLoopRun() override;
  void WillRunMainMessageLoop(
      std::unique_ptr<base::RunLoop>& run_loop) override;
  void PostMainMessageLoopRun() override;

  aloha::AlohaBrowserProfile* browser_context() {
    return browser_context_.get();
  }

 protected:
  AlohaContentClientMainParts();

#if BUILDFLAG(IS_APPLE)
  views::TestViewsDelegate* views_delegate() { return views_delegate_.get(); }
#endif

 private:
#if BUILDFLAG(IS_APPLE)
  display::ScopedNativeScreen desktop_screen_;
#endif

  std::unique_ptr<aloha::AlohaBrowserProfile> browser_context_;

  std::unique_ptr<views::TestViewsDelegate> views_delegate_;

  std::unique_ptr<base::RunLoop> run_loop_;
};

}  // namespace aloha

#endif  // ALOHA_VIEWS_CONTENT_CLIENT_VIEWS_CONTENT_CLIENT_MAIN_PARTS_H_
