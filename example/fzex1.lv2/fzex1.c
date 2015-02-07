/* `fzex1' is an example LV2 plugin demonstrating use of libfreeztile.
   Copyright (C) 2013-2015 Henrik Hedelund.

   This file is part of libfreeztile.

   libfreeztile is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   libfreeztile is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with libfreeztile; see the file COPYING.  If not see
   <http://www.gnu.org/licenses/>.  */

#include <stdlib.h>
#include <lv2.h>

#define FZEX1_URI "http://www.freeztile.org/plugins#fzex1"

/* Named port numbers.  */
enum {
  AUDIO_OUT_LEFT = 0,
  AUDIO_OUT_RIGHT,
  NUM_PORTS
};

/* Private plugin struct holding plugin data, state and resource
   references.  */
typedef struct {
  void *ports[NUM_PORTS];
} FzEx1;

/* Create a new instance of this plugin. This function is called by
   the host in the `instantiation' threading class.  */
static LV2_Handle
instantiate (const LV2_Descriptor *descriptor, double rate,
             const char *bundle_path,
             const LV2_Feature * const *features)
{
  FzEx1 *plugin = (FzEx1 *) malloc (sizeof (FzEx1));
  return (LV2_Handle) plugin;
}

/* Connect host ports to the plugin instance. Port buffers may only be
   accessed by functions in the same (`audo') threading class.  */
static void
connect_port (LV2_Handle instance, uint32_t port, void *data)
{
  FzEx1 *plugin = (FzEx1 *) instance;
  if (port < NUM_PORTS)
    plugin->ports[port] = data;
}

/* Initialize plugin in preparation for `run' by allocating required
   resources and resetting state data.  */
static void
activate (LV2_Handle instance)
{
}

/* Write NSAMPLES frames to audio output ports. This function runs in
   in the `audio' threading class and must be real-time safe.  */
static void
run (LV2_Handle instance, uint32_t nsamples)
{
}

/* Free any resources allocated in `activate'.  */
static void
deactivate (LV2_Handle instance)
{
}

/* Free any resources allocated in `instantiate'.  */
static void
cleanup (LV2_Handle instance)
{
  free (instance);
}

/* Return any extension data supported by this plugin.  */
static const void *
extension_data (const char *uri)
{
  return NULL;
}

/* `fzex1' plugin descriptor.  */
static const LV2_Descriptor descriptor = {
  FZEX1_URI,
  instantiate,
  connect_port,
  activate,
  run,
  deactivate,
  cleanup,
  extension_data
};

/* Export plugin to lv2 host.  */
LV2_SYMBOL_EXPORT
const LV2_Descriptor *
lv2_descriptor (uint32_t index)
{
  if (index == 0)
    return &descriptor;
  return NULL;
}
