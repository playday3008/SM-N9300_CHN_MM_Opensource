#define LOG_TAG "JackLogger"
#define LOG_NDEBUG 0

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include <utils/Log.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "JackError.h"

#include <semaphore.h>

#include <utils/CallStack.h>
#include <cutils/properties.h>

#include "JackLogger.h"

#include <list>
#include <string>

//using namespace android;

#define JACKLOG(fd,fp,...) \
    if(fp!=NULL) { fprintf(fp, __VA_ARGS__); fprintf(fp,"\n");} \
    else ALOGE(__VA_ARGS__);

#define DUMP_FILE_STRING_NUM 80
#define TIME_STRING_NUM 24

#define DUMP_FILE_MANAGE_MAX_COUNT 20

//char *_jackDumpCopyPath = "/mnt/extSdCard"; // /mnt/extSdCard
#define JACK_DUMP_COPY_PATH "/mnt/extSdCard"

//char *_jackDumpCopyDirectory = "jack";
#define JACK_CUMP_COPY_DIRECTORY "jack"

//const char *_jackDumpPath = "/data/snd/jack";
#define JACK_DUMP_PATH "/data/misc/jack"
//const char *_pcmWriteFilePath = "/data/snd/jack/JackLogWritePcm_%s.pcm";
#define PCM_WRITE_FILE_PATH "/data/misc/jack/JackLogWritePcm_%s.pcm"
//const char *_pcmReadFilePath = "/data/snd/jack/JackLogReadPcm_%s.pcm";
#define PCM_READ_FILE_PATH "/data/misc/jack/JackLogReadPcm_%s.pcm"
//const char *_saveDumpFilePath = "/data/snd/jack/JackLogDump_%s.txt";
#define SAVE_DUMP_FILE_PATH "/data/misc/jack/JackLogDump_%s.txt"
//const char *_timeString = "%dm%dd_%dh%dm%ds";
#define TIME_STRING "%dm%dd_%dh%dm%ds"


namespace Jack {
//    class JackLogger::_logAudioDriver;

//    extern "C" void * Instance(){
//        return JackLogger::getInstance();
//    }
    
//    JackLogger* JackLogger::mInstance = NULL;
//    int JackLogger::mRefCount = 0;
    JackLogger::LogAudioDriver JackLogger::logAudioDriver;

