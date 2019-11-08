#include "dnsmasq.h"

#ifdef HAVE_DHCP6
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/netlink.h>
#include <linux/route.h>
#include <linux/ipv6_route.h>
#include <linux/rtnetlink.h>
#include <linux/sockios.h>
#include <netinet/icmp6.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>
#include <netutils/ifc.h>

#define MAX_LINKHEADER_SIZE	256
#define BIGGER_THAN_ALL_MTUS	(64*1024)
#define GLOBAL_ADDRESS_INDEX 62
#define ICMPV6_TYPE_INDEX 54
#define TIMEOUT_VALUE 10 // 10 sec
#define DEBUG 1

struct route_list {
    char interface[IF_NAMESIZE+1];
    struct in6_addr dst;
    int active;
    struct route_list *next;
};

static struct route_list* g_active_route = NULL;
static pthread_t t_id = 0;
static int g_if_index = 0;
static int route_count = 0;
static struct timeval g_start_time;
static int g_table_num = -1;

static int update_start_time();
static int modify_from_rule(int table_num, char *action, char *addr);
extern int ifc_act_on_ipv6_route(int action, const char *ifname,
				 struct in6_addr dst, int prefix_length,
				 struct in6_addr gw);

void set_table_number(int table_num)
{
    g_table_num = table_num;
    my_syslog(MS_DHCP | LOG_INFO, "set table number:%d", g_table_num);
}

static int add_route_to_active_lists(struct in6_addr *dst, char *interface)
{
    struct route_list *route;
    struct route_list *new;
    struct sockaddr_in6 gw; 
    char dst_addr[256] = {'\0',};
    memset(&gw, 0, sizeof(struct sockaddr_in6)); // set :: to gw
    for (route = g_active_route; route; route = route->next) {
        if ((memcmp(dst, &route->dst, sizeof(struct in6_addr)) == 0) &&
            (strcmp(interface, route->interface) == 0 ) &&
            (route->active == 1)) {
#if DEBUG
            print_ipv6_address("already exists", &route->dst);
#endif
            return -1;
        }
    }

    if (ifc_act_on_ipv6_route(SIOCADDRT, interface ,*dst, 128, gw.sin6_addr) < 0) {
        my_syslog(MS_DHCP | LOG_INFO, "failed to configure ipv6 routing table");
        return -1;
    }

    inet_ntop(AF_INET6, dst, dst_addr, ADDRSTRLEN); 
    strcat(dst_addr,"/128");
    if (g_table_num != -1 ) {

        if (modify_from_rule(g_table_num, "add", dst_addr) < 0) {
            my_syslog(MS_DHCP | LOG_INFO, "failed to configure ip6 rule");

            if (ifc_act_on_ipv6_route(SIOCADDRT, interface, *dst, 128, gw.sin6_addr) < 0) {
                my_syslog(MS_DHCP | LOG_INFO,"failed to delete routing table");
            }
            return -1;
        }
    }
    modify_from_route(97, "add", dst_addr,interface);
    new = safe_malloc(sizeof(struct route_list));   
    memset(new, 0, sizeof(struct route_list));
    memcpy(&new->dst, dst, sizeof(struct in6_addr));
    strncpy(new->interface, interface, strlen(interface)+1);
    new->active = 1;
    route_count++;

#if DEBUG
    my_syslog(MS_DHCP | LOG_INFO, "%s count:%d inf:%s", __FUNCTION__, route_count, interface);
    print_ipv6_address("add route", &new->dst);
#endif

    route = new;
    route->next = g_active_route;
    g_active_route = route;

    return 1;
}

void flush_active_lists()
{
    struct route_list *route;
    struct route_list *del;

    for (route = g_active_route; route;) {
        if ( route->active == 0 ) {
            route_count--;
#if DEBUG
            my_syslog(MS_DHCP | LOG_INFO, "%s count:%d inf:%s", __FUNCTION__, route_count, route->interface);
            print_ipv6_address("flush route", &route->dst);
#endif
            del = route;
            route = route->next;
            free(del);
        } else {
            route = route->next;
        }
    }

    g_active_route = NULL;
}

