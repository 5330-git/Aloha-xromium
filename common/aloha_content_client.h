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

#include "aloha/common/aloha_origin_trial_policy.h"
#include "content/public/common/content_client.h"

namespace aloha {
// 过渡：先拷贝 `content::ShellContentClient` 类，然后再修改。
class AlohaContentClient : public content::ContentClient {
 public:
  // 由于 content 模块的限制，我们无法访问 `content::GetContentClient()` 函数。
  // 不过在 Aloha-Xromium 中，这个函数返回的是 `aloha::AlohaContentClient` 类的
  // 实例。因此我们可以自己实现对应的接口，存储一份实例指针。
  // 注意：这个实例是全局唯一的，应当与 `content::GetContentClient` 保持一致。
  static AlohaContentClient* GetContentClient();
  // content::SetContentClient 只会在 content::ContentClientCreator
  // 中创建，并且是跟随 ContentMainDelegate
  // 的。不过由于这个接口是导出的，为了防止我们存储的实例和
  // content::GetContentClient 返回的实例不一致，这里规定在 Aloha-Xromium
  // 中应该用这个接口替代 content::SetContentClient。
  static void SetContentClient(AlohaContentClient* client);

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
  aloha::AlohaOriginTrialPolicy origin_trial_policy_;
};
}  // namespace aloha

#endif
