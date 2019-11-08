/* 
	* Wavetable Synthesizer API set for android APK
	*/




#ifndef _JAMSYNTH_H_
#define _JAMSYNTH_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FLUIDSYNTH_API

#include "fluidsynth/types.h"
#include "fluidsynth/settings.h"
#include "fluidsynth/synth.h"
#include "fluidsynth/audio.h"
#include "fluidsynth/event.h"
#include "fluidsynth/seq.h"
#include "fluidsynth/seqbind.h"
#include "fluidsynth/midi.h"
#include "fluidsynth/sfont.h"


#define JAMSYNTH_OK		(0)
#define JAMSYNTH_FAIL	(-1)

typedef fluid_audio_driver_t		jamsynth_audio_driver_t;
typedef fluid_settings_t					jamsynth_settings_t;
typedef fluid_synth_t							jamsynth_synth_t;
typedef fluid_audio_func_t			jamsynth_audio_func_t;
typedef fluid_event_t						jamsynth_event_t;
typedef fluid_sequencer_t			jamsynth_sequencer_t;
typedef fluid_event_callback_t	jamsynth_event_callback_t;
typedef fluid_midi_event_t	jamsynth_midi_event_t;
typedef fluid_midi_router_t	jamsynth_midi_router_t;
typedef handle_midi_event_func_t handle_jamsynth_midi_event_func_t;
typedef fluid_midi_router_rule_t	jamsynth_midi_router_rule_t;
typedef fluid_midi_driver_t		jamsynth_midi_driver_t;
typedef fluid_midi_router_rule_type	jamsynth_midi_router_rule_type;
typedef fluid_player_t	jamsynth_player_t;
typedef fluid_sfont_t	jamsynth_sfont_t;
typedef fluid_synth_channel_info_t	jamsynth_synth_channel_info_t;
typedef fluid_preset_t	jamsynth_preset_t;

// audio.h
jamsynth_audio_driver_t* new_jamsynth_audio_driver(jamsynth_settings_t * settings, jamsynth_synth_t *synth);
jamsynth_audio_driver_t* new_jamsynth_audio_driver2(jamsynth_settings_t * settings, jamsynth_audio_func_t func, void* data);
void delete_jamsynth_audio_driver(jamsynth_audio_driver_t* driver);

//event.h
jamsynth_event_t*	new_jamsynth_event(void);
void delete_jamsynth_event(jamsynth_event_t* evt);
void jamsynth_event_set_source(jamsynth_event_t* evt, short src);
void jamsynth_event_set_dest(jamsynth_event_t* evt, short dest);
void jamsynth_event_timer(jamsynth_event_t* evt, void* data);
void jamsynth_event_note(jamsynth_event_t* evt, int channel, short key, short vel, unsigned int duration);
void jamsynth_event_noteon(jamsynth_event_t* evt, int channel, short key, short vel);
void jamsynth_event_noteoff(jamsynth_event_t* evt, int channel, short key);
void jamsynth_event_all_sounds_off(jamsynth_event_t* evt, int channel);
void jamsynth_event_all_notes_off(jamsynth_event_t* evt, int channel);


//seq.h
jamsynth_sequencer_t* new_jamsynth_sequencer(void);
jamsynth_sequencer_t* new_jamsynth_sequencer2(int use_system_timer);
void delete_jamsynth_sequencer(jamsynth_sequencer_t* seq);
int jamsynth_sequencer_get_use_system_timer(jamsynth_sequencer_t* seq);
short jamsynth_sequencer_register_client(jamsynth_sequencer_t* seq, const char *name, jamsynth_event_callback_t callback, void* data);
void jamsynth_sequencer_unregister_client(jamsynth_sequencer_t* seq, short id);
int jamsynth_sequencer_count_clients(jamsynth_sequencer_t* seq);
short jamsynth_sequencer_get_client_id(jamsynth_sequencer_t* seq, int index);
char* jamsynth_sequencer_get_client_name(jamsynth_sequencer_t* seq, int id);
int jamsynth_sequencer_client_is_dest(jamsynth_sequencer_t* seq, int id);
void jamsynth_sequencer_process(jamsynth_sequencer_t* seq, unsigned int msec);
void jamsynth_sequencer_send_now(jamsynth_sequencer_t* seq, jamsynth_event_t* evt);
int jamsynth_sequencer_send_at(jamsynth_sequencer_t* seq, jamsynth_event_t* evt, unsigned int time, int absolute);
void jamsynth_sequencer_remove_events(jamsynth_sequencer_t* seq, short source, short dest, int type);
unsigned int jamsynth_sequencer_get_tick(jamsynth_sequencer_t* seq);
void jamsynth_sequencer_set_time_scale(jamsynth_sequencer_t* seq, double scale);
double jamsynth_sequencer_get_time_scale(jamsynth_sequencer_t* seq);


