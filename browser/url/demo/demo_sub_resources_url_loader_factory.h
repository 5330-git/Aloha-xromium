// Copyright (c) 2025 The Aloha-Xromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_BROWSER_URL_DEMO_DEMO_SUB_RESOURCES_URL_LOADER_FACTORY_H_
#define ALOHA_BROWSER_URL_DEMO_DEMO_SUB_RESOURCES_URL_LOADER_FACTORY_H_
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "services/network/public/cpp/self_deleting_url_loader_factory.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h"

// 负责加载子资源的URL加载器工厂。一般 HTML 主体的请求的处理发生在 Navigation
// 相关的加载器中，但是 HTML
// 中有可能引用一些其它的资源，比如图片、CSS、JS等，这些资源的加载是通过子资源的加载器来实现的。
// 子资源的加载时，Chromium 会在
// content\browser\renderer_host\render_frame_host_impl.cc:
// RenderFrameHostImpl::CommitNavigation 中注册 file://
// 协议的URL加载器工厂。但对于 aloha
// 来说，直接在内核代码上添加会增加代码耦合度，幸运的是 Chromium 通过
// content::ContentBrowserClient 预留了一个接口
// `RegisterNonNetworkURLLoaderFactories`
// 来注册自定义的URL加载器工厂，所以我们可以在这个接口中注册我们的工厂
namespace aloha::url {
class DemoSubResourcesURLLoaderFactory
    : public network::SelfDeletingURLLoaderFactory {
 public:
  // static
  // 参考 content\browser\data_url_loader_factory.cc
  static mojo::PendingRemote<network::mojom::URLLoaderFactory> Create();
  constexpr static char kInterceptHost[] =
      "demo_sub_resources_url_loader_factory";
  explicit DemoSubResourcesURLLoaderFactory(
      mojo::PendingReceiver<network::mojom::URLLoaderFactory> factory_receiver);
  DemoSubResourcesURLLoaderFactory();
  ~DemoSubResourcesURLLoaderFactory() override;
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
