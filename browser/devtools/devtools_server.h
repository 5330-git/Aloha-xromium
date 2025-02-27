// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ALOHA_BROWSER_DEVTOOLS_DEVTOOLS_SERVER_H_
#define ALOHA_BROWSER_DEVTOOLS_DEVTOOLS_SERVER_H_

namespace content {
class BrowserContext;
}  // namespace content

namespace aloha::devtools {

void StartHttpHandler(content::BrowserContext* browser_context);
void StopHttpHandler();
int GetHttpHandlerPort();

}  // namespace aloha::devtools

#endif  // ALOHA_BROWSER_DEVTOOLS_DEVTOOLS_SERVER_H_
