#ifndef JACKLOGGER_H
#define JACKLOGGER_H

#include <binder/Parcel.h>

namespace Jack {
    
    class JackLogger 
    {
        private:
        
            //static JackLogger * mInstance;
            //static int mRefCount;

            static FILE *PcmWriteDump;
            static FILE *PcmReadDump;            
            static int mPcmDumpType;

			class LogAudioDriver {
                public:
                    char mUsingDriver[16];
                    char mPlaybackDeviceId[8];
                    char mCaptureDeviceId[8];
                    char tConnectDriver[2][32];
                    char tStartDriver[2][32];
                    char tFirstProcess[2][32];
                    int cFirstProcess;
            };

            //const long ALLOW_PROCESS_DELAY_TIME = 100000L; // 100 * 1000 - 10 milli seconds
            static LogAudioDriver logAudioDriver;
			
        public:
            typedef enum LogBase{
                LOG_BASE_JACKD = 0,
                LOG_BASE_APASERVICE,
                LOG_BASE_ANDROIDSHM,
                LOG_BASE_JACKSERVICE,
                LOG_BASE_MAX
            } LogBase;

            typedef enum TypeLogTime{
                LOG_TIME_PRINT = 0,
                LOG_TIME_AD_CONNECT,
                LOG_TIME_AD_START,
                LOG_TIME_AD_FIRSTPROCESS,
                LOG_TIME_MAX
            } TypeLogTime;

            typedef enum SaveLogSequence{
                SAVE_LOG_INVALID = -1,
                SAVE_LOG_START,
                SAVE_LOG_END,
                SAVE_LOG_MAX
            } SaveLogSequence;

            typedef enum LogCommand{
                LOG_COMMAND_PCM_DUMP = 0,
                LOG_COMMAND_SET_PARAM,
                LOG_COMMAND_GET_PARAM,
                LOG_COMMAND_MAX
            } LogCommand;

            typedef enum CommandParam{
                COMMAND_PARAM_PCM_DUMP_START = 0,
                COMMAND_PARAM_PCM_DUMP_STOP,
                COMMAND_PARAM_SET_PARAM,
                COMMAND_PARAM_GET_PARAM
            } CommandParam;

            typedef enum PcmDumpType{
                JACK_PCM_DUMP_NONE = 0,
                JACK_PCM_DUMP_WRITE,
                JACK_PCM_DUMP_READ
            } PcmDumpType;

		private:
            bool getCurrentTimeStr(char * dest);
            void release();
            int createPcmDumpFile();            
            void manageLogFiles();
            bool startPcmDump(PcmDumpType type);
            bool stopPcmDump();            

		public:
			JackLogger();
            ~JackLogger();
            //static JackLogger * getInstance();
            //void * deleteInstance();
            //void prCount();
            void setDriverDevices(const char * playback, const char * capture);
            void checkSystemTime(TypeLogTime type, SaveLogSequence seq);
            bool isFirstProcess();
            void dumpLog(int fd, FILE * fp, LogBase LogType);
            void excuteCommand(LogCommand command, CommandParam param, char * pParam = NULL, int pParamSize = 0);

            bool isStartedPcmDump();
            void dumpWriteData(float *buffer, int nframes);
            void dumpReadData(float *buffer, int nframes);            
            
    };
}
#endif

