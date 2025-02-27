// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_RENDERER_CONTENT_RENDERER_CLIENT_H_
#define ALOHA_RENDERER_CONTENT_RENDERER_CLIENT_H_

#include "content/public/renderer/content_renderer_client.h"

namespace aloha {

class ContentRendererClient : public content::ContentRendererClient {
 public:
  ContentRendererClient();
  ContentRendererClient(const ContentRendererClient&) = delete;
  ContentRendererClient& operator=(const ContentRendererClient&) = delete;
  ~ContentRendererClient() override;

 private:
  // content::ContentRendererClient:
  void RenderFrameCreated(content::RenderFrame* render_frame) override;
  void WebViewCreated(blink::WebView* web_view,
                              bool was_created_by_renderer,
                              const url::Origin* outermost_origin) override;
  void RenderThreadStarted() override;
};

}  // namespace aloha

#endif  // ALOHA_RENDERER_CONTENT_RENDERER_CLIENT_H_
