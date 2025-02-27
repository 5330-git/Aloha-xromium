// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_BROWSER_UI_WEB_WEBUI_H_
#define ALOHA_BROWSER_UI_WEB_WEBUI_H_

#include "content/public/browser/web_ui_controller.h"

namespace aloha {

class WebUI : public content::WebUIController {
 public:
  static constexpr char kHost[] = "main";

  explicit WebUI(content::WebUI* web_ui);
  WebUI(const WebUI&) = delete;
  WebUI& operator=(const WebUI&) = delete;
  ~WebUI() override;

 private:
  WEB_UI_CONTROLLER_TYPE_DECL();
};

}  // namespace aloha

#endif  // ALOHA_BROWSER_UI_WEB_WEBUI_H_
