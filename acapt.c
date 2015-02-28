#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <syslog.h>


int hour=1,min=1,sec=1,year=0,month=0,mday=0;
int stop=0;
int thread=0;
int stream_error=0;
static void daemon();
double dl_bytes_received;
CURL *curl;
CURLcode res;

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
        size_t written;
        written = fwrite(ptr, size, nmemb, stream);
        if(stop)
            return -1;
        return written;
}
char* syslog_write(char *msg){
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("easyfm", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	syslog (LOG_NOTICE, "%s",msg);
	closelog ();

}
void * curl_thread(){
	syslog_write("cURL thread started.");
        FILE *fp;
        char format[] = "/pathto/file_%d-%s-%s_%s.%s.%s.mp3";
        char outfilename[sizeof format+100];
        char mi[3];
        char mth[3];
        char dom[3];
        char hrs[3];
        char secs[3];
        sprintf(mth, "%d", month);
        sprintf(dom, "%d", mday);
        sprintf(hrs, "%d", hour);
        sprintf(mi, "%d", min);
        sprintf(secs, "%d", sec);
        if (month<10){
                sprintf(mth, "0%d", month);}
        if (mday<10){
                sprintf(dom, "0%d", mday); }
        if (hour<10){
                sprintf(hrs, "0%d", hour); }
        if (min<10){
                sprintf(mi, "0%d", min);}
        if (sec<10){
                sprintf(secs, "0%d", sec);}
        sprintf(outfilename,format,year,mth,dom,hrs,mi,secs);
        curl = curl_easy_init();
        if(curl) {
                fp = fopen(outfilename,"wb");
                curl_easy_setopt(curl, CURLOPT_URL, "http://streamserver:8000/easyfm_64.mp3");
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
                curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT,10);
                curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME,10);
                curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
                curl_easy_setopt(curl, CURLOPT_USERAGENT, "acapt v0.2a (Linux). by Tadas Ustinavičius");
                res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);
                fclose(fp);
        }
	curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &dl_bytes_received);
	if (!stop){
	    syslog_write("No data from stream. Terminating thread.");
	    stream_error=1;
	    }
        stop=0;
        thread=0;
        curl_global_cleanup();
        syslog_write("cURL thread terminated.");
        pthread_exit(0);
}
void time_date(void){
        time_t rawtime;
        time (&rawtime);
        struct tm *tm_struct = localtime(&rawtime);
        hour = tm_struct->tm_hour;
        min = tm_struct->tm_min;
        sec=  tm_struct->tm_sec;
        year=tm_struct->tm_year + 1900;
        month=tm_struct->tm_mon + 1;
        mday=tm_struct->tm_mday;
}
int main(){
	syslog_write("Starting program.");
        daemon();
        time_date();
        pthread_t tid;
        pthread_create(&tid,NULL,curl_thread,NULL);
        thread=1;
        while (1){
                if (!thread){
	                char received[sizeof dl_bytes_received+1];
    			sprintf(received,"Bytes received: %f",dl_bytes_received);
    			syslog_write(received);
    			if (stream_error){//sleep several seconds and retry download
            		    syslog_write("Waiting 10 seconds before retry");
            		    stream_error=0;
            		    sleep(10);
            		}
    			syslog_write("Creating new file.");
                        pthread_t tid;
                        pthread_create(&tid,NULL,curl_thread,NULL);
                        pthread_detach(tid);
                        thread=1;
                        sleep(1);
                }
        time_date();
        usleep(600000);
                if(!sec&&!min&&thread){
		    syslog_write("End of period reached. Signalling fwrite stop.");
                    stop=1;
                }
                while(stop){
                    usleep (1000);
                }

        }
}
void daemon(){
        pid_t mypid;
        FILE *pid;
        mypid=fork();
        if (mypid){
                pid=fopen("acapt-easyfm.pid","w");
                fprintf(pid,"%i",mypid);
                exit (0);
        }
}
