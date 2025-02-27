// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_BROWSER_UI_WEB_WEBSHELL_GUEST_VIEW_H_
#define ALOHA_BROWSER_UI_WEB_WEBSHELL_GUEST_VIEW_H_

#include "base/types/pass_key.h"
#include "components/guest_view/browser/guest_view_message_handler.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"

namespace aloha {

class WebshellGuestView : public guest_view::GuestViewMessageHandler {
 public:
  explicit WebshellGuestView(const content::GlobalRenderFrameHostId& frame_id,
                             base::PassKey<WebshellGuestView>);
  WebshellGuestView(const WebshellGuestView&) = delete;
  WebshellGuestView& operator=(const WebshellGuestView&) = delete;
  ~WebshellGuestView() override;

  static void Create(
      const content::GlobalRenderFrameHostId& frame_id,
      mojo::PendingAssociatedReceiver<guest_view::mojom::GuestViewHost>
          receiver);

 private:
  // guest_view::GuestViewMessageHandler:
  std::unique_ptr<guest_view::GuestViewManagerDelegate>
  CreateGuestViewManagerDelegate() const override;
};

}  // namespace aloha

#endif  // ALOHA_BROWSER_UI_WEB_WEBSHELL_GUEST_VIEW_H_
