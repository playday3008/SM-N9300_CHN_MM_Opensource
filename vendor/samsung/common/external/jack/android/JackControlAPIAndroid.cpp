// u/* -*- Mode: C++ ; c-basic-offset: 4 -*- */
/*
  JACK control API implementation

  Copyright (C) 2008 Nedko Arnaudov
  Copyright (C) 2008 Grame
  Copyright (C) 2013 Samsung Electronics

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef WIN32
#include <stdint.h>
#include <dirent.h>
#include <pthread.h>
#endif

#include "types.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>

#include "JackControlAPI.h"
#include "JackControlAPIAndroid.h"
#include "JackConstants.h"
#include "JackServerGlobals.h"
#include "JackEngineControl.h"

#include <iostream>
#include <fstream>

using namespace Jack;

struct jackctl_sigmask
{
    sigset_t signals;
};

static jackctl_sigmask sigmask;

static JSList* backup_ptr = NULL;
typedef struct backup_param {
    char id;
    union jackctl_parameter_value value;
} backup_param_t;

#ifdef SUPPORT_JACK_USBAUDIO
char curUsbAudioDeviceCapture[64]  = {0,};
char curUsbAudioDevicePlayback[64] = {0,};
#endif

//default parameters
typedef struct device_parameter {
    unsigned int verbose;
    char loopback[16];
    char device[64];
    char capture[64];
    char playback[64];
    char rate[16];
    char period[16];
    char nperiods[16];
    char capture_nch[8];
    char playback_nch[8];
} device_parameter_t;

SERVER_EXPORT int
jackctl_wait_signals_and_return(jackctl_sigmask_t * sigmask)
{
    int sig;
    bool waiting = true;

    while (waiting) {
    #if defined(sun) && !defined(__sun__) // SUN compiler only, to check
        sigwait(&sigmask->signals);
    #else
        sigwait(&sigmask->signals, &sig);
    #endif
        fprintf(stderr, "Jack main caught signal %d\n", sig);

        switch (sig) {
            case SIGUSR1:
                //jack_dump_configuration(engine, 1);
                break;
            case SIGUSR2:
                // driver exit
                waiting = false;
                break;
            case SIGTTOU:
                break;
            default:
                waiting = false;
                break;
        }
    }

    if (sig != SIGSEGV) {
        // unblock signals so we can see them during shutdown.
        // this will help prod developers not to lose sight of
        // bugs that cause segfaults etc. during shutdown.
        sigprocmask(SIG_UNBLOCK, &sigmask->signals, 0);
    }

    return sig;
}

SERVER_EXPORT bool
jackctl_driver_create_backup_parameters(jackctl_driver_t * driver_ctr)
{
    JSList* node_ptr;
    jackctl_parameter_t * param = NULL;

    if (backup_ptr != NULL) {
        jack_error("driver backup parameters are already stored.");
        return false;
    }

    const JSList* driver_params = jackctl_driver_get_parameters(driver_ctr);
    if (driver_params == NULL) {
        jack_error("driver parameters are null.");
        return false;
    }

    node_ptr = (JSList *)driver_params;
    while (node_ptr) {
        param = (jackctl_parameter_t*)node_ptr->data;
        if(jackctl_parameter_is_set(param)) {
            backup_param_t * backup = (backup_param_t *)malloc(sizeof(backup_param_t));
            backup->id = jackctl_parameter_get_id(param);
            jackctl_parameter_value temp = jackctl_parameter_get_value(param);
            memcpy(&backup->value, &temp, sizeof(jackctl_parameter_value));
            backup_ptr = jack_slist_append(backup_ptr, backup);  //backup
        }
        node_ptr = node_ptr->next;
    }

    return true;
}

SERVER_EXPORT bool
jackctl_driver_destroy_backup_parameters(void)
{
    JSList* node_ptr = backup_ptr;

    if (node_ptr == NULL) {
        jack_error("driver backup parameters are already destroyed.");
        return false;
    }

    while (node_ptr) {
        backup_param_t * backup = (backup_param_t *)node_ptr->data;
        node_ptr = jack_slist_remove(node_ptr, backup);
        if (backup != NULL) {
            free(backup);
        }
    }

    backup_ptr = NULL;

#ifdef SUPPORT_JACK_USBAUDIO
    unlink(jack_dev_dir "/dev.cmd");
#endif

    return true;
}

SERVER_EXPORT bool
jackctl_driver_restore_parameters(jackctl_driver_t * driver_ctr)
{
    JSList* node_ptr;
    JSList* backup_node_ptr;
    jackctl_parameter_t * param = NULL;
    union jackctl_parameter_value value;

    if (backup_ptr == NULL) {
        jack_error("driver backup parameters are null, can't restore them.");
        return false;
    }

    const JSList* driver_params = jackctl_driver_get_parameters(driver_ctr);
    if (driver_params == NULL) {
        jack_error("driver parameters are null.");
        return false;
    }

    node_ptr = (JSList *)driver_params;
    while (node_ptr) {
        param = (jackctl_parameter_t*)node_ptr->data;
        char id = jackctl_parameter_get_id(param);
        value = jackctl_parameter_get_value(param);

        //search id
        backup_node_ptr = backup_ptr;
        while (backup_node_ptr) {
            backup_param_t * backup = (backup_param_t *)backup_node_ptr->data;
            if (id == backup->id) {
            // update latency from "apa_settings.cfg"
                if(id == 'p') {
                    int usb;
                    int latency;
                    std::ifstream in("/data/misc/jack/apa_settings.cfg");
                    in >> usb;
                    in >> latency;
                    switch(latency) {
                        case 0:
                            value.ui = 480;
                            break;
                        case 1:
                            value.ui = 960;
                            break;
                        case 2:
                            value.ui = 240;
                            break;
                    }

                    //decision of sync or async mode (depends on platform)
#if defined (MID_ASYNC_MODE)					
                    if(value.ui <= 480) {
#else
                    if(value.ui <= 240) {
#endif					
                        jack_log("use async mode when use %d period", value.ui);
                        if(GetEngineControl()->fSyncMode == true)
                            GetEngineControl()->UpdateSyncModePending(false);
                    } else {
                        jack_log("use sync mode when use %d period", value.ui);
                        if(GetEngineControl()->fSyncMode == false)
                            GetEngineControl()->UpdateSyncModePending(true);
                    }
                } else {
                    memcpy(&value, &backup->value, sizeof(jackctl_parameter_value));
                }
                jackctl_parameter_set_value(param, &value);  //restore
            }
            backup_node_ptr = backup_node_ptr->next;
        }
        node_ptr = node_ptr->next;
    }

    return true;
}

SERVER_EXPORT bool
jackctl_driver_update_parameters(char * master_driver_name, jackctl_driver_t * driver_ctr)
{
    JSList* node_ptr;
    jackctl_parameter_t * param = NULL;
    union jackctl_parameter_value value;
    device_parameter_t *dev_param = NULL;

    if(strcmp(master_driver_name, "alsa")) {  //currently, only accept alsa master driver
        jack_error("accept only alsa driver.");
        return false;
    }

    //initialize parameters
    dev_param = (device_parameter_t *)malloc(sizeof(device_parameter_t));
    if(dev_param == NULL) {
        jack_error("parameters allocation failed.");
        return false;
    }
    memset(dev_param, 0, sizeof(device_parameter_t));
    dev_param->verbose = 0;

#ifdef SUPPORT_JACK_USBAUDIO
    //detect usb device & fill parameters.
    memset(curUsbAudioDeviceCapture, 0, sizeof(curUsbAudioDeviceCapture));
    memset(curUsbAudioDevicePlayback, 0, sizeof(curUsbAudioDevicePlayback));
    FILE *f = fopen(jack_dev_dir "/dev.cmd", "r");
    if (f == 0) {
        jack_log("not found dev.cmd %d, %s", errno, strerror(errno));
        free(dev_param);
        return false;
    }
    for (;;) {
        char buf[64 + 8];
        if (fgets(buf, sizeof(buf), f) == NULL) {
            jack_log("end of command");
            break;
        } else {
            jack_log("%s", buf);
            if(buf[0] == 'C') {
                strncpy(curUsbAudioDeviceCapture, buf + 2, 63);  //example) C,2:48000:0:1:0:0:0:16
                curUsbAudioDeviceCapture[63] = '\0';
            }
            else if(buf[0] == 'P') {
                strncpy(curUsbAudioDevicePlayback, buf + 2, 63);  //example) P,2:48000:0:1:0:4:48000:40
                curUsbAudioDevicePlayback[63] = '\0';
            }
        }
    }
    fclose(f);

    //example) 2:48000:0:1:0:0:0:16
    //         2:48000:0:1:0:4:48000:40
    if(strlen(curUsbAudioDeviceCapture) > 0 || strlen(curUsbAudioDevicePlayback) > 0) {  //found!
        unsigned int channel = 0, samplerate2 = 0, samplerate6 = 0, card = 0, device = 0, format = 0, samplerate24 = 0, supportedformat = 0;
        if(strlen(curUsbAudioDeviceCapture) > 0) {
            sscanf(curUsbAudioDeviceCapture, "%d:%d:%d:%d:%d:%d:%d:%d", &channel, &samplerate2, &samplerate6, &card, &device, &format, &samplerate24, &supportedformat);
            sprintf(dev_param->capture, "hw:%d,%d", card, device);
            sprintf(dev_param->capture_nch, "%d", channel);
            sprintf(dev_param->rate, "%d", samplerate2);
        }

        channel =  samplerate2 = samplerate6 = card = device = format = samplerate24 = supportedformat = 0;
        if(strlen(curUsbAudioDevicePlayback) > 0) {
            sscanf(curUsbAudioDevicePlayback, "%d:%d:%d:%d:%d:%d:%d:%d", &channel, &samplerate2, &samplerate6, &card, &device, &format, &samplerate24, &supportedformat);
            sprintf(dev_param->playback, "hw:%d,%d", card, device);
            sprintf(dev_param->playback_nch, "%d", channel);
            sprintf(dev_param->rate, "%d", samplerate2);
        }
    } else {
        jack_error("not found any external devices.");
        free(dev_param);
        return false;
    }
#endif

    const JSList* driver_params = jackctl_driver_get_parameters(driver_ctr);
    if (driver_params == NULL) {
        jack_error("driver parameters are null.");
        free(dev_param);
        return false;
    }

    node_ptr = (JSList *)driver_params;
    while (node_ptr) {
        param = (jackctl_parameter_t*)node_ptr->data;
        char id = jackctl_parameter_get_id(param);
        value = jackctl_parameter_get_value(param);

        // TODO: update parameters here...
        switch (id) {
            case 'C':
                strcpy(value.str, dev_param->capture);
                jackctl_parameter_set_value(param, &value);
                break;
            case 'P':
                strcpy(value.str, dev_param->playback);
                jackctl_parameter_set_value(param, &value);
                break;
            case 'i':
                value.ui = atoi(dev_param->capture_nch);
                jackctl_parameter_set_value(param, &value);
                break;
            case 'o':
                value.ui = atoi(dev_param->playback_nch);
                jackctl_parameter_set_value(param, &value);
                break;
            case 'r':
                value.ui = atoi(dev_param->rate);
                jackctl_parameter_set_value(param, &value);
                break;
        }
        node_ptr = node_ptr->next;
    }

    free(dev_param);
    return true;
}

SERVER_EXPORT jackctl_driver_t * jackctl_server_get_driver(jackctl_server_t *server, const char *driver_name)
{
    const JSList * node_ptr = jackctl_server_get_drivers_list(server);

    while (node_ptr) {
        if (strcmp(jackctl_driver_get_name((jackctl_driver_t *)node_ptr->data), driver_name) == 0) {
            return (jackctl_driver_t *)node_ptr->data;
        }
        node_ptr = jack_slist_next(node_ptr);
    }

    return NULL;
}


