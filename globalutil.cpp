#include "globalutil.h"
#include <XeleFtdcMduserApi.h>
#include <stdio.h>
#include <iostream>
#include <list>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include "TraderSpi.h"
#include <unordered_map>
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
//gap list map
unordered_map<double,double[]> map_price_gap;
//preious price
double previous_price = 0;
//����
int up_culculate = 0;
//�µ�
int down_culculate = 0;
//��һ�μ۸�����������
int price_gap = -1;
//���������ź�
int cul_times = 2;
//ӯ������
double profit = 0;
//���״���
unsigned int trade_num = 0;
// �µ���
int trade_default[];
//�ֱֲ�
int trade_bi=[];
int bidpst=0;
int askpst=0;
int great_than_three_bid = 0;
int great_than_three_ask = 0;
int bi=30;
int default_volume = 1;
double settlementPrice = 10;
//���潻������
int tradeInfo[];
void logEngine(){
    //cout<<"��������"<<endl;
    cout<<boosttoolsnamespace::CBoostTools::gbktoutf8("��������")<<endl;
	ofstream in;
	in.open(filepath,ios::app); //ios::trunc��ʾ�ڴ��ļ�ǰ���ļ����,������д��,�ļ��������򴴽�
    LogMsg *pData;
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
    }
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
string getCloseMethod(){
    //ƽ��  ���� '0';ƽ�� '1';ƽ�� '3';ƽ�� '4';ǿƽ '2'
    string orderoffset = "1";
    if(offset_flag == 1){
        orderoffset = "1";
    }else if(offset_flag == 3){
        orderoffset = "3";
    }else if(offset_flag == 4){
        orderoffset = "4";
    }
    return orderoffset;
}
void initPriceGap(){
    double price_tick = 0.5;
    double min_price = 0;
    double max_price = 10;
    //int gaps = 2*(max_price - min_price);
    double tmp_price = min_price;
    while(true){
        if(tmp_price <= max_price){
            double down_gap = (tmp_price - min_price)*2;
            double up_gap = down_gap + 1;
            double gap_list[2] = [down_gap,up_gap];
            map_price_gap[tmp_price] = gap_list;
        }else{
            break;
        }
        tmp_price += price_tick;
    }
}

