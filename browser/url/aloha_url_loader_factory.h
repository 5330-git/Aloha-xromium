// Copyright (c) 2025 The Aloha-Xromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_BROWSER_URL_ALOHA_URL_LOADER_FACTORY_H_
#define ALOHA_BROWSER_URL_ALOHA_URL_LOADER_FACTORY_H_

#include "base/memory/raw_ptr.h"
#include "content/public/browser/browser_context.h"
#include "services/network/public/cpp/self_deleting_url_loader_factory.h"
namespace aloha::url {
// Aloha-Xromium 的 URL 加载器工厂，负责 C++/JavaScript 混合应用的 URL 加载等
// 负责从文件系统中读取 Web 资源并生成 Response。
// 我们定义这些混合应用的 URL 格式为： aloha://apps/<app_name>/<file_path>
// 其中 <app_name> 为应用的名称，<file_path> 为应用中的文件路径。
// 例如： aloha://apps/aloha-app-main/index.html

// 有关加载器工厂可见：
// aloha::url::DemoNavigationURLLoaderFactory
// aloha::url::DemoSubResourcesURLLoaderFactory

// 参考 content\browser\loader\file_url_loader_factory.h :
// content::FileURLLoaderFactory
class AlohaURLLoaderFactory : public network::SelfDeletingURLLoaderFactory {
 public:
  // static
  static mojo::PendingRemote<network::mojom::URLLoaderFactory> Create(
      content::BrowserContext* browser_context);
  explicit AlohaURLLoaderFactory(
      mojo::PendingReceiver<network::mojom::URLLoaderFactory> factory_receiver,
      content::BrowserContext* browser_context);
  ~AlohaURLLoaderFactory() override;
  void CreateLoaderAndStart(
      mojo::PendingReceiver<network::mojom::URLLoader> loader,
      int32_t request_id,
      uint32_t options,
      const network::ResourceRequest& request,
      mojo::PendingRemote<network::mojom::URLLoaderClient> client,
      const net::MutableNetworkTrafficAnnotationTag& traffic_annotation)
      override;

 private:
  // 为了防止IO文件阻塞 UI
  // 线程，我们需要从线程池中创建一个单独的任务队列处理文件读取的问题
  const scoped_refptr<base::SequencedTaskRunner> task_runner_;
  base::raw_ptr<content::BrowserContext> browser_context_ = nullptr;
};

}  // namespace aloha::url
#endif
