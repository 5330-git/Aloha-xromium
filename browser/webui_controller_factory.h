// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_BROWSER_WEBUI_CONTROLLER_FACTORY_H_
#define ALOHA_BROWSER_WEBUI_CONTROLLER_FACTORY_H_

#include "content/public/browser/web_ui_controller_factory.h"

namespace aloha {

class WebUIControllerFactory : public content::WebUIControllerFactory {
 public:
  WebUIControllerFactory();
  WebUIControllerFactory(const WebUIControllerFactory&) = delete;
  WebUIControllerFactory& operator=(const WebUIControllerFactory&) = delete;
  ~WebUIControllerFactory() override;

 private:
  // content::WebUIControllerFactory:
  std::unique_ptr<content::WebUIController> CreateWebUIControllerForURL(
      content::WebUI* web_ui,
      const GURL& url) override;
  content::WebUI::TypeID GetWebUIType(content::BrowserContext* browser_context,
                                      const GURL& url) override;
  bool UseWebUIForURL(content::BrowserContext* browser_context,
                      const GURL& url) override;
};

}  // namespace aloha

#endif  // ALOHA_BROWSER_WEBUI_CONTROLLER_FACTORY_H_
