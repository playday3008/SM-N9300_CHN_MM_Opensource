/*
  Copyright (C) 2001 Paul Davis
  Copyright (C) 2004 Jack O'Quin

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

#ifndef __jackcontrol_h__
#define __jackcontrol_h__

#ifdef __cplusplus
extern "C"
{
#endif

#include <jack/jack.h>

/**
 * Open an external control client session with a JACK server.  This interface
 * is more complex but more powerful than jack_client_new().  With it,
 * clients may choose which of several servers to connect, and control
 * whether and how to start the server automatically, if it was not
 * already running.  There is also an option for JACK to generate a
 * unique client name, when necessary.
 *
 * @param client_name of at most jack_client_name_size() characters.
 * The name scope is local to each server.  Unless forbidden by the
 * @ref JackUseExactName option, the server will modify this name to
 * create a unique variant, if needed.
 *
 * @param options formed by OR-ing together @ref JackOptions bits.
 * Only the @ref JackOpenOptions bits are allowed.
 *
 * @param status (if non-NULL) an address for JACK to return
 * information from the open operation.  This status word is formed by
 * OR-ing together the relevant @ref JackStatus bits.
 *
 *
 * <b>Optional parameters:</b> depending on corresponding [@a options
 * bits] additional parameters may follow @a status (in this order).
 *
 * @arg [@ref JackServerName] <em>(char *) server_name</em> selects
 * from among several possible concurrent server instances.  Server
 * names are unique to each user.  If unspecified, use "default"
 * unless \$JACK_DEFAULT_SERVER is defined in the process environment.
 *
 * @return Opaque client handle if successful.  If this is NULL, the
 * open operation failed, @a *status includes @ref JackFailure and the
 * caller is not a JACK client.
 */
jack_client_t * jack_control_client_open (const char *client_name,
                                  jack_options_t options,
                                  jack_status_t *status, ...) JACK_OPTIONAL_WEAK_EXPORT;

#if defined (__ANDROID__)

/**
 * Stop Jackd via the control client
 * @param ext_client : external control client
 * @param param : reserved
 */
int jack_control_client_stop_jackd(jack_client_t* ext_client, int param) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Switch the master driver of Jackd
 * @param ext_client : external control client
 * @param type : the type to be switched. [ alsa, dummy]
 * @param param : reserved
 */
int jack_control_client_switch_master(jack_client_t* ext_client, int type, int param) JACK_OPTIONAL_WEAK_EXPORT;
#endif

#if defined (__ANDROID__) && defined (ENABLE_JACK_LOGGER)

/**
 * Start Jackd dump
 * @param ext_client : external control client
 * @param type : dump type [ pcm ]
 * @param param : reserved
 */
int jack_control_client_start_dump(jack_client_t* ext_client, int type, int param) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Stop Jackd dump
 * @param ext_client : external control client
 * @param param : reserved
 */
int jack_control_client_stop_dump(jack_client_t* ext_client, int param) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Dump the current status of Jackd
 * @param ext_client : external control client
 * @param param : reserved
 */
int jack_control_client_dump(jack_client_t* ext_client, int param) JACK_OPTIONAL_WEAK_EXPORT;

#endif



#ifdef __cplusplus
}
#endif

#endif /* __jackcontrol_h__ */