void reactive_active_lists(struct in6_addr *dst, int prefix_len, char *interface)
{
    struct route_list *route;
    struct sockaddr_in6 gw; 
    char dst_addr[256] = {'\0',};
    memset(&gw,0,sizeof(struct sockaddr_in6)); // set :: to gw

    for (route = g_active_route; route; route = route->next) {
        if ((is_same_net6(dst, &route->dst, prefix_len)) &&
            (strcmp(interface, route->interface) == 0 ) &&
            (route->active == 0)) {
            if (ifc_act_on_ipv6_route(SIOCADDRT, route->interface, route->dst, 128, gw.sin6_addr) < 0) {
                my_syslog(MS_DHCP | LOG_INFO,"failed to delete routing table");
            }

            if (g_table_num != -1) {
                inet_ntop(AF_INET6, &route->dst, dst_addr, ADDRSTRLEN); 
                strcat(dst_addr, "/128");
                modify_from_rule(g_table_num, "add", dst_addr);
                modify_from_route(g_table_num, "add", dst_addr,interface);
            }

            route->active = 1;

#if DEBUG
            my_syslog(MS_DHCP | LOG_INFO, "%s int:%s", __FUNCTION__, route->interface);
            print_ipv6_address("readd route", &route->dst);
#endif
        }
    }
}

void delete_active_lists(char *interface)
{
    struct route_list *route;
    struct sockaddr_in6 gw; 
    char dst_addr[256] = {'\0',};
    memset(&gw,0,sizeof(struct sockaddr_in6)); // set :: to gw

    for (route = g_active_route; route; route = route->next) {
        if ((strcmp(interface, route->interface) == 0) && (route->active == 1) ) {
            if (ifc_act_on_ipv6_route(SIOCDELRT, route->interface, route->dst, 128, gw.sin6_addr) < 0) {
                my_syslog(MS_DHCP | LOG_INFO,"failed to delete routing table");
            }

            if (g_table_num != -1) {
                inet_ntop(AF_INET6, &route->dst, dst_addr, ADDRSTRLEN); 
                strcat(dst_addr, "/128");
                modify_from_rule(g_table_num, "del", dst_addr);
            }

            route->active = 0;
            modify_from_route(g_table_num, "del", dst_addr,interface);

#if DEBUG
            my_syslog(MS_DHCP | LOG_INFO, "%s inf:%s", __FUNCTION__, interface);
            print_ipv6_address("del route", &route->dst);
#endif
        }
    }
}

int nametoindex(int ifc_sock, const char *name, int *if_indexp)
{
    int r;
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, name, IF_NAMESIZE);
    ifr.ifr_name[IF_NAMESIZE - 1] = 0;

    r = ioctl(ifc_sock, SIOCGIFINDEX, &ifr);
    if(r < 0) return -1;

    *if_indexp = ifr.ifr_ifindex;
    return 1;
}


int bindtodevice_with_index(int sock_fd, int if_index)
{
    char interface[IF_NAMESIZE+1];
    if (!indextoname(sock_fd, if_index, interface)) {
        my_syslog(MS_DHCP | LOG_INFO,"failed to convert if index to if name\n");
        return -1;
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, interface, strlen(interface)+1);

    if (setsockopt(sock_fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) == -1 &&
      errno != EPERM) {
        my_syslog(MS_DHCP| LOG_INFO, "failed to set SO_BINDTODEVICE on icmpv6 socket");
        return -1;
    }

    return 0;
}

static int set_promiscuous_mode_on(int fd, int ifindex)
{
   	struct packet_mreq mr;
    memset(&mr, 0, sizeof(mr));
    mr.mr_ifindex = ifindex;
    mr.mr_type    = PACKET_MR_PROMISC;
    if (setsockopt(fd, SOL_PACKET,
        PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        return -1;
    }

    return 0;
}

static int iface_get_mtu(int fd, const char *device)
{
	struct ifreq ifr;

	if (!device)
		return BIGGER_THAN_ALL_MTUS;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));

	if (ioctl(fd, SIOCGIFMTU, &ifr) == -1) {
		return -1;
	}

	return ifr.ifr_mtu;
}

