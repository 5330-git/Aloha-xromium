#include "aloha/browser/profile/aloha_browser_profile.h"

#include "aloha/browser/profile/aloha_content_index_provider.h"
#include "aloha/browser/profile/aloha_permission_manager.h"
#include "aloha/common/aloha_constants.h"
#include "aloha/common/aloha_paths.h"
#include "aloha_download_manager_delegate.h"
#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/keyed_service/core/simple_dependency_manager.h"
#include "components/keyed_service/core/simple_factory_key.h"
#include "components/keyed_service/core/simple_key_map.h"
#include "components/network_session_configurator/common/network_switches.h"
#include "components/origin_trials/browser/leveldb_persistence_provider.h"
#include "components/origin_trials/browser/origin_trials.h"
#include "components/origin_trials/common/features.h"
#include "content/public/browser/background_sync_controller.h"
#include "content/public/browser/content_index_provider.h"
#include "content/public/browser/reduce_accept_language_controller_delegate.h"
#include "content/public/browser/storage_partition.h"
#include "third_party/blink/public/common/origin_trials/trial_token_validator.h"


namespace aloha {

AlohaBrowserProfile::AlohaBrowserProfile(bool off_the_record,
                                         bool delay_services_creation)
    : off_the_record_(off_the_record) {
  InitWhileIOAllowed();
#if BUILDFLAG(IS_WIN)
  base::SetExtraNoExecuteAllowedPath(aloha::path_service::ALOHA_USER_DATA_DIR);
#endif  // BUILDFLAG(IS_WIN)

  if (!delay_services_creation) {
    BrowserContextDependencyManager::GetInstance()
        ->CreateBrowserContextServices(this);
  }
}

AlohaBrowserProfile::~AlohaBrowserProfile() {
  NotifyWillBeDestroyed();

  // The SimpleDependencyManager should always be passed after the
  // BrowserContextDependencyManager. This is because the KeyedService instances
  // in the BrowserContextDependencyManager's dependency graph can depend on the
  // ones in the SimpleDependencyManager's graph.
  DependencyManager::PerformInterlockedTwoPhaseShutdown(
      BrowserContextDependencyManager::GetInstance(), this,
      SimpleDependencyManager::GetInstance(), key_.get());

  SimpleKeyMap::GetInstance()->Dissociate(this);

  ShutdownStoragePartitions();
}

void AlohaBrowserProfile::InitWhileIOAllowed() {
  base::CommandLine* cmd_line = base::CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(::switches::kIgnoreCertificateErrors)) {
    ignore_certificate_errors_ = true;
  }

  if (cmd_line->HasSwitch(aloha::switches::kAlohaUserDataDir)) {
    path_ = cmd_line->GetSwitchValuePath(aloha::switches::kAlohaUserDataDir);
    if (base::DirectoryExists(path_) || base::CreateDirectory(path_)) {
      // BrowserContext needs an absolute path, which we would normally get via
      // PathService. In this case, manually ensure the path is absolute.
      if (!path_.IsAbsolute()) {
        path_ = base::MakeAbsoluteFilePath(path_);
      }
      if (!path_.empty()) {
        FinishInitWhileIOAllowed();
        base::PathService::OverrideAndCreateIfNeeded(
            aloha::path_service::ALOHA_USER_DATA_DIR, path_,
            /*is_absolute=*/true, /*create=*/false);
        return;
      }
    } else {
      LOG(WARNING) << "Unable to create data-path directory: " << path_.value();
    }
  }

  // 使用默认路径(由 AlohaPathProvider 提供)
  CHECK(
      base::PathService::Get(aloha::path_service::ALOHA_USER_DATA_DIR, &path_));

  FinishInitWhileIOAllowed();
}

void AlohaBrowserProfile::FinishInitWhileIOAllowed() {
  key_ = std::make_unique<SimpleFactoryKey>(path_, off_the_record_);
  SimpleKeyMap::GetInstance()->Associate(this, key_.get());
}

