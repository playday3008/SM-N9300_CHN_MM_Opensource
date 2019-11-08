/* FluidSynth Simple - An example of using fluidsynth
 *
 * This code is in the public domain.
 *
 * To compile:
 *   gcc -g -O -o fluidsynth_simple fluidsynth_simple.c -lfluidsynth
 *
 * To run
 *   fluidsynth_simple soundfont
 *
 * [Peter Hanappe]
 */


#include <stdio.h>
#include <signal.h>
#include <fluidsynth.h>

static fluid_settings_t* settings = NULL;
static fluid_synth_t* synth = NULL;
static fluid_audio_driver_t* adriver = NULL;
static fluid_midi_driver_t* mdriver = NULL;
static fluid_midi_router_t* router = NULL;

static void signal_handler(int sig)
{
	if (mdriver) {
		delete_fluid_midi_driver(mdriver);
	}
	if (router) {
		delete_fluid_midi_router(router);
	}
	if (adriver) {
		delete_fluid_audio_driver(adriver);
	}
	if (synth) {
		delete_fluid_synth(synth);
	}
	if (settings) {
		delete_fluid_settings(settings);
	}
	fprintf(stderr, "signal received, exiting ...\n");
	exit(0);
}

int main(int argc, char** argv) 
{
	//fluid_settings_t* settings;
	//fluid_synth_t* synth = NULL;
	//fluid_audio_driver_t* adriver = NULL;
	int err = 0;
	//fluid_midi_driver_t* mdriver = NULL;
	//fluid_midi_router_t* router = NULL;

	printf("FluidSynth argc %d\n", argc);

	if (argc != 2) {
		fprintf(stderr, "Usage: fluidsynth_simple [soundfont]\n");
		return 1;
	}

	/* Create the settings object. This example uses the default
	 * values for the settings. */
	settings = new_fluid_settings();
	if (settings == NULL) {
		fprintf(stderr, "Failed to create the settings\n");
		err = 2;
		goto cleanup;
	}

	fluid_settings_setnum(settings, "synth.gain", 2.0);

	/* Create the synthesizer */
	synth = new_fluid_synth(settings);
	if (synth == NULL) {
		fprintf(stderr, "Failed to create the synthesizer\n");
		err = 3;
		goto cleanup;
	}

	/* Load the soundfont */
	if (fluid_synth_sfload(synth, argv[1], 1) == -1) {
		fprintf(stderr, "Failed to load the SoundFont\n");
		err = 4;
		goto cleanup;
	}

	/* Create the audio driver. As soon as the audio driver is
	 * created, the synthesizer can be played. */
	adriver = new_fluid_audio_driver(settings, synth);
	if (adriver == NULL) {
		fprintf(stderr, "Failed to create the audio driver\n");
		err = 5;
		goto cleanup;
	}

	/* Create midi driver */
	router = new_fluid_midi_router(settings, fluid_synth_handle_midi_event, (void*)synth);

	if (router == NULL) {
		fprintf(stderr, "Failed to create the MIDI input router; no MIDI input\n"
			"will be available. You can access the synthesizer \n"
			"through the console.\n");
		} else {
		fluid_synth_set_midi_router(synth, router); /* Fixme, needed for command handler */
		//fluid_sequencer_register_fluidsynth(sequencer, synth);
		mdriver = new_fluid_midi_driver(settings, fluid_midi_router_handle_midi_event, (void*) router);
		if (mdriver == NULL) {
		fprintf(stderr, "Failed to create the MIDI thread; no MIDI input\n"
			"will be available. You can access the synthesizer \n"
			"through the console.\n");
		}
	}

	/* Play a note */
	//fluid_synth_noteon(synth, 0, 60, 100);

	//printf("Press \"Enter\" to stop: ");
	//fgetc(stdin);
	//printf("done\n");

	signal(SIGQUIT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);

	while(1)
	{
		sleep(1);
	}
	
 cleanup:
	
	if (mdriver) {
		delete_fluid_midi_driver(mdriver);
	}
	if (router) {
		delete_fluid_midi_router(router);
	}
	if (adriver) {
		delete_fluid_audio_driver(adriver);
	}
	if (synth) {
		delete_fluid_synth(synth);
	}
	if (settings) {
		delete_fluid_settings(settings);
	}
	
	return err;
}
