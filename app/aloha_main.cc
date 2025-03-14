#include "aloha/app/aloha_main.h"

#include "aloha/common/aloha_main_client.h"
#include "aloha_content_main_delegate.h"


namespace aloha {
int AlohaMain(HINSTANCE instance, sandbox::SandboxInterfaceInfo* sandbox_info) {
  aloha::AlohaMainClient::GetInstance()->PreAlohaMain();

  AlohaContentMainDelegate delegate;
  content::ContentMainParams params(&delegate);
  params.instance = instance;
  params.sandbox_info = sandbox_info;

  return content::ContentMain(std::move(params));
}
}  // namespace aloha
