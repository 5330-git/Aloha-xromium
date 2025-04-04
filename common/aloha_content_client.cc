#include "aloha/common/aloha_content_client.h"

#include <string_view>

#include "aloha/common/aloha_constants.h"
#include "base/command_line.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "build/build_config.h"
#include "content/public/common/content_client.h"
#include "content/public/common/content_switches.h"
#include "third_party/blink/public/strings/grit/blink_strings.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "url/url_util.h"

namespace aloha {

namespace {
  AlohaContentClient* g_aloha_content_client = nullptr;
}

AlohaContentClient* AlohaContentClient::GetContentClient() {
  return g_aloha_content_client;
}

void AlohaContentClient::SetContentClient(AlohaContentClient *client) {
  content::SetContentClient(client);
  g_aloha_content_client = client;
}

AlohaContentClient::AlohaContentClient() = default;
AlohaContentClient::~AlohaContentClient() {}

std::u16string AlohaContentClient::GetLocalizedString(int message_id) {
  return l10n_util::GetStringUTF16(message_id);
}

std::string_view AlohaContentClient::GetDataResource(
    int resource_id,
    ui::ResourceScaleFactor scale_factor) {
  return ui::ResourceBundle::GetSharedInstance().GetRawDataResourceForScale(
      resource_id, scale_factor);
}

base::RefCountedMemory* AlohaContentClient::GetDataResourceBytes(
    int resource_id) {
  return ui::ResourceBundle::GetSharedInstance().LoadDataResourceBytes(
      resource_id);
}

std::string AlohaContentClient::GetDataResourceString(int resource_id) {
  return ui::ResourceBundle::GetSharedInstance().LoadDataResourceString(
      resource_id);
}

gfx::Image& AlohaContentClient::GetNativeImageNamed(int resource_id) {
  return ui::ResourceBundle::GetSharedInstance().GetNativeImageNamed(
      resource_id);
}

blink::OriginTrialPolicy* AlohaContentClient::GetOriginTrialPolicy() {
  return &origin_trial_policy_;
}

void AlohaContentClient::AddAdditionalSchemes(Schemes* schemes) {
#if BUILDFLAG(IS_ANDROID)
  schemes->local_schemes.push_back(url::kContentScheme);
#endif

  // 有关 schemes 的更多信息，请参阅：url\url_util.cc: SchemeRegistry
  // 将 aloha:// 加入到标准协议中。
  schemes->standard_schemes.push_back(aloha::url::kAlohaScheme);
  schemes->secure_schemes.push_back(aloha::url::kAlohaScheme);
  schemes->cors_enabled_schemes.push_back(aloha::url::kAlohaScheme);
  schemes->savable_schemes.push_back(aloha::url::kAlohaScheme);
  schemes->service_worker_schemes.push_back(aloha::url::kAlohaScheme);
  schemes->csp_bypassing_schemes.push_back(aloha::url::kAlohaScheme);
  // 允许加载本地资源。
  schemes->local_schemes.push_back(aloha::url::kAlohaScheme);
  schemes->local_schemes.push_back(aloha::url::kAlohaDemoScheme);

  // 允许使用 WebStorage。(Cookies、)
  ::url::AddWebStorageScheme(aloha::url::kAlohaScheme);
  ::url::AddWebStorageScheme(aloha::url::kAlohaDemoScheme);
}
}  // namespace aloha
