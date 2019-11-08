#ifndef _FLUID_REALTIME_H
#define _FLUID_REALTIME_H

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC int fluid_getAndroidRealtime(int tid, int pro);

#endif
