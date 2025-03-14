// Copyright 2025 The Aloha-Xromium Authors
// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// 本文件将实现对 Chromium Embedder 接口的扩展（content::ContentClient）。所谓的
// Chromium Embedder 即 Chromium 嵌入器 (相较于 content 层而言)，比如
// chrome、msedge、electron 等（当然也包括我们 Aloha-Xromium 的 aloha）。
// content::ContentClient 拥有相当广泛的职责，包括：
// 1. 定义浏览器协议
// 2. 定义 GPU 信息
// 3. 定义插件信息
// ......

// 这里我们主要参考 `chrome::ChromeContentClient` 类和
// `content::ShellContentClient` 类。实现 `aloha::AlohaContentClient` 类。

#ifndef ALOHA_COMMON_ALOHA_CONTENT_CLIENT_H_
#define ALOHA_COMMON_ALOHA_CONTENT_CLIENT_H_
#include <string>
#include <string_view>
#include <vector>

#include "content/public/common/content_client.h"
#include "content/shell/common/shell_origin_trial_policy.h"

namespace aloha {
// 过渡：先拷贝 `content::ShellContentClient` 类，然后再修改。
class AlohaContentClient : public content::ContentClient {
  public:
  AlohaContentClient();
  ~AlohaContentClient() override;

  std::u16string GetLocalizedString(int message_id) override;
  std::string_view GetDataResource(
      int resource_id,
      ui::ResourceScaleFactor scale_factor) override;
  base::RefCountedMemory* GetDataResourceBytes(int resource_id) override;
  std::string GetDataResourceString(int resource_id) override;
  gfx::Image& GetNativeImageNamed(int resource_id) override;
  blink::OriginTrialPolicy* GetOriginTrialPolicy() override;
  void AddAdditionalSchemes(Schemes* schemes) override;

 private:
  content::ShellOriginTrialPolicy origin_trial_policy_;
};
}  // namespace aloha

#endif
