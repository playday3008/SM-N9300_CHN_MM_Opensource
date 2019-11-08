#include "JAMSynth.h"

//audio.h
jamsynth_audio_driver_t* 	
new_jamsynth_audio_driver(jamsynth_settings_t * settings, jamsynth_synth_t *synth)
{
		return new_fluid_audio_driver(settings, synth);
}

jamsynth_audio_driver_t*
new_jamsynth_audio_driver2(jamsynth_settings_t * settings, jamsynth_audio_func_t func, void* data)
{	  
		return new_fluid_audio_driver2(settings, func, data);					
}

void
delete_jamsynth_audio_driver(jamsynth_audio_driver_t* driver)
{
		delete_fluid_audio_driver(driver);
}

//event.h
jamsynth_event_t*	
new_jamsynth_event()
{
	return new_fluid_event();
}

void 
delete_jamsynth_event(jamsynth_event_t* evt)
{
		delete_fluid_event(evt);
}

void 
jamsynth_event_set_source(jamsynth_event_t* evt, short src)
{
		fluid_event_set_source(evt, src);
}

void 
jamsynth_event_set_dest(jamsynth_event_t* evt, short dest)
{
		fluid_event_set_dest(evt, dest);
}

void 
jamsynth_event_timer(jamsynth_event_t* evt, void* data)
{
		fluid_event_timer(evt, data);
}

void 
jamsynth_event_note(jamsynth_event_t* evt, int channel, short key, short vel, unsigned int duration)
{
		fluid_event_note(evt, channel, key, vel, duration);
}

void 
jamsynth_event_noteon(jamsynth_event_t* evt, int channel, short key, short vel)
{
		fluid_event_noteon(evt, channel, key, vel);
}

void 
jamsynth_event_noteoff(jamsynth_event_t* evt, int channel, short key)
{
		fluid_event_noteoff(evt, channel, key);
}

void 
jamsynth_event_all_sounds_off(jamsynth_event_t* evt, int channel)
{
		fluid_event_all_sounds_off(evt, channel);
}

void 
jamsynth_event_all_notes_off(jamsynth_event_t* evt, int channel)
{
		fluid_event_all_notes_off(evt, channel);
}

//seq.h
jamsynth_sequencer_t* 
new_jamsynth_sequencer(void)
{
		return new_fluid_sequencer();	
}

jamsynth_sequencer_t*
new_jamsynth_sequencer2(int use_system_timer)
{
		return new_fluid_sequencer2(use_system_timer);
}

void
delete_jamsynth_sequencer(jamsynth_sequencer_t* seq)
{
		delete_fluid_sequencer(seq);	
}

int
jamsynth_sequencer_get_use_system_timer(jamsynth_sequencer_t* seq)
{
		return fluid_sequencer_get_use_system_timer(seq);
}

short 
jamsynth_sequencer_register_client(jamsynth_sequencer_t* seq, const char *name, jamsynth_event_callback_t callback, void* data)
{
		return fluid_sequencer_register_client(seq, name, callback, data);
}

void
jamsynth_sequencer_unregister_client(jamsynth_sequencer_t* seq, short id)
{
		fluid_sequencer_unregister_client(seq, id);
}

int 
jamsynth_sequencer_count_clients(jamsynth_sequencer_t* seq)
{
		return fluid_sequencer_count_clients(seq);
}

short 
jamsynth_sequencer_get_client_id(jamsynth_sequencer_t* seq, int index)
{
		return fluid_sequencer_get_client_id(seq, index);
}

char* 
jamsynth_sequencer_get_client_name(jamsynth_sequencer_t* seq, int id)
{
		return fluid_sequencer_get_client_name(seq, id);
}

int
jamsynth_sequencer_client_is_dest(jamsynth_sequencer_t* seq, int id)
{
		return fluid_sequencer_client_is_dest(seq, id);
}

void
jamsynth_sequencer_process(jamsynth_sequencer_t* seq, unsigned int msec)
{
		fluid_sequencer_process(seq, msec);
}

