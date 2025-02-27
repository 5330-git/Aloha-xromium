// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/devtools/devtools_frontend.h"

#include <map>
#include <memory>
#include <string_view>

#include "aloha/browser/devtools/devtools_server.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/uuid.h"
#include "base/values.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_frontend_host.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "ipc/ipc_channel.h"
#include "url/gurl.h"

namespace aloha {

namespace {

static GURL GetFrontendURL() {
  return GURL(
      base::StringPrintf("http://127.0.0.1:%d/devtools/devtools_app.html",
                         devtools::GetHttpHandlerPort()));
}

// This constant should be in sync with
// the constant
// kMaxMessageChunkSize in chrome/browser/devtools/devtools_ui_bindings.cc.
constexpr size_t kMaxMessageChunkSize = IPC::Channel::kMaximumMessageSize / 4;

}  // namespace

class DevToolsFrontend::AgentHostClient
    : public content::WebContentsObserver,
      public content::DevToolsAgentHostClient {
 public:
  AgentHostClient(content::WebContents* devtools_contents,
                  content::WebContents* inspected_contents)
      : content::WebContentsObserver(devtools_contents),
        devtools_contents_(devtools_contents),
        inspected_contents_(inspected_contents) {}
  AgentHostClient(const AgentHostClient&) = delete;
  AgentHostClient& operator=(const AgentHostClient&) = delete;
  ~AgentHostClient() override = default;

  // content::DevToolsAgentHostClient
  void DispatchProtocolMessage(content::DevToolsAgentHost* agent_host,
                               base::span<const uint8_t> message) override {
    std::string_view str_message(reinterpret_cast<const char*>(message.data()),
                                 message.size());
    if (str_message.length() < kMaxMessageChunkSize) {
      CallClientFunction("DevToolsAPI", "dispatchMessage",
                         base::Value(std::string(str_message)));
    } else {
      size_t total_size = str_message.length();
      for (size_t pos = 0; pos < str_message.length();
           pos += kMaxMessageChunkSize) {
        std::string_view str_message_chunk =
            str_message.substr(pos, kMaxMessageChunkSize);

        CallClientFunction(
            "DevToolsAPI", "dispatchMessageChunk",
            base::Value(std::string(str_message_chunk)),
            base::Value(base::NumberToString(pos ? 0 : total_size)));
      }
    }
  }

  // 被inspect的页面关闭时会调用这个函数
  void AgentHostClosed(content::DevToolsAgentHost* agent_host) override {
    if (agent_host_ == agent_host) {
      // 释放 agent_host 的引用计数，同时允许下一次绑定
      agent_host_ = nullptr;
    }
  }

  // 离开 AgentHostClient 的范围后，获得的 devtools_contents_ 可能为 nullptr
  // 因此需要使用 weak_ptr 来保证 devtools_contents_ 的有效性
  base::WeakPtr<content::WebContents> GetDevtoolsWebContents() const {
    if (!devtools_contents_) {
      return base::WeakPtr<content::WebContents>();
    }
    return devtools_contents_->GetWeakPtr();
  }

  void SetDevtoolsWebContents(content::WebContents* devtools_contents) {
    CHECK(!devtools_contents_);
    devtools_contents_ = devtools_contents;
    Observe(devtools_contents_);
  }

  void Attach() {
    // AgentHostClient 实例的生命周期是跟随 DevToolsFrontend 的，而
    // DevToolsFrontend是跟随 inspected_contents_
    // 的（具体可以看DevtoolsFrontend::Pointer 类的实现）。因此
    // AgentHostClient不应该绑定到多个 webcontents (agent_host) 上
    CHECK(!agent_host_);

    agent_host_ =
        content::DevToolsAgentHost::GetOrCreateFor(inspected_contents_);
    agent_host_->AttachClient(this);
  }

  void CallClientFunction(
      const std::string& object_name,
      const std::string& method_name,
      base::Value arg1 = {},
      base::Value arg2 = {},
      base::Value arg3 = {},
      base::OnceCallback<void(base::Value)> cb = base::NullCallback()) {
    if (!devtools_contents_) {
      LOG(ERROR) << "DevToolsFrontend::AgentHostClient::CallClientFunction "
                    "devtools_contents_ is null"
                 << " object_name: " << object_name
                 << " method_name: " << method_name;
      return;
    }
    content::RenderFrameHost* host = devtools_contents_->GetPrimaryMainFrame();
    host->AllowInjectingJavaScript();

    base::Value::List arguments;
    if (!arg1.is_none()) {
      arguments.Append(std::move(arg1));
      if (!arg2.is_none()) {
        arguments.Append(std::move(arg2));
        if (!arg3.is_none()) {
          arguments.Append(std::move(arg3));
        }
      }
    }

    host->ExecuteJavaScriptMethod(base::ASCIIToUTF16(object_name),
                                  base::ASCIIToUTF16(method_name),
                                  std::move(arguments), std::move(cb));
  }

  // content::WebContentsObserver:
  void WebContentsDestroyed() override {
    // 清空监听的 webcontents
    devtools_contents_ = nullptr;
  }

 private:
  // content::WebContentsObserver:
  void ReadyToCommitNavigation(
      content::NavigationHandle* navigation_handle) override {
    content::RenderFrameHost* frame = navigation_handle->GetRenderFrameHost();
    // TODO(crbug.com/40185886): With MPArch there may be multiple main
    // frames. This caller was converted automatically to the primary main frame
    // to preserve its semantics. Follow up to confirm correctness.
    if (navigation_handle->IsInPrimaryMainFrame()) {
      frontend_host_ = content::DevToolsFrontendHost::Create(
          frame, base::BindRepeating(
                     &AgentHostClient::HandleMessageFromDevToolsFrontend,
                     base::Unretained(this)));
      return;
    }
    std::string origin =
        navigation_handle->GetURL().DeprecatedGetOriginAsURL().spec();
    auto it = extensions_api_.find(origin);
    if (it == extensions_api_.end()) {
      return;
    }
    std::string script = base::StringPrintf(
        "%s(\"%s\")", it->second.c_str(),
        base::Uuid::GenerateRandomV4().AsLowercaseString().c_str());
    content::DevToolsFrontendHost::SetupExtensionsAPI(frame, script);
  }

  void HandleMessageFromDevToolsFrontend(base::Value::Dict message) {
    const std::string* method = message.FindString("method");
    if (!method) {
      return;
    }

    int request_id = message.FindInt("id").value_or(0);
    base::Value::List* params_value = message.FindList("params");

    // Since we've received message by value, we can take the list.
    base::Value::List params;
    if (params_value) {
      params = std::move(*params_value);
    }

    if (*method == "dispatchProtocolMessage" && params.size() == 1) {
      const std::string* protocol_message = params[0].GetIfString();
      if (!agent_host_ || !protocol_message) {
        return;
      }
      agent_host_->DispatchProtocolMessage(
          this, base::as_bytes(base::make_span(*protocol_message)));
    } else if (*method == "loadCompleted") {
      CallClientFunction("DevToolsAPI", "setUseSoftMenu", base::Value(true));
    } else if (*method == "loadNetworkResource" && params.size() == 3) {
      // TODO(robliao): Add support for this if necessary.
      NOTREACHED();
    } else if (*method == "getPreferences") {
      SendMessageAck(request_id, base::Value(std::move(preferences_)));
      return;
    } else if (*method == "getHostConfig") {
      SendMessageAck(request_id, {});
      return;
    } else if (*method == "setPreference") {
      if (params.size() < 2) {
        return;
      }
      const std::string* name = params[0].GetIfString();

      // We're just setting params[1] as a value anyways, so just make sure it's
      // the type we want, but don't worry about getting it.
      if (!name || !params[1].is_string()) {
        return;
      }

      preferences_.Set(*name, std::move(params[1]));
    } else if (*method == "removePreference") {
      const std::string* name = params[0].GetIfString();
      if (!name) {
        return;
      }
      preferences_.Remove(*name);
    } else if (*method == "requestFileSystems") {
      CallClientFunction("DevToolsAPI", "fileSystemsLoaded",
                         base::Value(base::Value::Type::LIST));
    } else if (*method == "reattach") {
      if (!agent_host_) {
        return;
      }
      agent_host_->DetachClient(this);
      agent_host_->AttachClient(this);
    } else if (*method == "registerExtensionsAPI") {
      if (params.size() < 2) {
        return;
      }
      const std::string* origin = params[0].GetIfString();
      const std::string* script = params[1].GetIfString();
      if (!origin || !script) {
        return;
      }
      extensions_api_[*origin + "/"] = *script;
    } else {
      return;
    }

    if (request_id) {
      SendMessageAck(request_id, {});
    }
  }

  void SendMessageAck(int request_id, base::Value arg) {
    CallClientFunction("DevToolsAPI", "embedderMessageAck",
                       base::Value(request_id), std::move(arg));
  }

  raw_ptr<content::WebContents> devtools_contents_;
  const raw_ptr<content::WebContents> inspected_contents_;

  scoped_refptr<content::DevToolsAgentHost> agent_host_;
  std::unique_ptr<content::DevToolsFrontendHost> frontend_host_;

  std::map<std::string, std::string> extensions_api_;

  base::Value::Dict preferences_;
};

// 将 DevToolsFrontend 的实例生命周期和 inspected_contents
// 的生命周期绑定在一起，从而避免出现野指针的问题
class DevToolsFrontend::Pointer : public content::WebContentsUserData<Pointer> {
 public:
  ~Pointer() override = default;

