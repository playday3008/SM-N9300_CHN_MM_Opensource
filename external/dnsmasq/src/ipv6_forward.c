#include "dnsmasq.h"

#ifdef HAVE_DHCP6

#define DEBUG 1
#define BLOCK_PACKET 1

extern int run_ip_cmd(char*);
static char *IP6TABLES = "system/bin/ip6tables";
static int nat_count = 0;
static struct in6_addr zero_prefix;

static int del_last_colon_of_prefix(char* prefix)
{
    char *cmd = NULL;
    char *next_cmd = NULL;
    int addr_count = 0;
    char change_prefix[256] = {'\0',};
    strlcpy(change_prefix, prefix, 256);

    next_cmd = change_prefix;

    while ((cmd = strsep(&next_cmd,":"))) {
        if(cmd[0] != 0 ) {
            addr_count++;
        }
    }

    if (addr_count == 4) {
        return 1;
    }

    return 0;
}

static int set_block_rule(char *int_interface, char *ext_interface, int add)
{
    char *cmd;

    if (add) {
        asprintf(&cmd, "%s -%s natctrl_FORWARD -i %s -j DROP", IP6TABLES, (!add ? "A" : "D"), int_interface);
        if (run_ip_cmd(cmd) < 0) {
            my_syslog(MS_DHCP | LOG_INFO, "failed to run_ip_cmd");
            return -1;
        }
#if DEBUG
        my_syslog(MS_DHCP | LOG_INFO, "%s", cmd);
#endif
    }

    asprintf(&cmd, "%s -%s natctrl_FORWARD -i %s -j DROP", IP6TABLES, (add ? "A" : "D"), int_interface);
    if (run_ip_cmd(cmd) < 0) {
        my_syslog(MS_DHCP | LOG_INFO, "failed to run_ip_cmd");
        return -1;
    }
#if DEBUG
        my_syslog(MS_DHCP | LOG_INFO, "%s", cmd);
#endif

    return 1;
}

static int set_forward_from_ext_to_int(struct in6_addr prefix, char *int_interface, char *ext_interface, int add)
{
    char *cmd;

    //flush rule of interface in add
    if (add) {
        asprintf(&cmd, "%s -%s natctrl_FORWARD -i %s -o %s -j RETURN",
                IP6TABLES, (!add ? "A" : "D"), ext_interface, int_interface);
        if ((run_ip_cmd(cmd) < 0) && add ) {
            my_syslog(MS_DHCP | LOG_INFO, "failed to run_ip_cmd");
            return -1;
        }
#if DEBUG
        my_syslog(MS_DHCP | LOG_INFO, "%s", cmd);
#endif
    }

    asprintf(&cmd, "%s -%s natctrl_FORWARD -i %s -o %s -j RETURN",
            IP6TABLES, (add ? "A" : "D"), ext_interface, int_interface);
    if ((run_ip_cmd(cmd) < 0) && add ) {
        my_syslog(MS_DHCP | LOG_INFO, "failed to run_ip_cmd");
        return -1;
    }

#if DEBUG
    my_syslog(MS_DHCP | LOG_INFO, "%s", cmd);
#endif

    return 1;
}

