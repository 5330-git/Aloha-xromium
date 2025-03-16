#include "aloha/browser/url/demo/demo_sub_resources_url_loader_factory.h"

#include "aloha/common/aloha_constants.h"
#include "aloha/common/aloha_paths.h"
#include "aloha/grit/aloha_resources.h"
#include "base/memory/ref_counted_memory.h"
#include "demo_navigation_url_loader_factory.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/system/data_pipe_producer.h"
#include "mojo/public/cpp/system/string_data_source.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/url_loader_completion_status.h"
#include "services/network/public/mojom/url_loader.mojom.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "ui/base/models/image_model.h"

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
DemoSubResourcesURLLoaderFactory::Create() {
  mojo::PendingRemote<network::mojom::URLLoaderFactory> remote;
  // The DemoSubResourcesURLLoaderFactory will delete itself when there are no
  // more receivers - see the
  // network::SelfDeletingURLLoaderFactory::OnDisconnect method.
  new DemoSubResourcesURLLoaderFactory(remote.InitWithNewPipeAndPassReceiver());
  return remote;
}

DemoSubResourcesURLLoaderFactory::~DemoSubResourcesURLLoaderFactory() = default;
DemoSubResourcesURLLoaderFactory::DemoSubResourcesURLLoaderFactory(
    mojo::PendingReceiver<network::mojom::URLLoaderFactory> factory_receiver)
    : network::SelfDeletingURLLoaderFactory(std::move(factory_receiver)) {}

void DemoSubResourcesURLLoaderFactory::CreateLoaderAndStart(
    mojo::PendingReceiver<network::mojom::URLLoader> loader,
    int32_t request_id,
    uint32_t options,
    const network::ResourceRequest& request,
    mojo::PendingRemote<network::mojom::URLLoaderClient> client,
    const net::MutableNetworkTrafficAnnotationTag& traffic_annotation) {
  LOG(INFO) << "DemoSubResourcesURLLoaderFactory::CreateLoaderAndStart";
  if (request.url.scheme() == aloha::url::kAlohaDemoScheme &&
      request.url.host() == kInterceptHost) {
    auto response = network::mojom::URLResponseHead::New();
    // 填写响应
    response->headers =
        base::MakeRefCounted<net::HttpResponseHeaders>("HTTP/1.1 200 OK");

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
    std::string data;

    // 根据不同的资源类型返回默认的数据
    response->response_type = network::mojom::FetchResponseType::kBasic;
    response->response_time = base::Time::Now();
    if (request.url.ExtractFileName() == "test.js") {
      response->headers->AddHeader("Content-Type", "application/javascript");
      response->response_type = network::mojom::FetchResponseType::kDefault;
      response->charset = "utf-8";
      response->mime_type = "application/javascript";
      data = R"(
       var aloha_demo_api = {
         test: function() {
           console.log('test');
         }
       } 
       aloha_demo_api.test();
      )";
    } else if (request.url.ExtractFileName() == "test.png") {
      response->headers->AddHeader("Content-Type", "image/png");
      response->response_type = network::mojom::FetchResponseType::kDefault;
      response->mime_type = "image/png";
      ui::ImageModel image_model = ui::ImageModel::FromResourceId(IDR_ALOHA_ICON);
      auto image_bytes = image_model.GetImage().As1xPNGBytes();
      data = std::string(image_bytes->begin(), image_bytes->end());
    } else {
      // 数据不存在
    }
    (*remote_url_loader_client)
        ->OnReceiveResponse(std::move(response), std::move(consumer_handle),
                            std::nullopt);
    // Write HTML content to the data pipe
    auto producer =
        std::make_unique<mojo::DataPipeProducer>(std::move(producer_handle));
    mojo::DataPipeProducer* raw_producer_ptr = producer.get();


    // 下面参考 net::DataURLLoaderFactory 实现
    auto write_data = std::make_unique<WriteData>();
    write_data->client = std::move(*remote_url_loader_client);
    // data 的所有权转移到了 write_data 中，而 write_data
    // 在数据写入完成后才会销毁，因此可以使用 STRING_STAY_VALID_UNTIL_COMPLETION
    write_data->data = std::move(data);
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
