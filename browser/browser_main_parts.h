// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_BROWSER_BROWSER_MAIN_PARTS_H_
#define ALOHA_BROWSER_BROWSER_MAIN_PARTS_H_

#include <memory>
#include <string>

#include "aloha/browser/devtools/devtools_manager_delegate.h"
#include "base/files/scoped_temp_dir.h"
#include "build/build_config.h"
#include "content/public/browser/browser_main_parts.h"
#include "content/public/browser/web_contents_delegate.h"

#if BUILDFLAG(IS_MAC)
#include "ui/display/screen.h"
#endif  // BUILDFLAG(IS_MAC)

class GURL;

namespace content {
class BrowserContext;
class DevToolsManagerDelegate;
class WebContents;
class WebContentsViewDelegate;
}  // namespace content

namespace aloha {

class WebUIControllerFactory;

class BrowserMainParts : public content::BrowserMainParts {
 public:
  // 处理 新建 Web页面的功能
  class WebContentDelegate : public content::WebContentsDelegate {
   public:
    explicit WebContentDelegate(BrowserMainParts* browser_main_parts)
        : browser_main_parts_(browser_main_parts) {}
    // 新建标签页时会调用这个接口
    void WebContentsCreated(content::WebContents* source_contents,
                            int opener_render_process_id,
                            int opener_render_frame_id,
                            const std::string& frame_name,
                            const GURL& target_url,
                            content::WebContents* new_contents) override;

   private:
    base::raw_ptr<content::BrowserContext> browser_context_ = nullptr;
    base::raw_ptr<aloha::BrowserMainParts> browser_main_parts_ = nullptr;
  };
  static std::unique_ptr<BrowserMainParts> Create();

  BrowserMainParts(const BrowserMainParts&) = delete;
  BrowserMainParts& operator=(const BrowserMainParts&) = delete;
  ~BrowserMainParts() override;

  // 将由 aloha::ContentBrowserClient (派生自 content::ContentBrowserClient)
  // 的同名接口调用，负责窗口创建
  std::unique_ptr<content::WebContentsViewDelegate>
  CreateWebContentsViewDelegate(content::WebContents* web_contents);
  virtual std::unique_ptr<aloha::DevToolsManagerDelegate>
  CreateDevToolsManagerDelegate();
  content::WebContents* CreateAndShowWindow(GURL url,
                                            const std::u16string& title);
  WebContentDelegate* GetWebContentDelegate() {
    CHECK(web_contents_delegate_);
    return web_contents_delegate_.get();
  }

 protected:
  BrowserMainParts();
  virtual void InitializeUiToolkit();
  virtual void ShutdownUiToolkit();
  virtual void CreateAndShowWindowForWebContents(
      std::unique_ptr<content::WebContents> web_contents,
      const std::u16string& title) = 0;

  content::BrowserContext* browser_context() { return browser_context_.get(); }

  void OnWindowClosed();

 private:
  // content::BrowserMainParts:
  int PreMainMessageLoopRun() override;
  void WillRunMainMessageLoop(
      std::unique_ptr<base::RunLoop>& run_loop) override;
  void PostMainMessageLoopRun() override;

  void QuitMessageLoop();

  base::ScopedTempDir temp_dir_;
#if BUILDFLAG(IS_MAC)
  display::ScopedNativeScreen native_screen_;
#endif
  std::unique_ptr<WebUIControllerFactory> web_ui_controller_factory_;
  std::unique_ptr<content::BrowserContext> browser_context_;

  int content_windows_outstanding_ = 0;

  base::RepeatingClosure quit_run_loop_;
  std::unique_ptr<WebContentDelegate> web_contents_delegate_;

  base::WeakPtrFactory<BrowserMainParts> weak_factory_{this};
};

}  // namespace aloha

#endif  // ALOHA_BROWSER_BROWSER_MAIN_PARTS_H_
