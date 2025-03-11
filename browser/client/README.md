# 背景

> 在参考 views_examples_with_content 定制浏览器时，我们发现部分的网站比如 B 站会检查浏览器的 User Agent ，而 views_example_with_content 依赖的 ui/views/views_content_client 中并未定制 UserAgent ，因此有必要进行定制。为了让 aloha 代码的增量修改不影响内核变动，我们将 ui/views_content_client 的代码拷贝了过来，并根据业务需求进行部分接口的删减和增加



# 修改的接口

## AlohaContentBrowserClient

1. 解决没有 User Agent 的问题
   1. 参考：[User-Agent - HTTP | MDN](https://developer.mozilla.org/zh-CN/docs/Web/HTTP/Headers/User-Agent)
   2. > **User-Agent** [请求标头](https://developer.mozilla.org/zh-CN/docs/Glossary/Request_header)是一个特征字符串，使得服务器和对等网络能够识别发出请求的[用户代理](https://developer.mozilla.org/zh-CN/docs/Glossary/User_agent)的应用程序、操作系统、供应商或版本信息。
      >
