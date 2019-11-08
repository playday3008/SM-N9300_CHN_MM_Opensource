/*
Copyright (C) 2001-2003 Paul Davis
Copyright (C) 2004-2008 Grame

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

#include "JackDebugClient.h"
#include "JackLibClient.h"
#include "JackControlLibClient.h"
#include "JackChannel.h"
#include "JackLibGlobals.h"
#include "JackGlobals.h"
#include "JackCompilerDeps.h"
#include "JackTools.h"
#include "JackSystemDeps.h"
#include "JackServerLaunch.h"
#include <assert.h>

using namespace Jack;

#ifdef __cplusplus
extern "C"
{
#endif

#if defined (__ANDROID__)
	__attribute__((constructor)) static void _initLib(){jack_log("libjack init"); }; 
	__attribute__((destructor)) static void _finiLib(){jack_log("libjack fini"); Jack::JackLibGlobals::Cleanup();};
#endif


    jack_client_t * jack_client_new_aux (const char *client_name,
            jack_options_t options,
            jack_status_t *status);

    LIB_EXPORT jack_client_t * jack_client_open (const char *client_name,
            jack_options_t options,
            jack_status_t *status, ...);
    LIB_EXPORT int jack_client_close (jack_client_t *client);
    LIB_EXPORT int jack_get_client_pid (const char *name);

	LIB_EXPORT jack_client_t * jack_control_client_open (const char *client_name,
		jack_options_t options,
		jack_status_t *status, ...);

#if defined (__ANDROID__)
	LIB_EXPORT int jack_control_client_stop_jackd(jack_client_t* ext_client, int param);
	LIB_EXPORT int jack_control_client_switch_master(jack_client_t* ext_client, int type, int param);
#endif
#if defined (__ANDROID__) && defined (ENABLE_JACK_LOGGER)
	LIB_EXPORT int jack_control_client_start_dump(jack_client_t* ext_client, int type, int param);
	LIB_EXPORT int jack_control_client_stop_dump(jack_client_t* ext_client, int param);
	LIB_EXPORT int jack_control_client_dump(jack_client_t* ext_client, int param);
#endif

#ifdef __cplusplus
}
#endif

static jack_client_t * jack_client_open_aux (const char *client_name,
            jack_options_t options,
            jack_status_t *status, va_list ap);

JackLibGlobals* JackLibGlobals::fGlobals = NULL;
int JackLibGlobals::fClientCount = 0;

jack_client_t* jack_client_new_aux(const char* client_name, jack_options_t options, jack_status_t* status)
{
    jack_varargs_t va;          /* variable arguments */
    jack_status_t my_status;
    JackClient* client;

    if (client_name == NULL) {
        jack_error("jack_client_new called with a NULL client_name");
        return NULL;
    }

    jack_log("jack_client_new %s", client_name);

    if (status == NULL)         /* no status from caller? */
        status = &my_status;    /* use local status word */
    *status = (jack_status_t)0;

    /* validate parameters */
    if ((options & ~JackOpenOptions)) {
        int my_status1 = *status | (JackFailure | JackInvalidOption);
        *status = (jack_status_t)my_status1;
        return NULL;
    }

    /* parse variable arguments */
    jack_varargs_init(&va);

    JackLibGlobals::Init(); // jack library initialisation

    if (try_start_server(&va, options, status)) {
        jack_error("jack server is not running or cannot be started");
        JackLibGlobals::Destroy(); // jack library destruction
        return 0;
    }

    if (JACK_DEBUG) {
        client = new JackDebugClient(new JackLibClient(GetSynchroTable())); // Debug mode
    } else {
        client = new JackLibClient(GetSynchroTable());
    }

    int res = client->Open(va.server_name, client_name, va.session_id, options, status);
    if (res < 0) {
        delete client;
        JackLibGlobals::Destroy(); // jack library destruction
        int my_status1 = (JackFailure | JackServerError);
        *status = (jack_status_t)my_status1;
        return NULL;
    } else {
        return (jack_client_t*)client;
    }
}

