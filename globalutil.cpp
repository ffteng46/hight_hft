#include "globalutil.h"
#include <stdio.h>
#include <iostream>
#include <list>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include "TraderSpi.h"

#pragma comment(lib,"ws2_32.lib")
using namespace std;
//��־����·��
string filepath = "tradelog.txt";
//��־����·��
string filepath_mk = "mklog.txt";
//���������Ϣ����
list<string> mkdata;
extern boost::lockfree::queue<LogMsg*> mkdataqueue;
///��־��Ϣ����
extern boost::lockfree::queue<LogMsg*> logqueue;

void logEngine(){
    //cout<<"��������"<<endl;
    cout<<boosttoolsnamespace::CBoostTools::gbktoutf8("��������")<<endl;
	ofstream in;
	in.open(filepath,ios::app); //ios::trunc��ʾ�ڴ��ļ�ǰ���ļ����,������д��,�ļ��������򴴽�
    LogMsg *pData;/*
    while(1)
    {

        if(logqueue.empty()){
            sleep(1);
        }else if( logqueue.pop( pData ) ){
            //cout<<pData->getMsg()<<" "<<logqueue.empty()<<endl;
            string info;
            char cw[2048]={'\0'};
            info = pData->getMsg();
//            if(info.size() == 0){
//                continue;
//            }
            info = getCurrentSystemTime()+" "+info;
            cout<<"������־��"<<info<<";size="<<info.size()<<";cap="<<endl;
            info.copy(cw,info.size(),0);
            //cout<<"��־��"<<cw<<";size="<<strlen(cw)<<endl;
            in<<cw<<endl;
        }
         //cout<<"yigong="<<c<<endl;
    }*/
	in.close();//�ر��ļ�
}

void marketdataEngine(){
	ofstream in;
	in.open(filepath_mk,ios::app); //ios::trunc��ʾ�ڴ��ļ�ǰ���ļ����,������д��,�ļ��������򴴽�
    int c = 0;
//	while(1){
//        LogMsg *pData;
//        if(mkdataqueue.empty()){
//            sleep(1);
//        }else if(mkdataqueue.pop(pData)){
//            c++;
//            string info;
//            char cw[2048]={'\0'};

//            info = pData->getMsg();
//            if(info.size() == 0){
//                continue;
//            }
//            info = getCurrentSystemTime()+" "+info;
//            cout<<"������־��"<<info<<";size="<<info.size()<<endl;
//            info.copy(cw,info.size(),0);
//            //cout<<"��־��"<<cw<<";size="<<strlen(cw)<<endl;
//            in<<cw<<endl;
//        }
//        //cout<<"yigong="<<c<<endl;
//        //in.flush();
//	}
	in.close();//�ر��ļ�
}
// ��ȡϵͳ�ĵ�ǰʱ�䣬��λ΢��(us)
int64_t GetSysTimeMicros()
{
#ifdef _WIN32
	// ��1601��1��1��0:0:0:000��1970��1��1��0:0:0:000��ʱ��(��λ100ns)
#define EPOCHFILETIME   (116444736000000000UL)
	FILETIME ft;
	LARGE_INTEGER li;
	int64_t tt = 0;
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;
	// ��1970��1��1��0:0:0:000�����ڵ�΢����(UTCʱ��)
	tt = (li.QuadPart - EPOCHFILETIME) /10;
	return tt;
#else
	timeval tv;
	gettimeofday(&tv, 0);
	return (int64_t)tv.tv_sec * 1000000 + (int64_t)tv.tv_usec;
#endif // _WIN32
	return 0;
}
char* GetDiffTime(int64_t start,int64_t end){
	char char_diff[10]={'\0'};
	int64_t diff = end - start;
	sprintf(char_diff,"%d",diff);
	return char_diff;
}
vector<string> split(string str,string pattern){
    str += pattern;
    vector<string> strvev;
    int lenstr = str.size();
    for(int i=0;i<lenstr;i++){
        int pos = str.find(pattern,i);
        if(pos	< lenstr){
            string findstr = str.substr(i,pos - i);
            strvev.push_back(boost::trim_copy(findstr));
            i = pos + pattern.size() -1;
        }
    }
    return strvev;
}
void recordRunningMsg(string msg){

    LogMsg logmsg;
    logmsg.setMsg(msg);
    logqueue.push(&logmsg);
//    while(!logqueue.push(&logmsg));
    cout<<logmsg.getMsg()<<endl;
}
////��ȡ��ǰϵͳʱ��
string getCurrentSystemTime(){
    time_t tmp_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm* tm_time = localtime(&tmp_time);
    auto auto_time = put_time(tm_time,"%F %H:%M:%S");
    stringstream ss;
    ss<<auto_time;
    return ss.str();
}
void test(){
    int g;
    while(1){
        stringstream ss;
        ss<<g++;
        string ssg = ss.str();
        LogMsg log1;
        log1.setMsg(ssg);
        logqueue.push(&log1);
        mkdataqueue.push(&log1);
        sleep(2);
    }

}