	int JackLogger::mPcmDumpType = 0;
	FILE *JackLogger::PcmWriteDump = NULL;
	FILE *JackLogger::PcmReadDump = NULL;			 

    
//    JackLogger* JackLogger::getInstance(){
//        if(mInstance == NULL){
//            ALOGI("create Instance");
//            mInstance = new JackLogger();
//            memset(&logAudioDriver, 0, sizeof((_logAudioDriver)logAudioDriver));
//        }
//        mRefCount++;
//        return mInstance;
//    }
    bool JackLogger::getCurrentTimeStr(char * dest){
        bool ret = false;
        struct timeval systemTimeHighRes;
        struct tm buffer;

        char timeStrFormat[TIME_STRING_NUM] = {0, };
        
        if(dest == NULL){
            ALOGE("getCurrentTimeStr destination address is null");
            return ret;
        }

        if (gettimeofday(&systemTimeHighRes, 0) == -1){
            ALOGE("getCurrentTimeStr can't get time");
            return ret;
        }

        const struct tm* st = localtime_r(&systemTimeHighRes.tv_sec, &buffer);
        if(st == NULL){
            ALOGE("getCurrentTimeStr can't get local time");
            return ret;
        }
        
        snprintf(dest, TIME_STRING_NUM - 1, TIME_STRING, st->tm_mon + 1, st->tm_mday, st->tm_hour, st->tm_min, st->tm_sec);
        ALOGI("getCurrentTimeStr write file '%s'", dest);
        ret = true;
        return ret;
    }
//    void * JackLogger::deleteInstance(){
    void JackLogger::release(){
//        if(mInstance != NULL){
//            ALOGV("deleteInstance mRefCount %d", mRefCount);
//            if(--mRefCount <= 0) {
                char dumpFileName[DUMP_FILE_STRING_NUM] = {0, };
                char timeStr[TIME_STRING_NUM] = {0, };
                FILE *saveDumpFile = NULL;
                
                if(getCurrentTimeStr(timeStr)) {
                    snprintf(dumpFileName, DUMP_FILE_STRING_NUM - 1, SAVE_DUMP_FILE_PATH, timeStr);
                    ALOGI("deleteInstance write file '%s'", dumpFileName);

                    if(access(JACK_DUMP_PATH, F_OK) != 0){
                        ALOGE("deleteInstance '%s' is not exist, so make jack directory", JACK_DUMP_PATH);
                        umask(S_IWOTH|S_IROTH);
                        if(mkdir(JACK_DUMP_PATH, 0774) < 0){
                            ALOGE("deleteInstance create '%s' is failed, reason '%s'", JACK_DUMP_PATH, strerror(errno));
                        }
                    }
                    if(access(JACK_DUMP_PATH, F_OK) == 0){
                        saveDumpFile = fopen(dumpFileName, "wt");
                        if(saveDumpFile != NULL){
                            fputs("\n--- file dump start ---\n\n", saveDumpFile);
                            dumpLog(-1, saveDumpFile, LOG_BASE_ANDROIDSHM);
                            fputs("\n-----------------------\n", saveDumpFile);
                            dumpLog(-1, saveDumpFile, LOG_BASE_APASERVICE);
                            fputs("\n-----------------------\n", saveDumpFile);
                            dumpLog(-1, saveDumpFile, LOG_BASE_JACKSERVICE);
                            fputs("\n-----------------------\n", saveDumpFile);
                            dumpLog(-1, saveDumpFile, LOG_BASE_JACKD);
                            fputs("\n--- file dump end -----\n", saveDumpFile);
                            fclose(saveDumpFile);
                        }
                    }else{
                        ALOGE("deleteInstance can't store dump files in '%s'", JACK_DUMP_PATH);
                    }
                    
                }
                manageLogFiles();
                ALOGV("delete static instance!!");
//                delete mInstance;
//                mInstance = NULL;
//            }
//
//        }
//        return mInstance;
//        
    }
    
