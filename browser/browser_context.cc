// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/browser_context.h"

#include <memory>

#include "content/browser/browser_context_impl.h"
#include "content/browser/preloading/prefetch/prefetch_container.h"
#include "content/browser/preloading/prefetch/prefetch_service.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"

namespace aloha {

BrowserContext::BrowserContext(const base::FilePath& temp_dir_path)
    : temp_dir_path_(temp_dir_path) {
  // 添加 PrefetchContainer
  // content::PrefetchService* prefetch_service =
  // content::BrowserContextImpl::From(this)->GetPrefetchService(); auto
  // container = std::make_unique<content::PrefetchContainer>(

  // );
  // prefetch_service->AddPrefetchContainer();
  // content::BrowserContext::StartBrowserPrefetchRequest(const GURL &url, bool
  // javascript_enabled, std::optional<net::HttpNoVarySearchData>
  // no_vary_search_expected, const net::HttpRequestHeaders &additional_headers,
  // std::optional<PrefetchStartCallback> prefetch_start_callback)
}

BrowserContext::~BrowserContext() {
  NotifyWillBeDestroyed();
  ShutdownStoragePartitions();
}

// Creates a delegate to initialize a HostZoomMap and persist its information.
// This is called during creation of each StoragePartition.
std::unique_ptr<content::ZoomLevelDelegate>
BrowserContext::CreateZoomLevelDelegate(const base::FilePath& partition_path) {
  return nullptr;
}

base::FilePath BrowserContext::GetPath() {
  return temp_dir_path_;
}

bool BrowserContext::IsOffTheRecord() {
  return false;
}

content::DownloadManagerDelegate* BrowserContext::GetDownloadManagerDelegate() {
  return nullptr;
}

content::BrowserPluginGuestManager* BrowserContext::GetGuestManager() {
  return nullptr;
}

storage::SpecialStoragePolicy* BrowserContext::GetSpecialStoragePolicy() {
  return nullptr;
}

content::PlatformNotificationService*
BrowserContext::GetPlatformNotificationService() {
  return nullptr;
}

content::PushMessagingService* BrowserContext::GetPushMessagingService() {
  return nullptr;
}

content::StorageNotificationService*
BrowserContext::GetStorageNotificationService() {
  return nullptr;
}

content::SSLHostStateDelegate* BrowserContext::GetSSLHostStateDelegate() {
  return nullptr;
}

content::PermissionControllerDelegate*
BrowserContext::GetPermissionControllerDelegate() {
  return nullptr;
}

content::ReduceAcceptLanguageControllerDelegate*
BrowserContext::GetReduceAcceptLanguageControllerDelegate() {
  return nullptr;
}

content::ClientHintsControllerDelegate*
BrowserContext::GetClientHintsControllerDelegate() {
  return nullptr;
}

content::BackgroundFetchDelegate* BrowserContext::GetBackgroundFetchDelegate() {
  return nullptr;
}

content::BackgroundSyncController*
BrowserContext::GetBackgroundSyncController() {
  return nullptr;
}

content::BrowsingDataRemoverDelegate*
BrowserContext::GetBrowsingDataRemoverDelegate() {
  return nullptr;
}

}  // namespace aloha
