#include "aloha/browser/url/aloha_url_loader_factory.h"

#include <memory>

#include "aloha/browser/url/aloha_apps_url_loader.h"
#include "aloha/common/aloha_constants.h"
#include "aloha/common/aloha_paths.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/location.h"
#include "base/path_service.h"
#include "base/task/bind_post_task.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "components/file_access/scoped_file_access_delegate.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/file_url_loader.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "net/base/filename_util.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/mojom/fetch_api.mojom-shared.h"
#include "services/network/public/mojom/url_loader.mojom.h"

namespace aloha::url {

mojo::PendingRemote<network::mojom::URLLoaderFactory>
AlohaURLLoaderFactory::Create(content::BrowserContext* browser_context) {
  mojo::PendingRemote<network::mojom::URLLoaderFactory> remote;
  // The AlohaURLLoaderFactory will delete itself when there are no
  // more receivers - see the
  // network::SelfDeletingURLLoaderFactory::OnDisconnect method.
  new AlohaURLLoaderFactory(remote.InitWithNewPipeAndPassReceiver(),
                            browser_context);
  return remote;
}

// 这里我们选择 task_runner 的调度优先级和 TaskPriority::USER_BLOCKING :
// Loading and rendering a web page after the user clicks a link.
// 参考：content\browser\renderer_host\render_frame_host_impl.cc:11655

AlohaURLLoaderFactory::AlohaURLLoaderFactory(
    mojo::PendingReceiver<network::mojom::URLLoaderFactory> factory_receiver,
    content::BrowserContext* browser_context)
    : network::SelfDeletingURLLoaderFactory(std::move(factory_receiver)),
      task_runner_(base::ThreadPool::CreateSequencedTaskRunner(
          {base::MayBlock(), base::TaskPriority::USER_BLOCKING,
           base::TaskShutdownBehavior::SKIP_ON_SHUTDOWN})),
      browser_context_(browser_context) {}

AlohaURLLoaderFactory::~AlohaURLLoaderFactory() = default;

void AlohaURLLoaderFactory::CreateLoaderAndStart(
    mojo::PendingReceiver<network::mojom::URLLoader> loader,
    int32_t request_id,
    uint32_t options,
    const network::ResourceRequest& request,
    mojo::PendingRemote<network::mojom::URLLoaderClient> client,
    const net::MutableNetworkTrafficAnnotationTag& traffic_annotation) {
  LOG(INFO) << "AlohaURLLoaderFactory::CreateLoaderAndStart";
  if (!request.url.SchemeIs(aloha::url::kAlohaScheme)) {
    LOG(INFO) << "Skip Response: " << request.url.spec();
    return;
  }

  if (request.url.host() == aloha::url::kAlohaAppsHost) {
    base::FilePath resource_path;
    base::PathService::Get(aloha::path_service::ALOHA_WEB_APP_RESOURCES_DIR,
                           &resource_path);
    resource_path = resource_path.AppendASCII(request.url.path());
    // TODO(yeyun.anton): 实现重定向
    // 如果 url.path() 为空，也就是说请求的 url 是 aloha://apps/
    // 那么我们需要将请求重定向到 aloha://apps/aloha-app-main/index.html
    // if (request.url.path().empty()) {
    //   GURL new_url =
    //       GURL(aloha::url::kAlohaScheme + std::string("://") +
    //            aloha::url::kAlohaAppsHost + std::string("/") +
    //            aloha::webapp::internal::kAlohaHome + std::string("/"));
    //   network::ResourceRequest new_request;
    //   new_request.url = new_url;
    //   CreateLoaderAndStart(std::move(loader), request_id, options, new_request,
    //                        std::move(client), traffic_annotation);
    //   LOG(INFO) << "Redirect to: " << new_url.spec();
    //   return;
    // }

    auto cb = base::BindPostTask(
        task_runner_,
        base::BindOnce(
            &AlohaAppsURLLoader::CreateAndStart, browser_context_->GetPath(),
            request, network::mojom::FetchResponseType::kDefault,
            std::move(loader), std::move(client),
            DirectoryLoadingPolicy::kRespondWithListing,
            FileAccessPolicy::kRestricted, LinkFollowingPolicy::kFollow,
            std::unique_ptr<content::FileURLLoaderObserver>(),
            nullptr /* extra_response_headers */));
    if (auto* file_access = file_access::ScopedFileAccessDelegate::Get()) {
      // If the request has an initiator use it as source for the dlp check.
      // Requests with no initiator e.g. user actions the request should be
      // granted.
      if (request.request_initiator && !request.request_initiator->opaque()) {
        file_access->RequestFilesAccess({resource_path},
                                        request.request_initiator->GetURL(),
                                        std::move(cb));
      } else {
        file_access->RequestFilesAccessForSystem({resource_path},
                                                 std::move(cb));
      }
    } else {
      std::move(cb).Run(file_access::ScopedFileAccess::Allowed());
    }
    // }
  }
  // TODO(yeyun.anton): 注册其它的 HOST
}

}  // namespace aloha::url
