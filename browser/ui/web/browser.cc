// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/ui/web/browser.h"

#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "aloha/browser/ui/web/browser_page_handler.h"
#include "aloha/resources/browser/grit/aloha_browser_resources.h"

namespace aloha {

namespace {

constexpr char kMainUI[] = "browser";

}  // namespace

Browser::Browser(content::WebUI* web_ui)
    : ui::MojoWebUIController(web_ui, false) {
  content::WebUIDataSource* html_source =
      content::WebUIDataSource::CreateAndAdd(
          web_ui->GetWebContents()->GetBrowserContext(), kMainUI);
  html_source->AddResourcePath("index.js", IDR_ALOHA_BROWSER_INDEX_JS);
  html_source->AddResourcePath("index.css",
                               IDR_ALOHA_BROWSER_INDEX_CSS);
  html_source->AddResourcePath(
      "browser.mojom-webui.js",
      IDR_ALOHA_BROWSER_BROWSER_MOJOM_WEBUI_JS);
  html_source->SetDefaultResource(IDR_ALOHA_BROWSER_INDEX_HTML);
  LOG(INFO) << "In Browser";
}

Browser::~Browser() = default;

void Browser::BindInterface(
    mojo::PendingReceiver<aloha::mojom::PageHandlerFactory> receiver) {
  page_factory_receiver_.reset();
  page_factory_receiver_.Bind(std::move(receiver));
}

void Browser::CreatePageHandler(
    mojo::PendingReceiver<aloha::mojom::PageHandler> receiver) {
  auto* render_frame_host = web_ui()->GetRenderFrameHost();
  BrowserPageHandler::CreateForRenderFrameHost(*render_frame_host,
                                               std::move(receiver));
}

WEB_UI_CONTROLLER_TYPE_IMPL(Browser)

}  // namespace aloha
