// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// 为了剔除 testonly, 遂从 shell_permission_manager.cc 中复制了代码

#ifndef ALOHA_TEMP_DEF__BROWSER_SHELL_PERMISSION_MANAGER_H_
#define ALOHA_TEMP_DEF__BROWSER_SHELL_PERMISSION_MANAGER_H_

#include "base/functional/callback_forward.h"
#include "content/public/browser/permission_controller_delegate.h"
#include "content/public/browser/permission_result.h"

namespace blink {
enum class PermissionType;
}

namespace aloha {

class AlohaPermissionManager : public content::PermissionControllerDelegate {
 public:
  AlohaPermissionManager();

  AlohaPermissionManager(const AlohaPermissionManager&) = delete;
  AlohaPermissionManager& operator=(const AlohaPermissionManager&) = delete;

  ~AlohaPermissionManager() override;

  // PermissionManager implementation.
  void RequestPermissions(
      content::RenderFrameHost* render_frame_host,
      const content::PermissionRequestDescription& request_description,
      base::OnceCallback<
          void(const std::vector<blink::mojom::PermissionStatus>&)> callback)
      override;
  void ResetPermission(blink::PermissionType permission,
                       const GURL& requesting_origin,
                       const GURL& embedding_origin) override;
  void RequestPermissionsFromCurrentDocument(
      content::RenderFrameHost* render_frame_host,
      const content::PermissionRequestDescription& request_description,
      base::OnceCallback<
          void(const std::vector<blink::mojom::PermissionStatus>&)> callback)
      override;
  blink::mojom::PermissionStatus GetPermissionStatus(
      blink::PermissionType permission,
      const GURL& requesting_origin,
      const GURL& embedding_origin) override;
  content::PermissionResult GetPermissionResultForOriginWithoutContext(
      blink::PermissionType permission,
      const url::Origin& requesting_origin,
      const url::Origin& embedding_origin) override;
  blink::mojom::PermissionStatus GetPermissionStatusForCurrentDocument(
      blink::PermissionType permission,
      content::RenderFrameHost* render_frame_host,
      bool should_include_device_status) override;
  blink::mojom::PermissionStatus GetPermissionStatusForWorker(
      blink::PermissionType permission,
      content::RenderProcessHost* render_process_host,
      const GURL& worker_origin) override;
  blink::mojom::PermissionStatus GetPermissionStatusForEmbeddedRequester(
      blink::PermissionType permission,
      content::RenderFrameHost* render_frame_host,
      const url::Origin& overridden_origin) override;
};

}  // namespace content

#endif  // ALOHA_TEMP_DEF__BROWSER_SHELL_PERMISSION_MANAGER_H_
