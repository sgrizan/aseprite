/* Aseprite
 * Copyright (C) 2001-2015  David Capello
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/set_mask.h"
#include "app/commands/command.h"
#include "app/commands/commands.h"
#include "app/context_access.h"
#include "app/modules/gui.h"
#include "app/transaction.h"
#include "base/unique_ptr.h"
#include "doc/image.h"
#include "doc/mask.h"
#include "doc/primitives.h"
#include "doc/sprite.h"

namespace app {

class InvertMaskCommand : public Command {
public:
  InvertMaskCommand();
  Command* clone() const override { return new InvertMaskCommand(*this); }

protected:
  bool onEnabled(Context* context);
  void onExecute(Context* context);
};

InvertMaskCommand::InvertMaskCommand()
  : Command("InvertMask",
            "Invert Mask",
            CmdRecordableFlag)
{
}

bool InvertMaskCommand::onEnabled(Context* context)
{
  return context->checkFlags(ContextFlags::ActiveDocumentIsWritable |
                             ContextFlags::HasActiveSprite);
}

void InvertMaskCommand::onExecute(Context* context)
{
  bool hasMask = false;
  {
    const ContextReader reader(context);
    if (reader.document()->isMaskVisible())
      hasMask = true;
  }

  // without mask?...
  if (!hasMask) {
    // so we select all
    Command* mask_all_cmd =
      CommandsModule::instance()->getCommandByName(CommandId::MaskAll);
    context->executeCommand(mask_all_cmd);
  }
  // invert the current mask
  else {
    ContextWriter writer(context);
    Document* document(writer.document());
    Sprite* sprite(writer.sprite());

    // Select all the sprite area
    base::UniquePtr<Mask> mask(new Mask());
    mask->replace(sprite->bounds());

    // Remove in the new mask the current sprite marked region
    const gfx::Rect& maskBounds = document->mask()->bounds();
    doc::fill_rect(mask->bitmap(),
      maskBounds.x, maskBounds.y,
      maskBounds.x + maskBounds.w-1,
      maskBounds.y + maskBounds.h-1, 0);

    Mask* curMask = document->mask();
    if (curMask->bitmap()) {
      // Copy the inverted region in the new mask (we just modify the
      // document's mask temporaly here)
      curMask->freeze();
      curMask->invert();
      doc::copy_image(mask->bitmap(),
        curMask->bitmap(),
        curMask->bounds().x,
        curMask->bounds().y);
      curMask->invert();
      curMask->unfreeze();
    }

    // We need only need the area inside the sprite
    mask->intersect(sprite->bounds());

    // Set the new mask
    Transaction transaction(writer.context(), "Mask Invert", DoesntModifyDocument);
    transaction.execute(new cmd::SetMask(document, mask));
    transaction.commit();

    document->generateMaskBoundaries();
    update_screen_for_document(document);
  }
}

Command* CommandFactory::createInvertMaskCommand()
{
  return new InvertMaskCommand;
}

} // namespace app