int iface_bind(int fd, int ifindex)
{
	struct sockaddr_ll sll;
	int err;
	socklen_t errlen = sizeof(err);

	memset(&sll, 0, sizeof(sll));
	sll.sll_family		= PF_PACKET;
	sll.sll_ifindex		= ifindex;
	sll.sll_protocol	= htons(ETH_P_ALL);

	if (bind(fd, (struct sockaddr *) &sll, sizeof(sll)) == -1) {
		return -1;
	}

	/* Any pending errors, e.g., network is down? */
	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1) {
		return -1;
	}

	if (err > 0) {
		return -1;
	}

	return 1;
}

void print_ipv6_address(char *tag, struct in6_addr* addr)
{
    uint8_t	*s = (uint8_t*)addr;
    int index = 0;     
    my_syslog(MS_DHCP | LOG_INFO, "%s [ID:%d]ipv6:%x%x%x%x:%x%x%x%x:%x%x%x%x:%x%x%x%x:%x%x%x%x:%x%x%x%x:%x%x%x%x:%x%x%x%x", tag, pthread_self(),
    s[index]>>4,s[index]&0xf,s[index+1]>>4,s[index+1]&0xf,s[index+2]>>4,s[index+2]&0xf,s[index+3]>>4,s[index+3]&0xf,
    s[index+4]>>4,s[index+4]&0xf,s[index+5]>>4,s[index+5]&0xf,s[index+6]>>4,s[index+6]&0xf,s[index+7]>>4,s[index+7]&0xf,
    s[index+8]>>4,s[index+8]&0xf,s[index+9]>>4,s[index+9]&0xf,s[index+10]>>4,s[index+10]&0xf,s[index+11]>>4,s[index+11]&0xf,
    s[index+12]>>4,s[index+12]&0xf,s[index+13]>>4,s[index+13]&0xf,s[index+14]>>4,s[index+14]&0xf,s[index+15]>>4,s[index+15]&0xf);
}

