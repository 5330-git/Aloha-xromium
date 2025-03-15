#include "aloha/common/aloha_content_client.h"

#include <string_view>

#include "aloha/common/aloha_constants.h"
#include "base/command_line.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "build/build_config.h"
#include "content/public/common/content_switches.h"
#include "content/shell/common/shell_content_client.h"
#include "content/shell/common/shell_switches.h"
#include "third_party/blink/public/strings/grit/blink_strings.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"

namespace aloha {
AlohaContentClient::AlohaContentClient() = default;
AlohaContentClient::~AlohaContentClient() {}

std::u16string AlohaContentClient::GetLocalizedString(int message_id) {
  if (::switches::IsRunWebTestsSwitchPresent()) {
    switch (message_id) {
      case IDS_FORM_OTHER_DATE_LABEL:
        return u"<<OtherDate>>";
      case IDS_FORM_OTHER_MONTH_LABEL:
        return u"<<OtherMonth>>";
      case IDS_FORM_OTHER_WEEK_LABEL:
        return u"<<OtherWeek>>";
      case IDS_FORM_CALENDAR_CLEAR:
        return u"<<Clear>>";
      case IDS_FORM_CALENDAR_TODAY:
        return u"<<Today>>";
      case IDS_FORM_THIS_MONTH_LABEL:
        return u"<<ThisMonth>>";
      case IDS_FORM_THIS_WEEK_LABEL:
        return u"<<ThisWeek>>";
    }
  }
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
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(::switches::kTestRegisterStandardScheme)) {
    std::string scheme = command_line->GetSwitchValueASCII(
        ::switches::kTestRegisterStandardScheme);
    schemes->standard_schemes.emplace_back(std::move(scheme));
  }

  // 将 aloha:// 加入到标准协议中。
  schemes->standard_schemes.push_back(aloha::url::kAlohaScheme);
  schemes->secure_schemes.push_back(aloha::url::kAlohaScheme);
  schemes->cors_enabled_schemes.push_back(aloha::url::kAlohaScheme);
  schemes->savable_schemes.push_back(aloha::url::kAlohaScheme);
  // 允许加载本地资源。
  schemes->local_schemes.push_back(aloha::url::kAlohaScheme);
}
}  // namespace aloha
