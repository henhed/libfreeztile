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

/* Standard headers.  */
#include <stdlib.h>
#include <string.h>

/* LV2 headers.  */
#include <lv2core.lv2/lv2.h>
#include <urid.lv2/urid.h>
#include <midi.lv2/midi.h>
#include <atom.lv2/atom.h>
#include <atom.lv2/util.h>

/* libfreeztile headers.  */
#include "../../src/class.h"
#include "../../src/voice.h"
#include "../../src/node.h"
#include "../../src/graph.h"
#include "../../src/form.h"
#include "../../src/adsr.h"
#include "../../src/lfo.h"
#include "../../src/filter.h"
#include "../../src/delay.h"

#define FZEX1_URI "http://www.freeztile.org/plugins/fzex1"
#define POLYPHONY 4
#define NUM_ENGINES 2
#define NUM_CHANNELS 2

/* Named port numbers.  */
enum {
  AUDIO_OUT_LEFT = 0,
  AUDIO_OUT_RIGHT,
  MIDI_IN,
  E1_FORM_SHAPE,
  E1_FORM_SHIFT,
  E1_FORM_PITCH,
  E1_FORM_OFFSET,
  E1_FORM_GLISS,
  E1_ATK_AMP,
  E1_ATK_LEN,
  E1_DCY_AMP,
  E1_DCY_LEN,
  E1_STN_AMP,
  E1_STN_LEN,
  E1_RLS_LEN,
  E1_MOD_SHAPE,
  E1_MOD_FREQ,
  E1_MOD_DEPTH,
  E1_FLT_TYPE,
  E1_FLT_FREQ,
  E1_FLT_RES,
  E1_DLY_GAIN,
  E1_DLY_FEEDBACK,
  E1_DLY_TIME,
  E2_FORM_SHAPE,
  E2_FORM_SHIFT,
  E2_FORM_PITCH,
  E2_FORM_OFFSET,
  E2_FORM_GLISS,
  E2_ATK_AMP,
  E2_ATK_LEN,
  E2_DCY_AMP,
  E2_DCY_LEN,
  E2_STN_AMP,
  E2_STN_LEN,
  E2_RLS_LEN,
  E2_MOD_SHAPE,
  E2_MOD_FREQ,
  E2_MOD_DEPTH,
  E2_FLT_TYPE,
  E2_FLT_FREQ,
  E2_FLT_RES,
  E2_DLY_GAIN,
  E2_DLY_FEEDBACK,
  E2_DLY_TIME,
  NUM_PORTS
};

/* Sound unit struct with sample generating/manipulating objects and
   related parameters.  */
typedef struct {
  form_t *form;
  int_t form_shape;
  adsr_t *envelope;
  filter_t *filter;
  delay_t *delay;
  lfo_t *modulator;
  real_t mod_depth;
  int_t mod_shape;
} Engine;

/* Private plugin struct holding plugin data, state and resource
   references.  */
typedef struct {
  void *ports[NUM_PORTS];
  LV2_URID midi_urid;
  vpool_t *voice_pool;
  graph_t *graph;
  Engine engines[NUM_ENGINES];
  node_t *sinks[NUM_CHANNELS];
} FzEx1;

/* Create a new instance of this plugin. This function is called by
   the host in the `instantiation' threading class.  */
