// testTraderApi.cpp : 定义控制台应用程序的入口点。
//
#include "PublicFuncs.h"
#include "TraderSpi.h"
#include "USTPFtdcTraderApi.h"
#include "TraderSpi.h"
#include <iostream>
#include <string.h>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <list>
#include "Trade.h"
#include "globalutil.h"
#include <cstdio>
using namespace std;
unordered_map<string,unordered_map<string,int>> positionmap;
// UserApi对象
CUstpFtdcTraderApi* pUserApi;
CTraderSpi* pUserSpi;
boost::thread_group thread_log_group;
// 配置参数
char  MD_FRONT_ADDR[] = "tcp://116.228.171.216:61213";// 前置地址
char  FRONT_ADDR[] = "tcp://116.228.171.216:61205";// 前置地址 新湖期货
TUstpFtdcBrokerIDType  BROKER_ID={'\0'};				// 经纪公司代码
TUstpFtdcUserIDType INVESTOR_ID = {'\0'};			// 投资者代码
TUstpFtdcPasswordType  PASSWORD = {"\0"};			// 用户密码
//TThostFtdcInstrumentIDType INSTRUMENT_ID = "IF1509";	// 合约代码
//TThostFtdcDirectionType	DIRECTION = THOST_FTDC_D_Sell;	// 买卖方向
//TThostFtdcPriceType	LIMIT_PRICE = 38850;				// 价格
//char *ppInstrumentID[] = {"m1705-C-2450"};// 行情订阅列表
char **ppInstrumentID;// 行情订阅列表
//单一合约
char singleInstrument[30]={'\0'};
vector<string> quoteList ;
int iInstrumentID = 1;									// 行情订阅数量
//连接到服务器端的客户数量
int customercount=0;
// 请求编号
int iRequestID = 0;
int g_nOrdLocalID = 1;
//报单触发信号
int cul_times = 0;
//跌停价格
double min_price = 0;
//涨停价格
double max_price = 0;
//价格变动单位
double tick = 0;
//卖出价格振幅
int tickSpreadSell = 3;
//买入价格振幅
int tickSpreadBuy = 3;
//买平标志,1开仓；2平仓
int longPstIsClose = 1;
int shortPstIsClose = 1;
//价格浮动倍数
int bidmultipy = 1;
//价格浮动倍数卖
int askmultipy = 1;
//持仓预警值
int pstalarm = 3;
//默认下单量
int default_volume = 1;
//买卖价差比较值
int bid_ask_spread = 80;
//成交量基数
int trade_volume = 35;
//持仓限额
int limit_volume = 50;
///日志消息队列
boost::lockfree::queue<LogMsg*> logqueue(1000);
//存放行情消息队列
boost::lockfree::queue<LogMsg*> mkdataqueue(1000);
////组合开平标志: 开仓 '0';平仓 '1';平今 '3';平昨 '4';强平 '2'
int offset_flag = 1;
//前置id
char char_front_id[12] = {'\0'};
char char_session_id[20] = {'\0'};
string str_front_id;
string str_sessioin_id;
//longpstlimit
int longpstlimit = 0;
//shortpstlimit
int shortpstlimit = 0;
//g_nOrdLocalID对应关系
unordered_map<string,unordered_map<string,int64_t>> seq_map_g_nOrdLocalID;
//ordersysid对应关系
unordered_map<string,string> seq_map_ordersysid;
//初始化是否处理成交回报
bool isrtntradeprocess = false;
void TradeProcess::startTrade()
{
    //readInsList();
    datainit();
    //initPriceGap();
    string str = boosttoolsnamespace::CBoostTools::gbktoutf8("经纪公司代码");
    cout<<str<<endl;
    cout<<"经纪公司代码="<<BROKER_ID<<endl;
    cout<<"投资者代码="<<INVESTOR_ID<<endl;
    cout<<"用户密码="<<PASSWORD<<endl;
    cout<<"交易前置="<<FRONT_ADDR<<endl;
    cout<<"行情前置="<<MD_FRONT_ADDR<<endl;
    cout<<"报单触发信号="<<cul_times<<endl;
//    cout<<"跌停价格="<<min_price<<endl;
//    cout<<"涨停价格="<<max_price<<endl;
    cout<<"持仓预警值="<<pstalarm<<endl;
    cout<<"默认下单量="<<default_volume<<endl;
    cout<<"单一合约="<<singleInstrument<<endl;
    //cout<<"买卖价差比较值="<<bid_ask_spread<<endl;
//    cout<<"合约为："<<singleInstrument<<endl;
//    for(int i = 0;i < quoteList.size();i++){
//        //str_inslist = str_inslist + string(ppInstrumentID[i]) + " ";
//        cout<<ppInstrumentID[i]<<endl;
//    }
//    cout<<"合约数量为:"<<iInstrumentID<<endl;
////recordRunningMsg(msg);
//        //cout<<logmsg.getMsg()<<endl;
//    }
//    getchar();
    cout<<"=========================================="<<endl;
    cout<<"是否继续?0 否，1是"<<endl;
    int isok;
    cin>>isok;
    if(isok != 1){
        exit(0);
    }
    initThread(0);
    tradeinit();
//    cout<<"是否继续?0 否，1是"<<endl;
//    cin>>isok;
//    if(isok != 1){
//        exit(0);
//    }
}
/************************************************************************/
/* 初始化参数列表                                                                     */
/************************************************************************/

