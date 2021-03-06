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

#ifndef APP_CMD_ADD_CEL_H_INCLUDED
#define APP_CMD_ADD_CEL_H_INCLUDED
#pragma once

#include "app/cmd.h"
#include "app/cmd/with_cel.h"
#include "app/cmd/with_layer.h"

#include <sstream>

namespace doc {
  class Cel;
  class Layer;
}

namespace app {
namespace cmd {
  using namespace doc;

  class AddCel : public Cmd
               , public WithLayer
               , public WithCel {
  public:
    AddCel(Layer* layer, Cel* cel);

  protected:
    void onExecute() override;
    void onUndo() override;
    void onRedo() override;
    size_t onMemSize() const override {
      return sizeof(*this) +
        const_cast<std::stringstream*>(&m_stream)->tellp();
    }

  private:
    void addCel(Layer* layer, Cel* cel);
    void removeCel(Layer* layer, Cel* cel);

    std::stringstream m_stream;
  };

} // namespace cmd
} // namespace app

#endif