    void JackLogger::manageLogFiles(){
        ALOGI("manageLogFiles ++");
        DIR *dir;
        struct dirent *file;
        std::list<std::string> dumplist;
        dir = opendir(JACK_DUMP_PATH);
        
        if(dir != NULL){
  //          ALOGI("manageLogFiles find jack path '%s', dir %p", JACK_DUMP_PATH, dir);
            file = readdir(dir);
            while(file != NULL){
//                ALOGI("manageLogFiles file '%s'", file->d_name);
                if(strstr(file->d_name,"JackLogDump") != NULL){
                    char dumpLogFullPath[DUMP_FILE_STRING_NUM] = {0, };
                    snprintf(dumpLogFullPath, DUMP_FILE_STRING_NUM-1, "%s/%s", JACK_DUMP_PATH, file->d_name);
                    std::string dstr = dumpLogFullPath;
                    dumplist.push_back(dstr);
                }
                file = readdir(dir);
            }
            closedir(dir);
            int cnt = 1;
            std::list<std::string>::iterator it;
            dumplist.sort();
            for( it = dumplist.begin(); it != dumplist.end(); it++){
                std::string &str = *it;
//                ALOGI("manageLogFiles list %d, '%s'", cnt++, str.c_str());
            }

            dumplist.front();
            it = dumplist.begin();
            int delCnt = dumplist.size() - DUMP_FILE_MANAGE_MAX_COUNT;
            int err = 0;
            for(int i = 0; i < delCnt; i++){
                std::string &str = *it;
                ALOGE("manageLogFiles delete [%d/%d] file, '%s'", i + 1, delCnt, str.c_str());
                err = remove(str.c_str());
                if(err == 0){
                    it = dumplist.erase(it);
                }else{
                    ALOGE("manageLogFiles can't delete file, '%s', status %d, reason %s", str.c_str(), err, strerror(errno));
                }
            }
        }else{
            ALOGE("manageLogFiles can't open dir '%s', reason '%s'", JACK_DUMP_PATH, strerror(errno));
        }

        char pvalue[PROPERTY_VALUE_MAX];
        property_get("jack.force.copydump", pvalue, "0");

        char cForceCopyPath[DUMP_FILE_STRING_NUM] = {0, };
		strcpy(cForceCopyPath, JACK_DUMP_COPY_PATH);
        //bool bForceCopyPath = false;
        bool bCopyDump = false;
        if(pvalue[0] != '0'){
            if(strcmp(pvalue, "true") == 0){
                ALOGI("manageLogFiles move file to sdcard '%s'",JACK_CUMP_COPY_DIRECTORY );
                bCopyDump = true;
            }else if(pvalue[0] == '/'){
                int cntPvalue = strlen(pvalue);
                if(cntPvalue > 3 && cntPvalue < DUMP_FILE_STRING_NUM){
                    strncpy(cForceCopyPath, pvalue, cntPvalue);
					cForceCopyPath[cntPvalue] = 0;
                    //bForceCopyPath = true;
                    ALOGI("manageLogFiles set copy path '%s'",cForceCopyPath );
                }
                bCopyDump = true;
            }
        }
        //char *jackDumpCopyPath = NULL;
        //jackDumpCopyPath = (bForceCopyPath)? cForceCopyPath : JACK_DUMP_COPY_PATH;
        
        if(bCopyDump && access(cForceCopyPath, F_OK) == 0){ // if path is available 
            char jackDumpCopyFullPath[DUMP_FILE_STRING_NUM] = {0, };
            snprintf(jackDumpCopyFullPath, DUMP_FILE_STRING_NUM-1, "%s/%s", cForceCopyPath, JACK_CUMP_COPY_DIRECTORY);
//            ALOGI("manageLogFiles dest directory is '%s'", jackDumpCopyFullPath);
            
            if(strlen(jackDumpCopyFullPath) > 3 && strlen(jackDumpCopyFullPath) < DUMP_FILE_STRING_NUM){
                if(access(jackDumpCopyFullPath, F_OK) != 0){
                    umask(S_IWOTH|S_IROTH);
                    if(mkdir(jackDumpCopyFullPath, 0777) < 0){
                        ALOGE("manageLogFiles can't make external jack directory '%s -- ", strerror(errno));
                        return;
                    }
                    
                }
                dir = opendir(JACK_DUMP_PATH);
                file = readdir(dir);

                while(file != NULL){
                    if(strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0){
    //                    ALOGI("manageLogFiles skip no file '%s'", file->d_name);
                    }
                    else{
                        char copySrcFullPath[DUMP_FILE_STRING_NUM] = {0, };
                        snprintf(copySrcFullPath, DUMP_FILE_STRING_NUM-1, "%s/%s", JACK_DUMP_PATH, file->d_name);
                        char copyDestFullPath[DUMP_FILE_STRING_NUM] = {0, };
                        snprintf(copyDestFullPath, DUMP_FILE_STRING_NUM-1, "%s/%s", jackDumpCopyFullPath, file->d_name);                
                        if(access(copyDestFullPath, F_OK) != 0 ){ // file not exist in /mnt/extSdCard/jack
                            if(rename(copySrcFullPath, copyDestFullPath) < 0){
                                ALOGE("manageLogFiles rename '%s' to '%s' fail '%s'", copySrcFullPath, copyDestFullPath, strerror(errno));
                            }
                        }else{
                            ALOGE("manageLogFiles '%s' is already exist", copyDestFullPath);
                        }
                    }
                    file = readdir(dir);
                }
                closedir(dir);
            }
            else{
                ALOGE("manageLogFiles invalid copy dest '%s'", jackDumpCopyFullPath);
            }
        }else{
            ALOGE("manageLogFiles can't copy dump files to dest path '%s' reason : %d or %s", cForceCopyPath, bCopyDump, strerror(errno));
        }
        
        ALOGI("manageLogFiles --");
    }
    
