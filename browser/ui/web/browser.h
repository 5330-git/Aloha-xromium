// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_BROWSER_UI_WEB_BROWSER_H_
#define ALOHA_BROWSER_UI_WEB_BROWSER_H_

#include "content/public/browser/web_ui_controller.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "aloha/browser/ui/web/browser.mojom.h"
#include "ui/webui/mojo_web_ui_controller.h"
// #include "chrome/browser/ui/browser.h"

namespace aloha {

class Browser : public ui::MojoWebUIController,
                public aloha::mojom::PageHandlerFactory {
 public:
  static constexpr char kHost[] = "browser";

  explicit Browser(content::WebUI* web_ui);
  Browser(const Browser&) = delete;
  Browser& operator=(const Browser&) = delete;
  ~Browser() override;

  void BindInterface(
      mojo::PendingReceiver<aloha::mojom::PageHandlerFactory>
          receiver);

 private:
  // aloha::mojom::PageHandlerFactory:
  void CreatePageHandler(
      mojo::PendingReceiver<aloha::mojom::PageHandler> receiver)
      override;

  mojo::Receiver<aloha::mojom::PageHandlerFactory>
      page_factory_receiver_{this};

  WEB_UI_CONTROLLER_TYPE_DECL();
};


// // 用 chrome/browser/ui 下的 browser.h 中的 Browser 进行替换
// class AlohaBrowserWindow: public ::Browser {

// };

}  // namespace aloha

#endif  // ALOHA_BROWSER_UI_WEB_BROWSER_H_
