// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/webui_controller_factory.h"

#include "aloha/browser/ui/web/browser.h"
#include "aloha/browser/ui/web/webui.h"
#include "content/public/browser/web_ui_controller.h"
#include "url/gurl.h"

namespace {
static constexpr char kChromeScheme[] = "chrome";
// static constexpr char kAlohaScheme[] = "aloha";
}  // namespace

namespace aloha {

WebUIControllerFactory::WebUIControllerFactory() = default;

WebUIControllerFactory::~WebUIControllerFactory() = default;

std::unique_ptr<content::WebUIController>
WebUIControllerFactory::CreateWebUIControllerForURL(content::WebUI* web_ui,
                                                    const GURL& url) {
  if (!url.SchemeIs(kChromeScheme)) {
  // if (!url.SchemeIs(kAlohaScheme)) {
    return nullptr;
  }

  if (url.host_piece() == WebUI::kHost) {
    return std::make_unique<WebUI>(web_ui);
  }

  if (url.host_piece() == Browser::kHost) {
    return std::make_unique<Browser>(web_ui);
  }

  return nullptr;
}

content::WebUI::TypeID WebUIControllerFactory::GetWebUIType(
    content::BrowserContext* browser_context,
    const GURL& url) {
  if (url.SchemeIs(kChromeScheme) && url.host_piece() == WebUI::kHost) {
  // if (url.SchemeIs(kAlohaScheme) && url.host_piece() == WebUI::kHost) {
    return reinterpret_cast<content::WebUI::TypeID>(0x1);
  }

  if (url.SchemeIs(kChromeScheme) && url.host_piece() == Browser::kHost) {
  // if (url.SchemeIs(kAlohaScheme) && url.host_piece() == Browser::kHost) {
    return reinterpret_cast<content::WebUI::TypeID>(0x2);
  }

  return content::WebUI::kNoWebUI;
}

bool WebUIControllerFactory::UseWebUIForURL(
    content::BrowserContext* browser_context,
    const GURL& url) {
  return GetWebUIType(browser_context, url) != content::WebUI::kNoWebUI;
}

}  // namespace aloha