void OnRtnSHFEMarketData(CXeleShfeHighLevelOneMarketData *pDepthMarketData)
{
    cout<<"----------------------->"<<getCloseMethod()<<endl;
    //��������
    int64_t start_time = GetSysTimeMicros();
    auto chro_start_time = boost::chrono::high_resolution_clock::now();
    string marketdata;
    stringstream ss;
    char instrumentID[17] = pDepthMarketData->Instrument;
    TXeleMdFtdcTimeType UpdateTime;
    TXeleMdFtdcMillisecType UpdateMillisec;
    TXeleMdFtdcVolumeType Volume;
    TXeleMdFtdcPriceType lastPrice = pDepthMarketData->LastPrice;
    TXeleMdFtdcMoneyType Turnover;
    TXeleMdFtdcLargeVolumeType OpenInterest;
    TXeleMdFtdcPriceType bidPrice = pDepthMarketData->BidPrice;
    TXeleMdFtdcPriceType askPrice = pDepthMarketData->AskPrice;
    TXeleMdFtdcVolumeType BidVolume;
    TXeleMdFtdcVolumeType AskVolume;

    if(previous_price == 0){
        previous_price = lastPrice;
        return;
    }
    double gap_list[];
    //cant find
    if(map_price_gap.find(lastPrice) == map_price_gap.end()){
        string msg = "can not find map_price_gap item: price=" + boost::lexical_cast<string>(lastPrice);
        LogMsg logmsg = new LogMsg();
        logmsg.setMsg(msg);
        logqueue.push(&logmsg);
        return;
    }else{
        unordered_map<double,double[]>::iterator map_it = map_price_gap.find(lastPrice);
        gap_list = map_it->second;
    }
    double gap = 0;
    if(lastPrice > previous_price){
        gap = gap_list[0];
    }else if(lastPrice < previous_price){
        gap = gap_list[1];
    }else if(lastPrice == previous_price){
        previous_price = lastPrice;
        return;
    }
    previous_price = lastPrice;
    if(price_gap == -1){
        price_gap = gap;
        return;
    }else if(price_gap == gap){
        return;
    }else if(price_gap < gap){
        down_culculate = 0;
        up_culculate += 1;
    }else if(price_gap > gap){
        down_culculate += 1;
        up_culculate = 0;
    }

    price_gap = gap;
    if(up_culculate >= cul_times){
        //sell
        char char_orderdir[] = "1";
        //��ƽ�ж�
        if(isclose == 1){//����
            orderoffset = "0";
        }else if(isclose == 2){//ƽ��  ���� '0';ƽ�� '1';ƽ�� '3';ƽ�� '4';ǿƽ '2'
            orderoffset = getCloseMethod();
        }
        strcpy(char_orderoffset,orderoffset.c_str());
        pUserSpi->md_orderinsert(askPrice,char_orderdir,char_orderoffset,instrumentID,default_volume);
    }else if(down_culculate >= cul_times){
        //��
        char char_orderdir[] = "0";
        //��ƽ�ж�
        if(isclose == 1){//����
            orderoffset = "0";
        }else if(isclose == 2){//ƽ��  ���� '0';ƽ�� '1';ƽ�� '3';ƽ�� '4';ǿƽ '2'
            orderoffset = getCloseMethod();
        }
        strcpy(char_orderoffset,orderoffset.c_str());
        pUserSpi->md_orderinsert(askPrice,char_orderdir,char_orderoffset,instrumentID,default_volume);
    }

    //��������
    int64_t end1 = GetSysTimeMicros();
    string msg;
    char Instrument [17];
    TXeleMdFtdcTimeType UpdateTime;
    TXeleMdFtdcMillisecType UpdateMillisec;
    TXeleMdFtdcVolumeType Volume;
    TXeleMdFtdcPriceType LastPrice;
    TXeleMdFtdcMoneyType Turnover;
    TXeleMdFtdcLargeVolumeType OpenInterest;
    TXeleMdFtdcPriceType BidPrice;
    TXeleMdFtdcPriceType AskPrice;
    TXeleMdFtdcVolumeType BidVolume;
    TXeleMdFtdcVolumeType AskVolume;
    /*string msg = "�����źŻ���:" + string(GetDiffTime(end1,start_time));
    LogMsg logmsg;
    logmsg.setMsg(msg);
    logqueue.push(&logmsg)*/;

    ///����޸�ʱ��
    marketdata.append("UpdateTime=");
    marketdata.append(pDepthMarketData->UpdateTime);
    marketdata.append(sep);
    ///�����һ
    marketdata.append("BidPrice1=");
    ss << pDepthMarketData->BidPrice;
    string BidPrice1;
    ss >> BidPrice1;
    marketdata.append(BidPrice1);
    marketdata.append(sep);
    ss.clear();
    ///������һ
    char char_bv1[20] = {'\0'};
    sprintf(char_bv1,"%d",pDepthMarketData->BidVolume);
    marketdata.append("BidVolume1=");
    marketdata.append(char_bv1);
    marketdata.append(sep);
    ///������һ
    marketdata.append("AskPrice1=");
    ss << pDepthMarketData->AskPrice;
    string AskPrice1;
    ss >> AskPrice1;
    marketdata.append(AskPrice1);
    marketdata.append(sep);
    ss.clear();
    ///������һ
    char char_sv1[20] = {'\0'};
    sprintf(char_sv1,"%d",pDepthMarketData->AskVolume);
    marketdata.append("AskVolume1=");
    marketdata.append(char_sv1);
    marketdata.append(sep);
    marketdata.append("InstrumentID=");
    marketdata.append(pDepthMarketData->InstrumentID);
    marketdata.append(sep);
    ///���¼�
    marketdata.append("LastPrice=");
    ss << pDepthMarketData->LastPrice;
    string lastprice;
    ss >> lastprice;
    marketdata.append(lastprice);
    marketdata.append(sep);
    ///����
    TThostFtdcVolumeType	Volume = pDepthMarketData->Volume;
    char char_vol[20] = {'\0'};
    sprintf(char_vol,"%d",Volume);
    marketdata.append("Volume=");
    marketdata.append(char_vol);
    marketdata.append(sep);
    ///�ֲ���
    TThostFtdcLargeVolumeType	OpenInterest = pDepthMarketData->OpenInterest;
    char char_opi[20] = {'\0'};
    sprintf(char_opi,"%d",OpenInterest);
    marketdata.append("OpenInterest=");
    marketdata.append(char_opi);
    marketdata.append(sep);


    ///����޸ĺ���
    TThostFtdcMillisecType	UpdateMillisec = pDepthMarketData->UpdateMillisec;
    char char_ums[20] = {'\0'};
    sprintf(char_ums,"%d",UpdateMillisec);
    marketdata.append("UpdateMillisec=");
    marketdata.append(char_ums);
    marketdata.append(sep);

    /*
    ///����۶�
    TThostFtdcPriceType	BidPrice2;
    ///��������
    TThostFtdcVolumeType	BidVolume2;
    ///�����۶�
    TThostFtdcPriceType	AskPrice2;
    ///��������
    TThostFtdcVolumeType	AskVolume2;
    ///�������
    TThostFtdcPriceType	BidPrice3;
    ///��������
    TThostFtdcVolumeType	BidVolume3;
    ///��������
    TThostFtdcPriceType	AskPrice3;
    ///��������
    TThostFtdcVolumeType	AskVolume3;
    ///�������
    TThostFtdcPriceType	BidPrice4;
    ///��������
    TThostFtdcVolumeType	BidVolume4;
    ///��������
    TThostFtdcPriceType	AskPrice4;
    ///��������
    TThostFtdcVolumeType	AskVolume4;
    ///�������
    TThostFtdcPriceType	BidPrice5;
    ///��������
    TThostFtdcVolumeType	BidVolume5;
    ///��������
    TThostFtdcPriceType	AskPrice5;
    ///��������
    TThostFtdcVolumeType	AskVolume5;
    */
    ///���վ���
    marketdata.append("AveragePrice=");
    ss << pDepthMarketData->AveragePrice;
    string AveragePrice;
    ss >> AveragePrice;
    marketdata.append(AveragePrice);
    marketdata.append(sep);
    ss.clear();
//    logmsg.setMsg(marketdata);
//    mkdataqueue.push(&logmsg);
    //��������
    int64_t end2 = GetSysTimeMicros();
    auto chro_end_time = boost::chrono::high_resolution_clock::now();
    auto pro_time = boost::chrono::duration<double>(chro_end_time - chro_start_time).count();
    stringstream timss;
    timss<<pro_time;
    cout<<timss.str()<<endl;
    cout<<marketdata<<endl;
    //msg = "��¼���黨��:" + string(GetDiffTime(end2,end1)) + ";ʵ�ʴ���ʱ�䣺" + timss.str();
    LOG(INFO)<<msg;
}