void
jamsynth_sequencer_send_now(jamsynth_sequencer_t* seq, jamsynth_event_t* evt)
{
		fluid_sequencer_send_now(seq, evt);
}

int 
jamsynth_sequencer_send_at(jamsynth_sequencer_t* seq, jamsynth_event_t* evt, unsigned int time, int absolute)
{
		return fluid_sequencer_send_at(seq, evt, time, absolute);
}

void 
jamsynth_sequencer_remove_events(jamsynth_sequencer_t* seq, short source, short dest, int type)
{
		fluid_sequencer_remove_events(seq, source, dest, type);
}

unsigned int 
jamsynth_sequencer_get_tick(jamsynth_sequencer_t* seq)
{
		return fluid_sequencer_get_tick(seq);
}

void 
jamsynth_sequencer_set_time_scale(jamsynth_sequencer_t* seq, double scale)
{
		fluid_sequencer_set_time_scale(seq, scale);
}

double 
jamsynth_sequencer_get_time_scale(jamsynth_sequencer_t* seq)
{
		return fluid_sequencer_get_time_scale(seq);
}


//seqbind.h
short
jamsynth_sequencer_register_fluidsynth(fluid_sequencer_t* seq, fluid_synth_t* synth)
{
		return fluid_sequencer_register_fluidsynth(seq, synth);
}


//settings.h
jamsynth_settings_t* 
new_jamsynth_settings(void)
{
		return new_fluid_settings();
}

void
delete_jamsynth_settings(jamsynth_settings_t* settings)
{
		delete_fluid_settings(settings);
}

int 
jamsynth_settings_setstr(jamsynth_settings_t* settings, const char *name, const char *str)
{
		return fluid_settings_setstr(settings, name, str);
}

int 
jamsynth_settings_getstr(jamsynth_settings_t* settings, const char *name, char** str)
{
		return fluid_settings_getstr(settings, name, str);
}
int 
jamsynth_settings_setnum(jamsynth_settings_t* settings, const char *name, double val)
{
		return fluid_settings_setnum(settings, name, val);
}

int
jamsynth_settings_getnum(jamsynth_settings_t* settings, const char *name, double* val)
{
		return fluid_settings_getnum(settings, name, val);
}

int 
jamsynth_settings_setint(jamsynth_settings_t* settings, const char *name, int val)
{
		return fluid_settings_setint(settings, name, val);
}

int
jamsynth_settings_getint(jamsynth_settings_t* settings, const char *name, int* val)
{
		return fluid_settings_getint(settings, name, val);
}


//synth.h
jamsynth_synth_t*
new_jamsynth_synth(jamsynth_settings_t* settings)
{
		return new_fluid_synth(settings);
}

int 
delete_jamsynth_synth(jamsynth_synth_t* synth)
{
		return delete_fluid_synth(synth);
}

int 
jamsynth_synth_noteon(jamsynth_synth_t* synth, int chan, int key, int vel)
{
		return fluid_synth_noteon(synth, chan, key, vel);
}

int 
jamsynth_synth_noteoff(jamsynth_synth_t* synth, int chan, int key)
{
		return fluid_synth_noteoff(synth, chan, key);
}

int 
jamsynth_synth_cc(jamsynth_synth_t* synth, int chan, int ctrl, int val)
{
		return fluid_synth_cc(synth, chan, ctrl, val);
}

int 
jamsynth_synth_get_cc(jamsynth_synth_t* synth, int chan, int ctrl, int* pval)
{
		return fluid_synth_get_cc(synth, chan, ctrl, pval);
}

int 
jamsynth_synth_pitch_bend(jamsynth_synth_t* synth, int chan, int val)
{
		return fluid_synth_pitch_bend(synth, chan, val);
}

int 
jamsynth_synth_get_pitch_bend(jamsynth_synth_t* synth, int chan, int* ppitch_bend)
{
		return fluid_synth_get_pitch_bend(synth, chan, ppitch_bend);
}