static int set_forward_from_int_to_ext(struct in6_addr prefix, char *int_interface, char *ext_interface, int add)
{
    char *cmd;
    char active_prefix[256] = {'\0',};
    int del_last_colon = 0;

    inet_ntop(AF_INET6, &prefix, active_prefix, ADDRSTRLEN);
    if ((del_last_colon = del_last_colon_of_prefix(active_prefix))) {
        active_prefix[strlen(active_prefix)-1] = 0;
    }

    if (add) {
        if (del_last_colon) {
            asprintf(&cmd, "%s -%s natctrl_FORWARD -i %s -o %s -m iprange --src-range %s:-%sffff:ffff:ffff:ffff -j RETURN",
                IP6TABLES, (!add ? "A" : "D" ), int_interface, ext_interface, active_prefix, active_prefix);
        } else {
            asprintf(&cmd, "%s -%s natctrl_FORWARD -i %s -o %s -m iprange --src-range %s-%sffff:ffff:ffff:ffff -j RETURN",
                IP6TABLES, (!add ? "A" : "D" ), int_interface, ext_interface, active_prefix, active_prefix);
        }

        if ((run_ip_cmd(cmd) < 0) && add ) {
            my_syslog(MS_DHCP | LOG_INFO, "failed to run_ip_cmd");
            return -1;
        }
#if DEBUG
       my_syslog(MS_DHCP | LOG_INFO, "%s", cmd);
#endif
    }

    if (del_last_colon) {
        asprintf(&cmd, "%s -%s natctrl_FORWARD -i %s -o %s -m iprange --src-range %s:-%sffff:ffff:ffff:ffff -j RETURN",
            IP6TABLES, (add ? "A" : "D" ), int_interface, ext_interface, active_prefix, active_prefix);
    } else {
        asprintf(&cmd, "%s -%s natctrl_FORWARD -i %s -o %s -m iprange --src-range %s-%sffff:ffff:ffff:ffff -j RETURN",
            IP6TABLES, (add ? "A" : "D" ), int_interface, ext_interface, active_prefix, active_prefix);
    }
    if ((run_ip_cmd(cmd) < 0) && add ) {
        my_syslog(MS_DHCP | LOG_INFO, "failed to run_ip_cmd");
        asprintf(&cmd, "%s -%s natctrl_FORWARD -i %s -o %s -j RETURN",
                IP6TABLES, (!add ? "A" : "D"), ext_interface, int_interface);
        run_ip_cmd(cmd);
        return -1;
    }

#if DEBUG
    my_syslog(MS_DHCP | LOG_INFO, "%s", cmd);
#endif

    return 1;
}

static int set_tcpmss(int add)
{
    char *cmd;
    //flush rule of interface in add
    if (add) {
        asprintf(&cmd, "%s -%s natctrl_FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu",
                 IP6TABLES, (!add ? "A" : "D"));
#if DEBUG
        my_syslog(MS_DHCP | LOG_INFO, "%s", cmd);
#endif
        if ((run_ip_cmd(cmd) < 0) && add ) {
            my_syslog(MS_DHCP | LOG_INFO, "failed to run_ip_cmd");
            return -1;
        }
    }

    asprintf(&cmd, "%s -%s natctrl_FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu",
             IP6TABLES, (add ? "A" : "D"));

#if DEBUG
    my_syslog(MS_DHCP | LOG_INFO, "%s", cmd);
#endif

    if ((run_ip_cmd(cmd) < 0) && add ) {
        my_syslog(MS_DHCP | LOG_INFO, "failed to run_ip_cmd");
        return -1;
    }

    return 1;
}

int deprecate_old_prefix(struct in6_addr old_prefix, char *int_interface, char *ext_interface)
{

    if (memcmp(&old_prefix, &zero_prefix, sizeof(struct in6_addr)) == 0) {
        return -1;
    }

    set_tcpmss(0);
    set_forward_from_ext_to_int(old_prefix, int_interface, ext_interface, 0);
    set_forward_from_int_to_ext(old_prefix, int_interface, ext_interface, 0);
#if BLOCK_PACKET
    set_block_rule(int_interface, ext_interface, 0);
#endif

#if DEBUG
    my_syslog(MS_DHCP | LOG_INFO, "%s", __FUNCTION__);
    print_ipv6_address("deprecate prefix", &old_prefix);
#endif
    return 1;
}

int active_new_prefix(struct in6_addr new_prefix, char *int_interface, char *ext_interface)
{
    set_tcpmss(1);
    set_forward_from_ext_to_int(new_prefix, int_interface, ext_interface, 1);
    set_forward_from_int_to_ext(new_prefix, int_interface, ext_interface, 1);
#if BLOCK_PACKET
    set_block_rule(int_interface, ext_interface, 1);
#endif

#if DEBUG
    my_syslog(MS_DHCP | LOG_INFO, "%s", __FUNCTION__);
    print_ipv6_address("active prefix", &new_prefix);
#endif
    return 1;
}

#endif
