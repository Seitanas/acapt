#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <curl/curl.h>

int hour=1,min=1,sec=1,year=0,month=0,mday=0;
int stop=0;
static void daemon();
CURL *curl;
CURLcode res;
	
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);
	if(stop)
	    return -1;
	return written;
}
void * curl_thread(){
	FILE *fp;
	char format[] = "/pathto/file_%d-%s-%s_%s.%s.%s.mp3";
	char outfilename[sizeof format+100];
	char mi[3];
	char mth[3];
	char dom[3];
	char hrs[3];
	char secs[3];
	snprintf(mth, 10, "%d", month);
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
		curl_easy_setopt(curl, CURLOPT_URL, "http://streamserver:8000/stream.mp3");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "acapt v0.1 (Linux). by Tadas Ustinavičius");
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		fclose(fp);
	}
	stop=0;
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
	daemon();
	time_date();
	int thread=0;
	pthread_t tid;
	pthread_create(&tid,NULL,curl_thread,NULL);
	thread=1;
	while (1){
		if (!thread){
			pthread_t tid;
			pthread_create(&tid,NULL,curl_thread,NULL);
			sleep(1);
			thread=1;
		}
	time_date();
	usleep(600000);
		if(!sec&&!min){
			stop=1;
			while(stop){
				usleep (1000);
			    }
			thread=0;
		}
	}
}
void daemon(){
        pid_t mypid;
        FILE *pid;
        mypid=fork();
        if (mypid){
        	pid=fopen("acapt.pid","w");
        	fprintf(pid,"%i",mypid);
        	exit (0);
        }
}