void* configure_route_to_host(void* p_if_index)
{
    int sock_fd = -1;
    fd_set rset;
    int if_index = 0;
    int buffer_size = 0;
    unsigned char *buffer = NULL;
    struct in6_addr dst; 
	struct sockaddr_ll from;
	socklen_t fromlen;
	int packet_len;
    char interface[IF_NAMESIZE+1] = {'\0',};
    struct timeval check_time,tp; 

    if_index = *((int*)p_if_index);

    if ((sock_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
        my_syslog(MS_DHCP | LOG_INFO, "failed to open socket:%s", strerror(errno));
        return (void*)-1;
    }

    if (!indextoname(sock_fd, if_index, interface)){
        my_syslog(MS_DHCP | LOG_INFO,"failed to convert if index to if name");
        close(sock_fd);
        return (void*)-1;
    }

    buffer_size = iface_get_mtu(sock_fd, interface) + MAX_LINKHEADER_SIZE;
	buffer = safe_malloc(buffer_size);
    fromlen = sizeof(from);

    if (update_start_time() < 0) {
        close(sock_fd);
        free(buffer);
        return (void*)-1;
    }

    while(1) {
        FD_ZERO(&rset);
        FD_SET(sock_fd, &rset);
        tp.tv_sec = TIMEOUT_VALUE/2; //5 sec
        tp.tv_usec = 0;

        if (select(sock_fd+1, &rset, NULL, NULL, &tp) < 0) { 
            if (errno == EINTR) {
                continue;
            }
            close(sock_fd);
            free(buffer);
            return (void*)-1;
        }

        if (FD_ISSET(sock_fd, &rset)) {
            packet_len = recvfrom(sock_fd, buffer, buffer_size, MSG_TRUNC,(struct sockaddr *) &from, &fromlen);
            if (from.sll_pkttype == PACKET_OUTGOING) {
                continue;
            }

            if ((from.sll_pkttype == PACKET_MULTICAST) && (buffer[ICMPV6_TYPE_INDEX] == ND_NEIGHBOR_SOLICIT)) {
                if ((buffer[GLOBAL_ADDRESS_INDEX] != 256) && (buffer[GLOBAL_ADDRESS_INDEX+1] != 128)) {//FE80,it is not link local 

                    memset(&dst, 0, sizeof(struct in6_addr));
                    memcpy(&dst, &buffer[GLOBAL_ADDRESS_INDEX], sizeof(struct in6_addr));

                    if (!indextoname(sock_fd, from.sll_ifindex, interface)){
                        my_syslog(MS_DHCP | LOG_INFO,"failed to convert if index to if name");
                        close(sock_fd);
                        free(buffer);
                        return (void*)-1;
                    }

                    add_route_to_active_lists(&dst, interface);
                }
            }
        }

        if ( gettimeofday(&check_time, NULL) < 0) {
            my_syslog(MS_DHCP | LOG_INFO,"failed to get the current time");
            close(sock_fd);
            free(buffer);
            return (void*)-1;
        }


        if ((check_time.tv_sec-g_start_time.tv_sec) > TIMEOUT_VALUE) {
#if DEBUG
            my_syslog(MS_DHCP | LOG_INFO,"[ID:%d]timed out",pthread_self());
#endif
            t_id =0;
            break;
        }
    }

#if DEBUG
    my_syslog(MS_DHCP | LOG_INFO, "[ID:%d]finished to configure ipv6 routing table", pthread_self());
#endif

    close(sock_fd);
    free(buffer);

    return (void*)0;
}

static int update_start_time()
{
    if (gettimeofday(&g_start_time, NULL) < 0) {
        my_syslog(MS_DHCP | LOG_INFO,"failed to get the current time");
        return -1;
    }
    return 1;
}
        
int start_configure_route_to_host(int if_index)
{
    my_syslog(MS_DHCP | LOG_INFO, "%s", __FUNCTION__);
    g_if_index = if_index;
    if (t_id == 0) {
        return pthread_create(&t_id, NULL, configure_route_to_host, &g_if_index);
    }else{
        update_start_time();
        return 1;
    }
}

int run_ip_cmd(char * cmd) {
    FILE *fp = NULL;

    if (strlen(cmd) > 255) {
        return -1; 
    }

    if ((fp = popen(cmd,"r")) == NULL) {
        my_syslog(MS_DHCP | LOG_INFO, "failed to popen: %s", strerror(errno));
        free(cmd);
        return -1;
    }

    pclose(fp);
    free(cmd);

    return 1;
}

static int modify_from_rule(int table_num, char *action, char *addr){
    char *cmd;

    asprintf(&cmd, "%s %s rule %s from %s table %d", "system/bin/ip", "-6",
            action, addr, table_num);

#if DEBUG
    my_syslog(MS_DHCP | LOG_INFO, "%s", cmd);
#endif

    if (run_ip_cmd(cmd) < 0) {
        my_syslog(MS_DHCP | LOG_INFO, "failed to run_ip_cmd");
        return -1;
    }

    return 1;
}

int modify_from_route(int table_num, char *action, char *addr, char *iface){
    char *cmd;

    asprintf(&cmd, "%s %s route %s  %s dev %s table %d", "system/bin/ip", "-6",
            action, addr,iface, table_num);
  my_syslog(MS_DHCP | LOG_INFO,"modify_from_route : %s",cmd);
#if DEBUG
    my_syslog(MS_DHCP | LOG_INFO, "%s", cmd);
#endif

    if (run_ip_cmd(cmd) < 0) {
        my_syslog(MS_DHCP | LOG_INFO, "failed to run_ip_cmd");
        return -1;
    }

    return 1;
}
#endif
