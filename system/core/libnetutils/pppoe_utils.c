/*
 * Copyright 2008, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Utilities for managing the dhcpcd DHCP client daemon */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define LOG_TAG "pppoe_utils"
#include <cutils/log.h>
#include <cutils/properties.h>

#include <netutils/pppoe.h>

//static const int MAX_LEN = 1024;
//static const int PPPD_UNIT_NUMBER = 3;
//static const char PPPOE_COMMAND_FORMAT[] = "/system/bin/pppoe -I %s -T %d -m %d -U";

static const char DAEMON_NAME[] = "pppoe";
static const char DAEMON_PROP_NAME[]   = "init.svc.pppoe";
static const char PPPOE_PROP_NAME_PREFIX[] = "pppoe";

static const char WIFI_INTERFACE_PROP_NAME[] = "wifi.interface";

//static const char INTERFACE_PROP_NAME[] = "net.pppoe.interface";
//static const char USERNAME_PROP_NAME[] = "net.pppoe.username";
//static const char PASSWORD_PROP_NAME[] = "net.pppoe.password";
//static const char LCP_ECHO_INTERVAL_PROP_NAME[] = "net.pppoe.lcp_echo_interval";
//static const char LCP_ECHO_FAILURE_PROP_NAME[] = "net.pppoe.lcp_echo_failure";
//static const char MTU_PROP_NAME[] = "net.pppoe.mtu";
//static const char MRU_PROP_NAME[] = "net.pppoe.mru";
//static const char TIMEOUT_PROP_NAME[] = "net.pppoe.timeout";
//static const char MSS_PROP_NAME[] = "net.pppoe.mss";

#ifdef SAMSUNG_OPTIMIZATION
static const int NAP_TIME = 100;
#else
static const int NAP_TIME = 200;   /* wait for 200ms at a time */
                                  /* when polling for property values */
#endif

/*
 * Wait for a system property to be assigned a specified value.
 * If desired_value is NULL, then just wait for the property to
 * be created with any value. maxwait is the maximum amount of
 * time in seconds to wait before giving up.
 */
static int wait_for_property(const char *name, const char *desired_value, int maxwait)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    int maxnaps = (maxwait * 1000) / NAP_TIME;

    if (maxnaps < 1) {
        maxnaps = 1;
    }

    while (maxnaps-- > 0) {
        usleep(NAP_TIME * 1000);
        if (property_get(name, value, NULL)) {
            if (desired_value == NULL ||
                    strcmp(value, desired_value) == 0) {
                return 0;
            }
        }
    }
    return -1; /* failure */
}

static void pppoe_properties_set(const char *interface_value,
		const char *username_value,
		const char *password_value,
		const char *interval_value,
		const char *failure_value,
		const char *mtu_value,
		const char *mru_value,
		const char *timeout_value,
		const char *mss_value ) {
	property_set(INTERFACE_PROP_NAME, interface_value);
	property_set(USERNAME_PROP_NAME, username_value);
	property_set(PASSWORD_PROP_NAME, password_value);
	property_set(LCP_ECHO_INTERVAL_PROP_NAME, interval_value);
	property_set(LCP_ECHO_FAILURE_PROP_NAME, failure_value);
	property_set(MTU_PROP_NAME, mtu_value);
	property_set(MRU_PROP_NAME, mru_value);
	property_set(TIMEOUT_PROP_NAME, timeout_value);
	property_set(MSS_PROP_NAME, mss_value);
}

/*
 * Start the pppoe client daemon
 *
 */
int pppoe_start(pppoe_config config)
{
	ALOGD("interface:%s, username:%s, password:%s,"
		"interval:%d, failure:%d, mtu:%d, mru:%d, timeout:%d, mss:%d",
		config.interface, config.username, config.password, config.lcp_echo_interval, config.lcp_echo_failure,
		config.mtu, config.mru, config.timeout, config.mss);
	char interval_value[PROPERTY_VALUE_MAX];
	char failure_value[PROPERTY_VALUE_MAX];
	char mtu_value[PROPERTY_VALUE_MAX];
	char mru_value[PROPERTY_VALUE_MAX];
	char timeout_value[PROPERTY_VALUE_MAX];
	char mss_value[PROPERTY_VALUE_MAX];

	sprintf(interval_value, "%d", config.lcp_echo_interval);
	sprintf(failure_value, "%d", config.lcp_echo_failure);
	sprintf(mtu_value, "%d", config.mtu);
	sprintf(mru_value, "%d", config.mru);
	sprintf(timeout_value, "%d", config.timeout);
	sprintf(mss_value, "%d", config.mss);

	pppoe_properties_set(config.interface, config.username, config.password,
			interval_value, failure_value, mtu_value, mru_value, timeout_value, mss_value);
	const char *ctrl_prop = "ctl.start";
	const char *daemon_desired_status = "running";

	property_set(PPPOE_STATE_PROP_NAME, "");
	property_set(ctrl_prop, DAEMON_NAME);

	if (wait_for_property(DAEMON_PROP_NAME, daemon_desired_status, 5) < 0)
		return -1;

	return 0;
	/*
	//TODO: timeout
	if (wait_for_property(PPPOE_STATE_PROP_NAME, NULL, 20) < 0) {
		ALOGD("timeout wait for pppoe state");
		return -1;
	}
	if (!property_get(PPPOE_STATE_PROP_NAME, state_value, NULL)) {
		ALOGD("property not set");
		return -1;
	}

	if (strcmp(state_value, "online") == 0) {
		ALOGD("success");
		return 0;
	} else {
		// TODO: error code
		ALOGD("failed");
		return -1;
	}
	*/
}

/**
 * Stop the PPPOE client daemon.
 */
int pppoe_stop()
{
	ALOGD("pppoe_stop");
	const char *ctrl_prop = "ctl.stop";
	const char *desired_status = "stopped";
	/*
	char wifi_interface[PROPERTY_VALUE_MAX];
    char result_prop_name[PROPERTY_KEY_MAX];

	property_get(WIFI_INTERFACE_PROP_NAME, wifi_interface, NULL);

    snprintf(result_prop_name, sizeof(result_prop_name), "%s.%s.result",
            PPPOE_PROP_NAME_PREFIX,
			wifi_interface);
	*/

    /* Stop the daemon and wait until it's reported to be stopped */
	pppoe_properties_set(NULL, NULL, NULL, NULL,
			NULL, NULL, NULL, NULL, NULL);
    property_set(ctrl_prop, DAEMON_NAME);
	if (wait_for_property(DAEMON_PROP_NAME, desired_status, 5) < 0)
		return -1;

	//property_set(result_prop_name, "failed");
	// wait for the pppd is really killed.
	sleep(1);
	return 0;
}

