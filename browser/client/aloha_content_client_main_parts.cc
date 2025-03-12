// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/client/aloha_content_client_main_parts.h"

#include <utility>

#include "aloha/browser/client/aloha_content_client.h"
#include "aloha/browser/devtools/devtools_server.h"
#include "base/base64.h"
#include "base/memory/scoped_refptr.h"
#include "base/no_destructor.h"
#include "base/run_loop.h"
#include "build/build_config.h"
#include "content/public/common/result_codes.h"
#include "content/shell/browser/shell_browser_context.h"
#include "ui/base/ime/init/input_method_initializer.h"
#include "ui/views/test/desktop_test_views_delegate.h"

namespace aloha {

AlohaContentClientMainParts::AlohaContentClientMainParts(
    AlohaContentClient* views_content_client)
    : views_content_client_(views_content_client) {}

AlohaContentClientMainParts::~AlohaContentClientMainParts() {}

#if !BUILDFLAG(IS_APPLE)
void AlohaContentClientMainParts::PreBrowserMain() {}
#endif

int AlohaContentClientMainParts::PreMainMessageLoopRun() {
  ui::InitializeInputMethodForTesting();
  browser_context_ = std::make_unique<content::ShellBrowserContext>(false);
  // DevTools
  devtools::StartHttpHandler(browser_context_.get());

  views_delegate_ = std::make_unique<views::DesktopTestViewsDelegate>();
  run_loop_ = std::make_unique<base::RunLoop>();
  views_content_client()->set_quit_closure(run_loop_->QuitClosure());
  return content::RESULT_CODE_NORMAL_EXIT;
}

void AlohaContentClientMainParts::WillRunMainMessageLoop(
    std::unique_ptr<base::RunLoop>& run_loop) {
  run_loop = std::move(run_loop_);
}

void AlohaContentClientMainParts::PostMainMessageLoopRun() {
  browser_context_.reset();
  views_delegate_.reset();
}

// // 参考 chrome\browser\chrome_browser_main_win.cc
// void AlohaContentClientMainParts::PreCreateMainMessageLoop() {
//   // 目前还未实现 Preferences 相关的功能（即 PrefService 相关）所以 先临时写个
//   // demo:

//   scoped_refptr<PrefRegistrySimple> registry =
//       base::MakeRefCounted<PrefRegistrySimple>();
//   registry->RegisterStringPref(kOsCryptEncryptedKeyPrefName,
//                                "default key string");
//   registry->RegisterBooleanPref(kOsCryptAuditEnabledPrefName, false);
//   PrefServiceFactory pref_service_factory;
//   scoped_refptr<InMemoryPrefStore> in_memory_pref_store =
//       base::MakeRefCounted<InMemoryPrefStore>();
//   pref_service_factory.set_managed_prefs(in_memory_pref_store);
//   pref_service_factory.set_supervised_user_prefs(in_memory_pref_store);
//   pref_service_factory.set_extension_prefs(in_memory_pref_store);
//   pref_service_factory.set_supervised_user_prefs(in_memory_pref_store);
//   pref_service_factory.set_command_line_prefs(in_memory_pref_store);
//   pref_service_factory.set_user_prefs(in_memory_pref_store);
//   pref_service_factory.set_recommended_prefs(in_memory_pref_store);

//   pref_service_ = pref_service_factory.Create(registry);
//   // 必须通过 pref_service_ 覆盖默认值，否则 HasPrefPath 会返回 false
//   // 且要注意 key 是有固定格式的，而且要进行 Base64 编码
//   pref_service_->SetString(
//       kOsCryptEncryptedKeyPrefName,
//       base::Base64Encode(std::string(kDPAPIKeyPrefix) + " temp test key"));
//   CHECK(pref_service_->HasPrefPath("os_crypt.encrypted_key"));

//   // Initialize the OSCrypt.
//   bool os_crypt_init = OSCrypt::Init(pref_service_.get());
//   DCHECK(os_crypt_init);
// }

}  // namespace aloha
