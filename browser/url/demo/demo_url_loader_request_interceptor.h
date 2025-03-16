// Copyright (c) 2025 The Aloha-Xromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef ALOHA_BROWSER_URL_DEMO_URL_LOADER_REQUEST_INTERCEPTOR_H_
#define ALOHA_BROWSER_URL_DEMO_URL_LOADER_REQUEST_INTERCEPTOR_H_
#include "content/public/browser/url_loader_request_interceptor.h"


// 本文件将实现一个简单的 URL 请求拦截器，用于拦截 `aloha-demo://` 协议并响应
// 一个简单的 HTML 页面。

namespace aloha::url {
// URL 拦截器，用于拦截 aloha-demo:// 协议的请求
// 参考 components\pdf\browser\pdf_url_loader_request_interceptor.cc 实现
// 将其通过 `content::ContentBrowserClient::WillCreateURLLoaderRequestInterceptor`
// 注册到 Browser 进程中，会拦截和响应 aloha-demo:// 的请求
// scheme://host.domain:port/path/filename
class DemoURLLoaderRequestInterceptor
    : public content::URLLoaderRequestInterceptor {
 public:
  // 本拦截器只会拦截 aloha-demo://demo_url_loader_request_interceptor 下的请求
  constexpr static char kInterceptHost[] = "demo_url_loader_request_interceptor";
  void MaybeCreateLoader(
      const network::ResourceRequest& resource_request,
      content::BrowserContext* browser_context,
      content::URLLoaderRequestInterceptor::LoaderCallback callback) override;

  bool MaybeCreateLoaderForResponse(
      const network::URLLoaderCompletionStatus& status,
      const network::ResourceRequest& request,
      network::mojom::URLResponseHeadPtr* response_head,
      mojo::ScopedDataPipeConsumerHandle* response_body,
      mojo::PendingRemote<network::mojom::URLLoader>* loader,
      mojo::PendingReceiver<network::mojom::URLLoaderClient>* client_receiver,
      blink::ThrottlingURLLoader* url_loader) override;
};
}  // namespace aloha::url
#endif