//seqbind.h
short jamsynth_sequencer_register_fluidsynth(fluid_sequencer_t* seq, fluid_synth_t* synth);


//settings.h
jamsynth_settings_t* new_jamsynth_settings(void);
void delete_jamsynth_settings(jamsynth_settings_t* settings);
int jamsynth_settings_setstr(jamsynth_settings_t* settings, const char *name, const char *str);
int jamsynth_settings_getstr(jamsynth_settings_t* settings, const char *name, char** str);
int jamsynth_settings_setnum(jamsynth_settings_t* settings, const char *name, double val);
int jamsynth_settings_getnum(jamsynth_settings_t* settings, const char *name, double* val);
int jamsynth_settings_setint(jamsynth_settings_t* settings, const char *name, int val);
int jamsynth_settings_getint(jamsynth_settings_t* settings, const char *name, int* val);


//synth.h
jamsynth_synth_t* new_jamsynth_synth(jamsynth_settings_t* settings);
int delete_jamsynth_synth(jamsynth_synth_t* synth);
int jamsynth_synth_noteon(jamsynth_synth_t* synth, int chan, int key, int vel);
int jamsynth_synth_noteoff(jamsynth_synth_t* synth, int chan, int key);
int jamsynth_synth_cc(jamsynth_synth_t* synth, int chan, int ctrl, int val);
int jamsynth_synth_get_cc(jamsynth_synth_t* synth, int chan, int ctrl, int* pval);
int jamsynth_synth_pitch_bend(jamsynth_synth_t* synth, int chan, int val);
int jamsynth_synth_get_pitch_bend(jamsynth_synth_t* synth, int chan, int* ppitch_bend);
int jamsynth_synth_pitch_wheel_sens(jamsynth_synth_t* synth, int chan, int val);
int jamsynth_synth_get_pitch_wheel_sens(jamsynth_synth_t* synth, int chan, int* pval);
int jamsynth_synth_program_change(jamsynth_synth_t* synth, int chan, int program);
int jamsynth_synth_bank_select(jamsynth_synth_t* synth, int chan, unsigned int bank);
int jamsynth_synth_sfont_select(jamsynth_synth_t* synth, int chan, unsigned int sfont_id);
int jamsynth_synth_program_select(jamsynth_synth_t* synth, int chan, unsigned int sfont_id, unsigned int bank_num, unsigned int preset_num);

int jamsynth_synth_program_select_by_sfont_name (jamsynth_synth_t* synth, int chan, const char *sfont_name, unsigned int bank_num,unsigned int preset_num);
int jamsynth_synth_get_program(jamsynth_synth_t* synth, int chan, unsigned int* sfont_id, unsigned int* bank_num, unsigned int* preset_num);
int jamsynth_synth_unset_program (jamsynth_synth_t *synth, int chan);
int jamsynth_synth_get_channel_info (jamsynth_synth_t *synth, int chan, jamsynth_synth_channel_info_t *info);
int jamsynth_synth_program_reset(jamsynth_synth_t* synth);

int jamsynth_synth_system_reset(jamsynth_synth_t* synth);
int jamsynth_synth_sfload(jamsynth_synth_t* synth, const char* filename, int reset_presets);
int jamsynth_synth_sfreload(jamsynth_synth_t* synth, unsigned int id);
int jamsynth_synth_sfunload(jamsynth_synth_t* synth, unsigned int id, int reset_presets);

