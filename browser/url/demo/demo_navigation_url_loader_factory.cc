#include "aloha/browser/url/demo/demo_navigation_url_loader_factory.h"

#include "aloha/common/aloha_constants.h"
#include "aloha/common/aloha_paths.h"
#include "demo_navigation_url_loader_factory.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/system/data_pipe_producer.h"
#include "mojo/public/cpp/system/string_data_source.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/url_loader_completion_status.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"

namespace aloha::url {
namespace {
// 参考 content\browser\data_url_loader_factory.cc
struct WriteData {
  mojo::Remote<network::mojom::URLLoaderClient> client;
  std::string data;
  std::unique_ptr<mojo::DataPipeProducer> producer;
};
void OnWrite(std::unique_ptr<WriteData> write_data, MojoResult result) {
  if (result != MOJO_RESULT_OK) {
    write_data->client->OnComplete(
        network::URLLoaderCompletionStatus(net::ERR_FAILED));
    return;
  }

  network::URLLoaderCompletionStatus status(net::OK);
  status.encoded_data_length = write_data->data.size();
  status.encoded_body_length = write_data->data.size();
  status.decoded_body_length = write_data->data.size();
  write_data->client->OnComplete(status);
}
}  // namespace

mojo::PendingRemote<network::mojom::URLLoaderFactory>
DemoNavigationURLLoaderFactory::Create() {
  mojo::PendingRemote<network::mojom::URLLoaderFactory> remote;
  // The DemoNavigationURLLoaderFactory will delete itself when there are no
  // more receivers - see the
  // network::SelfDeletingURLLoaderFactory::OnDisconnect method.
  new DemoNavigationURLLoaderFactory(remote.InitWithNewPipeAndPassReceiver());
  return remote;
}

DemoNavigationURLLoaderFactory::~DemoNavigationURLLoaderFactory() = default;
DemoNavigationURLLoaderFactory::DemoNavigationURLLoaderFactory(
    mojo::PendingReceiver<network::mojom::URLLoaderFactory> factory_receiver)
    : network::SelfDeletingURLLoaderFactory(std::move(factory_receiver)) {}

void DemoNavigationURLLoaderFactory::CreateLoaderAndStart(
    mojo::PendingReceiver<network::mojom::URLLoader> loader,
    int32_t request_id,
    uint32_t options,
    const network::ResourceRequest& request,
    mojo::PendingRemote<network::mojom::URLLoaderClient> client,
    const net::MutableNetworkTrafficAnnotationTag& traffic_annotation) {
  LOG(INFO) << "DemoNavigationURLLoaderFactory::CreateLoaderAndStart";
  if (request.url.scheme() == aloha::url::kAlohaDemoScheme &&
      request.url.host() == kInterceptHost) {
    auto response = network::mojom::URLResponseHead::New();
    // 填写响应
    response->headers =
        base::MakeRefCounted<net::HttpResponseHeaders>("HTTP/1.1 200 OK");
    response->headers->AddCookie("test=test");
    response->headers->AddHeader("Content-Type", "text/html");
    // TODO(yeyun.anton): 实现 CORS Allow
    response->charset = "utf-8";
    response->mime_type = "text/html";
    response->response_type = network::mojom::FetchResponseType::kBasic;
    response->response_time = base::Time::Now();

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
    base::FilePath aloha_home_path;
    aloha::path_service::GetWebAppPath(&aloha_home_path,
                                       webapp::internal::kAlohaHome);
    constexpr char kAlohaResponseData[] = R"(<!DOCTYPE html>
<body>
  <div class='vertical-layout-group'>
  <h1>DemoNavigationURLLoaderFactory</h1>
  <h1>Hello World</h1>
  <img src='aloha-demo://demo_sub_resources_url_loader_factory/test.png' />
    <button
      onclick=window.open('aloha-demo://demo_url_loader_request_interceptor')>aloha-demo://demo_url_loader_request_interceptor</button>
    <br>
    <button
      onclick=window.open('aloha-demo://demo_navigation_url_loader_factory')>demo_navigation_url_loader_factory</button>
    <br>
    <button onclick=window.open('aloha-demo-unexsist-shceme://test2')>aloha-demo-unexsist-shceme://test2</button>
  </div>
  <script src='aloha-demo://demo_sub_resources_url_loader_factory/test.js'></script>
</body>
<style>
  .vertical-layout-group {
    display: flex;
    flex-direction: column;
    width: 50%;
    margin: 0 auto;
  }
</style>
</html>
)";
    // 下面参考 net::DataURLLoaderFactory 实现
    auto write_data = std::make_unique<WriteData>();
    write_data->client = std::move(*remote_url_loader_client);
    // data 的所有权转移到了 write_data 中，而 write_data
    // 在数据写入完成后才会销毁，因此可以使用 STRING_STAY_VALID_UNTIL_COMPLETION
    write_data->data = kAlohaResponseData;
    write_data->producer = std::move(producer);
    std::string_view data_view(write_data->data);
    raw_producer_ptr->Write(
        std::make_unique<mojo::StringDataSource>(
            data_view, mojo::StringDataSource::AsyncWritingMode::
                           STRING_STAYS_VALID_UNTIL_COMPLETION),
        base::BindOnce(OnWrite, std::move(write_data)));

    return;
  }
  LOG(INFO) << "Skip Response:" << request.url.spec();
}
}  // namespace aloha::url