  static DevToolsFrontend* Create(content::WebContents* web_contents) {
    CreateForWebContents(web_contents);
    Pointer* ptr = FromWebContents(web_contents);
    return ptr->Get();
  }

  DevToolsFrontend* Get() { return ptr_.get(); }

 private:
  friend class content::WebContentsUserData<Pointer>;
  Pointer(content::WebContents* web_contents)
      : content::WebContentsUserData<Pointer>(*web_contents),
        ptr_(new DevToolsFrontend(web_contents)) {}
  Pointer(const Pointer*) = delete;
  Pointer& operator=(const Pointer&) = delete;

  WEB_CONTENTS_USER_DATA_KEY_DECL();

  std::unique_ptr<DevToolsFrontend> ptr_;
};

WEB_CONTENTS_USER_DATA_KEY_IMPL(DevToolsFrontend::Pointer);

DevToolsFrontend::DevToolsFrontend(content::WebContents* inspected_contents)
    : frontend_url_(GetFrontendURL()),
      inspected_contents_(inspected_contents) {}
DevToolsFrontend::~DevToolsFrontend() = default;

// static
DevToolsFrontend* DevToolsFrontend::CreateAndGet(
    content::WebContents* inspected_contents) {
  return DevToolsFrontend::Pointer::Create(inspected_contents);
}
void DevToolsFrontend::SetDevtoolsWebContents(
    content::WebContents* devtools_contents) {
  // 不要重复创建 agent_host_client_
  if (!agent_host_client_) {
    agent_host_client_ = std::make_unique<AgentHostClient>(devtools_contents,
                                                           inspected_contents_);
    agent_host_client_->Attach();
    return;
  }

  // 只能同时绑定一个活跃的 devtools contents, 并且它们的 url 必须相同
  auto current_devtools_contents = agent_host_client_->GetDevtoolsWebContents();
  if (current_devtools_contents) {
    LOG(INFO) << "current devtools contents is not null";
    CHECK(devtools_contents->GetURL() == current_devtools_contents->GetURL());
    return;
  }

  // 没有活跃的 devtools contents, 则绑定新的 devtools contents
  agent_host_client_->SetDevtoolsWebContents(devtools_contents);
}

}  // namespace aloha
