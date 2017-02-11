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
//gap list map
unordered_map<double,double[]> map_price_gap;
//preious price
double previous_price = 0;
//上涨
int up_culculate = 0;
//下跌
int down_culculate = 0;
//上一次价格所处的区间
int price_gap = -1;
//报单触发信号
int cul_times = 2;
//盈利数字
double profit = 0;
//交易次数
unsigned int trade_num = 0;
// 下单量
int trade_default[];
//持仓比
int trade_bi=[];
int bidpst=0;
int askpst=0;
int great_than_three_bid = 0;
int great_than_three_ask = 0;
int bi=30;
int default_volume = 1;
double settlementPrice = 10;
//保存交易数据
int tradeInfo[];
void logEngine(){
    //cout<<"启动进程"<<endl;
    cout<<boosttoolsnamespace::CBoostTools::gbktoutf8("启动进程")<<endl;
	ofstream in;
	in.open(filepath,ios::app); //ios::trunc表示在打开文件前将文件清空,由于是写入,文件不存在则创建
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
            cout<<"运行日志："<<info<<";size="<<info.size()<<";cap="<<endl;
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
//            cout<<"行情日志："<<info<<";size="<<info.size()<<endl;
//            info.copy(cw,info.size(),0);
//            //cout<<"日志："<<cw<<";size="<<strlen(cw)<<endl;
//            in<<cw<<endl;
//        }
//        //cout<<"yigong="<<c<<endl;
//        //in.flush();
//	}
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
string getCloseMethod(){
    //平仓  开仓 '0';平仓 '1';平今 '3';平昨 '4';强平 '2'
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
    //处理行情
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
        //开平判断
        if(isclose == 1){//开仓
            orderoffset = "0";
        }else if(isclose == 2){//平仓  开仓 '0';平仓 '1';平今 '3';平昨 '4';强平 '2'
            orderoffset = getCloseMethod();
        }
        strcpy(char_orderoffset,orderoffset.c_str());
        pUserSpi->md_orderinsert(askPrice,char_orderdir,char_orderoffset,instrumentID,default_volume);
    }else if(down_culculate >= cul_times){
        //买
        char char_orderdir[] = "0";
        //开平判断
        if(isclose == 1){//开仓
            orderoffset = "0";
        }else if(isclose == 2){//平仓  开仓 '0';平仓 '1';平今 '3';平昨 '4';强平 '2'
            orderoffset = getCloseMethod();
        }
        strcpy(char_orderoffset,orderoffset.c_str());
        pUserSpi->md_orderinsert(askPrice,char_orderdir,char_orderoffset,instrumentID,default_volume);
    }

    //处理行情
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
    /*string msg = "处理信号花费:" + string(GetDiffTime(end1,start_time));
    LogMsg logmsg;
    logmsg.setMsg(msg);
    logqueue.push(&logmsg)*/;

    ///最后修改时间
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
    char char_bv1[20] = {'\0'};
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
    char char_sv1[20] = {'\0'};
    sprintf(char_sv1,"%d",pDepthMarketData->AskVolume);
    marketdata.append("AskVolume1=");
    marketdata.append(char_sv1);
    marketdata.append(sep);
    marketdata.append("InstrumentID=");
    marketdata.append(pDepthMarketData->InstrumentID);
    marketdata.append(sep);
    ///最新价
    marketdata.append("LastPrice=");
    ss << pDepthMarketData->LastPrice;
    string lastprice;
    ss >> lastprice;
    marketdata.append(lastprice);
    marketdata.append(sep);
    ///数量
    TThostFtdcVolumeType	Volume = pDepthMarketData->Volume;
    char char_vol[20] = {'\0'};
    sprintf(char_vol,"%d",Volume);
    marketdata.append("Volume=");
    marketdata.append(char_vol);
    marketdata.append(sep);
    ///持仓量
    TThostFtdcLargeVolumeType	OpenInterest = pDepthMarketData->OpenInterest;
    char char_opi[20] = {'\0'};
    sprintf(char_opi,"%d",OpenInterest);
    marketdata.append("OpenInterest=");
    marketdata.append(char_opi);
    marketdata.append(sep);


    ///最后修改毫秒
    TThostFtdcMillisecType	UpdateMillisec = pDepthMarketData->UpdateMillisec;
    char char_ums[20] = {'\0'};
    sprintf(char_ums,"%d",UpdateMillisec);
    marketdata.append("UpdateMillisec=");
    marketdata.append(char_ums);
    marketdata.append(sep);

    /*
    ///申买价二
    TThostFtdcPriceType	BidPrice2;
    ///申买量二
    TThostFtdcVolumeType	BidVolume2;
    ///申卖价二
    TThostFtdcPriceType	AskPrice2;
    ///申卖量二
    TThostFtdcVolumeType	AskVolume2;
    ///申买价三
    TThostFtdcPriceType	BidPrice3;
    ///申买量三
    TThostFtdcVolumeType	BidVolume3;
    ///申卖价三
    TThostFtdcPriceType	AskPrice3;
    ///申卖量三
    TThostFtdcVolumeType	AskVolume3;
    ///申买价四
    TThostFtdcPriceType	BidPrice4;
    ///申买量四
    TThostFtdcVolumeType	BidVolume4;
    ///申卖价四
    TThostFtdcPriceType	AskPrice4;
    ///申卖量四
    TThostFtdcVolumeType	AskVolume4;
    ///申买价五
    TThostFtdcPriceType	BidPrice5;
    ///申买量五
    TThostFtdcVolumeType	BidVolume5;
    ///申卖价五
    TThostFtdcPriceType	AskPrice5;
    ///申卖量五
    TThostFtdcVolumeType	AskVolume5;
    */
    ///当日均价
    marketdata.append("AveragePrice=");
    ss << pDepthMarketData->AveragePrice;
    string AveragePrice;
    ss >> AveragePrice;
    marketdata.append(AveragePrice);
    marketdata.append(sep);
    ss.clear();
//    logmsg.setMsg(marketdata);
//    mkdataqueue.push(&logmsg);
    //处理行情
    int64_t end2 = GetSysTimeMicros();
    auto chro_end_time = boost::chrono::high_resolution_clock::now();
    auto pro_time = boost::chrono::duration<double>(chro_end_time - chro_start_time).count();
    stringstream timss;
    timss<<pro_time;
    cout<<timss.str()<<endl;
    cout<<marketdata<<endl;
    //msg = "记录行情花费:" + string(GetDiffTime(end2,end1)) + ";实际处理时间：" + timss.str();
    LOG(INFO)<<msg;
}