std::unique_ptr<content::ZoomLevelDelegate>
AlohaBrowserProfile::CreateZoomLevelDelegate(const base::FilePath&) {
  return nullptr;
}

base::FilePath AlohaBrowserProfile::GetPath() {
  return path_;
}

bool AlohaBrowserProfile::IsOffTheRecord() {
  return off_the_record_;
}

content::DownloadManagerDelegate*
AlohaBrowserProfile::GetDownloadManagerDelegate() {
  if (!download_manager_delegate_.get()) {
    download_manager_delegate_ =
        std::make_unique<aloha::AlohaDownloadManagerDelegate>();
    download_manager_delegate_->SetDownloadManager(GetDownloadManager());
  }

  return download_manager_delegate_.get();
}

content::BrowserPluginGuestManager* AlohaBrowserProfile::GetGuestManager() {
  return nullptr;
}

storage::SpecialStoragePolicy* AlohaBrowserProfile::GetSpecialStoragePolicy() {
  return nullptr;
}

content::PlatformNotificationService*
AlohaBrowserProfile::GetPlatformNotificationService() {
  return nullptr;
}

content::PushMessagingService* AlohaBrowserProfile::GetPushMessagingService() {
  return nullptr;
}

content::StorageNotificationService*
AlohaBrowserProfile::GetStorageNotificationService() {
  return nullptr;
}

content::SSLHostStateDelegate* AlohaBrowserProfile::GetSSLHostStateDelegate() {
  return nullptr;
}

content::PermissionControllerDelegate*
AlohaBrowserProfile::GetPermissionControllerDelegate() {
  if (!permission_manager_.get()) {
    permission_manager_ = std::make_unique<aloha::AlohaPermissionManager>();
  }
  return permission_manager_.get();
}

content::ClientHintsControllerDelegate*
AlohaBrowserProfile::GetClientHintsControllerDelegate() {
  return client_hints_controller_delegate_;
}

content::BackgroundFetchDelegate*
AlohaBrowserProfile::GetBackgroundFetchDelegate() {
  return nullptr;
}

// TODO(yeyun.anton): 调研实现
content::BackgroundSyncController*
AlohaBrowserProfile::GetBackgroundSyncController() {
  if (!background_sync_controller_) {
    // background_sync_controller_ =
    //     std::make_unique<content::MockBackgroundSyncController>();
  }
  return background_sync_controller_.get();
}

content::BrowsingDataRemoverDelegate*
AlohaBrowserProfile::GetBrowsingDataRemoverDelegate() {
  return nullptr;
}

content::ContentIndexProvider* AlohaBrowserProfile::GetContentIndexProvider() {
  if (!content_index_provider_) {
    content_index_provider_ =
        std::make_unique<aloha::AlohaContentIndexProvider>();
  }
  return content_index_provider_.get();
}

// TODO(yeyun.anton): 调研实现
content::ReduceAcceptLanguageControllerDelegate*
AlohaBrowserProfile::GetReduceAcceptLanguageControllerDelegate() {
  if (!reduce_accept_lang_controller_delegate_) {
    // reduce_accept_lang_controller_delegate_ =
    //     std::make_unique<content::MockReduceAcceptLanguageControllerDelegate>(
    //         content::GetShellLanguage());
  }
  return reduce_accept_lang_controller_delegate_.get();
}

content::OriginTrialsControllerDelegate*
AlohaBrowserProfile::GetOriginTrialsControllerDelegate() {
  if (!origin_trials::features::IsPersistentOriginTrialsEnabled()) {
    return nullptr;
  }

  if (!origin_trials_controller_delegate_) {
    origin_trials_controller_delegate_ =
        std::make_unique<origin_trials::OriginTrials>(
            std::make_unique<origin_trials::LevelDbPersistenceProvider>(
                GetPath(),
                GetDefaultStoragePartition()->GetProtoDatabaseProvider()),
            std::make_unique<blink::TrialTokenValidator>());
  }
  return origin_trials_controller_delegate_.get();
}
}  // namespace aloha
