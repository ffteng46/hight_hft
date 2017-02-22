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
//日志保存路径
string filepath = "tradelog.txt";
//日志保存路径
string filepath_mk = "mklog.txt";
//存放行情消息队列
list<string> mkdata;
extern boost::lockfree::queue<LogMsg*> mkdataqueue;
///日志消息队列
extern boost::lockfree::queue<LogMsg*> logqueue;
extern int long_offset_flag;
extern int short_offset_flag;
//买平标志,1开仓；2平仓
extern int longPstIsClose;
extern int shortPstIsClose;
//价格变动单位
extern double tick;
//跌停价格
extern double min_price;
//涨停价格
extern double max_price;
// UserApi对象
extern CTraderSpi* pUserSpi;
extern int isTest;
//gap list map
unordered_map<double,vector<double>> map_price_gap;
extern char singleInstrument[30];
extern int default_volume;
//preious price
double previous_price = 0;

//上涨
int up_culculate = 0;
//下跌
int down_culculate = 0;
//上一次价格所处的区间
int last_gap = -1;
//报单触发信号
extern int cul_times;
//盈利数字
double profit = 0;
//交易次数
unsigned int trade_num = 0;
// 下单量
int trade_default[4]={3,6,9,12};
//持仓比
int trade_bi[1];

int bidpst=0;
int askpst=0;
int great_than_three_bid = 0;
int great_than_three_ask = 0;
int bi=30;
extern int realLongPstLimit;
extern int realShortPstLimit;
//卖出报单触发信号
extern int askCulTimes;
//买入报单触发信号
extern int bidCulTimes;
extern int pstalarm;
double settlementPrice = 10;
string sep = ";";
//保存交易数据
int tradeInfo[1];
void querySleep(){
    sleep(1);
}

void logEngine(){
    //cout<<"启动进程"<<endl;
    cout<<boosttoolsnamespace::CBoostTools::gbktoutf8("启动进程")<<endl;
	ofstream in;
	in.open(filepath,ios::app); //ios::trunc表示在打开文件前将文件清空,由于是写入,文件不存在则创建

    while(1)
    {
        LogMsg *pData;
        if(logqueue.empty()){
            sleep(100);
        }else if( logqueue.pop( pData ) ){
            //cout<<pData->getMsg()<<" "<<logqueue.empty()<<endl;
            string info;
            char cw[2048]={'\0'};
            info = pData->getMsg();
            info = getCurrentSystemTime()+" "+info;
            //cout<<"运行日志："<<info<<";size="<<info.size()<<";cap="<<endl;
            info.copy(cw,info.size(),0);
            //cout<<"日志："<<cw<<";size="<<strlen(cw)<<endl;
            in<<cw<<endl;
        }
         //cout<<"yigong="<<c<<endl;
    }
	in.close();//关闭文件
}

