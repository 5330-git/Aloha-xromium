// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aloha/browser/client/aloha_content_client_main_parts.h"

#import <Cocoa/Cocoa.h>

#include <utility>

#include "aloha/common/aloha_main_client.h"
#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/path_service.h"
#include "content/public/browser/context_factory.h"
#include "content/public/common/content_paths.h"
#include "content/public/common/result_codes.h"
#include "ui/views/test/test_views_delegate.h"

// A simple NSApplicationDelegate that provides a basic mainMenu and can
// activate a task when the application has finished loading.
@interface AlohaContentClientAppController : NSObject <NSApplicationDelegate> {
 @private
  base::OnceClosure _onApplicationDidFinishLaunching;
}

// Set the task to run after receiving -applicationDidFinishLaunching:.
- (void)setOnApplicationDidFinishLaunching:(base::OnceClosure)task;

@end

namespace aloha {

namespace {

class AlohaContentClientMainPartsMac : public AlohaContentClientMainParts {
 public:
  AlohaContentClientMainPartsMac();

  AlohaContentClientMainPartsMac(const AlohaContentClientMainPartsMac&) =
      delete;
  AlohaContentClientMainPartsMac& operator=(
      const AlohaContentClientMainPartsMac&) = delete;

  ~AlohaContentClientMainPartsMac() override;

  // content::BrowserMainParts:
  int PreMainMessageLoopRun() override;

 private:
  AlohaContentClientAppController* __strong app_controller_;
};

AlohaContentClientMainPartsMac::AlohaContentClientMainPartsMac(
    )
    : AlohaContentClientMainParts() {
  // Cache the child process path to avoid triggering an AssertIOAllowed.
  base::FilePath child_process_exe;
  base::PathService::Get(content::CHILD_PROCESS_EXE, &child_process_exe);

  app_controller_ = [[AlohaContentClientAppController alloc] init];
  NSApplication.sharedApplication.delegate = app_controller_;
}

int AlohaContentClientMainPartsMac::PreMainMessageLoopRun() {
  AlohaContentClientMainParts::PreMainMessageLoopRun();

  views_delegate()->set_context_factory(content::GetContextFactory());

  // On Mac, the task must be deferred to applicationDidFinishLaunching. If not,
  // the widget can activate, but (even if configured) the mainMenu won't be
  // ready to switch over in the OSX aloha, so it will look strange.
  NSWindow* window_context = nil;
  [app_controller_ setOnApplicationDidFinishLaunching:
                       base::BindOnce(&AlohaMainClient::OnPreMainMessageLoopRun,
                                      base::Unretained(AlohaMainClient::GetInstance()),
                                      base::Unretained(browser_context()),
                                      base::Unretained(window_context))];

  return content::RESULT_CODE_NORMAL_EXIT;
}

AlohaContentClientMainPartsMac::~AlohaContentClientMainPartsMac() {
  NSApplication.sharedApplication.delegate = nil;
}

}  // namespace

// static
std::unique_ptr<AlohaContentClientMainParts>
AlohaContentClientMainParts::Create() {
  return std::make_unique<AlohaContentClientMainPartsMac>();
}

// static
void AlohaContentClientMainParts::PreBrowserMain() {
  // Simply instantiating an instance of ShellCrApplication serves to register
  // it as the application class. Do make sure that no other code has done this
  // first, though.
  CHECK_EQ(NSApp, nil);
  [ShellCrApplication sharedApplication];
}

}  // namespace aloha

@implementation AlohaContentClientAppController

- (void)setOnApplicationDidFinishLaunching:(base::OnceClosure)task {
  _onApplicationDidFinishLaunching = std::move(task);
}

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
  // To get key events, the application needs to have an activation policy.
  // Unbundled apps (i.e. those without an Info.plist) default to
  // NSApplicationActivationPolicyProhibited.
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

  // Create a basic mainMenu object using the executable filename.
  NSMenu* mainMenu = [[NSMenu alloc] initWithTitle:@""];
  NSMenuItem* appMenuItem = [mainMenu addItemWithTitle:@""
                                                action:nullptr
                                         keyEquivalent:@""];
  [NSApp setMainMenu:mainMenu];

  NSMenu* appMenu = [[NSMenu alloc] initWithTitle:@""];
  NSString* appName = NSProcessInfo.processInfo.processName;
  // TODO(tapted): Localize "Quit" if this is ever used for a released binary.
  // At the time of writing, ui_strings.grd has "Close" but not "Quit".
  NSString* quitTitle = [@"Quit " stringByAppendingString:appName];
  [appMenu addItemWithTitle:quitTitle
                     action:@selector(terminate:)
              keyEquivalent:@"q"];
  [appMenuItem setSubmenu:appMenu];

  CHECK([NSApp isKindOfClass:[ShellCrApplication class]]);

  std::move(_onApplicationDidFinishLaunching).Run();
}

@end