int 
jamsynth_synth_pitch_wheel_sens(jamsynth_synth_t* synth, int chan, int val)
{
		return fluid_synth_pitch_wheel_sens(synth, chan, val);
}

int
jamsynth_synth_get_pitch_wheel_sens(jamsynth_synth_t* synth, int chan, int* pval)
{
		return fluid_synth_get_pitch_wheel_sens(synth, chan, pval);
}

int 
jamsynth_synth_program_change(jamsynth_synth_t* synth, int chan, int program)
{
		return fluid_synth_program_change(synth, chan, program);
}

int
jamsynth_synth_bank_select(jamsynth_synth_t* synth, int chan, unsigned int bank)
{
		return fluid_synth_bank_select(synth, chan, bank);
}

int 
jamsynth_synth_sfont_select(jamsynth_synth_t* synth, int chan, unsigned int sfont_id)
{
		return fluid_synth_sfont_select(synth, chan, sfont_id);
}

int
jamsynth_synth_program_select(jamsynth_synth_t* synth, int chan, unsigned int sfont_id, unsigned int bank_num, unsigned int preset_num)
{
		return fluid_synth_program_select(synth, chan, sfont_id, bank_num, preset_num);
}

int 
jamsynth_synth_program_select_by_sfont_name (jamsynth_synth_t* synth, int chan, const char *sfont_name, unsigned int bank_num,unsigned int preset_num)
{
		return fluid_synth_program_select_by_sfont_name (synth, chan, sfont_name, bank_num,preset_num);
}

int 
jamsynth_synth_get_program(jamsynth_synth_t* synth, int chan, unsigned int* sfont_id, unsigned int* bank_num, unsigned int* preset_num)
{
		return fluid_synth_get_program(synth, chan, sfont_id, bank_num, preset_num);
}

int 
jamsynth_synth_unset_program (jamsynth_synth_t *synth, int chan)
{
		return fluid_synth_unset_program (synth, chan);
}

int
jamsynth_synth_get_channel_info (jamsynth_synth_t *synth, int chan, jamsynth_synth_channel_info_t *info)
{
		return fluid_synth_get_channel_info (synth, chan, info);
}

int jamsynth_synth_program_reset(jamsynth_synth_t* synth)
{
		return fluid_synth_program_reset(synth);
}

int
jamsynth_synth_system_reset(jamsynth_synth_t* synth)
{
		return fluid_synth_system_reset(synth);
}

int 
jamsynth_synth_sfload(jamsynth_synth_t* synth, const char* filename, int reset_presets)
{
		return fluid_synth_sfload(synth, filename, reset_presets);
}

int 
jamsynth_synth_sfreload(jamsynth_synth_t* synth, unsigned int id)
{
		return fluid_synth_sfreload(synth, id);
}

int 
jamsynth_synth_sfunload(jamsynth_synth_t* synth, unsigned int id, int reset_presets)
{
		return fluid_synth_sfunload(synth, id, reset_presets);
}

int
jamsynth_synth_add_sfont(jamsynth_synth_t* synth, jamsynth_sfont_t* sfont)
{
		return fluid_synth_add_sfont(synth, sfont);
}

void 
jamsynth_synth_remove_sfont(jamsynth_synth_t* synth, jamsynth_sfont_t* sfont)
{
		fluid_synth_remove_sfont(synth, sfont);
}

int 
jamsynth_synth_sfcount(jamsynth_synth_t* synth)
{
		return fluid_synth_sfcount(synth);
}

jamsynth_sfont_t* 
jamsynth_synth_get_sfont(jamsynth_synth_t* synth, unsigned int num)
{
		return fluid_synth_get_sfont(synth, num);
}

jamsynth_sfont_t* 
jamsynth_synth_get_sfont_by_id(jamsynth_synth_t* synth, unsigned int id)
{
		return fluid_synth_get_sfont_by_id(synth, id);
}