int jamsynth_synth_add_sfont(jamsynth_synth_t* synth, jamsynth_sfont_t* sfont);
void jamsynth_synth_remove_sfont(jamsynth_synth_t* synth, jamsynth_sfont_t* sfont);
int jamsynth_synth_sfcount(jamsynth_synth_t* synth);
jamsynth_sfont_t* jamsynth_synth_get_sfont(jamsynth_synth_t* synth, unsigned int num);
jamsynth_sfont_t* jamsynth_synth_get_sfont_by_id(jamsynth_synth_t* synth, unsigned int id);
jamsynth_sfont_t *jamsynth_synth_get_sfont_by_name (jamsynth_synth_t* synth, const char *name);
                                                             
int jamsynth_synth_set_bank_offset(jamsynth_synth_t* synth, int sfont_id, int offset);
int jamsynth_synth_get_bank_offset(jamsynth_synth_t* synth, int sfont_id);
void jamsynth_synth_set_reverb(jamsynth_synth_t* synth, double roomsize, double damping, double width, double level);
void jamsynth_synth_set_reverb_on(jamsynth_synth_t* synth, int on);
double jamsynth_synth_get_reverb_roomsize(jamsynth_synth_t* synth);
double jamsynth_synth_get_reverb_damp(jamsynth_synth_t* synth);
double jamsynth_synth_get_reverb_level(jamsynth_synth_t* synth);
double jamsynth_synth_get_reverb_width(jamsynth_synth_t* synth);
void jamsynth_synth_set_chorus(jamsynth_synth_t* synth, int nr, double level, double speed, double depth_ms, int type);
void jamsynth_synth_set_chorus_on(jamsynth_synth_t* synth, int on);
int jamsynth_synth_get_chorus_nr(jamsynth_synth_t* synth);
double jamsynth_synth_get_chorus_level(jamsynth_synth_t* synth);
double jamsynth_synth_get_chorus_speed_Hz(jamsynth_synth_t* synth);
double jamsynth_synth_get_chorus_depth_ms(jamsynth_synth_t* synth);
int jamsynth_synth_get_chorus_type(jamsynth_synth_t* synth); 
void jamsynth_synth_set_gain(jamsynth_synth_t* synth, float gain);
int jamsynth_synth_handle_midi_event(void* data, jamsynth_midi_event_t* event);
void jamsynth_synth_set_midi_router(jamsynth_synth_t* synth, jamsynth_midi_router_t* router);



//midi.h                                
fluid_midi_router_t* new_jamsynth_midi_router(jamsynth_settings_t* settings, handle_jamsynth_midi_event_func_t handler, void* event_handler_data); 

int delete_jamsynth_midi_router(jamsynth_midi_router_t* handler); 
int jamsynth_midi_router_set_default_rules (jamsynth_midi_router_t *router);
int jamsynth_midi_router_clear_rules (jamsynth_midi_router_t *router);
int jamsynth_midi_router_add_rule (jamsynth_midi_router_t *router, jamsynth_midi_router_rule_t *rule, int type);
jamsynth_midi_router_rule_t *new_jamsynth_midi_router_rule (void);
void delete_jamsynth_midi_router_rule (jamsynth_midi_router_rule_t *rule);
void jamsynth_midi_router_rule_set_chan (jamsynth_midi_router_rule_t *rule, int min, int max, float mul, int add);
void jamsynth_midi_router_rule_set_param1 (jamsynth_midi_router_rule_t *rule, int min, int max, float mul, int add);
void jamsynth_midi_router_rule_set_param2 (jamsynth_midi_router_rule_t *rule, int min, int max, float mul, int add);
int jamsynth_midi_router_handle_midi_event(void* data, jamsynth_midi_event_t* event);
int jamsynth_midi_dump_prerouter(void* data, jamsynth_midi_event_t* event); 
int jamsynth_midi_dump_postrouter(void* data, jamsynth_midi_event_t* event); 
jamsynth_midi_driver_t* new_jamsynth_midi_driver(jamsynth_settings_t* settings, handle_jamsynth_midi_event_func_t handler, void* event_handler_data);
void delete_jamsynth_midi_driver(jamsynth_midi_driver_t* driver);


enum jamsynth_player_status
{
  JAMSYNTH_PLAYER_READY,           /**< Player is ready */
  JAMSYNTH_PLAYER_PLAYING,         /**< Player is currently playing */
  JAMSYNTH_PLAYER_DONE             /**< Player is finished playing */
};