    JackLogger::JackLogger(){
        //ALOGI("JackLogger, calledCount %d", mRefCount);
        mPcmDumpType = 0;
        memset(&logAudioDriver, 0, sizeof((LogAudioDriver)logAudioDriver));
    }
    
    JackLogger::~JackLogger(){
        //ALOGI("~JackLogger, calledCount %d", mRefCount);
		//release();
    }

    //void JackLogger::prCount(){
        /*
        CallStack st;
        st.update(0,30);
        st.log("JackLogger",ANDROID_LOG_ERROR,"prCount");
        */
        //ALOGE("[%d:%d] %p instance, prCount %d", getpid(), gettid(), mInstance, mRefCount);
    //}
    void JackLogger::setDriverDevices(const char * playback, const char * capture){
        //if(mInstance != NULL){
            ALOGI("setDriverDevices playback [%s], capture [%s], addr [%p, %p]", playback, capture, &logAudioDriver.mPlaybackDeviceId, &logAudioDriver.mPlaybackDeviceId);
            strncpy(logAudioDriver.mPlaybackDeviceId, playback, strlen(playback));
            strncpy(logAudioDriver.mCaptureDeviceId, capture, strlen(playback));
        //}
    }
    
    void JackLogger::checkSystemTime(TypeLogTime type, SaveLogSequence seq){
        struct timeval systemTimeHighRes;
        struct tm buffer;

        ALOGI("checkSystemTime type %d, seq %d", type, seq);
        if(seq <= SAVE_LOG_INVALID || seq >= SAVE_LOG_MAX){
            ALOGE("checkSystemTime invalid sequence type %d", seq);
            return;
        }

        if(type < LOG_TIME_PRINT || type >= LOG_TIME_MAX){
            ALOGE("checkSystemTime invalid log time type %d", type);
            return;
        }

        if (gettimeofday(&systemTimeHighRes, 0) == -1){
            ALOGE("checkSystemTime can't get time");
            return;
        }

        const struct tm* st = localtime_r(&systemTimeHighRes.tv_sec, &buffer);
        if(st == NULL){
            ALOGE("checkSystemTime can't get local time");
            return;
        }
        char current_t[32] = {0, };
        const char *t_type = "%d-%d-%d %d:%d:%d:%u";

        snprintf(current_t, 31, t_type, st->tm_year + 1900, st->tm_mon + 1, st->tm_mday, st->tm_hour, st->tm_min, st->tm_sec, systemTimeHighRes.tv_usec / 1000);
        ALOGI("checkSystemTime '%s', _log %p", current_t, &logAudioDriver);
        switch(type){
            case LOG_TIME_PRINT:
                ALOGE("checkSystemTime print time '%s' ", current_t);
                break;
            case LOG_TIME_AD_CONNECT:
                strncpy(logAudioDriver.tConnectDriver[seq], current_t, strlen(current_t));
                break;
            case LOG_TIME_AD_START:
                if(seq == SAVE_LOG_START && logAudioDriver.cFirstProcess == 0)
                    logAudioDriver.cFirstProcess = 1;                
                strncpy(logAudioDriver.tStartDriver[seq], current_t, strlen(current_t));
                break;
            case LOG_TIME_AD_FIRSTPROCESS:
                if(seq == SAVE_LOG_END && logAudioDriver.cFirstProcess == 1)
                    logAudioDriver.cFirstProcess = 2;
                strncpy(logAudioDriver.tFirstProcess[seq], current_t, strlen(current_t));                
                break;
            default:
                ALOGE("Invalid type %d", type);
                break;
        }
                
        
    }
    bool JackLogger::isFirstProcess(){
//        ALOGE("isFirstProcess cFirstProcess %d", mInstance->logAudioDriver.cFirstProcess);
        return (logAudioDriver.cFirstProcess == 2) ? false : true;
    }
    void JackLogger::dumpLog(int fd, FILE *fp, LogBase LogType){
        ALOGI("dumpLog fd %d, fp %p, type %d", fd, fp, LogType);
//        if(mInstance == NULL){
//            ALOGE("dumpLog instance is null!");
//            return;
//        }

        switch(LogType){
            case LOG_BASE_JACKD:
                JACKLOG(fd, fp, "- Jackd Dump :");
                JACKLOG(fd, fp, "  : playback device id '%s'", logAudioDriver.mPlaybackDeviceId)
                JACKLOG(fd, fp, "  : capture device id '%s'", logAudioDriver.mCaptureDeviceId)               
                JACKLOG(fd, fp, "  : pcm connect try time '%s'", logAudioDriver.tConnectDriver[0])
                JACKLOG(fd, fp, "  : pcm connect done time '%s'", logAudioDriver.tConnectDriver[1])
                JACKLOG(fd, fp, "  : pcm start try time '%s'", logAudioDriver.tStartDriver[0])
                JACKLOG(fd, fp, "  : pcm start done time '%s'", logAudioDriver.tStartDriver[1])
                JACKLOG(fd, fp, "  : pcm first processing try time '%s'", logAudioDriver.tFirstProcess[0])
                JACKLOG(fd, fp, "  : pcm first processing done time '%s'", logAudioDriver.tFirstProcess[1])
                break;
            case LOG_BASE_APASERVICE:
                JACKLOG(fd, fp, "- APA Service Dump :")
                break;
            case LOG_BASE_ANDROIDSHM:
                JACKLOG(fd, fp, "- Android Shm Dump :")
                break;
            case LOG_BASE_JACKSERVICE:
                JACKLOG(fd, fp, "- JackService Dump :")
                break;
            default:
                ALOGE("Invalid Log Type %d", LogType);
                break;

        }
    }
    
