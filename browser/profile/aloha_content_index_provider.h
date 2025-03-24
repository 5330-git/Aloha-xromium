// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_TEMP_DEF__BROWSER_SHELL_CONTENT_INDEX_PROVIDER_H_
#define ALOHA_TEMP_DEF__BROWSER_SHELL_CONTENT_INDEX_PROVIDER_H_

#include <map>
#include <string>
#include <vector>

#include "content/public/browser/content_index_provider.h"
#include "ui/gfx/geometry/size.h"
#include "url/origin.h"

namespace aloha {

// When using AlohaContentIndexProvider, IDs need to be globally unique,
// instead of per Service Worker.
class AlohaContentIndexProvider : public content::ContentIndexProvider {
 public:
  AlohaContentIndexProvider();

  AlohaContentIndexProvider(const AlohaContentIndexProvider&) = delete;
  AlohaContentIndexProvider& operator=(const AlohaContentIndexProvider&) =
      delete;

  ~AlohaContentIndexProvider() override;

  // ContentIndexProvider implementation.
  std::vector<gfx::Size> GetIconSizes(
      blink::mojom::ContentCategory category) override;
  void OnContentAdded(content::ContentIndexEntry entry) override;
  void OnContentDeleted(int64_t service_worker_registration_id,
                        const url::Origin& origin,
                        const std::string& description_id) override;

  // Returns the Service Worker Registration ID and the origin of the Content
  // Index entry registered with |id|. If |id| does not exist, invalid values
  // are returned, which would cause DB tasks to fail.
  std::pair<int64_t, url::Origin> GetRegistrationDataFromId(
      const std::string& id);

  void set_icon_sizes(std::vector<gfx::Size> icon_sizes) {
    icon_sizes_ = std::move(icon_sizes);
  }

 private:
  // Map from |description_id| to <|service_worker_registration_id|, |origin|>.
  std::map<std::string, std::pair<int64_t, url::Origin>> entries_;

  std::vector<gfx::Size> icon_sizes_;
};

}  // namespace content

#endif  // ALOHA_TEMP_DEF__BROWSER_SHELL_CONTENT_INDEX_PROVIDER_H_