jamsynth_player_t* new_jamsynth_player(jamsynth_synth_t* synth);
int delete_jamsynth_player(jamsynth_player_t* player);
int jamsynth_player_add(jamsynth_player_t* player, const char *midifile);
int jamsynth_player_add_mem(jamsynth_player_t* player, const void *buffer, size_t len);
int jamsynth_player_play(jamsynth_player_t* player);
int jamsynth_player_stop(jamsynth_player_t* player);
int jamsynth_player_join(jamsynth_player_t* player);
int jamsynth_player_set_loop(jamsynth_player_t* player, int loop);
int jamsynth_player_set_midi_tempo(jamsynth_player_t* player, int tempo);
int jamsynth_player_set_bpm(jamsynth_player_t* player, int bpm);
int jamsynth_player_get_status(jamsynth_player_t* player);
int jamsynth_player_set_playback_callback(jamsynth_player_t* player, handle_jamsynth_midi_event_func_t handler, void* handler_data);


//sfont.h  
//rewrite fluid_sfont.h 

#define jamsynth_sfloader_delete(_loader) { if ((_loader) && (_loader)->free) (*(_loader)->free)(_loader); }
#define jamsynth_sfloader_load(_loader, _filename) (*(_loader)->load)(_loader, _filename)

#define delete_jamsynth_sfont(_sf)   ( ((_sf) && (_sf)->free)? (*(_sf)->free)(_sf) : 0)
#define jamsynth_sfont_get_name(_sf) (*(_sf)->get_name)(_sf)
#define jamsynth_sfont_get_preset(_sf,_bank,_prenum) (*(_sf)->get_preset)(_sf,_bank,_prenum)
#define jamsynth_sfont_iteration_start(_sf) (*(_sf)->iteration_start)(_sf)
#define jamsynth_sfont_iteration_next(_sf,_pr) (*(_sf)->iteration_next)(_sf,_pr)
#define jamsynth_sfont_get_data(_sf) (_sf)->data
#define jamsynth_sfont_set_data(_sf,_p) { (_sf)->data = (void*) (_p); }

#define delete_jamsynth_preset(_preset) \
  { if ((_preset) && (_preset)->free) { (*(_preset)->free)(_preset); }}

#define jamsynth_preset_get_data(_preset) (_preset)->data
#define jamsynth_preset_set_data(_preset,_p) { (_preset)->data = (void*) (_p); }
#define jamsynth_preset_get_name(_preset) (*(_preset)->get_name)(_preset)
#define jamsynth_preset_get_banknum(_preset) (*(_preset)->get_banknum)(_preset)
#define jamsynth_preset_get_num(_preset) (*(_preset)->get_num)(_preset)

#define jamsynth_preset_noteon(_preset,_synth,_ch,_key,_vel) \
  (*(_preset)->noteon)(_preset,_synth,_ch,_key,_vel)

#define jamsynth_preset_notify(_preset,_reason,_chan) \
  { if ((_preset) && (_preset)->notify) { (*(_preset)->notify)(_preset,_reason,_chan); }}


#define jamsynth_sample_incr_ref(_sample) { (_sample)->refcount++; }

#define jamsynth_sample_decr_ref(_sample) \
  (_sample)->refcount--; \
  if (((_sample)->refcount == 0) && ((_sample)->notify)) \
    (*(_sample)->notify)(_sample, FLUID_SAMPLE_DONE);

//External effect (REV, CHO, Delay)
void jamsynth_init_lxdsp();
void jamsynth_deinit_lxdsp();
void jamsynth_reset_lxdsp();
void jamsynth_set_lxdsp_delay_on(int on);
void jamsynth_set_lxdsp_delay(float dry, float wet, int mode, float time, float feedback, float tone);
void jamsynth_set_lxdsp_rev_on(int on);
void jamsynth_set_lxdsp_rev(float dry, float wet, int mode, float time, float tone);
void jamsynth_set_lxdsp_chorus_on(int on);
void jamsynth_set_lxdsp_chorus(float dry, float wet, float rate, float width);

void jamsynth_set_isfilerendering(int render);
int jamsynth_get_isfilerendering();

#ifdef __cplusplus
}
#endif

#endif