static LV2_Handle
instantiate (const LV2_Descriptor *descriptor, double rate,
             const char *bundle_path,
             const LV2_Feature * const *features)
{
  FzEx1 *plugin = (FzEx1 *) malloc (sizeof (FzEx1));
  if (!plugin)
    return NULL;
  memset (plugin, 0, sizeof (FzEx1));

  LV2_URID_Map *map = NULL;
  for (uint32_t i = 0; features[i]; ++i)
    {
      if (!strcmp (features[i]->URI, LV2_URID__map))
        map = (LV2_URID_Map *) features[i]->data;
    }

  if (!map)
    {
      free (plugin);
      return NULL;
    }

  plugin->midi_urid = map->map (map->handle, LV2_MIDI__MidiEvent);
  fz_set_sample_rate (rate);

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
  FzEx1 *plugin = (FzEx1 *) instance;
  plugin->voice_pool = fz_new (vpool_c, POLYPHONY);
  plugin->graph = fz_new (graph_c);

  /* Create and connect engine objects.  */
  uint_t ei;
  for (ei = 0; ei < NUM_ENGINES; ++ei)
    {
      Engine *engine = &plugin->engines[ei];
      engine->form_shape = SHAPE_SINE;
      engine->form = fz_new (form_c, engine->form_shape);
      engine->envelope = fz_new (adsr_c);
      engine->filter = fz_new (filter_c);
      engine->delay = fz_new (delay_c);
      engine->mod_shape = SHAPE_SINE;
      engine->mod_depth = 0;
      engine->modulator = fz_new (lfo_c, engine->mod_shape,
                                  (real_t) 0);

      /* Connect ADSR to form amplitude.  */
      fz_node_connect ((node_t *) engine->form,
                       (mod_t *) engine->envelope, FORM_SLOT_AMP,
                       NULL);

      /* Connect LFO to form frequency.  */
      fz_node_connect ((node_t *) engine->form,
                       (mod_t *) engine->modulator, FORM_SLOT_FREQ,
                       &engine->mod_depth);

      /* Add engine form, filter and delay to graph.  */
      fz_graph_add_node (plugin->graph, (node_t *) engine->form);
      fz_graph_add_node (plugin->graph, (node_t *) engine->filter);
      fz_graph_add_node (plugin->graph, (node_t *) engine->delay);
      fz_graph_connect (plugin->graph,
                        (node_t *) engine->form,
                        (node_t *) engine->filter);
      fz_graph_connect (plugin->graph,
                        (node_t *) engine->filter,
                        (node_t *) engine->delay);
      /* Nodes are retained by the graph and will be released when
         graph is deleted in `deactivate'.  */
      fz_del (engine->delay);
      fz_del (engine->filter);
      fz_del (engine->form);
    }

  /* Create graph sinks to be mapped to audio output ports.  */
  for (uint_t ci = 0; ci < NUM_CHANNELS; ++ci)
    {
      plugin->sinks[ci] = fz_new (node_c);
      fz_graph_add_node (plugin->graph, plugin->sinks[ci]);
      fz_del (plugin->sinks[ci]);

      /* Connect engine delays to sinks.  */
      for (ei = 0; ei < NUM_ENGINES; ++ei)
        fz_graph_connect (plugin->graph,
                          (node_t *) plugin->engines[ei].delay,
                          plugin->sinks[ci]);
    }

  /* Prepare graph to generate a fairly large number of samples here
     to reduce the chance of reallocation of internal buffers in the
     time sensitive `run' function.  */
  fz_graph_prepare (plugin->graph, 8192);
}

/* Macro for accessing a given port for a specific engine.  */
#define NTH_ENGINE_PORT(plugin, port, i) \
  (float *) ((FzEx1 *) (plugin))->ports[ \
    (port) + (E2_FORM_SHAPE - E1_FORM_SHAPE) * (i)]

/* Helper function to `run' for interpreting engine control ports.  */
static inline void
update_engine_controls (FzEx1 *plugin)
{
  for (uint_t i = 0; i < NUM_ENGINES; ++i)
    {
      Engine *e = &plugin->engines[i];

      /* Update form shape if it changed.  */
      float form_shape = *NTH_ENGINE_PORT (plugin, E1_FORM_SHAPE, i);
      if (e->form_shape != (int_t) form_shape)
        e->form_shape = fz_form_set_shape (e->form,
                                           (int_t) form_shape);
      fz_form_set_shifting (e->form,
                            *NTH_ENGINE_PORT (plugin, E1_FORM_SHIFT, i));

      fz_form_set_pitch (e->form,
                         *NTH_ENGINE_PORT (plugin, E1_FORM_PITCH, i)
                         + *NTH_ENGINE_PORT (plugin, E1_FORM_OFFSET, i));

      fz_form_set_portamento (e->form,
                              *NTH_ENGINE_PORT (plugin, E1_FORM_GLISS, i));

      /* Update ADSR.  */
      fz_adsr_set_a_amp (e->envelope,
                         *NTH_ENGINE_PORT (plugin, E1_ATK_AMP, i));
      fz_adsr_set_a_len (e->envelope,
                         *NTH_ENGINE_PORT (plugin, E1_ATK_LEN, i));
      fz_adsr_set_d_amp (e->envelope,
                         *NTH_ENGINE_PORT (plugin, E1_DCY_AMP, i));
      fz_adsr_set_d_len (e->envelope,
                         *NTH_ENGINE_PORT (plugin, E1_DCY_LEN, i));
      fz_adsr_set_s_amp (e->envelope,
                         *NTH_ENGINE_PORT (plugin, E1_STN_AMP, i));
      fz_adsr_set_s_len (e->envelope,
                         *NTH_ENGINE_PORT (plugin, E1_STN_LEN, i));
      fz_adsr_set_r_len (e->envelope,
                         *NTH_ENGINE_PORT (plugin, E1_RLS_LEN, i));

      /* Update LFO Shape.  */
      float mod_shape = *NTH_ENGINE_PORT (plugin, E1_MOD_SHAPE, i);
      if (e->mod_shape != (int_t) mod_shape)
        e->mod_shape = fz_lfo_set_shape (e->modulator,
                                         (int_t) mod_shape);

      /* Update LFO frequency.  */
      fz_lfo_set_frequency (e->modulator,
                            *NTH_ENGINE_PORT (plugin, E1_MOD_FREQ, i));

      /* Update LFO depth.  */
      e->mod_depth = *NTH_ENGINE_PORT (plugin, E1_MOD_DEPTH, i);

      /* Update filter type, cutoff and resonance.  */
      fz_filter_set_type (e->filter, (int_t)
                          *NTH_ENGINE_PORT (plugin, E1_FLT_TYPE, i));
      fz_filter_set_frequency (e->filter,
                               *NTH_ENGINE_PORT (plugin, E1_FLT_FREQ, i));
      fz_filter_set_resonance (e->filter,
                               *NTH_ENGINE_PORT (plugin, E1_FLT_RES, i));

      /* Update delay gain, feedback and time.  */
      fz_delay_set_gain (e->delay,
                         *NTH_ENGINE_PORT (plugin, E1_DLY_GAIN, i));
      fz_delay_set_feedback (e->delay,
                             *NTH_ENGINE_PORT (plugin, E1_DLY_FEEDBACK, i));
      fz_delay_set_delay (e->delay,
                          *NTH_ENGINE_PORT (plugin, E1_DLY_TIME, i));
    }
}