    void JackLogger::excuteCommand(LogCommand command, CommandParam param, char * pParam, int pParamSize){
        ALOGI("excuteParam command %d, param %d", command, param);
        switch(command){
            case LOG_COMMAND_PCM_DUMP:
                if(param == COMMAND_PARAM_PCM_DUMP_START){
                    if(createPcmDumpFile() != -1){
                        if(!startPcmDump((PcmDumpType)(JACK_PCM_DUMP_WRITE | JACK_PCM_DUMP_READ))){
                            ALOGE("excuteCommand start pcm dump fail");
                        }
                    }
                }
                if(param == COMMAND_PARAM_PCM_DUMP_STOP){
                    if(!isStartedPcmDump()){
                        ALOGE("excuteCommand pcm dump not started %d", mPcmDumpType);
                    }

                    stopPcmDump();
                    
                }
                
                break;
            case LOG_COMMAND_SET_PARAM:
                if(pParam != NULL){
                    
                }else{
                    ALOGE("set param source param is null");
                }
                break;
            case LOG_COMMAND_GET_PARAM:
                if(pParam != NULL){
                    
                }else{
                    ALOGE("get param source param is null");
                }                
                break;
            default:
                ALOGE("Invalid command %d", command);
                break;
        }
    }

    int JackLogger::createPcmDumpFile(){
        char timeStr[TIME_STRING_NUM] = {0, };
        char pcmWriteFileName[DUMP_FILE_STRING_NUM] = {0, };
        char pcmReadFileName[DUMP_FILE_STRING_NUM] = {0, };        
        
        if(getCurrentTimeStr(timeStr)){
            if(access(JACK_DUMP_PATH, F_OK) != 0){
                ALOGE("createPcmDumpFile '%s' is not exist, so make jack directory", JACK_DUMP_PATH);
                umask(S_IWOTH|S_IROTH);
                if(mkdir(JACK_DUMP_PATH, 0774) < 0){
                    ALOGE("createPcmDumpFile create '%s' is failed, reason '%s'", JACK_DUMP_PATH, strerror(errno));
                    ALOGE("createPcmDumpFile make pcm dump fail!!, make dump directory fail");
                    return -1;
                }
            }
            
            if(access(JACK_DUMP_PATH, F_OK) == 0){
                snprintf(pcmWriteFileName, DUMP_FILE_STRING_NUM - 1, PCM_WRITE_FILE_PATH, timeStr);
                snprintf(pcmReadFileName, DUMP_FILE_STRING_NUM - 1, PCM_READ_FILE_PATH, timeStr);        
                ALOGI("createPcmDumpFile write file '%s'", pcmWriteFileName);
                ALOGI("createPcmDumpFile read file '%s'", pcmWriteFileName);        
                if(PcmWriteDump != NULL){
                    ALOGE("createPcmDumpFile already use pcm dump file");
                }else{
                    PcmWriteDump = fopen(pcmWriteFileName, "wb");
                    if(PcmWriteDump == NULL){
                        ALOGE("createPcmDumpFile can't make '%s'", pcmWriteFileName);
                    }
                    
                    PcmReadDump = fopen(pcmReadFileName, "wb");
                    if(PcmReadDump == NULL){
                        ALOGE("createPcmDumpFile can't make '%s'", pcmReadFileName);
                    }
                }
            }else{
                ALOGE("createPcmDumpFile get pcm dump fail!!, destination directory not exist");
            }
        }
        return 0;
    }
    
