#include "aloha/browser/url/aloha_apps_url_loader.h"

#include "aloha/common/aloha_paths.h"
#include "aloha_apps_url_loader.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "content/public/browser/content_browser_client.h"
#include "aloha/common/aloha_constants.h"
#include "aloha/common/aloha_content_client.h"
#include "base/files/file_util.h"
#include "base/i18n/time_formatting.h"
#include "base/win/shortcut.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/system/file_data_source.h"
#include "mojo/public/cpp/system/string_data_source.h"
#include "net/base/filename_util.h"
#include "net/base/mime_sniffer.h"
#include "net/base/mime_util.h"
#include "services/network/public/cpp/features.h"
#include "services/network/public/cpp/resource_request.h"

namespace aloha::url {

constexpr size_t kDefaultFileDirectoryLoaderPipeSize = 65536;

GURL AppendUrlSeparator(const GURL& url) {
  std::string new_path = url.path() + '/';
  GURL::Replacements replacements;
  replacements.SetPathStr(new_path);
  return url.ReplaceComponents(replacements);
}

net::Error ConvertMojoResultToNetError(MojoResult result) {
  switch (result) {
    case MOJO_RESULT_OK:
      return net::OK;
    case MOJO_RESULT_NOT_FOUND:
      return net::ERR_FILE_NOT_FOUND;
    case MOJO_RESULT_PERMISSION_DENIED:
      return net::ERR_ACCESS_DENIED;
    case MOJO_RESULT_RESOURCE_EXHAUSTED:
      return net::ERR_INSUFFICIENT_RESOURCES;
    case MOJO_RESULT_ABORTED:
      return net::ERR_ABORTED;
    default:
      return net::ERR_FAILED;
  }
}

MojoResult ConvertNetErrorToMojoResult(net::Error net_error) {
  // Note: For now, only return specific errors that our obervers care about.
  switch (net_error) {
    case net::OK:
      return MOJO_RESULT_OK;
    case net::ERR_FILE_NOT_FOUND:
      return MOJO_RESULT_NOT_FOUND;
    case net::ERR_ACCESS_DENIED:
      return MOJO_RESULT_PERMISSION_DENIED;
    case net::ERR_INSUFFICIENT_RESOURCES:
      return MOJO_RESULT_RESOURCE_EXHAUSTED;
    case net::ERR_ABORTED:
    case net::ERR_CONNECTION_ABORTED:
      return MOJO_RESULT_ABORTED;
    default:
      return MOJO_RESULT_UNKNOWN;
  }
}

bool ConvertAlohaAppsUrlToFilePath(const GURL& url, base::FilePath* out_path) {
  if (!url.is_valid() || !url.SchemeIs(aloha::url::kAlohaScheme) ||
      url.host() != aloha::url::kAlohaAppsHost) {
    return false;
  }
  std::string path = url.path();
  // 默认加载 kAlohaHome
  if (path.empty() || path.ends_with("/")) {
    aloha::path_service::GetWebAppPath(out_path,
                                       aloha::webapp::internal::kAlohaHome);
  } else {
    base::PathService::Get(aloha::path_service::ALOHA_WEB_APP_RESOURCES_DIR,
                           out_path);
    *out_path = out_path->AppendASCII(path);
  }
  return true;
}

AlohaAppsURLLoader::AlohaAppsURLLoader() = default;
AlohaAppsURLLoader::~AlohaAppsURLLoader() = default;

AlohaAppsURLLoader::RedirectData::RedirectData() = default;
AlohaAppsURLLoader::RedirectData::~RedirectData() = default;

void AlohaAppsURLLoader::CreateAndStart(
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
    file_access::ScopedFileAccess file_access) {
  // Owns itself. Will live as long as its URLLoader and URLLoaderClient
  // bindings are alive - essentially until either the client gives up or all
  // file data has been sent to it.
  auto* file_url_loader = new AlohaAppsURLLoader;
  file_url_loader->Start(
      profile_path, request, response_type, std::move(loader),
      std::move(client_remote), directory_loading_policy, file_access_policy,
      link_following_policy, std::move(observer),
      std::move(extra_response_headers), std::move(file_access));
}

void AlohaAppsURLLoader::FollowRedirect(
    const std::vector<std::string>& removed_headers,
    const net::HttpRequestHeaders& modified_headers,
    const net::HttpRequestHeaders& modified_cors_exempt_headers,
    const std::optional<GURL>& new_url) {
  // |removed_headers| and |modified_headers| are unused. It doesn't make
  // sense for files. The AlohaAppsURLLoader can redirect only to another
  // file.
  LOG(INFO) << "AlohaAppsURLLoader::FollowRedirect";
  std::unique_ptr<RedirectData> redirect_data = std::move(redirect_data_);
  if (redirect_data->is_directory) {
    // AlohaAppsURLDirectoryLoader::CreateAndStart(
    //     redirect_data->profile_path, redirect_data->request,
    //     redirect_data->response_type, receiver_.Unbind(), client_.Unbind(),
    //     std::move(redirect_data->observer),
    //     std::move(redirect_data->extra_response_headers));
  } else {
    AlohaAppsURLLoader::CreateAndStart(
        redirect_data->profile_path, redirect_data->request,
        redirect_data->response_type, receiver_.Unbind(), client_.Unbind(),
        redirect_data->directory_loading_policy,
        redirect_data->file_access_policy, redirect_data->link_following_policy,
        std::move(redirect_data->observer),
        std::move(redirect_data->extra_response_headers),
        std::move(*redirect_data->file_access.release()));
  }
  MaybeDeleteSelf();
}

void AlohaAppsURLLoader::SetPriority(net::RequestPriority priority,
                                     int32_t intra_priority_value) {}
void AlohaAppsURLLoader::PauseReadingBodyFromNet() {}

void AlohaAppsURLLoader::ResumeReadingBodyFromNet() {}

void AlohaAppsURLLoader::Start(
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
    file_access::ScopedFileAccess file_access) {
  // ClusterFuzz depends on the following VLOG to get resource dependencies.
  // See crbug.com/715656.
  VLOG(1) << "AlohaAppsURLLoader::Start: " << request.url;

  auto head = network::mojom::URLResponseHead::New();
  head->request_start = base::TimeTicks::Now();
  head->response_start = base::TimeTicks::Now();
  head->response_type = response_type;
  head->headers = extra_response_headers;
  receiver_.Bind(std::move(loader));
  receiver_.set_disconnect_handler(base::BindOnce(
      &AlohaAppsURLLoader::OnMojoDisconnect, base::Unretained(this)));

  client_.Bind(std::move(client_remote));

  if (!file_access.is_allowed()) {
    OnClientComplete(net::ERR_FAILED, std::move(observer));
    return;
  }

  base::FilePath path;
  // if (!net::FileURLToFilePath(request.url, &path)) {
  if (!ConvertAlohaAppsUrlToFilePath(request.url, &path)) {
    OnClientComplete(net::ERR_FAILED, std::move(observer));
    return;
  }

  base::File::Info info;
  if (!base::GetFileInfo(path, &info)) {
    OnClientComplete(net::ERR_FILE_NOT_FOUND, std::move(observer));
    return;
  }

  if (info.is_directory) {
    if (directory_loading_policy == DirectoryLoadingPolicy::kFail) {
      OnClientComplete(net::ERR_FILE_NOT_FOUND, std::move(observer));
      return;
    }

    DCHECK_EQ(directory_loading_policy,
              DirectoryLoadingPolicy::kRespondWithListing);

    net::RedirectInfo redirect_info;
    redirect_info.new_method = "GET";
    redirect_info.status_code = 301;
    redirect_info.new_url = path.EndsWithSeparator()
                                ? request.url
                                : AppendUrlSeparator(request.url);
    head->encoded_data_length = 0;

    redirect_data_ = std::make_unique<RedirectData>();
    redirect_data_->is_directory = true;
    redirect_data_->profile_path = std::move(profile_path);
    redirect_data_->request = request;
    redirect_data_->directory_loading_policy = directory_loading_policy;
    redirect_data_->file_access_policy = file_access_policy;
    redirect_data_->link_following_policy = link_following_policy;
    redirect_data_->request.url = redirect_info.new_url;
    redirect_data_->observer = std::move(observer);
    redirect_data_->response_type = response_type;
    redirect_data_->extra_response_headers = std::move(extra_response_headers);
    redirect_data_->file_access =
        std::make_unique<file_access::ScopedFileAccess>(std::move(file_access));

    client_->OnReceiveRedirect(redirect_info, std::move(head));
    return;
  }

  // Full file path with all symbolic links resolved.
  base::FilePath full_path = base::MakeAbsoluteFilePath(path);
  if (file_access_policy == FileAccessPolicy::kRestricted &&
      !aloha::AlohaContentClient::GetContentClient()
           ->browser()
           ->IsFileAccessAllowed(path, full_path, profile_path)) {
    OnClientComplete(net::ERR_ACCESS_DENIED, std::move(observer));
    return;
  }

#if BUILDFLAG(IS_WIN)
  base::FilePath shortcut_target;
  if (link_following_policy == LinkFollowingPolicy::kFollow &&
      base::EqualsCaseInsensitiveASCII(path.Extension(), ".lnk") &&
      base::win::ResolveShortcut(path, &shortcut_target, nullptr)) {
    // Follow Windows shortcuts
    redirect_data_ = std::make_unique<RedirectData>();
    if (!base::GetFileInfo(shortcut_target, &info)) {
      OnClientComplete(net::ERR_FILE_NOT_FOUND, std::move(observer));
      return;
    }

    GURL new_url = net::FilePathToFileURL(shortcut_target);
    if (info.is_directory && !path.EndsWithSeparator()) {
      new_url = AppendUrlSeparator(new_url);
    }

    net::RedirectInfo redirect_info;
    redirect_info.new_method = "GET";
    redirect_info.status_code = 301;
    redirect_info.new_url = new_url;
    head->encoded_data_length = 0;

    redirect_data_->is_directory = info.is_directory;
    redirect_data_->profile_path = std::move(profile_path);
    redirect_data_->request = request;
    redirect_data_->response_type = response_type;
    redirect_data_->directory_loading_policy = directory_loading_policy;
    redirect_data_->file_access_policy = file_access_policy;
    redirect_data_->link_following_policy = link_following_policy;
    redirect_data_->request.url = redirect_info.new_url;
    redirect_data_->observer = std::move(observer);
    redirect_data_->extra_response_headers = std::move(extra_response_headers);
    redirect_data_->file_access =
        std::make_unique<file_access::ScopedFileAccess>(std::move(file_access));

    client_->OnReceiveRedirect(redirect_info, std::move(head));
    return;
  }
#endif  // BUILDFLAG(IS_WIN)

  mojo::ScopedDataPipeProducerHandle producer_handle;
  mojo::ScopedDataPipeConsumerHandle consumer_handle;

  // Request the larger size data pipe for file:// URL loading.
  uint32_t data_pipe_size = network::features::GetDataPipeDefaultAllocationSize(
      network::features::DataPipeAllocationSize::kLargerSizeIfPossible);
  // This should already be static_asserted in network::features, but good
  // to double-check.
  DCHECK(data_pipe_size >= net::kMaxBytesToSniff)
      << "Default file data pipe size must be at least as large as a "
         "MIME-type sniffing buffer.";

  if (mojo::CreateDataPipe(data_pipe_size, producer_handle, consumer_handle) !=
      MOJO_RESULT_OK) {
    OnClientComplete(net::ERR_FAILED, std::move(observer));
    return;
  }

  // Should never be possible for this to be a directory. If
  // AlohaAppsURLLoader is used to respond to a directory request, it must be
  // because the URL path didn't have a trailing path separator. In that case
  // we finish with a redirect above which will in turn be handled by
  // AlohaAppsURLDirectoryLoader.
  DCHECK(!info.is_directory);
  if (observer) {
    observer->OnStart();
  }

  auto file_data_source = std::make_unique<mojo::FileDataSource>(
      base::File(path, base::File::FLAG_OPEN | base::File::FLAG_READ));

  std::vector<char> initial_read_buffer(net::kMaxBytesToSniff);
  auto read_result =
      file_data_source->Read(0u, base::span<char>(initial_read_buffer));
  if (read_result.result != MOJO_RESULT_OK) {
    // This can happen when the file is unreadable (which can happen during
    // corruption). We need to be sure to inform the observer that we've
    // finished reading so that it can proceed.
    if (observer) {
      observer->OnRead(base::span<char>(), &read_result);
      observer->OnDone();
    }
    client_->OnComplete(network::URLLoaderCompletionStatus(
        ConvertMojoResultToNetError(read_result.result)));
    client_.reset();
    MaybeDeleteSelf();
    return;
  }
  if (observer) {
    observer->OnRead(base::span<char>(initial_read_buffer), &read_result);
  }

  uint64_t initial_read_size = read_result.bytes_read;

  net::HttpByteRange byte_range;
  if (std::optional<std::string> range_header =
          request.headers.GetHeader(net::HttpRequestHeaders::kRange);
      range_header) {
    // Handle a simple Range header for a single range.
    std::vector<net::HttpByteRange> ranges;
    bool fail = false;
    if (net::HttpUtil::ParseRangeHeader(*range_header, &ranges) &&
        ranges.size() == 1) {
      byte_range = ranges[0];
      if (!byte_range.ComputeBounds(info.size)) {
        fail = true;
      }
    } else {
      fail = true;
    }

    if (fail) {
      OnClientComplete(net::ERR_REQUEST_RANGE_NOT_SATISFIABLE,
                       std::move(observer));
      return;
    }
  }

  uint64_t first_byte_to_send = 0;
  uint64_t total_bytes_to_send = info.size;

  if (byte_range.IsValid()) {
    first_byte_to_send = byte_range.first_byte_position();
    total_bytes_to_send =
        byte_range.last_byte_position() - first_byte_to_send + 1;
  }

  total_bytes_written_ = total_bytes_to_send;

  head->content_length = base::saturated_cast<int64_t>(total_bytes_to_send);

  if (first_byte_to_send < initial_read_size) {
    // Write any data we read for MIME sniffing, constraining by range where
    // applicable. This will always fit in the pipe (see DCHECK above, and
    // assertions near network::features::GetDataPipeDefaultAllocationSize()).
    base::span<const uint8_t> bytes_to_write =
        base::as_byte_span(initial_read_buffer)
            .subspan(base::checked_cast<size_t>(first_byte_to_send));
    bytes_to_write = bytes_to_write.first(base::checked_cast<size_t>(std::min(
        static_cast<uint64_t>(bytes_to_write.size()), total_bytes_to_send)));

    size_t actually_written_bytes = 0;
    MojoResult result = producer_handle->WriteData(
        bytes_to_write, MOJO_WRITE_DATA_FLAG_NONE, actually_written_bytes);
    if (result != MOJO_RESULT_OK ||
        actually_written_bytes != bytes_to_write.size()) {
      OnFileWritten(std::move(observer), result);
      return;
    }

    // Discount the bytes we just sent from the total range.
    first_byte_to_send = initial_read_size;
    total_bytes_to_send -= actually_written_bytes;
  }

  if (!net::GetMimeTypeFromFile(full_path, &head->mime_type)) {
    std::string new_type;
    net::SniffMimeType(
        std::string_view(initial_read_buffer.data(), read_result.bytes_read),
        request.url, head->mime_type,
        aloha::AlohaContentClient::GetContentClient()
                ->browser()
                ->ForceSniffingFileUrlsForHtml()
            ? net::ForceSniffFileUrlsForHtml::kEnabled
            : net::ForceSniffFileUrlsForHtml::kDisabled,
        &new_type);
    head->mime_type.assign(new_type);
    head->did_mime_sniff = true;
  }
  if (!head->headers) {
    head->headers =
        base::MakeRefCounted<net::HttpResponseHeaders>("HTTP/1.1 200 OK");
  }
  head->headers->AddHeader(net::HttpRequestHeaders::kContentType,
                           head->mime_type);
  // We add a Last-Modified header to file responses so that our
  // implementation of document.lastModified can access it (crbug.com/875299).
  head->headers->AddHeader(net::HttpResponseHeaders::kLastModified,
                           base::TimeFormatHTTP(info.last_modified));
  client_->OnReceiveResponse(std::move(head), std::move(consumer_handle),
                             std::nullopt);

  if (total_bytes_to_send == 0) {
    // There's definitely no more data, so we're already done.
    OnFileWritten(std::move(observer), MOJO_RESULT_OK);
    return;
  }

  // In case of a range request, seek to the appropriate position before
  // sending the remaining bytes asynchronously. Under normal conditions
  // (i.e., no range request) this Seek is effectively a no-op.
  file_data_source->SetRange(first_byte_to_send,
                             first_byte_to_send + total_bytes_to_send);
  if (observer) {
    observer->OnSeekComplete(first_byte_to_send);
  }

  data_producer_ =
      std::make_unique<mojo::DataPipeProducer>(std::move(producer_handle));
  data_producer_->Write(std::make_unique<mojo::FilteredDataSource>(
                            std::move(file_data_source), std::move(observer)),
                        base::BindOnce(&AlohaAppsURLLoader::OnFileWritten,
                                       base::Unretained(this), nullptr));
}

void AlohaAppsURLLoader::OnMojoDisconnect() {
  data_producer_.reset();
  receiver_.reset();
  client_.reset();
  MaybeDeleteSelf();
}

void AlohaAppsURLLoader::OnClientComplete(
    net::Error net_error,
    std::unique_ptr<content::FileURLLoaderObserver> observer) {
  client_->OnComplete(network::URLLoaderCompletionStatus(net_error));
  client_.reset();
  if (observer) {
    if (net_error != net::OK) {
      mojo::DataPipeProducer::DataSource::ReadResult result;
      result.result = ConvertNetErrorToMojoResult(net_error);
      observer->OnRead(base::span<char>(), &result);
    }
    observer->OnDone();
  }
  MaybeDeleteSelf();
}

void AlohaAppsURLLoader::MaybeDeleteSelf() {
  if (!receiver_.is_bound() && !client_.is_bound()) {
    delete this;
  }
}

void AlohaAppsURLLoader::OnFileWritten(
    std::unique_ptr<content::FileURLLoaderObserver> observer,
    MojoResult result) {
  // All the data has been written now. Close the data pipe. The consumer will
  // be notified that there will be no more data to read from now.
  data_producer_.reset();
  if (observer) {
    observer->OnDone();
  }

  if (result == MOJO_RESULT_OK) {
    network::URLLoaderCompletionStatus status(net::OK);
    status.encoded_data_length = total_bytes_written_;
    status.encoded_body_length = total_bytes_written_;
    status.decoded_body_length = total_bytes_written_;
    client_->OnComplete(status);
  } else {
    client_->OnComplete(network::URLLoaderCompletionStatus(net::ERR_FAILED));
  }
  client_.reset();
  MaybeDeleteSelf();
}

}  // namespace aloha::url
