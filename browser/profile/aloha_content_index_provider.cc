// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/profile/aloha_content_index_provider.h"
namespace aloha {

AlohaContentIndexProvider::AlohaContentIndexProvider()
    : icon_sizes_({{96, 96}}) {}

AlohaContentIndexProvider::~AlohaContentIndexProvider() = default;

std::vector<gfx::Size> AlohaContentIndexProvider::GetIconSizes(
    blink::mojom::ContentCategory category) {
  return icon_sizes_;
}

void AlohaContentIndexProvider::OnContentAdded(content::ContentIndexEntry entry) {
  entries_[entry.description->id] = {
      entry.service_worker_registration_id,
      url::Origin::Create(entry.launch_url.DeprecatedGetOriginAsURL())};
}

void AlohaContentIndexProvider::OnContentDeleted(
    int64_t service_worker_registration_id,
    const url::Origin& origin,
    const std::string& description_id) {
  entries_.erase(description_id);
}

std::pair<int64_t, url::Origin>
AlohaContentIndexProvider::GetRegistrationDataFromId(const std::string& id) {
  if (!entries_.count(id)) {
    return {-1, url::Origin()};
  }
  return entries_[id];
}

}  // namespace content
