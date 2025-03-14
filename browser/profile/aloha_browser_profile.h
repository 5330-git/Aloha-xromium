// aloha_browser_profile.h
// 参考 chrome\browser\profiles\profile.h 实现
// 背景：此前 aloha 使用的 BrowserContext 是 content::AlohaBrowserProfile
// 不再能满足我们的业务需求，比如我们需要管理 PrefService
// 来管理用户偏好（目前的使用场景是需要通过 PrefService 初始化 OSCrypt
// 的密钥以支持 Cookie的持久化存储）
// 再比如我们需要自定义 content::BrowserContext::GetPath() 来返回用户数据目录
#include "content/public/browser/browser_context.h"
#include "content/shell/browser/shell_browser_context.h"

namespace aloha {

// 过渡阶段，先完全拷贝 content::ShellBrowserContext
class AlohaBrowserProfile : public content::BrowserContext {
 public:
  // If |delay_services_creation| is true, the owner is responsible for calling
  // CreateBrowserContextServices() for this BrowserContext.
  explicit AlohaBrowserProfile(bool off_the_record,
                               bool delay_services_creation = false);

  AlohaBrowserProfile(const AlohaBrowserProfile&) = delete;
  AlohaBrowserProfile& operator=(const AlohaBrowserProfile&) = delete;

  ~AlohaBrowserProfile() override;

  void set_client_hints_controller_delegate(
      content::ClientHintsControllerDelegate* delegate) {
    client_hints_controller_delegate_ = delegate;
  }

  // BrowserContext implementation.
  base::FilePath GetPath() override;
  std::unique_ptr<content::ZoomLevelDelegate> CreateZoomLevelDelegate(
      const base::FilePath& partition_path) override;
  bool IsOffTheRecord() override;
  content::DownloadManagerDelegate* GetDownloadManagerDelegate() override;
  content::BrowserPluginGuestManager* GetGuestManager() override;
  storage::SpecialStoragePolicy* GetSpecialStoragePolicy() override;
  content::PlatformNotificationService* GetPlatformNotificationService()
      override;
  content::PushMessagingService* GetPushMessagingService() override;
  content::StorageNotificationService* GetStorageNotificationService() override;
  content::SSLHostStateDelegate* GetSSLHostStateDelegate() override;
  content::PermissionControllerDelegate* GetPermissionControllerDelegate()
      override;
  content::BackgroundFetchDelegate* GetBackgroundFetchDelegate() override;
  content::BackgroundSyncController* GetBackgroundSyncController() override;
  content::BrowsingDataRemoverDelegate* GetBrowsingDataRemoverDelegate()
      override;
  content::ContentIndexProvider* GetContentIndexProvider() override;
  content::ClientHintsControllerDelegate* GetClientHintsControllerDelegate()
      override;
  content::ReduceAcceptLanguageControllerDelegate*
  GetReduceAcceptLanguageControllerDelegate() override;
  content::OriginTrialsControllerDelegate* GetOriginTrialsControllerDelegate()
      override;

 protected:
  bool ignore_certificate_errors() const { return ignore_certificate_errors_; }

  std::unique_ptr<content::ShellDownloadManagerDelegate>
      download_manager_delegate_;
  std::unique_ptr<content::PermissionControllerDelegate> permission_manager_;
  std::unique_ptr<content::BackgroundSyncController>
      background_sync_controller_;
  std::unique_ptr<content::ContentIndexProvider> content_index_provider_;
  std::unique_ptr<content::ReduceAcceptLanguageControllerDelegate>
      reduce_accept_lang_controller_delegate_;
  std::unique_ptr<content::OriginTrialsControllerDelegate>
      origin_trials_controller_delegate_;

 private:
  // Performs initialization of the AlohaBrowserProfile while IO is still
  // allowed on the current thread.
  void InitWhileIOAllowed();
  void FinishInitWhileIOAllowed();

  const bool off_the_record_;
  bool ignore_certificate_errors_ = false;
  base::FilePath path_;
  std::unique_ptr<SimpleFactoryKey> key_;
  raw_ptr<content::ClientHintsControllerDelegate>
      client_hints_controller_delegate_ = nullptr;
};
}  // namespace aloha