jamsynth_sfont_t *
jamsynth_synth_get_sfont_by_name (jamsynth_synth_t* synth, const char *name)
{
		return fluid_synth_get_sfont_by_name (synth, name);
}

int 
jamsynth_synth_set_bank_offset(jamsynth_synth_t* synth, int sfont_id, int offset)
{
		return fluid_synth_set_bank_offset(synth, sfont_id, offset);
}

int 
jamsynth_synth_get_bank_offset(jamsynth_synth_t* synth, int sfont_id)
{
		return fluid_synth_get_bank_offset(synth, sfont_id);
}

void 
jamsynth_synth_set_reverb(jamsynth_synth_t* synth, double roomsize, double damping, double width, double level)
{
		fluid_synth_set_reverb(synth, roomsize, damping, width, level);
}

void 
jamsynth_synth_set_reverb_on(jamsynth_synth_t* synth, int on)
{
		fluid_synth_set_reverb_on(synth, on);
}

double 
jamsynth_synth_get_reverb_roomsize(jamsynth_synth_t* synth)
{
		return fluid_synth_get_reverb_roomsize(synth);
}

double 
jamsynth_synth_get_reverb_damp(jamsynth_synth_t* synth)
{
		return fluid_synth_get_reverb_damp(synth);
}

double 
jamsynth_synth_get_reverb_level(jamsynth_synth_t* synth)
{
		return fluid_synth_get_reverb_level(synth);
}

double 
jamsynth_synth_get_reverb_width(jamsynth_synth_t* synth)
{
		return fluid_synth_get_reverb_width(synth);
}

void 
jamsynth_synth_set_chorus(jamsynth_synth_t* synth, int nr, double level, double speed, double depth_ms, int type)
{
		fluid_synth_set_chorus(synth, nr, level, speed, depth_ms, type);
}

void 
jamsynth_synth_set_chorus_on(jamsynth_synth_t* synth, int on)
{
		fluid_synth_set_chorus_on(synth, on);
}

int 
jamsynth_synth_get_chorus_nr(jamsynth_synth_t* synth)
{
		return fluid_synth_get_chorus_nr(synth);
}

double 
jamsynth_synth_get_chorus_level(jamsynth_synth_t* synth)
{
		return fluid_synth_get_chorus_level(synth);
}

double 
jamsynth_synth_get_chorus_speed_Hz(jamsynth_synth_t* synth)
{
		return fluid_synth_get_chorus_speed_Hz(synth);
}

double 
jamsynth_synth_get_chorus_depth_ms(jamsynth_synth_t* synth)
{
		return fluid_synth_get_chorus_depth_ms(synth);
}

int 
jamsynth_synth_get_chorus_type(jamsynth_synth_t* synth)
{
		return fluid_synth_get_chorus_type(synth);
}

void 
jamsynth_synth_set_gain(jamsynth_synth_t* synth, float gain)
{
		fluid_synth_set_gain(synth, gain);
}

int 
jamsynth_synth_handle_midi_event(void* data, jamsynth_midi_event_t* event)
{
		return fluid_synth_handle_midi_event(data, event);
}

void 
jamsynth_synth_set_midi_router(fluid_synth_t* synth, fluid_midi_router_t* router)
{
		fluid_synth_set_midi_router(synth, router);
}


//midi.h
jamsynth_midi_router_t*
new_jamsynth_midi_router(fluid_settings_t* settings, handle_jamsynth_midi_event_func_t handler, void* event_handler_data)
{
		return new_fluid_midi_router(settings, handler, event_handler_data);
}

int 
delete_jamsynth_midi_router(jamsynth_midi_router_t* handler)
{
		return delete_fluid_midi_router(handler);
}

int 
jamsynth_midi_router_set_default_rules (jamsynth_midi_router_t *router)
{
		return fluid_midi_router_set_default_rules(router);
}

