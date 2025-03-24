
// 为了剔除 testonly， 遂从 shell_download_manager_delegate.cc 中拷贝以下代码
#ifndef ALOHA_TEMP_DEF__BROWSER_SHELL_DOWNLOAD_MANAGER_DELEGATE_H_
#define ALOHA_TEMP_DEF__BROWSER_SHELL_DOWNLOAD_MANAGER_DELEGATE_H_

#include <stdint.h>

#include "base/functional/callback_forward.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "components/download/public/common/download_item.h"
#include "components/download/public/common/download_target_info.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/download_manager_delegate.h"

namespace aloha {

class DownloadManager;

class AlohaDownloadManagerDelegate : public content::DownloadManagerDelegate {
 public:
  AlohaDownloadManagerDelegate();

  AlohaDownloadManagerDelegate(const AlohaDownloadManagerDelegate&) = delete;
  AlohaDownloadManagerDelegate& operator=(const AlohaDownloadManagerDelegate&) =
      delete;

  ~AlohaDownloadManagerDelegate() override;

  void SetDownloadManager(content::DownloadManager* manager);

  void Shutdown() override;
  bool DetermineDownloadTarget(
      download::DownloadItem* download,
      download::DownloadTargetCallback* callback) override;
  bool ShouldOpenDownload(
      download::DownloadItem* item,
      content::DownloadOpenDelayedCallback callback) override;
  void GetNextId(content::DownloadIdCallback callback) override;

  // Inhibits prompting and sets the default download path.
  void SetDownloadBehaviorForTesting(
      const base::FilePath& default_download_path);

 private:
  friend class base::RefCountedThreadSafe<AlohaDownloadManagerDelegate>;

  using FilenameDeterminedCallback =
      base::OnceCallback<void(const base::FilePath&)>;

  static void GenerateFilename(const GURL& url,
                               const std::string& content_disposition,
                               const std::string& suggested_filename,
                               const std::string& mime_type,
                               const base::FilePath& suggested_directory,
                               FilenameDeterminedCallback callback);
  void OnDownloadPathGenerated(uint32_t download_id,
                               download::DownloadTargetCallback callback,
                               const base::FilePath& suggested_path);
  void ChooseDownloadPath(uint32_t download_id,
                          download::DownloadTargetCallback callback,
                          const base::FilePath& suggested_path);

  raw_ptr<content::DownloadManager> download_manager_;
  base::FilePath default_download_path_;
  bool suppress_prompting_;
  base::WeakPtrFactory<AlohaDownloadManagerDelegate> weak_ptr_factory_{this};
};

}  // namespace aloha

#endif  // ALOHA_TEMP_DEF__BROWSER_SHELL_DOWNLOAD_MANAGER_DELEGATE_H_
