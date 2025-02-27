#ifndef _XXX_H
#define _XXX_H
#include "components/custom_handlers/protocol_handler_registry.h"
#include "components/renderer_context_menu/render_view_context_menu_base.h"
namespace {
class AlohaRenderViewContextMenu
    : public RenderViewContextMenuBase,
      public custom_handlers::ProtocolHandlerRegistry::Observer {};
}  // namespace
#endif