int 
jamsynth_midi_router_clear_rules (jamsynth_midi_router_t *router)
{
		return fluid_midi_router_clear_rules (router);
}

int 
jamsynth_midi_router_add_rule(jamsynth_midi_router_t *router, jamsynth_midi_router_rule_t *rule, int type)
{
		return fluid_midi_router_add_rule(router, rule, type);
}

jamsynth_midi_router_rule_t *
new_jamsynth_midi_router_rule (void)
{
		return new_fluid_midi_router_rule ();
}

void 
delete_jamsynth_midi_router_rule (jamsynth_midi_router_rule_t *rule)
{
		delete_fluid_midi_router_rule(rule); 
}

void 
jamsynth_midi_router_rule_set_chan (jamsynth_midi_router_rule_t *rule, int min, int max, float mul, int add)
{
		fluid_midi_router_rule_set_chan (rule, min, max, mul, add);
}

void 
jamsynth_midi_router_rule_set_param1 (jamsynth_midi_router_rule_t *rule, int min, int max, float mul, int add)
{
		fluid_midi_router_rule_set_param1(rule,  min,  max,  mul,  add);
}

void 
jamsynth_midi_router_rule_set_param2 (jamsynth_midi_router_rule_t *rule, int min, int max, float mul, int add)
{
		fluid_midi_router_rule_set_param2(rule, min, max, mul, add);
}

int 
jamsynth_midi_router_handle_midi_event(void* data, jamsynth_midi_event_t* event)
{
		return fluid_midi_router_handle_midi_event(data, event);
}

int 
jamsynth_midi_dump_prerouter(void* data, jamsynth_midi_event_t* event)
{
		return fluid_midi_dump_prerouter(data, event);
}

int 
jamsynth_midi_dump_postrouter(void* data, jamsynth_midi_event_t* event)
{
		return fluid_midi_dump_postrouter(data, event);
}

jamsynth_midi_driver_t* 
new_jamsynth_midi_driver(jamsynth_settings_t* settings, handle_jamsynth_midi_event_func_t handler, void* event_handler_data)
{
		return new_fluid_midi_driver(settings, handler, event_handler_data);
}

void 
delete_jamsynth_midi_driver(jamsynth_midi_driver_t* driver)
{
		delete_fluid_midi_driver(driver);
}

jamsynth_player_t* 
new_jamsynth_player(jamsynth_synth_t* synth)
{
		return new_fluid_player(synth);
}

int 
delete_jamsynth_player(jamsynth_player_t* player)
{
		return delete_fluid_player(player);
}

int 
jamsynth_player_add(jamsynth_player_t* player, const char *midifile)
{
		return fluid_player_add(player, midifile);
}

int 
jamsynth_player_add_mem(jamsynth_player_t* player, const void *buffer, size_t len)
{
		return fluid_player_add_mem(player, buffer, len);
}

int 
jamsynth_player_play(jamsynth_player_t* player)
{
		return fluid_player_play(player);
}

int 
jamsynth_player_stop(jamsynth_player_t* player)
{
		return fluid_player_stop(player);
}

int 
jamsynth_player_join(jamsynth_player_t* player)
{
		return fluid_player_join(player);
}

int 
jamsynth_player_set_loop(jamsynth_player_t* player, int loop)
{
		return fluid_player_set_loop(player, loop);
}

int 
jamsynth_player_set_midi_tempo(jamsynth_player_t* player, int tempo)
{
		return fluid_player_set_midi_tempo(player, tempo);
}

int 
jamsynth_player_set_bpm(jamsynth_player_t* player, int bpm)
{
		return fluid_player_set_bpm(player, bpm);
}

int 
jamsynth_player_get_status(jamsynth_player_t* player)
{
		return fluid_player_get_status(player);
}

int 
jamsynth_player_set_playback_callback(jamsynth_player_t* player, handle_jamsynth_midi_event_func_t handler, void* handler_data)
{
		return fluid_player_set_playback_callback(player, handler, handler_data);
}