    bool JackLogger::startPcmDump(PcmDumpType type){
        bool ret = true;
        ALOGI("startPcmDump type %d, current type %d", type, mPcmDumpType);
        if(mPcmDumpType != JACK_PCM_DUMP_NONE){
            ALOGE("startPcmDump already used or previouse dump is not complte %d", mPcmDumpType);
            ret = false;
        }else{
            mPcmDumpType = type;
        }
        return ret;
    }
    
    bool JackLogger::stopPcmDump(){
        bool ret = true;
        int tDumpType = mPcmDumpType;
        ALOGI("stopPcmDump current type %d", mPcmDumpType);
        if(mPcmDumpType == JACK_PCM_DUMP_NONE){
            ALOGE("stopPcmDump dump is not stared!! %d", mPcmDumpType);
            ret = false;
        }
        mPcmDumpType = JACK_PCM_DUMP_NONE;
        
        usleep(25);
        
        if(tDumpType & JACK_PCM_DUMP_WRITE){
            if(PcmWriteDump){
                fclose(PcmWriteDump);
                PcmWriteDump = NULL;
            }else{
                ALOGE("stopPcmDump write pcm file is not opened");
            }
        }
        if(tDumpType & JACK_PCM_DUMP_READ){
            if(PcmReadDump){
                fclose(PcmReadDump);
                PcmReadDump = NULL;
            }else{
                ALOGE("stopPcmDump read pcm file is not opened");
            }
        }

        return ret;
    }
    
    bool JackLogger::isStartedPcmDump(){
        return (mPcmDumpType == JACK_PCM_DUMP_NONE) ? false : true;
    }
    
    void JackLogger::dumpWriteData(float *buffer, int nframes){
        if(PcmWriteDump){
            fwrite(buffer, sizeof(float), nframes, PcmWriteDump);
        }
    }
    
    void JackLogger::dumpReadData(float *buffer, int nframes){
        if(PcmReadDump){
            fwrite(buffer, sizeof(float), nframes, PcmReadDump);
        }
    }
}