void TradeProcess::datainit(){
    ifstream myfile("config/global.properties");
    if(!myfile){
        cout<<"读取global.properties文件失败"<<endl;
    }else{
        string str;
        while(getline(myfile,str)){
            int pos = str.find("#");
            if(pos == 0){
                cout<<"注释:"<<str<<endl;
            }else{
                vector<string> vec = split(str,"=");
                cout<<str<<endl;
                //cout<<vec[0]<<"=="<<vec[1]<<endl;
                if("investorid"==vec[0]){
                    strcpy(INVESTOR_ID,vec[1].c_str());
                }else if("brokerid"==vec[0]){
                    strcpy(BROKER_ID,vec[1].c_str());
                }else if("default_volume"==vec[0]){
                    default_volume = boost::lexical_cast<int>(vec[1]);
                }else if("password"==vec[0]){
                    strcpy(PASSWORD,vec[1].c_str());
                }else if("tradeFrontAddr"==vec[0]){
                    strcpy(FRONT_ADDR,vec[1].c_str());
                }else if("singleInstrument"==vec[0]){
                    strcpy(singleInstrument,vec[1].c_str());
                }else if("mdFrontAddr"==vec[0]){
                    strcpy(MD_FRONT_ADDR,vec[1].c_str());
                }else if("cul_times"==vec[0]){
                    cul_times = boost::lexical_cast<int>(vec[1]);
                }else if("min_price"==vec[0]){
                    min_price = boost::lexical_cast<double>(vec[1]);
                }else if("max_price"==vec[0]){
                    max_price = boost::lexical_cast<double>(vec[1]);
                }else if("pstalarm"==vec[0]){
                    pstalarm = boost::lexical_cast<int>(vec[1]);
                    longpstlimit = pstalarm;
                    shortpstlimit = pstalarm;
                }else if("instrumentList" == vec[0]){
                    /************************************************************************/
                    /* 如果读到      instrumentList，则保存到本程序中                                                               */
                    /************************************************************************/

                    const char *expr = vec[1].c_str();
                    //cout<<expr<<endl;
                    char *inslist = new char[strlen(expr)+1];
                    strcpy(inslist, expr);
                    //cout<<inslist<<endl;
                    const char * splitlt = ","; //分割符号
                    char *plt = 0;
                    plt = strtok(inslist,splitlt);
                    while(plt!=NULL) {
                        quoteList.push_back(plt);
                        //cout<<plt<<endl;
                        plt = strtok(NULL,splitlt); //指向下一个指针
                    }
                    //动态分配字符数组
                    ppInstrumentID = new char*[quoteList.size()];
                    for(int i = 0,j = quoteList.size();i < j;i ++){
                        const char * tt2 = quoteList[i].c_str();
                        char* pid = new char[strlen(tt2) + 1];
                        strcpy(pid,tt2);
                        ppInstrumentID[i] = pid;
                        cout<<ppInstrumentID[i]<<endl;
                    }
                    iInstrumentID = quoteList.size();
                    //cout<< ppInstrumentID_tmp[0]<<"="<<ppInstrumentID_tmp[1]<<" "<<iInstrumentID<<endl;
                }

            }

        }
    }

}

void TradeProcess::tradeinit(){
    cout<<"start to init tradeapi"<<endl;
    CUstpFtdcTraderApi *pTrader = CUstpFtdcTraderApi::CreateFtdcTraderApi("");
    g_puserapi=pTrader;
    pUserApi = pTrader;
    CTraderSpi spi(pTrader);
    pTrader->RegisterFront(FRONT_ADDR);
    pTrader->SubscribePrivateTopic(USTP_TERT_RESTART);
    pTrader->SubscribePublicTopic(USTP_TERT_RESTART);
    pTrader->RegisterSpi(&spi);
    pTrader->Init();

    pTrader->Join();
    pTrader->Release();
    cout<<"end init api"<<endl;
//    pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi("log1");			// 创建UserApi
//    pUserSpi = new CTraderSpi();
//    pUserApi->RegisterSpi((CThostFtdcTraderSpi*)pUserSpi);			// 注册事件类
//    pUserApi->SubscribePublicTopic(THOST_TERT_RESUME);					// 注册公有流
//    pUserApi->SubscribePrivateTopic(THOST_TERT_RESUME);					// 注册私有流
//    pUserApi->RegisterFront(FRONT_ADDR);							// connect
//    pUserApi->Init();
//    cout<<"end init tradeapi"<<endl;
//    pUserApi->Join();

}
void create(){
    while(true){
        sleep(1);
        LogMsg *logmsg = new LogMsg();
        logmsg->setMsg("msg");
        mkdataqueue.push(logmsg);
    }

}

void TradeProcess::initThread(int sendtype)
{
    printf("经纪公司编号=[%s]\n","pInvestorMargin->BrokerID");
    thread_log_group.create_thread(marketdataEngine);
    //thread_log_group.create_thread(create);
//    thread_log_group.create_thread(test);
    //thread_log_group.create_thread(marketdataEngine);
    //thread_log_group.join_all();
    //HANDLE loghdl = CreateThread(NULL,0,logEngine,NULL,0,NULL);
    /**

    CloseHandle(loghdl);
    if (sendtype == 0){
        HANDLE hd1 = CreateThread(NULL,0,sendByClient,NULL,0,NULL);
        CloseHandle(hd1);
    } else if(sendtype == 1){
        HANDLE hd1 = CreateThread(NULL,0,tradeServer,NULL,0,NULL);
        CloseHandle(hd1);
    }
    **/
}
