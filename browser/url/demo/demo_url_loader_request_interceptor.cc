#include "demo_url_loader_request_interceptor.h"

#include "aloha/common/aloha_constants.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "mojo/public/cpp/system/data_pipe.h"
#include "mojo/public/cpp/system/data_pipe_producer.h"
#include "mojo/public/cpp/system/string_data_source.h"
#include "net/base/net_errors.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"

namespace aloha::url {
void DemoURLLoaderRequestInterceptor::MaybeCreateLoader(
    const network::ResourceRequest& resource_request,
    content::BrowserContext* browser_context,
    content::URLLoaderRequestInterceptor::LoaderCallback callback) {
  // 检查请求的 URL 是否为自定义协议
  LOG(INFO) << "DemoURLLoaderRequestInterceptor::MaybeCreateLoader";
  if (resource_request.url.scheme() == aloha::url::kAlohaScheme &&
      resource_request.url.host() == "demo") {
    // 创建自定义的 URLLoader 处理请求
    mojo::PendingRemote<network::mojom::URLLoader> loader;
    mojo::PendingRemote<network::mojom::URLLoaderClient> client;

    auto request_handler = [](const network::ResourceRequest& resource_request,
                              mojo::PendingReceiver<network::mojom::URLLoader>
                                  loader,
                              mojo::PendingRemote<
                                  network::mojom::URLLoaderClient> client) {
      // Response Write
      auto response = network::mojom::URLResponseHead::New();
      // 填写响应
      response->headers =
          base::MakeRefCounted<net::HttpResponseHeaders>("HTTP/1.1 200 OK");
      // TODO(yeyun.anton): 实现 CORS Allow

      response->mime_type = "text/html";

      // 创建数据管道，将数据从 Browser 进程发到 Render 进程
      mojo::ScopedDataPipeProducerHandle producer_handle;
      mojo::ScopedDataPipeConsumerHandle consumer_handle;

      auto remote_url_loader_client =
          std::make_unique<mojo::Remote<network::mojom::URLLoaderClient>>(
              std::move(client));
      if (mojo::CreateDataPipe(nullptr, producer_handle, consumer_handle) !=
          MOJO_RESULT_OK) {
        (*remote_url_loader_client)
            ->OnComplete(network::URLLoaderCompletionStatus(
                net::ERR_INSUFFICIENT_RESOURCES));
        return;
      }

      (*remote_url_loader_client)
          ->OnReceiveResponse(std::move(response), std::move(consumer_handle),
                              std::nullopt);
      // Write HTML content to the data pipe
      auto producer =
          std::make_unique<mojo::DataPipeProducer>(std::move(producer_handle));
      mojo::DataPipeProducer* raw_producer_ptr = producer.get();
      constexpr char kTestAlohaResponse[] = R"(
        <html>
        <body>
        <h1>Hello World</h1>
        <button onclick=window.open('aloha://demo/demo_url_loader_request_interceptor')>aloha://test</button>
        <button onclick=window.open('aloha://test2')>aloha://test2</button>
        <a href='file:///D:/codes/build-chromium/chromium/src/aloha/resources/browser/aloha-app-main/dist/index.html'>to main page from file</a>
        </body>
        </html>
        )";
      raw_producer_ptr->Write(
          std::make_unique<mojo::StringDataSource>(
              kTestAlohaResponse, mojo::StringDataSource::AsyncWritingMode::
                                      STRING_STAYS_VALID_UNTIL_COMPLETION),
          base::BindOnce<>(
              [](std::unique_ptr<mojo::DataPipeProducer> producer,
                 std::unique_ptr<mojo::Remote<network::mojom::URLLoaderClient>>
                     remote_url_loader_client,
                 MojoResult result) {
                LOG(INFO) << "Write Complete";
                if (result != MOJO_RESULT_OK) {
                  LOG(INFO) << "Write Error";
                  (*remote_url_loader_client)
                      ->OnComplete(
                          network::URLLoaderCompletionStatus(net::ERR_FAILED));
                } else {
                  LOG(INFO) << "Write Success";
                  (*remote_url_loader_client)
                      ->OnComplete(network::URLLoaderCompletionStatus(net::OK));
                }
              },
              // 把 producer 和 remote_url_loader_client 的所有权移入到
              // lambda 表达式中，离开时销毁
              std::move(producer), std::move(remote_url_loader_client)));
    };

    std::move(callback).Run(base::BindOnce<>(request_handler));
    return;
  }

  // 如果不处理该请求，则传入空的 RequestHandler
  std::move(callback).Run(base::NullCallback());
  //
}

bool DemoURLLoaderRequestInterceptor::MaybeCreateLoaderForResponse(
    const network::URLLoaderCompletionStatus& status,
    const network::ResourceRequest& request,
    network::mojom::URLResponseHeadPtr* response_head,
    mojo::ScopedDataPipeConsumerHandle* response_body,
    mojo::PendingRemote<network::mojom::URLLoader>* loader,
    mojo::PendingReceiver<network::mojom::URLLoaderClient>* client_receiver,
    blink::ThrottlingURLLoader* url_loader) {
  LOG(INFO) << "DemoURLLoaderRequestInterceptor::MaybeCreateLoaderForResponse";
  return false;
}
}  // namespace aloha::url
