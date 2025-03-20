// Copyright (c) 2025 The Aloha-Xromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef ALOHA_BROWSER_URL_ALOHA_APPS_URL_LOADER_H_
#define ALOHA_BROWSER_URL_ALOHA_APPS_URL_LOADER_H_

#include "base/files/file_path.h"
#include "components/file_access/scoped_file_access.h"
#include "content/public/browser/file_url_loader.h"
#include "mojo/public/c/system/types.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "net/base/directory_lister.h"
#include "net/base/directory_listing.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "url/gurl.h"

namespace aloha::url {

extern const size_t kDefaultFileDirectoryLoaderPipeSize;

// Policy to control how a FileURLLoader will handle directory URLs.
enum class DirectoryLoadingPolicy {
  // File paths which refer to directories are allowed and will load as an
  // HTML directory listing.
  kRespondWithListing,

  // File paths which refer to directories are treated as non-existent and
  // will result in FILE_NOT_FOUND errors.
  kFail,
};

// Policy to control whether or not file access constraints imposed by content
// or its embedder should be honored by a FileURLLoader.
enum class FileAccessPolicy {
  // Enforces file acccess policy defined by content and/or its embedder.
  kRestricted,

  // Ignores file access policy, allowing contents to be loaded from any
  // resolvable file path given.
  kUnrestricted,
};

// Policy to control whether or not a FileURLLoader should follow filesystem
// links (e.g. Windows shortcuts) where applicable.
enum class LinkFollowingPolicy {
  kFollow,
  kDoNotFollow,
};

GURL AppendUrlSeparator(const GURL& url);

net::Error ConvertMojoResultToNetError(MojoResult result);

MojoResult ConvertNetErrorToMojoResult(net::Error net_error);

// 将 aloha://apps/ URL 转换为文件路径，如果转换成功则返回 true。
bool ConvertAlohaAppsUrlToFilePath(const GURL& url, base::FilePath* out_path);

// 从 content\browser\loader\file_url_loader_factory.cc 中迁移过来
// 供 Aloha-Xromium 读取文件和响应请求
class AlohaAppsURLLoader : public network::mojom::URLLoader {
 public:
  static void CreateAndStart(
      const base::FilePath& profile_path,
      const network::ResourceRequest& request,
      network::mojom::FetchResponseType response_type,
      mojo::PendingReceiver<network::mojom::URLLoader> loader,
      mojo::PendingRemote<network::mojom::URLLoaderClient> client_remote,
      DirectoryLoadingPolicy directory_loading_policy,
      FileAccessPolicy file_access_policy,
      LinkFollowingPolicy link_following_policy,
      std::unique_ptr<content::FileURLLoaderObserver> observer,
      scoped_refptr<net::HttpResponseHeaders> extra_response_headers,
      file_access::ScopedFileAccess file_access);

  AlohaAppsURLLoader(const AlohaAppsURLLoader&) = delete;
  AlohaAppsURLLoader& operator=(const AlohaAppsURLLoader&) = delete;

  // network::mojom::URLLoader:
  // services\network\public\mojom\url_loader.mojom
  void FollowRedirect(
      const std::vector<std::string>& removed_headers,
      const net::HttpRequestHeaders& modified_headers,
      const net::HttpRequestHeaders& modified_cors_exempt_headers,
      const std::optional<GURL>& new_url) override;
  void SetPriority(net::RequestPriority priority,
                   int32_t intra_priority_value) override;
  void PauseReadingBodyFromNet() override;
  void ResumeReadingBodyFromNet() override;

 private:
  // Used to save outstanding redirect data while waiting for FollowRedirect
  // to be called. Values default to their most restrictive in case they are
  // not set.
  struct RedirectData {
    bool is_directory = false;
    base::FilePath profile_path;
    network::ResourceRequest request;
    network::mojom::FetchResponseType response_type;
    mojo::PendingReceiver<network::mojom::URLLoader> loader;
    DirectoryLoadingPolicy directory_loading_policy =
        DirectoryLoadingPolicy::kFail;
    FileAccessPolicy file_access_policy = FileAccessPolicy::kRestricted;
    LinkFollowingPolicy link_following_policy =
        LinkFollowingPolicy::kDoNotFollow;
    std::unique_ptr<content::FileURLLoaderObserver> observer;
    scoped_refptr<net::HttpResponseHeaders> extra_response_headers;
    std::unique_ptr<file_access::ScopedFileAccess> file_access;
    RedirectData();
    ~RedirectData();
  };

  AlohaAppsURLLoader();
  ~AlohaAppsURLLoader() override;

  void Start(const base::FilePath& profile_path,
             const network::ResourceRequest& request,
             network::mojom::FetchResponseType response_type,
             mojo::PendingReceiver<network::mojom::URLLoader> loader,
             mojo::PendingRemote<network::mojom::URLLoaderClient> client_remote,
             DirectoryLoadingPolicy directory_loading_policy,
             FileAccessPolicy file_access_policy,
             LinkFollowingPolicy link_following_policy,
             std::unique_ptr<content::FileURLLoaderObserver> observer,
             scoped_refptr<net::HttpResponseHeaders> extra_response_headers,
             file_access::ScopedFileAccess file_access);

  void OnMojoDisconnect();

  void OnClientComplete(
      net::Error net_error,
      std::unique_ptr<content::FileURLLoaderObserver> observer);

  void MaybeDeleteSelf();

  void OnFileWritten(std::unique_ptr<content::FileURLLoaderObserver> observer,
                     MojoResult result);

  std::unique_ptr<mojo::DataPipeProducer> data_producer_;
  mojo::Receiver<network::mojom::URLLoader> receiver_{this};
  mojo::Remote<network::mojom::URLLoaderClient> client_;
  std::unique_ptr<RedirectData> redirect_data_;

  // In case of successful loads, this holds the total number of bytes written
  // to the response (this may be smaller than the total size of the file when
  // a byte range was requested).
  // It is used to set some of the URLLoaderCompletionStatus data passed back
  // to the URLLoaderClients (eg SimpleURLLoader).
  uint64_t total_bytes_written_ = 0;
};
}  // namespace aloha::url

#endif
