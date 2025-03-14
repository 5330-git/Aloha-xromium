// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/common/aloha_main_client.h"

#include <utility>

#include "aloha/browser/client/aloha_content_main_delegate.h"
#include "aloha/grit/aloha_resources.h"
#include "base/base64.h"
#include "base/metrics/histogram_functions.h"
#include "base/metrics/histogram_macros.h"
#include "base/process/process.h"
#include "base/strings/sys_string_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "components/os_crypt/sync/os_crypt.h"
#include "components/prefs/default_pref_store.h"
#include "components/prefs/in_memory_pref_store.h"
#include "components/prefs/pref_registry.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_service_factory.h"
#include "components/prefs/pref_store.h"
#include "content/public/app/content_main.h"
#include "ui/base/l10n/l10n_util.h"
#if BUILDFLAG(IS_WIN)
#include <windows.h>

#include "dpapi.h"
#endif

namespace aloha {
namespace {
AlohaMainClient* g_aloha_main_client = nullptr;

// Contains base64 random key encrypted with DPAPI.
constexpr char kOsCryptEncryptedKeyPrefName[] = "os_crypt.encrypted_key";
// Whether or not an attempt has been made to enable audit for the DPAPI
// encryption backing the random key.
constexpr char kOsCryptAuditEnabledPrefName[] = "os_crypt.audit_enabled";
constexpr char kDPAPIKeyPrefix[] = "DPAPI";

// 参考 components\os_crypt\sync\os_crypt_win.cc
// 测试加密Cookies临时使用：因为目前没有支持落盘到disk的PrefService
// 没有办法保证每次启动的密钥都是一致，这将导致加密的Cookies无法解密
// 为了测试Cookies数据落盘的正确性，需要保证每次使用的都是合法的相同密钥
// TODO(yeyun.anton): 移除该接口，实现落盘PrefService

bool EncryptStringWithDPAPI(const std::string& plaintext,
                            std::string* ciphertext) {
  DATA_BLOB input;
  input.pbData =
      const_cast<BYTE*>(reinterpret_cast<const BYTE*>(plaintext.data()));
  input.cbData = static_cast<DWORD>(plaintext.length());

  BOOL result = FALSE;
  DATA_BLOB output;
  {
    SCOPED_UMA_HISTOGRAM_TIMER("OSCrypt.Win.Encrypt.Time");
    result = ::CryptProtectData(
        /*pDataIn=*/&input,
        /*szDataDescr=*/
        base::SysUTF8ToWide("Aloha-Xromium Browser").c_str(),
        /*pOptionalEntropy=*/nullptr,
        /*pvReserved=*/nullptr,
        /*pPromptStruct=*/nullptr, /*dwFlags=*/CRYPTPROTECT_AUDIT,
        /*pDataOut=*/&output);
  }
  base::UmaHistogramBoolean("OSCrypt.Win.Encrypt.Result", result);
  if (!result) {
    PLOG(ERROR) << "Failed to encrypt";
    return false;
  }

  // this does a copy
  ciphertext->assign(reinterpret_cast<std::string::value_type*>(output.pbData),
                     output.cbData);

  LocalFree(output.pbData);
  return true;
}
}  // namespace


// static
AlohaMainClient* AlohaMainClient::GetInstance() {
  return g_aloha_main_client;
}

#if BUILDFLAG(IS_WIN)
AlohaMainClient::AlohaMainClient(HINSTANCE instance,
                                 sandbox::SandboxInterfaceInfo* sandbox_info)
    : instance_(instance), sandbox_info_(sandbox_info) {}

// static
void AlohaMainClient::InitInstance(
    HINSTANCE instance,
    sandbox::SandboxInterfaceInfo* sandbox_info) {
  CHECK(!g_aloha_main_client);
  g_aloha_main_client = new AlohaMainClient(instance, sandbox_info);
}
#else
AlohaMainClient::AlohaMainClient(int argc, const char** argv)
    : argc_(argc), argv_(argv) {}

// static
void AlohaMainClient::InitInstance(int argc, const char** argv) {
  CHECK(!g_aloha_main_client);
  g_aloha_main_client = new AlohaMainClient(argc, argv);
}
#endif

AlohaMainClient::~AlohaMainClient() {}

int AlohaMainClient::AlohaMain() {
  // TEMP USING START
  // 初始化 PrefService 和 OSCrypt
  // 目前还未实现 Preferences 相关的功能（即 PrefService 相关）所以 先临时写个
  // demo:

  scoped_refptr<PrefRegistrySimple> registry =
      base::MakeRefCounted<PrefRegistrySimple>();
  registry->RegisterStringPref(kOsCryptEncryptedKeyPrefName,
                               "default key string");
  registry->RegisterBooleanPref(kOsCryptAuditEnabledPrefName, false);
  PrefServiceFactory pref_service_factory;
  scoped_refptr<InMemoryPrefStore> in_memory_pref_store =
      base::MakeRefCounted<InMemoryPrefStore>();
  pref_service_factory.set_managed_prefs(in_memory_pref_store);
  pref_service_factory.set_supervised_user_prefs(in_memory_pref_store);
  pref_service_factory.set_extension_prefs(in_memory_pref_store);
  pref_service_factory.set_supervised_user_prefs(in_memory_pref_store);
  pref_service_factory.set_command_line_prefs(in_memory_pref_store);
  pref_service_factory.set_user_prefs(in_memory_pref_store);
  pref_service_factory.set_recommended_prefs(in_memory_pref_store);

  pref_service_ = pref_service_factory.Create(registry);
  // 必须通过 pref_service_ 覆盖默认值，否则 HasPrefPath 会返回 false
  // 且要注意 key 是有固定格式的，而且要进行 Base64 编码
  // OSCrypt 会对 key 进行 DPAPI 加密，然后转化成 Base64 编码进行存储
  // 加密文本时会依此从 Base64 解码，然后进行 DPAPI 解密得到明文密钥然后使用
  // AES-256-GCM 加密目标文本。因此需要保证密钥的长度足够(256位，即32字节)

  std::string key_for_test_cookies_persistence;
  std::string key_for_test_cookies_persistence_encrypted;
  CHECK(EncryptStringWithDPAPI("abcdefghijklmnopqrstuvwxyz123456",
                               &key_for_test_cookies_persistence_encrypted));
  key_for_test_cookies_persistence_encrypted =
      base::Base64Encode(std::string(kDPAPIKeyPrefix) +
                         key_for_test_cookies_persistence_encrypted);
  pref_service_->SetString(kOsCryptEncryptedKeyPrefName,
                           key_for_test_cookies_persistence_encrypted);
  CHECK(pref_service_->HasPrefPath("os_crypt.encrypted_key"));

  // Initialize the OSCrypt.
  bool os_crypt_init = OSCrypt::Init(pref_service_.get());
  DCHECK(os_crypt_init);
  LOG(INFO) << "OSCrypt key: " << OSCrypt::GetRawEncryptionKey();
  // TEMP USING END


  // INIT COMMAND LINE

  AlohaContentMainDelegate delegate;
  content::ContentMainParams params(&delegate);

#if BUILDFLAG(IS_WIN)
  params.instance = instance_;
  params.sandbox_info = sandbox_info_;
#else
  params.argc = argc_;
  params.argv = argv_;
#endif

  return content::ContentMain(std::move(params));
}

void AlohaMainClient::OnPreMainMessageLoopRun(
    content::BrowserContext* browser_context,
    gfx::NativeWindow window_context) {
  std::move(on_pre_main_message_loop_run_callback_)
      .Run(browser_context, window_context);
}

void AlohaMainClient::OnResourcesLoaded() {
  if (on_resources_loaded_callback_) {
    std::move(on_resources_loaded_callback_).Run();
  }
}

}  // namespace aloha
