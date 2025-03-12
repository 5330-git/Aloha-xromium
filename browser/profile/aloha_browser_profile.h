// aloha_browser_profile.h
// 参考 chrome\browser\profiles\profile.h 实现
// 背景：此前 aloha 使用的 BrowserContext 是 content::ShellBrowserContext
// 不再能满足我们的业务需求，比如我们需要管理 PrefService
// 来管理用户偏好（目前的使用场景是需要通过 PrefService 初始化 OSCrypt
// 的密钥以支持 Cookie的持久化存储）

namespace aloha {
  class AlohaBrowserProfile: content::BrowserContext {
  public:
    AlohaBrowserProfile();
    virtual ~AlohaBrowserProfile();  
  }
}
