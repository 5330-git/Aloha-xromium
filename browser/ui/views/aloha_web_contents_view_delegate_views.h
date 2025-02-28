#ifndef _XX_H
#define _XX_H
#include <memory>
#include <tuple>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
// #include "components/renderer_context_menu/context_menu_delegate.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view_delegate.h"

namespace aloha {
class AlohaWebContentsViewDelegate : public content::WebContentsViewDelegate
//  public ContextMenuDelegate {
{
 public:
//   using CreateContentWindowFunc =
//       base::RepeatingCallback<content::WebContents*(const GURL&)>;

  explicit AlohaWebContentsViewDelegate(content::WebContents* web_contents);
                            //    CreateContentWindowFunc func);
  // : ContextMenuDelegate(web_contents), web_contents_(web_contents) {}
  AlohaWebContentsViewDelegate(const AlohaWebContentsViewDelegate&) = delete;
  AlohaWebContentsViewDelegate& operator=(const AlohaWebContentsViewDelegate&) =
      delete;
  ~AlohaWebContentsViewDelegate() override;

 protected:
  // content::WebContentsViewDelegate:
  void ShowContextMenu(content::RenderFrameHost& render_frame_host,
                       const content::ContextMenuParams& params) override;

  // ContextMenuDelegate
  //   std::unique_ptr<RenderViewContextMenuBase> BuildMenu(
  //       content::RenderFrameHost& render_frame_host,
  //       const content::ContextMenuParams& params) override;
  //   void ShowMenu(std::unique_ptr<RenderViewContextMenuBase> menu) override;

 private:
  // 主端 host webui 的对象
  const raw_ptr<content::WebContents> web_contents_;
};
}  // namespace aloha

#endif
