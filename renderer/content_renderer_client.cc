// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/renderer/content_renderer_client.h"

#include "aloha/renderer/render_frame_observer.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/web/web_custom_element.h"

namespace aloha {

ContentRendererClient::ContentRendererClient() = default;

ContentRendererClient::~ContentRendererClient() = default;

void ContentRendererClient::RenderFrameCreated(
    content::RenderFrame* render_frame) {
  LOG(INFO) << "RenderFrameCreated";
  if (!render_frame->IsMainFrame()) {
    return;
  }

  std::unique_ptr<RenderFrameObserver> observer =
      std::make_unique<RenderFrameObserver>(render_frame);
  RenderFrameObserver* observer_ptr = observer.get();
  observer_ptr->SelfOwn(std::move(observer));
}

void ContentRendererClient::RenderThreadStarted() {
  blink::WebCustomElement::AddEmbedderCustomElementName(
      blink::WebString("webview"));
}

void ContentRendererClient::WebViewCreated(
    blink::WebView* web_view,
    bool was_created_by_renderer,
    const url::Origin* outermost_origin) {
  LOG(INFO) << "-- WebViewCreated";
}

}  // namespace aloha
