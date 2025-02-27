// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_COMMON_CONTENT_CLIENT_H_
#define ALOHA_COMMON_CONTENT_CLIENT_H_

#include <string_view>

#include "content/public/common/content_client.h"

namespace aloha {

class ContentClient : public content::ContentClient {
 public:
  ContentClient();
  ContentClient(const ContentClient&) = delete;
  ContentClient& operator=(const ContentClient&) = delete;
  ~ContentClient() override;

 private:
  // content::ContentClient:
  std::string_view GetDataResource(
      int resource_id,
      ui::ResourceScaleFactor scale_factor) override;
  base::RefCountedMemory* GetDataResourceBytes(int resource_id) override;
};

}  // namespace aloha

#endif  // ALOHA_COMMON_CONTENT_CLIENT_H_