static jack_client_t* jack_client_open_aux(const char* client_name, jack_options_t options, jack_status_t* status, va_list ap)
{
    jack_varargs_t va;		/* variable arguments */
    jack_status_t my_status;
    JackClient* client;

    if (client_name == NULL) {
        jack_error("jack_client_open called with a NULL client_name");
        return NULL;
    }

    jack_log("jack_client_open %s", client_name);

    if (status == NULL)			/* no status from caller? */
        status = &my_status;	/* use local status word */
    *status = (jack_status_t)0;

    /* validate parameters */
    if ((options & ~JackOpenOptions)) {
        int my_status1 = *status | (JackFailure | JackInvalidOption);
        *status = (jack_status_t)my_status1;
        return NULL;
    }

    /* parse variable arguments */
    jack_varargs_parse(options, ap, &va);

    JackLibGlobals::Init(); // jack library initialisation

    if (try_start_server(&va, options, status)) {
        jack_error("jack server is not running or cannot be started");
        JackLibGlobals::Destroy(); // jack library destruction
        return 0;
    }

    if (JACK_DEBUG) {
        client = new JackDebugClient(new JackLibClient(GetSynchroTable())); // Debug mode
    } else {
        client = new JackLibClient(GetSynchroTable());
    }

    int res = client->Open(va.server_name, client_name, va.session_id, options, status);
    if (res < 0) {
        delete client;
        JackLibGlobals::Destroy(); // jack library destruction
        int my_status1 = (JackFailure | JackServerError);
        *status = (jack_status_t)my_status1;
        return NULL;
    } else {
        return (jack_client_t*)client;
    }
}

static jack_client_t* jack_control_client_open_aux(const char* client_name, jack_options_t options, jack_status_t* status, va_list ap)
{
    jack_varargs_t va;		/* variable arguments */
    jack_status_t my_status;
    JackClient* client;

    if (client_name == NULL) {
        jack_error("jack_control_client_open called with a NULL client_name");
        return NULL;
    }

    jack_log("jack_control_client_open %s", client_name);

    if (status == NULL)			/* no status from caller? */
        status = &my_status;	/* use local status word */
    *status = (jack_status_t)0;

    /* validate parameters */
    if ((options & ~JackOpenOptions)) {
        int my_status1 = *status | (JackFailure | JackInvalidOption);
        *status = (jack_status_t)my_status1;
        return NULL;
    }

    /* parse variable arguments */
    jack_varargs_parse(options, ap, &va);

    JackLibGlobals::Init(); // jack library initialisation

    if (try_start_server(&va, options, status)) {
        jack_error("jack server is not running or cannot be started");
        JackLibGlobals::Destroy(); // jack library destruction
        return 0;
    }

    if (JACK_DEBUG) {
        client = new JackDebugClient(new JackControlLibClient(GetSynchroTable())); // Debug mode
    } else {
        client = new JackControlLibClient(GetSynchroTable());
    }

    int res = client->Open(va.server_name, client_name, va.session_id, options, status);
    if (res < 0) {
        delete client;
        JackLibGlobals::Destroy(); // jack library destruction
        int my_status1 = (JackFailure | JackServerError);
        *status = (jack_status_t)my_status1;
        return NULL;
    } else {
        return (jack_client_t*)client;
    }
}


LIB_EXPORT jack_client_t* jack_client_open(const char* ext_client_name, jack_options_t options, jack_status_t* status, ...)
{
    JackGlobals::CheckContext("jack_client_open");

    try {
        assert(JackGlobals::fOpenMutex);
        JackGlobals::fOpenMutex->Lock();
        va_list ap;
        va_start(ap, status);
        jack_client_t* res = jack_client_open_aux(ext_client_name, options, status, ap);
        va_end(ap);
        JackGlobals::fOpenMutex->Unlock();
        return res;
    } catch (std::bad_alloc& e) {
        JackGlobals::fOpenMutex->Unlock();
        jack_error("Memory allocation error...");
        return NULL;
    } catch (...) {
        JackGlobals::fOpenMutex->Unlock();
        jack_error("Unknown error...");
        return NULL;
    }
}

LIB_EXPORT jack_client_t* jack_control_client_open(const char* ext_client_name, jack_options_t options, jack_status_t* status, ...)
{
    JackGlobals::CheckContext("jack_control_client_open");

    try {
        assert(JackGlobals::fOpenMutex);
        JackGlobals::fOpenMutex->Lock();
        va_list ap;
        va_start(ap, status);
        jack_client_t* res = jack_control_client_open_aux(ext_client_name, options, status, ap);
        va_end(ap);
        JackGlobals::fOpenMutex->Unlock();
        return res;
    } catch (std::bad_alloc& e) {
        JackGlobals::fOpenMutex->Unlock();
        jack_error("Memory allocation error...");
        return NULL;
    } catch (...) {
        JackGlobals::fOpenMutex->Unlock();
        jack_error("Unknown error...");
        return NULL;
    }
}