/* Write NSAMPLES frames to audio output ports. This function runs in
   in the `audio' threading class and must be real-time safe.  */
static void
run (LV2_Handle instance, uint32_t nsamples)
{
  FzEx1 *plugin = (FzEx1 *) instance;
  update_engine_controls (plugin);

  /* Look for new midi events.  */
  const LV2_Atom_Sequence *events =
    (const LV2_Atom_Sequence *) plugin->ports[MIDI_IN];

  LV2_ATOM_SEQUENCE_FOREACH (events, event)
    {
      if (event->body.type != plugin->midi_urid)
        continue;
      const uint8_t * const msg = (const uint8_t *) (event + 1);
      switch (lv2_midi_message_type (msg))
        {
        case LV2_MIDI_MSG_NOTE_ON:
          /* Press note msg[1] with velocity msg[2].  */
          fz_vpool_press (plugin->voice_pool, (uint_t) msg[1],
                          ((real_t) msg[2]) / 127);
          break;
        case LV2_MIDI_MSG_NOTE_OFF:
          /* Release note msg[1].  */
          fz_vpool_release (plugin->voice_pool, (uint_t) msg[1]);
          break;
        default:
          break;
        }
    }

  /* Generate output.  */
  real_t *sinks[] = {
    fz_list_data (fz_graph_buffer (plugin->graph, plugin->sinks[0])),
    fz_list_data (fz_graph_buffer (plugin->graph, plugin->sinks[1]))
  };
  float *outputs[] = {
    (float *) plugin->ports[AUDIO_OUT_LEFT],
    (float *) plugin->ports[AUDIO_OUT_RIGHT]
  };

  /* Render graph with each active voice.  */
  voice_t *voice;
  const list_t *voices = fz_vpool_voices (plugin->voice_pool);
  size_t nvoices = fz_len ((const ptr_t) voices);
  for (uint_t vi = 0; vi < nvoices; ++vi)
    {
      voice = fz_ref_at (voices, vi, voice_t);

      /* Prepare graph to generate NSAMPLES samples. This is not
         necessarily real-time safe but since we've prepared the graph
         in `activate', graphs' internal buffers should not be
         reallocated unless NSAMPLES is a really large number.  */
      fz_graph_prepare (plugin->graph, nsamples);

      fz_graph_render (plugin->graph, voice);

      /* Copy samples from graph sinks to the output ports.  */
      for (uint_t oi = 0; oi < NUM_CHANNELS; ++oi)
        {
          if (sinks[oi] == NULL || outputs[oi] == NULL)
            continue;

          for (uint_t si = 0; si < nsamples; ++si)
            outputs[oi][si] += (float) sinks[oi][si];
        }

      /* /\* Help voice pool prioritizing voice stealing by killing voices */
      /*    we know to be silent.  *\/ */
      /* bool_t voice_is_silent = TRUE; */
      /* for (uint_t ei = 0; ei < NUM_ENGINES; ++ei) */
      /*   if (!fz_adsr_is_silent (plugin->engines[ei].envelope, */
      /*                           voice)) */
      /*     { */
      /*       voice_is_silent = FALSE; */
      /*       break; */
      /*     } */
      /* if (voice_is_silent) */
      /*   fz_vpool_kill (plugin->voice_pool, voice); */
    }
}

/* Free any resources allocated in `activate'.  */
static void
deactivate (LV2_Handle instance)
{
  FzEx1 *plugin = (FzEx1 *) instance;
  for (uint_t i = 0; i < NUM_ENGINES; ++i)
    {
      fz_del (plugin->engines[i].modulator);
      fz_del (plugin->engines[i].envelope);
    }
  /* Sinks and engine forms are released by graph.  */
  fz_del (plugin->graph);
  fz_del (plugin->voice_pool);
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
