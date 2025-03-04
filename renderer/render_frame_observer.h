// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_RENDERER_RENDER_FRAME_OBSERVER_H_
#define ALOHA_RENDERER_RENDER_FRAME_OBSERVER_H_

#include <memory>

#include "content/public/renderer/render_frame_observer.h"

namespace aloha {

class RenderFrameObserver : public content::RenderFrameObserver {
 public:
  explicit RenderFrameObserver(content::RenderFrame* render_frame);
  RenderFrameObserver(const RenderFrameObserver&) = delete;
  RenderFrameObserver& operator=(const RenderFrameObserver&) = delete;
  ~RenderFrameObserver() override;

  void SelfOwn(std::unique_ptr<RenderFrameObserver> this_instance);

 private:
  // content::RenderFrameObserver:
  void OnDestruct() override;
  void DidStartNavigation(
      const GURL& url,
      std::optional<blink::WebNavigationType> navigation_type) override;
  void ReadyToCommitNavigation(
      blink::WebDocumentLoader* document_loader) override;

  std::unique_ptr<RenderFrameObserver> this_instance_;
};

}  // namespace aloha

#endif  // ALOHA_RENDERER_RENDER_FRAME_OBSERVER_H_
