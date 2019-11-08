#include <fluid_realtime.h>
#include <pthread.h>
#include <pthread_internal.h>
#include <mediautils/SchedulingPolicyService.h>
#include <fluid_sys.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>


#define SET_PRIORITY_SOCKET_NAME "/dev/socket/jack/set.priority"

void fluid_requestPriority(int pid, int tid, int priority){
	int sock  = socket(AF_UNIX, SOCK_DGRAM, 0);

	if(-1 == sock) {
	  fluid_log(FLUID_ERR,"failed to create socket\n");
	  return;
	}

	char message[108] = {0,};
	snprintf(message, sizeof(message), "*4%d,%d,%d,", pid, tid, priority);
	

	struct sockaddr_un	 target_addr;
	memset(&target_addr, 0, sizeof(target_addr));
	target_addr.sun_family = AF_UNIX;

    if ( UNIX_PATH_MAX > strlen(SET_PRIORITY_SOCKET_NAME) ) {
        strcpy(target_addr.sun_path, SET_PRIORITY_SOCKET_NAME);
    } else {
		close(sock);
        return;
    }

	target_addr.sun_path[0] = '\0';  // to abstract address
	int aByte = (int)sendto(sock, message, strlen(message)+1, 0, (struct sockaddr*)&target_addr, sizeof(target_addr));
	if(aByte == -1) {
		fluid_log(FLUID_ERR,"fail to send a priority message[%s]", message);
	}

	close(sock);
}


int fluid_getAndroidRealtime(pid_t tid, int pro)    
{
    int res;
    pthread_internal_t* thread_;
    thread_ = reinterpret_cast<pthread_internal_t*>(tid);

//    if ((res = android::requestPriority(getpid(), thread_->tid, pro)) != 0) {
//        fluid_log(FLUID_ERR, "Failed to get SCHED_FIFO priority pid %d tid %d; error %d",
//		    getpid(), thread_->tid, res);
//        return -1;
//    }
	fluid_requestPriority(getpid(), thread_->tid, pro);

    return 0;
}

