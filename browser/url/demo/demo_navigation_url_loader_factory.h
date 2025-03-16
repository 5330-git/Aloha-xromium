// Copyright (c) 2025 The Aloha-Xromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_BROWSER_URL_DEMO_DEMO_NAVIGATION_URL_LOADER_FACTORY_H_
#define ALOHA_BROWSER_URL_DEMO_DEMO_NAVIGATION_URL_LOADER_FACTORY_H_
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "services/network/public/cpp/self_deleting_url_loader_factory.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"

// 除了通过 URL 请求拦截器（参见：
// demo_url_loader_request_interceptor.h）拦截和响应 aloha-demo:// 的请求外，
// 还可以通过 URL 加载器工厂拦截和响应 aloha-demo:// 的请求。
// URLLoaderFactory 用于创建 URLLoader，用于拦截和响应 aloha-demo:// 的请求
// 可以通过
// `content::ContentBrowserClient::CreateNonNetworkNavigationURLLoaderFactory`
// 接口复写创建 URL 加载器工厂的逻辑
// （参见：content\browser\loader\navigation_url_loader_impl.cc:
// NavigationURLLoaderImpl::CreateTerminalNonNetworkLoaderFactory）
// PS: 从执行顺序上看，URL 请求拦截器的优先级高于 URL 加载器工厂。

// 参考 net::DataURLLoaderFactory 实现。
namespace aloha::url {
class DemoNavigationURLLoaderFactory
    : public network::SelfDeletingURLLoaderFactory {
 public:
  // static
  // 参考 content\browser\data_url_loader_factory.cc
  static mojo::PendingRemote<network::mojom::URLLoaderFactory> Create();
  constexpr static char kInterceptHost[] = "demo_navigation_url_loader_factory";
  explicit DemoNavigationURLLoaderFactory(
      mojo::PendingReceiver<network::mojom::URLLoaderFactory> factory_receiver);
  DemoNavigationURLLoaderFactory();
  ~DemoNavigationURLLoaderFactory() override;
  void CreateLoaderAndStart(
      mojo::PendingReceiver<network::mojom::URLLoader> loader,
      int32_t request_id,
      uint32_t options,
      const network::ResourceRequest& request,
      mojo::PendingRemote<network::mojom::URLLoaderClient> client,
      const net::MutableNetworkTrafficAnnotationTag& traffic_annotation)
      override;

 private:
  std::string default_response_data_body_;
};
}  // namespace aloha::url
#endif