void marketdataEngine(){
	ofstream in;
	in.open(filepath_mk,ios::app); //ios::trunc表示在打开文件前将文件清空,由于是写入,文件不存在则创建
    int c = 0;
    while(1){
        LogMsg *pData;
        if(mkdataqueue.empty()){
            sleep(100);
        }else if(mkdataqueue.pop(pData)){
            c++;
            string info;
            char cw[2048]={'\0'};

            info = pData->getMsg();
            if(info.size() == 0){
                continue;
            }
            info = getCurrentSystemTime()+" "+info;
            //cout<<"行情日志："<<info<<";size="<<info.size()<<endl;
            info.copy(cw,info.size(),0);
            //cout<<"日志："<<cw<<";size="<<strlen(cw)<<endl;
            in<<cw<<endl;
        }
        //cout<<"yigong="<<c<<endl;
        //in.flush();
    }
	in.close();//关闭文件
}
// 获取系统的当前时间，单位微秒(us)
int64_t GetSysTimeMicros()
{
#ifdef _WIN32
	// 从1601年1月1日0:0:0:000到1970年1月1日0:0:0:000的时间(单位100ns)
#define EPOCHFILETIME   (116444736000000000UL)
	FILETIME ft;
	LARGE_INTEGER li;
	int64_t tt = 0;
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;
	// 从1970年1月1日0:0:0:000到现在的微秒数(UTC时间)
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
////获取当前系统时间
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
string getCloseMethod(string type){
    //平仓  开仓 '0';平仓 '1';平今 '3';平昨 '4';强平 '2'
    string orderoffset = "1";
    if("sell" == type){
        if(short_offset_flag == 1){
            orderoffset = "1";
        }else if(short_offset_flag == 3){
            orderoffset = "3";
        }else if(short_offset_flag == 4){
            orderoffset = "4";
        }
    }
    if("buy" == type){
        if(long_offset_flag == 1){
            orderoffset = "1";
        }else if(long_offset_flag == 3){
            orderoffset = "3";
        }else if(long_offset_flag == 4){
            orderoffset = "4";
        }
    }

    return orderoffset;
}
void initPriceGap(){
    cerr << "--->>> " << "initPriceGap"  << endl;
    //int gaps = 2*(max_price - min_price);
    double tmp_price = min_price;
    while(true){
        if(tmp_price <= max_price){
            double down_gap = (tmp_price - min_price)*2;
            double up_gap = down_gap + 2*tick;
            vector<double> gap_list;
            gap_list.push_back(down_gap);
            gap_list.push_back(up_gap);
//            gap_list[0] = down_gap;
//            gap_list[1] = up_gap;
            map_price_gap[tmp_price] = gap_list;
            char c_tmp[100];
            sprintf(c_tmp,"price=%f down=%f up=%f",tmp_price,gap_list[0],gap_list[1]);
            cout<<c_tmp<<endl;
            LOG(INFO) << c_tmp;
        }else{
            break;
        }
        tmp_price += tick;
    }
}

void OnRtnSHFEMarketData(CXeleShfeHighLevelOneMarketData *pDepthMarketData)
{
    //cout<<"----------------------->"<<getCloseMethod()<<endl;
    //处理行情
    int64_t start_time = GetSysTimeMicros();
    auto chro_start_time = boost::chrono::high_resolution_clock::now();
    string marketdata;
    stringstream ss;
    char instrumentID[17] = {'\0'};
    strcpy(instrumentID,pDepthMarketData->Instrument);
    int com = strcmp(singleInstrument,instrumentID);
    if(com != 0){
        //cout<<"want instrumentid="<<singleInstrument<<",actual instrumentid="<<instrumentID<<endl;
        string msg;
        char c_msg[300];
        sprintf(c_msg,"want instrumentid=%s,actual instrumentid=%s",singleInstrument,instrumentID);
        LOG(INFO)<<string(c_msg);
        return;
    }else{
        char c_p[20];
        sprintf(c_p,"%f",pDepthMarketData->LastPrice);
        cout<<"actual instrumentid="<<instrumentID<<",price="<<string(c_p)<<endl;
    }
    TXeleMdFtdcMillisecType UpdateMillisec = pDepthMarketData->UpdateMillisec;
    TXeleMdFtdcVolumeType Volume = pDepthMarketData->Volume;
    TXeleMdFtdcPriceType lastPrice = pDepthMarketData->LastPrice;
    char buf[10];
    sprintf(buf, "%.2f", lastPrice);
    sscanf(buf, "%lf", &lastPrice);
//    unsigned char s[20];
////    double dRetval;

//    sprintf(s,"%.*lf",2,lastPrice);
//    sscanf(s,"%lf",&lastPrice);
    TXeleMdFtdcMoneyType Turnover = pDepthMarketData->Turnover;
    TXeleMdFtdcLargeVolumeType OpenInterest = pDepthMarketData->OpenInterest;
    TXeleMdFtdcPriceType bidPrice = pDepthMarketData->BidPrice;
    TXeleMdFtdcPriceType askPrice = pDepthMarketData->AskPrice;
//    TXeleMdFtdcVolumeType BidVolume = pDepthMarketData->BidVolume;
//    TXeleMdFtdcVolumeType AskVolume = pDepthMarketData->AskVolume;
    ///最后修改时间
    TXeleMdFtdcTimeType UpdateTime;
    strcpy(UpdateTime, pDepthMarketData->UpdateTime);
    marketdata.append("UpdateTime=");
    marketdata.append(pDepthMarketData->UpdateTime);
    marketdata.append(sep);
    ///申买价一
    marketdata.append("BidPrice1=");
    ss << pDepthMarketData->BidPrice;
    string BidPrice1;
    ss >> BidPrice1;
    marketdata.append(BidPrice1);
    marketdata.append(sep);
    ss.clear();
    ///申买量一
    char char_bv1[30] = {'\0'};
    sprintf(char_bv1,"%d",pDepthMarketData->BidVolume);
    marketdata.append("BidVolume1=");
    marketdata.append(char_bv1);
    marketdata.append(sep);
    ///申卖价一
    marketdata.append("AskPrice1=");
    ss << pDepthMarketData->AskPrice;
    string AskPrice1;
    ss >> AskPrice1;
    marketdata.append(AskPrice1);
    marketdata.append(sep);
    ss.clear();
    ///申卖量一
    char char_sv1[30] = {'\0'};
    sprintf(char_sv1,"%d",pDepthMarketData->AskVolume);
    marketdata.append("AskVolume1=");
    marketdata.append(char_sv1);
    marketdata.append(sep);
    marketdata.append("InstrumentID=");
    marketdata.append(instrumentID);
    marketdata.append(sep);
    ///最新价
    marketdata.append("LastPrice=");
    ss << pDepthMarketData->LastPrice;
    string lastprice;
    ss >> lastprice;
    marketdata.append(lastprice);
    marketdata.append(sep);
    ///数量
//    TUstpFtdcVolumeType	Volume = pDepthMarketData->Volume;
    char char_vol[30] = {'\0'};
    sprintf(char_vol,"%d",Volume);
    marketdata.append("Volume=");
    marketdata.append(char_vol);
    marketdata.append(sep);
    ///持仓量
//    TUstpFtdcLargeVolumeType	OpenInterest = pDepthMarketData->OpenInterest;
    char char_opi[30] = {'\0'};
    sprintf(char_opi,"%d",OpenInterest);
    marketdata.append("OpenInterest=");
    marketdata.append(char_opi);
    marketdata.append(sep);


    ///最后修改毫秒
//    TUstpFtdcMillisecType	UpdateMillisec = pDepthMarketData->UpdateMillisec;
    char char_ums[30] = {'\0'};
    sprintf(char_ums,"%d",UpdateMillisec);
    marketdata.append("UpdateMillisec=");
    marketdata.append(char_ums);
    marketdata.append(sep);
    ///当日均价
    marketdata.append("turnover=");
    ss << Turnover;
    string str_Turnover;
    ss >> str_Turnover;
    marketdata.append(str_Turnover);
    marketdata.append(sep);
    ss.clear();
    LogMsg *logmsg = new LogMsg();
    logmsg->setMsg(marketdata);
    mkdataqueue.push(logmsg);


    if((realLongPstLimit + realShortPstLimit) >= pstalarm){
        cout<<"pstatalam "<<endl;
    }
    if(previous_price == 0){
        previous_price = lastPrice;
        return;
    }
    vector<double> gap_list;
    //cant find
    if(map_price_gap.find(lastPrice) == map_price_gap.end()){
        string msg = "can not find map_price_gap item: price=" + boost::lexical_cast<string>(lastPrice);
        cout<<msg<<endl;
//        LogMsg *logmsg = new LogMsg();
//        logmsg->setMsg(msg);
//        logqueue.push(logmsg);
        LOG(INFO)<<msg;
        return;
    }else{
        unordered_map<double,vector<double>>::iterator map_it = map_price_gap.find(lastPrice);
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
    char c_lastp[20];
    sprintf(c_lastp,"%f",lastPrice);
    char c_pre_p[20];
    sprintf(c_pre_p,"%f",previous_price);
    char c_cur_gap[20];
    sprintf(c_cur_gap,"%f",gap);
    char c_last_gap[20];
    sprintf(c_last_gap,"%f",last_gap);
    char tmp_msg[256];
    sprintf(tmp_msg,"currPrice=%s,prePrice=%s,currGap=%s,lastGap=%s",c_lastp,c_pre_p,c_cur_gap,c_last_gap);
    //cout<<tmp_msg<<endl;
    LOG(INFO)<<string(tmp_msg);
    previous_price = lastPrice;
    if(last_gap == -1){
        last_gap = gap;
        //cout<<"init gap"<<endl;
        return;
    }else if(last_gap == gap){
        //cout<<"last_gap==gap"<<endl;
        return;
    }else if(last_gap < gap){
        down_culculate = 0;
        up_culculate += 1;
    }else if(last_gap > gap){
        down_culculate += 1;
        up_culculate = 0;
    }
    //开平
    char char_orderoffset[3]={'\0'};
    string orderoffset;
    last_gap = gap;
    //
    char c_upcul[5]={'\0'};
    char c_downcul[5]={'\0'};
    char c_price[20]={'\0'};
    if(up_culculate >= askCulTimes){
        //sell
        char char_orderdir[] = "1";
        //开平判断
        if(shortPstIsClose == 1){//开仓
            orderoffset = "0";
        }else if(shortPstIsClose == 2){//平仓  开仓 '0';平仓 '1';平今 '3';平昨 '4';强平 '2'
            orderoffset = getCloseMethod("sell");
        }
        strcpy(char_orderoffset,orderoffset.c_str());
        //cout<<"sell"<<endl;

        if(isTest == 1){
            double orderpirce = 0;
            if((askPrice - tick) > bidPrice){
                orderpirce = askPrice - tick;
            }else{
                orderpirce = bidPrice;
            }
            sprintf(c_price,"%f",orderpirce);
            pUserSpi->md_orderinsert(orderpirce,char_orderdir,char_orderoffset,instrumentID,default_volume);
        }else if(isTest == 2){
            sprintf(c_price,"%f",max_price);
            pUserSpi->md_orderinsert(max_price,char_orderdir,char_orderoffset,instrumentID,default_volume);
        }
        char c_msg[300];
        sprintf(c_upcul,"%d",up_culculate);
        sprintf(c_downcul,"%d",down_culculate);
        sprintf(c_msg,"order: instrumentid=%s,direction=%s,offsetflag=%s,price=%s,up_culculate=%s,down_culculate=%s",
                singleInstrument,char_orderdir,char_orderoffset,c_price,c_upcul,c_downcul);
        LOG(INFO)<<string(c_msg);
        LogMsg *tradeMsg = new LogMsg();
        tradeMsg->setMsg(string(c_msg));
        logqueue.push(tradeMsg);
    }else if(down_culculate >= bidCulTimes){
        //买
        char char_orderdir[] = "0";
        //开平判断
        if(longPstIsClose == 1){//开仓
            orderoffset = "0";
        }else if(longPstIsClose == 2){//平仓  开仓 '0';平仓 '1';平今 '3';平昨 '4';强平 '2'
            orderoffset = getCloseMethod("buy");
        }
        strcpy(char_orderoffset,orderoffset.c_str());
        //cout<<"buy"<<endl;

        if(isTest == 1){
            double orderpirce = 0;

            if((bidPrice + tick) < askPrice){
                orderpirce = bidPrice + tick;
            }else{
                orderpirce = askPrice;
            }
            sprintf(c_price,"%f",orderpirce);
            pUserSpi->md_orderinsert(orderpirce,char_orderdir,char_orderoffset,instrumentID,default_volume);
        }else if(isTest == 2){
            sprintf(c_price,"%f",min_price);
            pUserSpi->md_orderinsert(min_price,char_orderdir,char_orderoffset,instrumentID,default_volume);
        }
        char c_msg[300];
        sprintf(c_upcul,"%d",up_culculate);
        sprintf(c_downcul,"%d",down_culculate);

        sprintf(c_msg,"order: instrumentid=%s,direction=%s,offsetflag=%s,price=%s,up_culculate=%s,down_culculate=%s",
                singleInstrument,char_orderdir,char_orderoffset,c_price,c_upcul,c_downcul);
        LOG(INFO)<<string(c_msg);
        LogMsg *tradeMsg = new LogMsg();
        tradeMsg->setMsg(string(c_msg));
        logqueue.push(tradeMsg);
    }

    //处理行情
    int64_t end1 = GetSysTimeMicros();
    string msg;

    /*string msg = "处理信号花费:" + string(GetDiffTime(end1,start_time));
    LogMsg logmsg;
    logmsg.setMsg(msg);
    logqueue.push(&logmsg)*/;


//    logmsg.setMsg(marketdata);
//    mkdataqueue.push(&logmsg);
    //处理行情
    int64_t end2 = GetSysTimeMicros();
    auto chro_end_time = boost::chrono::high_resolution_clock::now();
    auto pro_time = boost::chrono::duration<double>(chro_end_time - chro_start_time).count();
//    stringstream timss;
//    timss<<pro_time;
//    cout<<timss.str()<<endl;
//    cout<<marketdata<<endl;
    //msg = "记录行情花费:" + string(GetDiffTime(end2,end1)) + ";实际处理时间：" + timss.str();
   // LOG(INFO)<<msg;
}