#if defined (__ANDROID__)
LIB_EXPORT int jack_control_client_stop_jackd(jack_client_t* ext_client, int param)
{
    JackGlobals::CheckContext("jack_control_stop_jackd");

	assert(JackGlobals::fOpenMutex);
	JackGlobals::fOpenMutex->Lock();
	int res = -1;
	jack_log("jack_control_stop_jackd");
	JackControlLibClient* client = (JackControlLibClient*)ext_client;
	if (client == NULL) {
		jack_error("jack_control_stop_jackd called with a NULL client");
	} else {
		res = client->StopJackd(param);
		jack_log("jack_control_stop_jackd res = %d", res);
	}
	JackGlobals::fOpenMutex->Unlock();
	return res;
}

LIB_EXPORT int jack_control_client_switch_master(jack_client_t* ext_client, int type, int param)
{
    JackGlobals::CheckContext("jack_control_switch_master");

	assert(JackGlobals::fOpenMutex);
	JackGlobals::fOpenMutex->Lock();
	int res = -1;
	jack_log("jack_control_switch_master");
	JackControlLibClient* client = (JackControlLibClient*)ext_client;
	if (client == NULL) {
		jack_error("jack_control_switch_master called with a NULL client");
	} else {
		res = client->SwitchMaster(type, param);
		jack_log("jack_control_switch_master res = %d", res);
	}
	JackGlobals::fOpenMutex->Unlock();
	return res;
}
#endif

#if defined (__ANDROID__) && defined (ENABLE_JACK_LOGGER)
LIB_EXPORT int jack_control_client_dump(jack_client_t* ext_client, int param)
{
    JackGlobals::CheckContext("jack_control_dump");

	assert(JackGlobals::fOpenMutex);
	JackGlobals::fOpenMutex->Lock();
	int res = -1;
	jack_log("jack_control_dump");
	JackControlLibClient* client = (JackControlLibClient*)ext_client;
	if (client == NULL) {
		jack_error("jack_control_dump called with a NULL client");
	} else {
		res = client->Dump(param);
		jack_log("jack_control_dump res = %d", res);
	}
	JackGlobals::fOpenMutex->Unlock();
	return res;
}

LIB_EXPORT int jack_control_client_start_dump(jack_client_t* ext_client, int type, int param)
{
    JackGlobals::CheckContext("jack_control_start_dump");

	assert(JackGlobals::fOpenMutex);
	JackGlobals::fOpenMutex->Lock();
	int res = -1;
	jack_log("jack_control_start_dump");
	JackControlLibClient* client = (JackControlLibClient*)ext_client;
	if (client == NULL) {
		jack_error("jack_control_start_dump called with a NULL client");
	} else {
		res = client->StartDump(type, param);
		jack_log("jack_control_start_dump res = %d", res);
	}
	JackGlobals::fOpenMutex->Unlock();
	return res;
}

LIB_EXPORT int jack_control_client_stop_dump(jack_client_t* ext_client, int param)
{
    JackGlobals::CheckContext("jack_control_stop_dump");

	assert(JackGlobals::fOpenMutex);
	JackGlobals::fOpenMutex->Lock();
	int res = -1;
	jack_log("jack_control_stop_dump");
	JackControlLibClient* client = (JackControlLibClient*)ext_client;
	if (client == NULL) {
		jack_error("jack_control_stop_dump called with a NULL client");
	} else {
		res = client->StopDump(param);
		jack_log("jack_control_stop_dump res = %d", res);
	}
	JackGlobals::fOpenMutex->Unlock();
	return res;
}
#endif

LIB_EXPORT int jack_client_close(jack_client_t* ext_client)
{
    JackGlobals::CheckContext("jack_client_close");

    assert(JackGlobals::fOpenMutex);
    JackGlobals::fOpenMutex->Lock();
    int res = -1;
    jack_log("jack_client_close");
    JackClient* client = (JackClient*)ext_client;
    if (client == NULL) {
        jack_error("jack_client_close called with a NULL client");
    } else {
        res = client->Close();
        delete client;
        JackLibGlobals::Destroy(); // jack library destruction
        jack_log("jack_client_close res = %d", res);
    }
    JackGlobals::fOpenMutex->Unlock();
    return res;
}

LIB_EXPORT int jack_get_client_pid(const char *name)
{
    jack_error("jack_get_client_pid : not implemented on library side");
    return 0;
}

