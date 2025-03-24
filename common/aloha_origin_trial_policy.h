// 为了去除 testonly，我们需要将对 content::shell 的依赖都去除掉。
// 相应地换成我们自己的实现
// 以下内容从 content\shell\common\shell_origin_trial_policy.h 中复制而来
#ifndef ALOHA_COMMON_ALOHA_ORIGIN_TRAIL_POLICY_H_
#define ALOHA_COMMON_ALOHA_ORIGIN_TRAIL_POLICY_H_
#include <vector>

#include "third_party/blink/public/common/origin_trials/origin_trial_policy.h"

namespace aloha {

class AlohaOriginTrialPolicy : public blink::OriginTrialPolicy {
 public:
  AlohaOriginTrialPolicy();

  AlohaOriginTrialPolicy(const AlohaOriginTrialPolicy&) = delete;
  AlohaOriginTrialPolicy& operator=(const AlohaOriginTrialPolicy&) = delete;

  ~AlohaOriginTrialPolicy() override;

  // blink::OriginTrialPolicy interface
  bool IsOriginTrialsSupported() const override;
  const std::vector<blink::OriginTrialPublicKey>& GetPublicKeys()
      const override;
  bool IsOriginSecure(const GURL& url) const override;

 private:
  std::vector<blink::OriginTrialPublicKey> public_keys_;
};
}  // namespace aloha

#endif
