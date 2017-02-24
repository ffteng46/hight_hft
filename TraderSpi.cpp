// TraderSpi.cpp: implementation of the CTraderSpi class.
//
//////////////////////////////////////////////////////////////////////

#include "TraderSpi.h"
#include <unistd.h>
#include <iostream>
#include "globalutil.h"
#include "XeleFtdcMduserApi.h"
#include <fstream>
#include <unordered_map>
#include <string.h>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/algorithm/string.hpp>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace std;
extern unordered_map<string,unordered_map<string,int>> positionmap;
// UserApi对象
extern CUstpFtdcTraderApi* pUserApi;
extern FILE * g_fpRecv;
extern char  FRONT_ADDR[];// 前置地址 新湖期货
extern char singleInstrument[30];
// 请求编号
extern int iRequestID;
extern int g_nOrdLocalID;
extern double tick;
extern int isclose;
extern int long_offset_flag;
extern int short_offset_flag;
//卖出报单触发信号
extern int askCulTimes;
//买入报单触发信号
extern int bidCulTimes;
//上涨
extern int up_culculate;
//下跌
extern int down_culculate;
//报单触发信号
extern int cul_times;
extern int limit_volume;
extern bool isrtntradeprocess;
//跌停价格
extern double min_price;
//涨停价格
extern double max_price;
//价格变动单位
extern double tick;
extern boost::lockfree::queue<LogMsg*> logqueue;
extern unordered_map<string,unordered_map<string,int64_t>> seq_map_orderref;
extern unordered_map<string,string> seq_map_ordersysid;
//买平标志,1开仓；2平仓
extern int longPstIsClose;
extern int shortPstIsClose;
//positionmap可重入锁
boost::recursive_mutex pst_mtx;
//orderinsertkey与ordersysid对应关系锁
boost::recursive_mutex order_mtx;
//longpstlimit
extern int longpstlimit;
//shortpstlimit
extern int shortpstlimit;
//记录时间
int ret = 0;
int start_process = 0;
int realLongPstLimit = 0;
int realShortPstLimit = 0;
int lastABSSpread = 0;
int firstGap = 2;
int secondGap = 5;
extern TUstpFtdcBrokerIDType  BROKER_ID;				// 经纪公司代码
extern TUstpFtdcUserIDType INVESTOR_ID;			// 投资者代码
extern TUstpFtdcPasswordType  PASSWORD ;			// 用户密码
CTraderSpi::CTraderSpi(CUstpFtdcTraderApi *pTrader):m_pUserApi(pTrader)
{

}

CTraderSpi::~CTraderSpi()
{

}

//记录时间
int CTraderSpi::md_orderinsert(double price,char *dir,char *offset,char * ins,int ordervolume){
    TUstpFtdcUserOrderLocalIDType UserOrderLocalID;
   // cout<<"1"<<endl;
//    char InstrumentID[31];
//    strcpy(InstrumentID,ins);
    char Direction[2];
    strcpy(Direction,dir);
    ///投机 '1';套保'3'
    char HedgeFlag[]="1";
    //组合开平标志: 开仓 '0';平仓 '1';平今 '3';平昨 '4';强平 '2'
    char OffsetFlag[2];
    strcpy(OffsetFlag,offset);
    ///价格 double
    TUstpFtdcPriceType Price = price;
    //开仓手数
    int Volume = ordervolume;
    //报单引用编号
    //sprintf(UserOrderLocalID,"%d",++g_nOrdLocalID);
    //cout<<"------->"<<ORDER_REF<<endl;
    //cout<<"2"<<endl;
    //++g_nOrdLocalID;
    //报单结构体
    CUstpFtdcInputOrderField req;
    ///经纪公司代码
    strcpy(req.BrokerID, BROKER_ID);
    ///投资者代码
    strcpy(req.InvestorID, INVESTOR_ID);
    ///合约代码// UserApi对象
    //CUstpFtdcTraderApi* pUserApi;
    strcpy(req.InstrumentID, ins);
    //cout<<"instrumentid="<<string(req.InstrumentID)<<endl;
    ///报单引用
    sprintf(req.UserOrderLocalID,"%d",++g_nOrdLocalID);
    //strcpy(req.UserOrderLocalID, UserOrderLocalID);
   // cout<<"3"<<endl;
    ///用户代码
    //	TUstpFtdcUserIDType	UserID;
    strcpy(req.UserID,INVESTOR_ID);
    //cout<<"31"<<endl;
    ///报单价格条件: 限价
    req.OrderPriceType = USTP_FTDC_OPT_LimitPrice;
    ///买卖方向:
    ///
//    strcpy(&req.Direction,dir);
    req.Direction = dir[0];
    //cout<<"32"<<endl;
    ///组合开平标志: 开仓
    //strcpy(&req.OffsetFlag ,offset);
    req.OffsetFlag = offset[0];
    //cout<<"33"<<endl;
    ///组合投机套保标志
    //strcpy(&req.HedgeFlag,HedgeFlag);
    req.HedgeFlag = HedgeFlag[0];
    //cout<<"34"<<endl;
    ///价格
    req.LimitPrice = Price;
    //cout<<"4"<<endl;
    strcpy(req.ExchangeID,"SHFE");
    ///数量: 1
    req.Volume = Volume;
    ///有效期类型: 当日有效
    //req.TimeCondition = THOST_FTDC_TC_GFD;
    req.TimeCondition = USTP_FTDC_TC_IOC;
    ///GTD日期
    //TUstpFtdcDateType	GTDDate;
    strcpy(req.GTDDate,"");
    ///成交量类型: 任何数量
    req.VolumeCondition = USTP_FTDC_VC_AV;
    ///最// 请求编号
    //int iRequestID = 0;
    //小成交量: 1
    req.MinVolume = 1;
    ///触发条件: 立即
    //req.ContingentCondition = THOST_FTDC_CC_Immediately;
    ///止损价
    //TUstpFtdcPriceType	StopPrice;
    //cout<<"5"<<endl;
    req.StopPrice = 0;
    ///强平原因: 非强平
    req.ForceCloseReason = USTP_FTDC_FCR_NotForceClose;
    ///自动挂起标志: 否
    req.IsAutoSuspend = 0;
    ///业务单元
    //	TUstpFtdcBusinessUnitType	BusinessUnit;
    strcpy(req.BusinessUnit,"");
    ///请求编号
    //	TUstpFtdcRequestIDType	RequestID;
    ///用户强评标志: 否
    //req.UserForceClose = 0;///经纪公司代码
    //
    //cout<<"6"<<endl;
    //Show(req);
    int nRequestID = ++iRequestID;
//    char char_order_index[10]={'\0'};
//    sprintf(char_order_index,"%d",nRequestID);
//    //req.RequestID = nRequestID;

//    nRequestID = ++iRequestID;
//    char char_query_index[10]={'\0'};
//    sprintf(char_query_index,"%d",nRequestID);

    //下单开始时间
//    int64_t ist_time = GetSysTimeMicros();
    //orderinsertkey
//    string str_osk = str_front_id + str_sessioin_id + string(req.g_nOrdLocalID);
//    unordered_map<string,int64_t> tmpmap ;
//    tmpmap["ist_time"] = ist_time;
//    seq_map_g_nOrdLocalID[str_osk] = tmpmap;
    //委托类操作，使用客户端定义的请求编号格式
    //cout<<"order"<<endl;
    int iResult = pUserApi->ReqOrderInsert(&req,++iRequestID);
    cerr << "--->>> ReqOrderInsert:" << ((iResult == 0) ? "success" : "failed") << endl;
//    string msg = "order_requestid=" + string(char_order_index) + ";orderinsert_requestid=" + string(char_query_index);
//    //LogMsg lmsg;
//    //lmsg.setMsg(msg);;
//    //logqueue.push(&lmsg);
//    LOG(INFO) << msg;
    return 0;
}
void CTraderSpi::OnFrontConnected()
{
    cerr << "--->>> " << "OnFrontConnected" << endl;
    cout<<"交易前置="<<FRONT_ADDR<<endl;

    ///用户登录请求;
    ReqUserLogin();

}
void CTraderSpi::ReqUserLogin()
{
    cout<<PASSWORD<<":"<<BROKER_ID<<" "<<INVESTOR_ID<<" "<<PASSWORD<<endl;
    char char_msg[1024];
    sprintf(char_msg, "发送用户登录请求:BROKER_ID:%s;INVESTOR_ID:%s;PASSWORD:%s",BROKER_ID, INVESTOR_ID,PASSWORD);
    string msg=string(char_msg);
    LOG(INFO)<<msg;
    cout << "BROKER_ID:"<< BROKER_ID<<";INVESTOR_ID:"<<INVESTOR_ID<<";PASSWORD="<<PASSWORD <<endl;
    querySleep();
    cerr << "hello" <<endl;
    CUstpFtdcReqUserLoginField reqUserLogin;
    memset(&reqUserLogin,0,sizeof(CUstpFtdcReqUserLoginField));
//    strcpy(reqUserLogin.BrokerID,BROKER_ID);
//    strcpy(reqUserLogin.UserID, INVESTOR_ID);
//    strcpy(reqUserLogin.Password, PASSWORD);
    strcpy(reqUserLogin.BrokerID,BROKER_ID);
    strcpy(reqUserLogin.UserID, INVESTOR_ID);
    strcpy(reqUserLogin.Password, PASSWORD);
    strcpy(reqUserLogin.UserProductInfo,"");
    int iResult = pUserApi->ReqUserLogin(&reqUserLogin, ++iRequestID);
    printf("请求登录，BrokerID=[%s]UserID=[%s]\n",BROKER_ID,INVESTOR_ID);
    cerr << "--->>> 发送用户登录请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
#ifdef WIN32
    Sleep(100);
#else
    usleep(100);
#endif

}
//前置了断开
void CTraderSpi:: OnFrontDisconnected(int nReason)
{
    cerr << "OnFrontDisconnected "<<FRONT_ADDR ;
    char char_msg[1028] = {'\0'};
    if(nReason == 4097){
        cerr << "--->>>前置连接失败： Reason = " << nReason <<",网络读失败";
        sprintf(char_msg,"--->>>前置连接失败： Reason = %d,网络读失败",nReason);
    }else if(nReason == 4098){
        cerr << "--->>>前置连接失败： Reason = " << nReason <<",网络写失败";
    }if(nReason == 8193){
        cerr << "--->>>前置连接失败： Reason = " << nReason <<",接受心跳超时";
    }if(nReason == 8194){
        cerr << "--->>>前置连接失败： Reason = " << nReason <<",发送心跳失败";
    }if(nReason == 8195){
        cerr << "--->>>前置连接失败： Reason = " << nReason <<",收到错误报文";
    }
//    LogMsg d;
    string msg2 = boosttoolsnamespace::CBoostTools::gbktoutf8(string(char_msg));
    /*d.setMsg(msg2);
//    logqueue.push( &d */
    LOG(INFO)<<msg2;
    sleep(10);
}
void CTraderSpi::OnRspUserLogin(CUstpFtdcRspUserLoginField *pRspUserLogin, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "--->>> " << "OnRspUserLogin" << endl;
    //printf("登录失败...错误原因：%s\n",pRspInfo->ErrorMsg);
    if (bIsLast && !IsErrorRspInfo(pRspInfo))
    //if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
        g_nOrdLocalID=atoi(pRspUserLogin->MaxOrderLocalID);
        if(g_nOrdLocalID < 90000000){
            g_nOrdLocalID += 90000000;
        }
        printf("-----------------------------\n");
        printf("登录成功，最大本地报单号:%d\n",g_nOrdLocalID);
        printf("-----------------------------\n");
        //请求响应日志
        char char_msg[1024] = {'\0'};
        sprintf(char_msg,"--->>>登陆成功,获取当前报单引用 =%d",g_nOrdLocalID );
        string msg(char_msg);
        LOG(INFO)<<msg;
        //ReqQryInvestorPosition();
        ///投资者结算结果确认;
        /// extern char singleInstrument[30];

        ReqInvestorAccount();
        cout<<msg<<endl;

    }else{
        printf("-----------------------------\n");
        printf("登录失败...错误原因：%s\n",pRspInfo->ErrorMsg);
        printf("-----------------------------\n");
        return;
    }

    //StartAutoOrder();
}
void CTraderSpi::ReqInvestorAccount(){
    CUstpFtdcQryInvestorAccountField QryInvestorAcc;
    memset(&QryInvestorAcc,0,sizeof(CUstpFtdcQryInvestorAccountField));
    strcpy(QryInvestorAcc.BrokerID,BROKER_ID);
    strcpy(QryInvestorAcc.InvestorID,INVESTOR_ID);
    int iResult =  pUserApi->ReqQryInvestorAccount(&QryInvestorAcc,++iRequestID);
    char char_rst[10] = {'\0'};
    sprintf(char_rst,"%d",iResult);
    cerr << "--->>> 请求查询投资者account: " << ((iResult == 0) ? "成功" : "失败")<<",result="<<char_rst << endl;
    //发送请求日志
    char char_msg[1024];
    sprintf(char_msg, "--->>> 请求查询投资者account: %s", ((iResult == 0) ? "成功" : "失败"));
    string msg(char_msg);
    LOG(INFO)<<msg;
    return ;
}
void CTraderSpi::ReqQryInvestorPosition()
{
    querySleep();
    for(unordered_map<string,unordered_map<string,int>>::iterator tmpit = positionmap.begin();tmpit != positionmap.end();tmpit ++){
        positionmap.erase(tmpit);
    }
    CUstpFtdcQryInvestorPositionField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, BROKER_ID);
    strcpy(req.InvestorID, INVESTOR_ID);
    //strcpy(req.InstrumentID, INSTRUMENT_ID);
    int iResult = pUserApi->ReqQryInvestorPosition(&req, ++iRequestID);
    char char_rst[10] = {'\0'};
    sprintf(char_rst,"%d",iResult);
    cerr << "--->>> 请求查询投资者持仓: " << ((iResult == 0) ? "成功" : "失败")<<",result="<<char_rst << endl;
    //发送请求日志
    char char_msg[1024];
    sprintf(char_msg, "--->>> 发送请求查询投资者持仓: %s", ((iResult == 0) ? "成功" : "失败"));
    string msg(char_msg);
    LOG(INFO)<<msg;
    //logqueue.push(&logmsg);
}
//查询投资者持仓
void CTraderSpi::OnRspQryInvestorPosition(CUstpFtdcRspInvestorPositionField *pRspInvestorPosition, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "--->>> " << "OnRspQryInvestorPosition" << endl;
    if(!IsErrorRspInfo(pRspInfo) && pRspInvestorPosition){
        initpst(pRspInvestorPosition);
    }
    if (bIsLast && !IsErrorRspInfo(pRspInfo))
    {
        int isbeginmk = 0;
        unordered_map<string,unordered_map<string,int>>::iterator tmpit = positionmap.begin();
        if(tmpit == positionmap.end()){
            cout<<"当前无持仓信息"<<endl;
        }else{
            for(;tmpit != positionmap.end();tmpit ++){
                string str_instrument = tmpit->first;
                unordered_map<string,int> tmppst = tmpit->second;
                char char_tmp_pst[10] = {'\0'};
                char char_longyd_pst[10] = {'\0'};
                char char_longtd_pst[10] = {'\0'};
                sprintf(char_tmp_pst,"%d",tmppst["longTotalPosition"]);
                sprintf(char_longyd_pst,"%d",tmppst["longYdPosition"]);
                sprintf(char_longtd_pst,"%d",tmppst["longTdPosition"]);
                char char_tmp_pst2[10] = {'\0'};
                char char_shortyd_pst[10] = {'\0'};
                char char_shorttd_pst[10] = {'\0'};
                sprintf(char_tmp_pst2,"%d",tmppst["shortTotalPosition"]);
                sprintf(char_shortyd_pst,"%d",tmppst["shortYdPosition"]);
                sprintf(char_shorttd_pst,"%d",tmppst["shortTdPosition"]);
                if(tmppst["longYdPosition"] > 0){
                    shortPstIsClose = 2;
                    short_offset_flag = 4;
                }
                if(tmppst["shortYdPosition"] > 0){
                    longPstIsClose = 2;
                    long_offset_flag = 4;
                }
//                int longpst = tmppst["longTotalPosition"];
//                int shortpst = tmppst["shortTotalPosition"];
//                char char_longpst[12] = {'\0'};
//                char char_shortpst[12] = {'\0'};
//                sprintf(char_longpst,"%d",longpst);
//                sprintf(char_shortpst,"%d",shortpst);
                string pst_msg = "持仓结构:"+str_instrument + ",多头持仓量=" + string(char_tmp_pst) + ",今仓数量=" + string(char_longtd_pst) + ",昨仓数量=" + string(char_longyd_pst) +
                        ";空头持仓量=" + string(char_tmp_pst2) + ",今仓数量=" + string(char_shorttd_pst) + ",昨仓数量=" + string(char_shortyd_pst) ;
                cout<<pst_msg<<endl;
                LOG(INFO)<<pst_msg;
            }
        }
        //call tradeParaProcess method to set close or open
        tradeParaProcessTwo();
        cout<<"是否启动策略程序?0 否，1是"<<endl;
        cin>>isbeginmk;
        if(isbeginmk == 1){
            start_process = 1;
            isrtntradeprocess = true;
            // 初始化hangqing api
            initMarketDataApi();
//            mduserapi = CUstpFtdcMdApi::CreateFtdcMdApi();			// 创建UserApi
//            CUstpFtdcMdSpi* pUserSpi = new CMdSpi();
//            mduserapi->RegisterSpi(pUserSpi);						// 注册事件类
//            mduserapi->RegisterFront(MD_FRONT_ADDR);					// connect
//            mduserapi->Init();
        }
    }
}
void CTraderSpi::OnRspOrderInsert(CUstpFtdcInputOrderField *pInputOrder, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo != NULL && IsErrorRspInfo(pRspInfo))
    {
        //cerr << "--->>> " << "OnRspOrderInsert" << "响应请求编号："<<nRequestID<< " CTP回报请求编号"<<pInputOrder->RequestID<<endl;
        string sInputOrderInfo = getInvestorOrderInsertInfo(pInputOrder);
        cout<<sInputOrderInfo<<endl;
        string sResult;
        char cErrorID[10]={'\0'};
        sprintf(cErrorID,"%d",pRspInfo->ErrorID);
        char cRequestid[100];
        sprintf(cRequestid,"%d",nRequestID);
        sResult.append("报单回报信息--->>> ErrorID=");
        sResult.append(cErrorID);
        sResult.append(", ErrorMsg=");
        sResult.append(boosttoolsnamespace::CBoostTools::gbktoutf8(pRspInfo->ErrorMsg));
        sInputOrderInfo.append(sResult);
        sInputOrderInfo.append("响应请求编号nRequestID：");
        sInputOrderInfo.append(cRequestid);
        sInputOrderInfo.append( " 回报请求编号pInputOrder->UserOrderLocalID:");
        sInputOrderInfo.append(pInputOrder->UserOrderLocalID);
        //记录失败的对冲报单
        LOG(INFO)<<sInputOrderInfo;
    }
    string ordreInfo = getInvestorOrderInsertInfo(pInputOrder);
    LOG(INFO)<<ordreInfo;
//	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
//	{
//		printf("-----------------------------\n");
//        printf("order insert failed! the reason is：%s\n",pRspInfo->ErrorMsg);
//		printf("-----------------------------\n");
//		return;
//	}
//	if(pInputOrder==NULL)
//	{
//        printf("no order info!\n");
//		return;
//	}
//	printf("-----------------------------\n");
//    printf("order insert success!!\n");
//	printf("-----------------------------\n");
//	return ;
	
}


void CTraderSpi::OnRtnTrade(CUstpFtdcTradeField *pTrade)
{
    ret ++;
    //cerr << "--->>> " << "OnRtnTrade"  << endl;
    //处理持仓
    int64_t start = GetSysTimeMicros();
    //auto start = boost::chrono::high_resolution_clock::now();
    char* ordersysid = pTrade->OrderSysID;
    string str_ordersysid = boost::trim_copy(string(ordersysid));
//    if(seq_map_ordersysid.find(str_ordersysid) != seq_map_ordersysid.end()){
//        string tmpstr = "error:查询不到ordersysid="+str_ordersysid+" 的报单信息";
//         LOG(INFO)<<tmpstr;
//    }else if(isrtntradeprocess){
        //根据ordersysid查询orderinsertkey
//        unordered_map<string,string>::iterator tmpit = seq_map_ordersysid.find(str_ordersysid);
//		string orderinsertkey = tmpit->second;
//		if(orderinsertkey.size() != 0){
//            unordered_map<string,unordered_map<string,int64_t>>::iterator tmptrade = seq_map_orderref.find(tmpit->second);
//            unordered_map<string,int64_t> timemap = tmptrade->second;//遍历报单的相关时间差
//            timemap["exchange_rtntrade"] = start;
//            unordered_map<string,int64_t>::iterator mapit = timemap.begin();
//			string str_time_msg;
//			while(mapit!=timemap.end()){
//				string key = mapit->first;
////                stringstream ss;
////                ss<<boost::chrono::duration<double>(start -mapit->second).count();
//                char char_time[21] = {'\0'};
//                sprintf(char_time,"%d",mapit->second);
//                string str_time = ss.str();
//				str_time_msg.append(key + "=" +str_time +";" );
//				mapit ++ ;
//			}
//             LOG(INFO)<<str_time_msg;
//		}
//    }
    processtrade(pTrade);
    int64_t end1 = GetSysTimeMicros();
    string tradeInfo = storeInvestorTrade(pTrade);
    LOG(INFO)<<tradeInfo;
    int64_t end2 = GetSysTimeMicros();
 //   string msg = "处理成交花费:" + string(GetDiffTime(end1,start));
 //   LOG(INFO)<<msg;
    //recordRunningMsg(msg);
//    recordRunningMsg(tradeInfo);
    //msg = "记录成交花费:" + string(GetDiffTime(end2,end1));
    //recordRunningMsg(msg);
//    logqueue.push(&logmsg);
//	printf("-----------------------------\n");
//    printf("received rtn trade\n");
//	Show(pTrade);
//	printf("-----------------------------\n");
//	return;
}
void CTraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
    cerr << "--->>> " << "OnHeartBeatWarning" << endl;
    cerr << "--->>> nTimerLapse = " << nTimeLapse << endl;
}
void CTraderSpi::OnRspError(CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "--->>> " << "OnRspError" << endl;
    string sResult;
    char cErrorID[100] ;
    char cRequestid[100];
//	itoa(pRspInfo->ErrorID,cErrorID,10);
    sprintf(cErrorID,"%d",pRspInfo->ErrorID);
    sprintf(cRequestid,"%d",nRequestID);
    sResult.append("CTP错误回报--->>> ErrorID=");
    sResult.append(cErrorID);
    sResult.append(", ErrorMsg=");
    sResult.append( boosttoolsnamespace::CBoostTools::gbktoutf8(pRspInfo->ErrorMsg));
    sResult.append(",requestid=");
    sResult.append(cRequestid);
    //记录错误回报报单信息
    //write(sResult,"d:\\test\\log.txt");
    cout<<sResult<<endl;
//    LogMsg logmsg;
//    logmsg.setMsg(sResult);
//    logqueue.push(&logmsg);
    IsErrorRspInfo(pRspInfo);
}
void CTraderSpi::ReqQryInstrument(char *instrumentid)
{
//    CUstpFtdcQryInstrumentField QryInstrument;
//    memset(&QryInstrument,0,sizeof(CUstpFtdcQryInstrumentField));
//    //strcpy(QryInstrument.ExchangeID,"CFFEX");
//    //strcpy(QryInstrument.InstrumentID,"IF1206");
//    pUserApi->ReqQryInstrument(&QryInstrument,++g_nOrdLocalID);
    cerr << "--->>> " << "ReqQryInstrument,ins="<<instrumentid << endl;
    CUstpFtdcQryInstrumentField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.InstrumentID, instrumentid);
    //strcpy(req.ExchangeID,"SHFE");
    int iResult = pUserApi->ReqQryInstrument(&req, ++iRequestID);
    char char_rst[10] = {'\0'};
    sprintf(char_rst,"%d",iResult);
    cerr << "--->>> 请求查询合约: " << ((iResult == 0) ? "成功" : "失败") <<",result="<<char_rst<< endl;
    querySleep();
    //ReqQryInvestorPosition();
}
void CTraderSpi::Show(CUstpFtdcOrderField *pOrder)
{
	printf("-----------------------------\n");
	printf("交易所代码=[%s]\n",pOrder->ExchangeID);
	printf("交易日=[%s]\n",pOrder->TradingDay);
	printf("会员编号=[%s]\n",pOrder->ParticipantID);
	printf("下单席位号=[%s]\n",pOrder->SeatID);
	printf("投资者编号=[%s]\n",pOrder->InvestorID);
	printf("客户号=[%s]\n",pOrder->ClientID);
	printf("系统报单编号=[%s]\n",pOrder->OrderSysID);
	printf("本地报单编号=[%s]\n",pOrder->OrderLocalID);
	printf("用户本地报单号=[%s]\n",pOrder->UserOrderLocalID);
	printf("合约代码=[%s]\n",pOrder->InstrumentID);
	printf("报单价格条件=[%c]\n",pOrder->OrderPriceType);
	printf("买卖方向=[%c]\n",pOrder->Direction);
	printf("开平标志=[%c]\n",pOrder->OffsetFlag);
	printf("投机套保标志=[%c]\n",pOrder->HedgeFlag);
	printf("价格=[%lf]\n",pOrder->LimitPrice);
	printf("数量=[%d]\n",pOrder->Volume);
	printf("报单来源=[%c]\n",pOrder->OrderSource);
	printf("报单状态=[%c]\n",pOrder->OrderStatus);
	printf("报单时间=[%s]\n",pOrder->InsertTime);
	printf("撤销时间=[%s]\n",pOrder->CancelTime);
	printf("有效期类型=[%c]\n",pOrder->TimeCondition);
	printf("GTD日期=[%s]\n",pOrder->GTDDate);
	printf("最小成交量=[%d]\n",pOrder->MinVolume);
	printf("止损价=[%lf]\n",pOrder->StopPrice);
	printf("强平原因=[%c]\n",pOrder->ForceCloseReason);
	printf("自动挂起标志=[%d]\n",pOrder->IsAutoSuspend);
	printf("-----------------------------\n");
	return ;
}

void CTraderSpi::OnRtnOrder(CUstpFtdcOrderField *pOrder)
{
    string msg = "OnRtnOrder:";
    msg.append(getRtnOrder(pOrder));
    LOG(INFO)<<msg;
//	printf("-----------------------------\n");
//	printf("收到报单回报\n");
//	Show(pOrder);
//	printf("-----------------------------\n");
//	return ;
}

void CTraderSpi::OnRspOrderAction(CUstpFtdcOrderActionField *pOrderAction, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("撤单失败 错误原因：%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if(pOrderAction==NULL)
	{
		printf("没有撤单数据\n");
		return;
	}
	printf("-----------------------------\n");
	printf("撤单成功\n");
	printf("-----------------------------\n");
	return ;
}
void CTraderSpi::OnRspUserPasswordUpdate(CUstpFtdcUserPasswordUpdateField *pUserPasswordUpdate, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("修改密码失败 错误原因：%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if(pUserPasswordUpdate==NULL)
	{
		printf("没有修改密码数据\n");
		return;
	}
	printf("-----------------------------\n");
	printf("修改密码成功\n");
	printf("-----------------------------\n");
	return ;
}

void CTraderSpi::OnErrRtnOrderInsert(CUstpFtdcInputOrderField *pInputOrder, CUstpFtdcRspInfoField *pRspInfo)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("报单错误回报失败 错误原因：%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if(pInputOrder==NULL)
	{
		printf("没有数据\n");
		return;
	}
	printf("-----------------------------\n");
	printf("报单错误回报\n");
	printf("-----------------------------\n");
	return ;
}
void CTraderSpi::OnErrRtnOrderAction(CUstpFtdcOrderActionField *pOrderAction, CUstpFtdcRspInfoField *pRspInfo)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("撤单错误回报失败 错误原因：%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if(pOrderAction==NULL)
	{
		printf("没有数据\n");
		return;
	}
	printf("-----------------------------\n");
	printf("撤单错误回报\n");
	printf("-----------------------------\n");
	return ;
}

void CTraderSpi::OnRspQryOrder(CUstpFtdcOrderField *pOrder, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("查询报单失败 错误原因：%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if(pOrder==NULL)
	{
		printf("没有查询到报单数据\n");
		return;
	}
	Show(pOrder);
	return ;
}
void CTraderSpi::Show(CUstpFtdcTradeField *pTrade)
{
	printf("-----------------------------\n");
	printf("交易所代码=[%s]\n",pTrade->ExchangeID);
	printf("交易日=[%s]\n",pTrade->TradingDay);
	printf("会员编号=[%s]\n",pTrade->ParticipantID);
	printf("下单席位号=[%s]\n",pTrade->SeatID);
	printf("投资者编号=[%s]\n",pTrade->InvestorID);
	printf("客户号=[%s]\n",pTrade->ClientID);
	printf("成交编号=[%s]\n",pTrade->TradeID);

	printf("用户本地报单号=[%s]\n",pTrade->UserOrderLocalID);
	printf("合约代码=[%s]\n",pTrade->InstrumentID);
	printf("买卖方向=[%c]\n",pTrade->Direction);
	printf("开平标志=[%c]\n",pTrade->OffsetFlag);
	printf("投机套保标志=[%c]\n",pTrade->HedgeFlag);
	printf("成交价格=[%lf]\n",pTrade->TradePrice);
	printf("成交数量=[%d]\n",pTrade->TradeVolume);
	printf("清算会员编号=[%s]\n",pTrade->ClearingPartID);
	
	printf("-----------------------------\n");
	return ;
}
void CTraderSpi::OnRspQryTrade(CUstpFtdcTradeField *pTrade, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("查询成交失败 错误原因：%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if(pTrade==NULL)
	{
		printf("没有查询到成交数据");
		return;
	}
	Show(pTrade);
	return ;
}
void CTraderSpi::OnRspQryExchange(CUstpFtdcRspExchangeField *pRspExchange, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("查询交易所失败 错误原因：%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if (pRspExchange==NULL)
	{
		printf("没有查询到交易所信息\n");
		return ;
	}
	printf("-----------------------------\n");
	printf("[%s]\n",pRspExchange->ExchangeID);
	printf("[%s]\n",pRspExchange->ExchangeName);
	printf("-----------------------------\n");
	return;
}

void CTraderSpi::OnRspQryInvestorAccount(CUstpFtdcRspInvestorAccountField *pRspInvestorAccount, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (bIsLast && !IsErrorRspInfo(pRspInfo)){
        printf("-----------------------------\n");
        printf("投资者编号=[%s]\n",pRspInvestorAccount->InvestorID);
        printf("资金帐号=[%s]\n",pRspInvestorAccount->AccountID);
        printf("上次结算准备金=[%lf]\n",pRspInvestorAccount->PreBalance);
        printf("入金金额=[%lf]\n",pRspInvestorAccount->Deposit);
        printf("出金金额=[%lf]\n",pRspInvestorAccount->Withdraw);
        printf("冻结的保证金=[%lf]\n",pRspInvestorAccount->FrozenMargin);
        printf("冻结手续费=[%lf]\n",pRspInvestorAccount->FrozenFee);
        printf("手续费=[%lf]\n",pRspInvestorAccount->Fee);
        printf("平仓盈亏=[%lf]\n",pRspInvestorAccount->CloseProfit);
        printf("持仓盈亏=[%lf]\n",pRspInvestorAccount->PositionProfit);
        printf("可用资金=[%lf]\n",pRspInvestorAccount->Available);
        printf("多头冻结的保证金=[%lf]\n",pRspInvestorAccount->LongFrozenMargin);
        printf("空头冻结的保证金=[%lf]\n",pRspInvestorAccount->ShortFrozenMargin);
        printf("多头保证金=[%lf]\n",pRspInvestorAccount->LongMargin);
        printf("空头保证金=[%lf]\n",pRspInvestorAccount->ShortMargin);
        printf("-----------------------------\n");
        querySleep();
        ReqQryInstrument(singleInstrument);
    }else{
        printf("-----------------------------\n");
        printf("查询投资者账户失败,ERRORID=%d,错误原因：%s\n",pRspInfo->ErrorID,pRspInfo->ErrorMsg);
        printf("-----------------------------\n");
        return;
    }
}

void CTraderSpi::OnRspQryUserInvestor(CUstpFtdcRspUserInvestorField *pUserInvestor, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("查询可用投资者失败 错误原因：%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if (pUserInvestor==NULL)
	{
		printf("No data\n");
		return ;
	}
	
	printf("InvestorID=[%s]\n",pUserInvestor->InvestorID);
}

void CTraderSpi::Show(CUstpFtdcRspInstrumentField *pRspInstrument)
{
	printf("-----------------------------\n");
	printf("交易所代码=[%s]\n",pRspInstrument->ExchangeID);
	printf("品种代码=[%s]\n",pRspInstrument->ProductID);
	printf("品种名称=[%s]\n",pRspInstrument->ProductName);
	printf("合约代码=[%s]\n",pRspInstrument->InstrumentID);
	printf("合约名称=[%s]\n",pRspInstrument->InstrumentName);
	printf("交割年份=[%d]\n",pRspInstrument->DeliveryYear);
	printf("交割月=[%d]\n",pRspInstrument->DeliveryMonth);
	printf("限价单最大下单量=[%d]\n",pRspInstrument->MaxLimitOrderVolume);
	printf("限价单最小下单量=[%d]\n",pRspInstrument->MinLimitOrderVolume);
	printf("市价单最大下单量=[%d]\n",pRspInstrument->MaxMarketOrderVolume);
	printf("市价单最小下单量=[%d]\n",pRspInstrument->MinMarketOrderVolume);
	
	printf("数量乘数=[%d]\n",pRspInstrument->VolumeMultiple);
	printf("报价单位=[%lf]\n",pRspInstrument->PriceTick);
	printf("币种=[%c]\n",pRspInstrument->Currency);
	printf("多头限仓=[%d]\n",pRspInstrument->LongPosLimit);
	printf("空头限仓=[%d]\n",pRspInstrument->ShortPosLimit);
	printf("跌停板价=[%lf]\n",pRspInstrument->LowerLimitPrice);
	printf("涨停板价=[%lf]\n",pRspInstrument->UpperLimitPrice);
	printf("昨结算=[%lf]\n",pRspInstrument->PreSettlementPrice);
	printf("合约交易状态=[%c]\n",pRspInstrument->InstrumentStatus);
	
	printf("创建日=[%s]\n",pRspInstrument->CreateDate);
	printf("上市日=[%s]\n",pRspInstrument->OpenDate);
	printf("到期日=[%s]\n",pRspInstrument->ExpireDate);
	printf("开始交割日=[%s]\n",pRspInstrument->StartDelivDate);
	printf("最后交割日=[%s]\n",pRspInstrument->EndDelivDate);
	printf("挂牌基准价=[%lf]\n",pRspInstrument->BasisPrice);
	printf("当前是否交易=[%d]\n",pRspInstrument->IsTrading);
	printf("基础商品代码=[%s]\n",pRspInstrument->UnderlyingInstrID);
	printf("持仓类型=[%c]\n",pRspInstrument->PositionType);
	printf("执行价=[%lf]\n",pRspInstrument->StrikePrice);
	printf("期权类型=[%c]\n",pRspInstrument->OptionsType);
	printf("-----------------------------\n");
	
}
void CTraderSpi::OnRspQryInstrument(CUstpFtdcRspInstrumentField *pRspInstrument, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0){
		printf("查询交易编码失败 错误原因：%s\n",pRspInfo->ErrorMsg);
		return;
	}
	
	if (pRspInstrument==NULL){
		printf("没有查询到合约数据\n");
		return ;
    }else{
        Show(pRspInstrument);
        tick = pRspInstrument->PriceTick;
        min_price = pRspInstrument->LowerLimitPrice;
        max_price = pRspInstrument->UpperLimitPrice;
        ReqQryInvestorPosition();
    }
	

//	return ;
}

void CTraderSpi::OnRspQryTradingCode(CUstpFtdcRspTradingCodeField *pTradingCode, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("查询交易编码失败 错误原因：%s\n",pRspInfo->ErrorMsg);
		return;
	}
	
	if (pTradingCode==NULL)
	{
		printf("没有查询到交易编码\n");
		return ;
	}
	printf("-----------------------------\n");
	printf("交易所代码=[%s]\n",pTradingCode->ExchangeID);
	printf("经纪公司编号=[%s]\n",pTradingCode->BrokerID);
	printf("投资者编号=[%s]\n",pTradingCode->InvestorID);
	printf("客户代码=[%s]\n",pTradingCode->ClientID);
	printf("客户代码权限=[%d]\n",pTradingCode->ClientRight);
	printf("是否活跃=[%c]\n",pTradingCode->IsActive);
	printf("-----------------------------\n");
	return ;
}

//void CTraderSpi::OnRspQryInvestorPosition(CUstpFtdcRspInvestorPositionField *pRspInvestorPosition, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
//{
//	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
//	{
//		printf("查询投资者持仓 错误原因：%s\n",pRspInfo->ErrorMsg);
//		return;
//	}
	
//	if (pRspInvestorPosition==NULL)
//	{
//		printf("没有查询到投资者持仓\n");
//		return ;
//	}
//	printf("-----------------------------\n");
//	printf("交易所代码=[%s]\n",pRspInvestorPosition->ExchangeID);
//	printf("经纪公司编号=[%s]\n",pRspInvestorPosition->BrokerID);
//	printf("投资者编号=[%s]\n",pRspInvestorPosition->InvestorID);
//	printf("客户代码=[%s]\n",pRspInvestorPosition->ClientID);
//	printf("合约代码=[%s]\n",pRspInvestorPosition->InstrumentID);
//	printf("买卖方向=[%c]\n",pRspInvestorPosition->Direction);
//	printf("今持仓量=[%d]\n",pRspInvestorPosition->Position);
//	printf("-----------------------------\n");
//	return ;

//}

	///投资者手续费率查询应答
void CTraderSpi::OnRspQryInvestorFee(CUstpFtdcInvestorFeeField *pInvestorFee, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("查询投资者手续费率失败 错误原因：%s\n",pRspInfo->ErrorMsg);
		return;
	}
	
	if (pInvestorFee==NULL)
	{
		printf("没有查询到投资者手续费率\n");
		return ;
	}
	printf("-----------------------------\n");
	printf("经纪公司编号=[%s]\n",pInvestorFee->BrokerID);
	printf("合约代码=[%s]\n",pInvestorFee->InstrumentID);
	printf("客户代码=[%s]\n",pInvestorFee->ClientID);
	printf("开仓手续费按比例=[%f]\n",pInvestorFee->OpenFeeRate);
	printf("开仓手续费按手数=[%f]\n",pInvestorFee->OpenFeeAmt);
	printf("平仓手续费按比例=[%f]\n",pInvestorFee->OffsetFeeRate);
	printf("平仓手续费按手数=[%f]\n",pInvestorFee->OffsetFeeAmt);
	printf("平今仓手续费按比例=[%f]\n",pInvestorFee->OTFeeRate);
	printf("平今仓手续费按手数=[%f]\n",pInvestorFee->OTFeeAmt);
	printf("-----------------------------\n");
	return ;

}

	///投资者保证金率查询应答
void CTraderSpi::OnRspQryInvestorMargin(CUstpFtdcInvestorMarginField *pInvestorMargin, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("查询投资者保证金率失败 错误原因：%s\n",pRspInfo->ErrorMsg);
		return;
	}
	
	if (pInvestorMargin==NULL)
	{
		printf("没有查询到投资者保证金率\n");
		return ;
	}
	printf("-----------------------------\n");
	printf("经纪公司编号=[%s]\n",pInvestorMargin->BrokerID);
	printf("合约代码=[%s]\n",pInvestorMargin->InstrumentID);
	printf("客户代码=[%s]\n",pInvestorMargin->ClientID);
	printf("多头占用保证金按比例=[%f]\n",pInvestorMargin->LongMarginRate);
	printf("多头保证金按手数=[%f]\n",pInvestorMargin->LongMarginAmt);
	printf("空头占用保证金按比例=[%f]\n",pInvestorMargin->ShortMarginRate);
	printf("空头保证金按手数=[%f]\n",pInvestorMargin->ShortMarginAmt);
	printf("-----------------------------\n");
	return ;

}


void CTraderSpi::OnRspQryComplianceParam(CUstpFtdcRspComplianceParamField *pRspComplianceParam, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("查询合规参数失败 错误原因：%s\n",pRspInfo->ErrorMsg);
		return;
	}
	
	if (pRspComplianceParam==NULL)
	{
		printf("没有查询到合规参数\n");
		return ;
	}
	printf("-----------------------------\n");
	printf("经纪公司编号=[%s]\n",pRspComplianceParam->BrokerID);
	printf("客户代码=[%s]\n",pRspComplianceParam->ClientID);
	printf("每日最大报单笔=[%d]\n",pRspComplianceParam->DailyMaxOrder);
	printf("每日最大撤单笔=[%d]\n",pRspComplianceParam->DailyMaxOrderAction);
	printf("每日最大错单笔=[%d]\n",pRspComplianceParam->DailyMaxErrorOrder);
	printf("每日最大报单手=[%d]\n",pRspComplianceParam->DailyMaxOrderVolume);
	printf("每日最大撤单手=[%d]\n",pRspComplianceParam->DailyMaxOrderActionVolume);
	printf("-----------------------------\n");
	return ;


}


int icount=0;
void CTraderSpi::OnRtnInstrumentStatus(CUstpFtdcInstrumentStatusField *pInstrumentStatus)
{
	if (pInstrumentStatus==NULL)
	{
		printf("没有合约状态信息\n");
		return ;
	}
	icount++;
	
//	printf("-----------------------------\n");
//	printf("交易所代码=[%s]\n",pInstrumentStatus->ExchangeID);
//	printf("品种代码=[%s]\n",pInstrumentStatus->ProductID);
//	printf("品种名称=[%s]\n",pInstrumentStatus->ProductName);
//	printf("合约代码=[%s]\n",pInstrumentStatus->InstrumentID);
//	printf("合约名称=[%s]\n",pInstrumentStatus->InstrumentName);
//	printf("交割年份=[%d]\n",pInstrumentStatus->DeliveryYear);
//	printf("交割月=[%d]\n",pInstrumentStatus->DeliveryMonth);
//	printf("限价单最大下单量=[%d]\n",pInstrumentStatus->MaxLimitOrderVolume);
//	printf("限价单最小下单量=[%d]\n",pInstrumentStatus->MinLimitOrderVolume);
//	printf("市价单最大下单量=[%d]\n",pInstrumentStatus->MaxMarketOrderVolume);
//	printf("市价单最小下单量=[%d]\n",pInstrumentStatus->MinMarketOrderVolume);
	
//	printf("数量乘数=[%d]\n",pInstrumentStatus->VolumeMultiple);
//	printf("报价单位=[%lf]\n",pInstrumentStatus->PriceTick);
//	printf("币种=[%c]\n",pInstrumentStatus->Currency);
//	printf("多头限仓=[%d]\n",pInstrumentStatus->LongPosLimit);
//	printf("空头限仓=[%d]\n",pInstrumentStatus->ShortPosLimit);
//	printf("跌停板价=[%lf]\n",pInstrumentStatus->LowerLimitPrice);
//	printf("涨停板价=[%lf]\n",pInstrumentStatus->UpperLimitPrice);
//	printf("昨结算=[%lf]\n",pInstrumentStatus->PreSettlementPrice);
//	printf("合约交易状态=[%c]\n",pInstrumentStatus->InstrumentStatus);
	
//	printf("创建日=[%s]\n",pInstrumentStatus->CreateDate);
//	printf("上市日=[%s]\n",pInstrumentStatus->OpenDate);
//	printf("到期日=[%s]\n",pInstrumentStatus->ExpireDate);
//	printf("开始交割日=[%s]\n",pInstrumentStatus->StartDelivDate);
//	printf("最后交割日=[%s]\n",pInstrumentStatus->EndDelivDate);
//	printf("挂牌基准价=[%lf]\n",pInstrumentStatus->BasisPrice);
//	printf("当前是否交易=[%d]\n",pInstrumentStatus->IsTrading);
//	printf("基础商品代码=[%s]\n",pInstrumentStatus->UnderlyingInstrID);
//	printf("持仓类型=[%c]\n",pInstrumentStatus->PositionType);
//	printf("执行价=[%lf]\n",pInstrumentStatus->StrikePrice);
//	printf("期权类型=[%c]\n",pInstrumentStatus->OptionsType);

//	printf("-----------------------------\n");
//	printf("[%d]",icount);
//	return ;

}


void CTraderSpi::OnRtnInvestorAccountDeposit(CUstpFtdcInvestorAccountDepositResField *pInvestorAccountDepositRes)
{
	if (pInvestorAccountDepositRes==NULL)
	{
		printf("没有资金推送信息\n");
		return ;
	}

	printf("-----------------------------\n");
	printf("经纪公司编号=[%s]\n",pInvestorAccountDepositRes->BrokerID);
	printf("用户代码＝[%s]\n",pInvestorAccountDepositRes->UserID);
	printf("投资者编号=[%s]\n",pInvestorAccountDepositRes->InvestorID);
	printf("资金账号=[%s]\n",pInvestorAccountDepositRes->AccountID);
	printf("资金流水号＝[%s]\n",pInvestorAccountDepositRes->AccountSeqNo);
	printf("金额＝[%s]\n",pInvestorAccountDepositRes->Amount);
	printf("出入金方向＝[%s]\n",pInvestorAccountDepositRes->AmountDirection);
	printf("可用资金＝[%s]\n",pInvestorAccountDepositRes->Available);
	printf("结算准备金＝[%s]\n",pInvestorAccountDepositRes->Balance);
	printf("-----------------------------\n");
	return ;

}
//提取投资者报单信息
string getInvestorOrderInsertInfo(CUstpFtdcInputOrderField *order)
{
    ///经纪公司代码
    char	*BrokerID = order->BrokerID;
    ///投资者代码
    char	*InvestorID = order->InvestorID;
    ///合约代码
    char	*InstrumentID = order->InstrumentID;
    ///报单引用
    char	*nOrdLocalID = order->UserOrderLocalID;
    ///用户代码
    char	*UserID = order->UserID;
    ///报单价格条件
    char	OrderPriceType = order->OrderPriceType;
    ///买卖方向
    char	Direction[] = {order->Direction,'\0'};
    ///组合开平标志
    char	*CombOffsetFlag =&order->OffsetFlag;
    ///组合投机套保标志
    char	*CombHedgeFlag = &order->HedgeFlag;
    ///价格
    TUstpFtdcPriceType	limitPrice = order->LimitPrice;
    char LimitPrice[100];
    sprintf(LimitPrice,"%f",limitPrice);
    ///数量
    TUstpFtdcVolumeType	volumeTotalOriginal = order->Volume;
    char VolumeTotalOriginal[100];
    sprintf(VolumeTotalOriginal,"%d",volumeTotalOriginal);
    ///有效期类型
    TUstpFtdcTimeConditionType	TimeCondition = order->TimeCondition;
    ///GTD日期
    //TUstpFtdcDateType	GTDDate = order->GTDDate;
    ///成交量类型
    TUstpFtdcVolumeConditionType	VolumeCondition[] = {order->VolumeCondition,'\0'};
    ///最小成交量
    TUstpFtdcVolumeType	MinVolume = order->MinVolume;
    ///触发条件
    //TUstpFtdcContingentConditionType	ContingentCondition = order->ContingentCondition;
    ///止损价
    TUstpFtdcPriceType	StopPrice = order->StopPrice;
    ///强平原因
    TUstpFtdcForceCloseReasonType	ForceCloseReason = order->ForceCloseReason;
    ///自动挂起标志
    TUstpFtdcBoolType	IsAutoSuspend = order->IsAutoSuspend;
    ///业务单元
    //TUstpFtdcBusinessUnitType	BusinessUnit = order->BusinessUnit;
    ///请求编号
    //TUstpFtdcRequestIDType	requestID = order->RequestID;
    char RequestID[100]={'\0'};
    //sprintf(RequestID,"%d",requestID);
    ///用户强评标志
    TUstpFtdcBoolType	UserForceClose = order->ForceCloseReason;

    string ordreInfo;
    ordreInfo.append("BrokerID=").append(BrokerID);ordreInfo.append("\t");
    ordreInfo.append("InvestorID=").append(InvestorID);ordreInfo.append("\t");
    ordreInfo.append("InstrumentID=").append(InstrumentID);ordreInfo.append("\t");
    ordreInfo.append("g_nOrdLocalID=").append(nOrdLocalID);ordreInfo.append("\t");
    ordreInfo.append("UserID=").append(UserID);ordreInfo.append("\t");
    ordreInfo.append("Direction").append(Direction);ordreInfo.append("\t");
    ordreInfo.append("CombOffsetFlag").append(CombOffsetFlag);ordreInfo.append("\t");
    ordreInfo.append("CombHedgeFlag").append(CombHedgeFlag);ordreInfo.append("\t");
    ordreInfo.append("LimitPrice").append(LimitPrice);ordreInfo.append("\t");
    ordreInfo.append("VolumeTotalOriginal").append(VolumeTotalOriginal);ordreInfo.append("\t");
    ordreInfo.append("VolumeCondition").append(VolumeCondition);ordreInfo.append("\t");
    ordreInfo.append("RequestID").append(RequestID);ordreInfo.append("\t");
    return ordreInfo;
}
bool CTraderSpi::IsErrorRspInfo(CUstpFtdcRspInfoField *pRspInfo)
{
    // 如果ErrorID != 0, 说明收到了错误的响应
    bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
   // cout<<"iserror"<<bResult<<" "<< pRspInfo<<endl;
//    string errmsg = boosttoolsnamespace::CBoostTools::gbktoutf8(pRspInfo->ErrorMsg);

    char char_msg[1024]={'\0'};
    if (bResult){
        string errmsg =pRspInfo->ErrorMsg;
        if(pRspInfo->ErrorID == 12){//重复的ref
            g_nOrdLocalID += 1000;
            cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << errmsg <<",g_nOrdLocalID增加="<<g_nOrdLocalID<<endl;
            sprintf(char_msg, "--->>> ErrorID=%d,ErrorMsg=%s,g_nOrdLocalID增加=%d",pRspInfo->ErrorID,  boosttoolsnamespace::CBoostTools::gbktoutf8(pRspInfo->ErrorMsg),g_nOrdLocalID);

        }else if(pRspInfo->ErrorID == 36){//资金不足
            longPstIsClose = 2;
            shortPstIsClose = 2;
            cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << errmsg <<",isclose平仓开仓方式修改为:"<<longPstIsClose<<endl;
            sprintf(char_msg, "--->>> ErrorID=%d,ErrorMsg=%s,isclose平仓开仓方式修改为:%d",pRspInfo->ErrorID,  boosttoolsnamespace::CBoostTools::gbktoutf8(pRspInfo->ErrorMsg),longPstIsClose);
        }else if(pRspInfo->ErrorID == 31){//平仓量超过持仓量
            longPstIsClose = 1;
            shortPstIsClose = 1;
            cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << errmsg <<",isclose平仓开仓方式修改为:"<<longPstIsClose<<endl;
            sprintf(char_msg, "--->>> ErrorID=%d,ErrorMsg=%s,isclose平仓开仓方式修改为:%d",pRspInfo->ErrorID,  boosttoolsnamespace::CBoostTools::gbktoutf8(pRspInfo->ErrorMsg),longPstIsClose);
        }/*else if(pRspInfo->ErrorID == 51){//平昨仓位不足
            offset_flag = 3;
            cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << errmsg <<",平仓方式修改为平今:"<<offset_flag<<endl;
            sprintf(char_msg, "--->>> ErrorID=%d,ErrorMsg=%s,平仓方式修改为平今:%d",pRspInfo->ErrorID,  pRspInfo->ErrorMsg,offset_flag);
        }else if(pRspInfo->ErrorID == 50){//平今仓位不足
            isclose = 1;
            cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << errmsg <<",isclose平仓开仓方式修改为:"<<isclose<<endl;
            sprintf(char_msg, "--->>> ErrorID=%d,ErrorMsg=%s,isclose平仓开仓方式修改为:%d",pRspInfo->ErrorID,  pRspInfo->ErrorMsg,isclose);
        }*/else if(pRspInfo->ErrorID == 60){//不合法的登录
            cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << errmsg <<endl;
            sprintf(char_msg, "--->>> ErrorID=%d,ErrorMsg=%s",pRspInfo->ErrorID,errmsg);
        }else{
            cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << boosttoolsnamespace::CBoostTools::gbktoutf8(pRspInfo->ErrorMsg) <<endl;

            sprintf(char_msg, "--->>> ErrorID=%d,ErrorMsg=%s",pRspInfo->ErrorID,  boosttoolsnamespace::CBoostTools::gbktoutf8(pRspInfo->ErrorMsg));
        }
        string msg(char_msg);
        //cout<<"----------"<<msg<<endl;
        LOG(INFO)<<msg;
    }
    return bResult;
}
//初始化持仓信息
void CTraderSpi::initpst(CUstpFtdcRspInvestorPositionField *pInvestorPosition)
{
    boost::recursive_mutex::scoped_lock SLock(pst_mtx);
    ///合约代码
    char	*InstrumentID = pInvestorPosition->InstrumentID;
    string str_instrumentid= string(InstrumentID);
    ///持仓多空方向
    TUstpFtdcDirectionType	dir = pInvestorPosition->Direction;
    char PosiDirection[] = {dir,'\0'};
    ///投机套保标志
    TUstpFtdcHedgeFlagType	flag = pInvestorPosition->HedgeFlag;
    char HedgeFlag[] = {flag,'\0'};
    ///上日持仓
    TUstpFtdcVolumeType	ydPosition = pInvestorPosition->YdPosition;
    char YdPosition[100];
    sprintf(YdPosition,"%d",ydPosition);
    ///今日持仓
    TUstpFtdcVolumeType	position = pInvestorPosition->Position;
    char Position[100];
    sprintf(Position,"%d",position);

    string str_dir = string(PosiDirection);

    ///持仓日期
    //TUstpFtdcPositionDateType	positionDate = pInvestorPosition->PositionDate;
    //char PositionDate[] = {positionDate,'\0'};
    if(positionmap.find(str_instrumentid) == positionmap.end()){//暂时没有处理，不需要考虑多空方向
        unordered_map<string,int> tmpmap;
        if("0" == str_dir){//买
            //多头
            tmpmap["longTdPosition"] = position;
            tmpmap["longYdPosition"] = ydPosition;
            tmpmap["longTotalPosition"] = position + ydPosition;
            //空头
            tmpmap["shortTdPosition"] = 0;
            tmpmap["shortYdPosition"] = 0;
            tmpmap["shortTotalPosition"] = 0;
        }else if("1" == str_dir){//空
            //空头
            tmpmap["shortTdPosition"] = position;
            tmpmap["shortYdPosition"] = ydPosition;
            tmpmap["shortTotalPosition"] = position + ydPosition;
            //多头
            tmpmap["longTdPosition"] = 0;
            tmpmap["longYdPosition"] = 0;
            tmpmap["longTotalPosition"] = 0;
        }else{
            cout<<InstrumentID<<";error:持仓类型无法判断PosiDirection="<<str_dir<<endl;
            exit;
        }
        positionmap[str_instrumentid] = tmpmap;
    }else{
        unordered_map<string,unordered_map<string,int>>::iterator tmpmap  = positionmap.find(str_instrumentid);
        //对应的反方向应该已经存在，这里后续需要确认
        if("0" == str_dir){//多
            //多头
            tmpmap->second["longTdPosition"] = position + tmpmap->second["longTdPosition"];
            tmpmap->second["longYdPosition"] = ydPosition + tmpmap->second["longYdPosition"];
            tmpmap->second["longTotalPosition"] = tmpmap->second["longTdPosition"] + tmpmap->second["longYdPosition"] + tmpmap->second["longTotalPosition"];
        }else if("1" == str_dir){//空
            //空头
            tmpmap->second["shortTdPosition"] = position + tmpmap->second["shortTdPosition"];
            tmpmap->second["shortYdPosition"] = ydPosition + tmpmap->second["shortYdPosition"];
            tmpmap->second["shortTotalPosition"] = tmpmap->second["shortTdPosition"] + tmpmap->second["shortYdPosition"] + tmpmap->second["shortTotalPosition"];
        }else{
            cout<<InstrumentID<<";error:持仓类型无法判断PosiDirection="<<str_dir<<endl;
            exit;
        }
    }
    storeInvestorPosition(pInvestorPosition);
}
int CTraderSpi::processtrade(CUstpFtdcTradeField *pTrade)
{
    if(start_process == 0){
        return 0;
    }
    ///买卖方向
    TUstpFtdcDirectionType	direction = pTrade->Direction;
    char Direction[]={direction,'\0'};
    //sprintf(Direction,"%s",direction);
    ///开平标志
    TUstpFtdcOffsetFlagType	offsetFlag = pTrade->OffsetFlag;
    char OffsetFlag[]={offsetFlag,'\0'};
    ///合约代码
    char	*InstrumentID =pTrade->InstrumentID;
    string str_inst = string(InstrumentID);
    //买卖方向
    string str_dir = string(Direction);
    //开平方向
    string str_offset = string(OffsetFlag);
    //锁持仓处理
    boost::recursive_mutex::scoped_lock SLock(pst_mtx);
    unordered_map<string,unordered_map<string,int>>::iterator map_iterator = positionmap.find(str_inst);
    //新开仓
    if(map_iterator == positionmap.end()){
        unordered_map<string,int> tmpmap;
        if(str_dir == "0"){//买
            //多头
            tmpmap["longTdPosition"] = pTrade->TradeVolume;
            tmpmap["longYdPosition"] = 0;
            tmpmap["longTotalPosition"] = pTrade->TradeVolume;
            //空头
            tmpmap["shortTdPosition"] = 0;
            tmpmap["shortYdPosition"] = 0;
            tmpmap["shortTotalPosition"] = 0;
        }else if(str_dir == "1"){//卖
            //空头
            tmpmap["shortTdPosition"] = pTrade->TradeVolume;
            tmpmap["shortYdPosition"] = 0;
            tmpmap["shortTotalPosition"] = pTrade->TradeVolume;
            //多头
            tmpmap["longTdPosition"] = 0;
            tmpmap["longYdPosition"] = 0;
            tmpmap["longTotalPosition"] = 0;
        }
        positionmap[str_inst] = tmpmap;
    }else{
        ///平仓
//        #define USTP_FTDC_OF_Close '1'
//        ///强平
//        #define USTP_FTDC_OF_ForceClose '2'
//        ///平今
//        #define USTP_FTDC_OF_CloseToday '3'
//        ///平昨
//        #define USTP_FTDC_OF_CloseYesterday '4'
        if(str_dir == "0"){//买
            if(str_offset == "0"){//买开仓,多头增加
                map_iterator->second["longTdPosition"] = map_iterator->second["longTdPosition"] + pTrade->TradeVolume;
                int tmp_tdpst = map_iterator->second["longTdPosition"];
                int tmp_ydpst = map_iterator->second["longYdPosition"];
                realLongPstLimit = tmp_tdpst + tmp_ydpst;
                map_iterator->second["longTotalPosition"] = realLongPstLimit;
            }else if(str_offset == "1" ){//买平仓,空头减少
                int tmp_tdpst = map_iterator->second["shortTdPosition"];
                int tmp_ydpst = map_iterator->second["shortYdPosition"];
                //int tmp_num = map_iterator->second["shortTotalPosition"];
                if(tmp_tdpst > 0){
                    if(tmp_tdpst <= pTrade->TradeVolume){
                        tmp_ydpst = tmp_ydpst - (pTrade->TradeVolume - tmp_tdpst);
                        tmp_tdpst = 0;
                    }else{
                        tmp_tdpst = tmp_tdpst - pTrade->TradeVolume;
                    }
                }else if(tmp_tdpst == 0){
                    tmp_ydpst = tmp_ydpst - pTrade->TradeVolume;
                }else{
                    cout<<"tdposition is error!!!"<<endl;
                }
                realShortPstLimit = tmp_ydpst + tmp_tdpst;
                map_iterator->second["shortTdPosition"] = tmp_tdpst;
                map_iterator->second["shortYdPosition"] = tmp_ydpst;
                map_iterator->second["shortTotalPosition"] = realShortPstLimit;
//                if(tmp_ydpst == 0){//buy open
//                    longPstIsClose = 1;
//                    long_offset_flag = 1;
//                }
            }else if(str_offset == "3"){//平今
                int tmp_tdpst = map_iterator->second["shortTdPosition"];
                int tmp_ydpst = map_iterator->second["shortYdPosition"];
                tmp_tdpst = tmp_tdpst - pTrade->TradeVolume;
                realShortPstLimit = tmp_ydpst + tmp_tdpst;
                map_iterator->second["shortTdPosition"] = tmp_tdpst;
                map_iterator->second["shortTotalPosition"] = realShortPstLimit;
            }else if(str_offset == "4"){//平昨
                int tmp_tdpst = map_iterator->second["shortTdPosition"];
                int tmp_ydpst = map_iterator->second["shortYdPosition"];
                if(tmp_ydpst == 0){
                    char c_err[100];
                    sprintf(c_err,"shortYdPosition is zero!!!,please check this rtn trade.");
                    cout<<c_err<<endl;
                    LOG(INFO)<<c_err;
                }
                tmp_ydpst = tmp_ydpst - pTrade->TradeVolume;

                realShortPstLimit = tmp_ydpst + tmp_tdpst;
                map_iterator->second["shortYdPosition"] = tmp_ydpst;
                map_iterator->second["shortTotalPosition"] = realShortPstLimit;
//                if(tmp_ydpst == 0){
//                    longPstIsClose = 1;
//                    long_offset_flag = 1;
//                }
            }
        }else if(str_dir == "1"){//卖
            if(str_offset == "0"){//卖开仓,空头增加
                map_iterator->second["shortTdPosition"] = map_iterator->second["shortTdPosition"] + pTrade->TradeVolume;
                int tmp_tdpst = map_iterator->second["shortTdPosition"];
                int tmp_ydpst = map_iterator->second["shortYdPosition"];
                realShortPstLimit = tmp_tdpst + tmp_ydpst;
                map_iterator->second["shortTotalPosition"] = realShortPstLimit;
            }else if(str_offset == "1"){//卖平仓,多头减少
                int tmp_tdpst = map_iterator->second["longTdPosition"];
                int tmp_ydpst = map_iterator->second["longYdPosition"];
                //int tmp_num = map_iterator->second["longTotalPosition"];
                if(tmp_tdpst > 0){
                    if(tmp_tdpst <= pTrade->TradeVolume){
                        tmp_ydpst = tmp_ydpst - (pTrade->TradeVolume - tmp_tdpst);
                        tmp_tdpst = 0;
                    }else{
                        tmp_tdpst = tmp_tdpst - pTrade->TradeVolume;
                    }
                }else if(tmp_tdpst == 0){
                    tmp_ydpst = tmp_ydpst - pTrade->TradeVolume;
                }else{
                    cout<<"tdposition is error!!!"<<endl;
                }
                realLongPstLimit = tmp_ydpst + tmp_tdpst;
                map_iterator->second["longTdPosition"] = tmp_tdpst;
                map_iterator->second["longYdPosition"] = tmp_ydpst;
                map_iterator->second["longTotalPosition"] = realLongPstLimit;
//                if(tmp_ydpst == 0){//sell open
//                    shortPstIsClose = 1;
//                    short_offset_flag = 1;
//                }
            }else if(str_offset == "3"){//平今
                int tmp_tdpst = map_iterator->second["longTdPosition"];
                int tmp_ydpst = map_iterator->second["longYdPosition"];
                tmp_tdpst = tmp_tdpst - pTrade->TradeVolume;
                realLongPstLimit = tmp_ydpst + tmp_tdpst;
                map_iterator->second["longTdPosition"] = tmp_tdpst;
                map_iterator->second["longTotalPosition"] = realLongPstLimit;
            }else if(str_offset == "4"){//平昨
                int tmp_tdpst = map_iterator->second["longTdPosition"];
                int tmp_ydpst = map_iterator->second["longYdPosition"];
                if(tmp_ydpst == 0){
                    char c_err[100];
                    sprintf(c_err,"longYdPosition is zero!!!,please check this rtn trade.");
                    cout<<c_err<<endl;
                    LOG(INFO)<<c_err;
                }
                tmp_ydpst = tmp_ydpst - pTrade->TradeVolume;
                realLongPstLimit = tmp_ydpst + tmp_tdpst;
                map_iterator->second["longYdPosition"] = tmp_ydpst;
                map_iterator->second["longTotalPosition"] = realLongPstLimit;
//                if(tmp_ydpst == 0){//sell open
//                    shortPstIsClose = 1;
//                    short_offset_flag = 1;
//                }
            }
        }
    }
    tradeParaProcessTwo();
    string tmpmsg;
    for(unordered_map<string,unordered_map<string,int>>::iterator it=positionmap.begin();it != positionmap.end();it ++){
        tmpmsg.append(it->first).append("持仓情况:");
        char char_tmp_pst[10] = {'\0'};
        char char_longyd_pst[10] = {'\0'};
        char char_longtd_pst[10] = {'\0'};
        sprintf(char_tmp_pst,"%d",it->second["longTotalPosition"]);
        sprintf(char_longyd_pst,"%d",it->second["longYdPosition"]);
        sprintf(char_longtd_pst,"%d",it->second["longTdPosition"]);
        tmpmsg.append("多头数量=");
        tmpmsg.append(char_tmp_pst);
        tmpmsg.append(";今仓数量=");
        tmpmsg.append(char_longtd_pst);
        tmpmsg.append(";昨仓数量=");
        tmpmsg.append(char_longyd_pst);
        char char_tmp_pst2[10] = {'\0'};
        char char_shortyd_pst[10] = {'\0'};
        char char_shorttd_pst[10] = {'\0'};
        sprintf(char_tmp_pst2,"%d",it->second["shortTotalPosition"]);
        sprintf(char_shortyd_pst,"%d",it->second["shortYdPosition"]);
        sprintf(char_shorttd_pst,"%d",it->second["shortTdPosition"]);
        tmpmsg.append("空头数量=");
        tmpmsg.append(char_tmp_pst2);
        tmpmsg.append(";今仓数量=");
        tmpmsg.append(char_shorttd_pst);
        tmpmsg.append(";昨仓数量=");
        tmpmsg.append(char_shortyd_pst);
    }
    cout<<tmpmsg<<endl;
    LOG(INFO)<<tmpmsg;
//    LogMsg *logmsg = new LogMsg();
//    logmsg->setMsg(tmpmsg);
//    logqueue.push(logmsg);
    return 0;
}
void CTraderSpi::tradeParaProcess(){
    for(unordered_map<string,unordered_map<string,int>>::iterator map_iterator=positionmap.begin();map_iterator != positionmap.end();map_iterator ++){
        string tmpmsg;
        realShortPstLimit = map_iterator->second["shortTotalPosition"];
        realLongPstLimit = map_iterator->second["longTotalPosition"];
        int shortYdPst = map_iterator->second["shortYdPosition"];
        int longYdPst = map_iterator->second["longYdPosition"];
        // buy or open judge
        if(realLongPstLimit > longpstlimit){ //多头超过持仓限额，且必须空头有持仓才能多头平仓
            char char_limit[10] = {'\0'};
            sprintf(char_limit,"%d",realLongPstLimit);
            if(realShortPstLimit == 0){
                longPstIsClose = 1;
                tmpmsg.append("多头持仓量=");
                tmpmsg.append(char_limit).append("大于longpstlimit,but realShortPstLimit is zero,仍然为多头开仓");
                //LOG(INFO)<<tmpmsg;
            }else{
                longPstIsClose = 2;
                if(shortYdPst > 0){//privious to close yesterday position
                    long_offset_flag = 4;
                }else{
                    long_offset_flag = 3;
                }
                tmpmsg.append("多头持仓量=");
                tmpmsg.append(char_limit).append("大于longpstlimit,and realShortPstLimit is not zero,修改为多头平仓");
                //LOG(INFO)<<tmpmsg;
            }
        }else if(realShortPstLimit > shortpstlimit){//空头开平仓判断
            char char_limit[10] = {'\0'};
            sprintf(char_limit,"%d",realShortPstLimit);
            if(realLongPstLimit == 0){
                shortPstIsClose = 1;
                tmpmsg.append("空头持仓量=");
                tmpmsg.append(char_limit).append("大于shortpstlimit,but realLongPstLimit is zero,仍然为空头开仓");
                //LOG(INFO)<<tmpmsg;
            }else{
                shortPstIsClose = 2;
                if(longYdPst > 0){//privious to close yesterday position
                    short_offset_flag = 4;
                }else{
                    short_offset_flag = 3;
                }
                tmpmsg.append("空头持仓量=");
                tmpmsg.append(char_limit).append("大于shortpstlimit,and realLongPstLimit is not zero,修改为空头平仓");
                //LOG(INFO)<<tmpmsg;
            }
        }
//        LogMsg *logmsg = new LogMsg();
//        logmsg->setMsg(tmpmsg);
//        logqueue.push(logmsg);
        cout<<tmpmsg<<endl;
        LOG(INFO)<<tmpmsg;

        //spread set
        int bidAkdSpread = abs(realShortPstLimit - realLongPstLimit);
        if(bidAkdSpread < lastABSSpread && bidAkdSpread >= 5){

            char c_bas[10];
            sprintf(c_bas,"%d",bidAkdSpread);
            char c_lasts[10];
            sprintf(c_lasts,"%d",lastABSSpread);
            lastABSSpread = bidAkdSpread;
            string tmpmsg1 = "current bidAkdSpread=" + string(c_bas) + ",lastABSSpread=" + string(c_lasts) +
                    ",cul_times seting is available!!";
//            LogMsg *logmsg1 = new LogMsg();
//            logmsg1->setMsg(tmpmsg1);
//            logqueue.push(logmsg1);
            cout<<tmpmsg1<<endl;
            LOG(INFO)<<tmpmsg1;
            return;//avaialable
        }
        string s_msg;
        char c_bss[10];
        char c_realShortPstLimit[10];
        char c_realLongPstLimit[10];
        sprintf(c_bss,"%d",bidAkdSpread);
        sprintf(c_realShortPstLimit,"%d",realShortPstLimit);
        sprintf(c_realLongPstLimit,"%d",realLongPstLimit);
        if(bidAkdSpread >= 3 && bidAkdSpread <10){
            lastABSSpread = bidAkdSpread;
            if(realShortPstLimit > realLongPstLimit){//increase ask(sell) spread
                char c_pre_askCulTimes[10];
                sprintf(c_pre_askCulTimes,"%d",askCulTimes);
                askCulTimes += 1;
                char c_asktimes[100];
                if(up_culculate >= askCulTimes){
                    int tmp_cul = ((4*askCulTimes)/5);
                    sprintf(c_asktimes ,"up_culculate set from %d to %d;",up_culculate,tmp_cul);
                    up_culculate = tmp_cul;
                }
                char c_after_askCulTimes[10];
                sprintf(c_after_askCulTimes,"%d",askCulTimes);
                s_msg = string(c_asktimes) + "bidpst=" + string(c_realLongPstLimit) + ",askpst=" + string(c_realShortPstLimit) + ",spread=" + string(c_bss) +",>=5 and <10,askCulTimes is set from " +
                        string(c_pre_askCulTimes) + "to " + string(c_after_askCulTimes);
            }else{
                char c_pre_bidCulTimes[10];
                sprintf(c_pre_bidCulTimes,"%d",bidCulTimes);
                bidCulTimes += 1;
                char c_times[100];
                if(down_culculate >= bidCulTimes){
                    int tmp_cul = ((4*bidCulTimes)/5);
                    sprintf(c_times,"down_culculate set from %d to %d;",down_culculate,tmp_cul);
                    down_culculate = tmp_cul;
                }
                char c_after_bidCulTimes[10];
                sprintf(c_after_bidCulTimes,"%d",bidCulTimes);
                s_msg = string(c_times) +  "bidpst=" + string(c_realLongPstLimit) + ",askpst=" + string(c_realShortPstLimit) + ",spread=" + string(c_bss) +",>=5 and <10,bidCulTimes is set from " +
                        string(c_pre_bidCulTimes) + "to " + string(c_after_bidCulTimes);

            }
        }else if(bidAkdSpread >= 10){
            lastABSSpread = bidAkdSpread;
            if(realShortPstLimit > realLongPstLimit){//increase ask(sell) spread
                char c_pre_askCulTimes[10];
                sprintf(c_pre_askCulTimes,"%d",askCulTimes);
                askCulTimes += 2;
                char c_asktimes[100];
                if(up_culculate >= askCulTimes){
                    int tmp_cul = ((4*askCulTimes)/5);
                    sprintf(c_asktimes ,"up_culculate set from %d to %d;",up_culculate,tmp_cul);
                    up_culculate = tmp_cul;
                }
                char c_after_askCulTimes[10];
                sprintf(c_after_askCulTimes,"%d",askCulTimes);
                s_msg = string(c_asktimes) +  "bidpst=" + string(c_realLongPstLimit) + ",askpst=" + string(c_realShortPstLimit) + ",spread=" + string(c_bss) +",>=10,askCulTimes is set from " +
                        string(c_pre_askCulTimes) + "to " + string(c_after_askCulTimes);
            }else{
                char c_pre_bidCulTimes[10];
                sprintf(c_pre_bidCulTimes,"%d",bidCulTimes);
                bidCulTimes += 2;
                char c_times[100];
                if(down_culculate >= bidCulTimes){
                    int tmp_cul = ((4*bidCulTimes)/5);
                    sprintf(c_times,"down_culculate set from %d to %d;",down_culculate,tmp_cul);
                    down_culculate = tmp_cul;
                }
                char c_after_bidCulTimes[10];
                sprintf(c_after_bidCulTimes,"%d",bidCulTimes);
                s_msg = string(c_times) +  "bidpst=" + string(c_realLongPstLimit) + ",askpst=" + string(c_realShortPstLimit) + ",spread=" + string(c_bss) +",>=10,bidCulTimes is set from " +
                        string(c_pre_bidCulTimes) + "to " + string(c_after_bidCulTimes);
            }
        }else{
            char c_askCulTimes[10];
            sprintf(c_askCulTimes,"%d",askCulTimes);
            char c_bidCulTimes[10];
            sprintf(c_bidCulTimes,"%d",bidCulTimes);
            askCulTimes = cul_times;
            bidCulTimes = cul_times;
            char c_culTime[10];
            sprintf(c_culTime,"%d",cul_times);
            s_msg = "bidpst=" + string(c_realLongPstLimit) + ",askpst=" + string(c_realShortPstLimit) + ",spread=" + string(c_bss) +",<5,bidCulTimes is set from " +
                    string(c_bidCulTimes) + "to " + string(c_culTime) + ";askCulTimes is set from " + string(c_askCulTimes) + "to " + string(c_culTime);
        }
//        logmsg = new LogMsg();
//        logmsg->setMsg(s_msg);
//        logqueue.push(logmsg);
        cout<<s_msg<<endl;
        LOG(INFO)<<s_msg;
    }
}
void CTraderSpi::tradeParaProcessTwo(){
    for(unordered_map<string,unordered_map<string,int>>::iterator map_iterator=positionmap.begin();map_iterator != positionmap.end();map_iterator ++){
        string tmpmsg;
        realShortPstLimit = map_iterator->second["shortTotalPosition"];
        realLongPstLimit = map_iterator->second["longTotalPosition"];
        int shortYdPst = map_iterator->second["shortYdPosition"];
        int longYdPst = map_iterator->second["longYdPosition"];
        if(longYdPst > 0){
            shortPstIsClose = 2;
            short_offset_flag = 4;
        }
        if(shortYdPst > 0){
            longPstIsClose = 2;
            long_offset_flag = 4;
        }
        // buy or open judge
        if(realLongPstLimit > longpstlimit){ //多头超过持仓限额，且必须空头有持仓才能多头平仓
            char char_limit[10] = {'\0'};
            sprintf(char_limit,"%d",realLongPstLimit);
            longPstIsClose = 11;//long can not to open new position
            tmpmsg.append("多头持仓量=");
            tmpmsg.append(char_limit).append("大于longpstlimit,long can not to open new position");
        }else if(realShortPstLimit > shortpstlimit){//空头开平仓判断
            char char_limit[10] = {'\0'};
            sprintf(char_limit,"%d",realShortPstLimit);
            shortPstIsClose = 11;
            tmpmsg.append("空头持仓量=");
            tmpmsg.append(char_limit).append("大于shortpstlimit,short can not to open new position");
        }
        cout<<tmpmsg<<endl;
        LOG(INFO)<<tmpmsg;
        //spread set
        int bidAkdSpread = abs(realShortPstLimit - realLongPstLimit);
        if(bidAkdSpread >= firstGap && bidAkdSpread < secondGap && realShortPstLimit  > realLongPstLimit){
            bidCulTimes += 2;
            if(down_culculate >= bidCulTimes){
                down_culculate = (4*down_culculate)/5;
            }
        }else if(bidAkdSpread >= secondGap && realShortPstLimit > realLongPstLimit){
            bidCulTimes += 4;
            if(down_culculate >= bidCulTimes){
                down_culculate = (4*down_culculate)/5;
            }
        }else if(bidAkdSpread >= firstGap && bidAkdSpread < secondGap && realShortPstLimit < realLongPstLimit){
            askCulTimes += 2;
            if(up_culculate >= askCulTimes){
                up_culculate = (4*up_culculate)/5;
            }
        }else if(bidAkdSpread >= secondGap && realShortPstLimit < realLongPstLimit){
            askCulTimes += 4;
            if(up_culculate >= askCulTimes){
                up_culculate = (4*up_culculate)/5;
            }
        }else{
            bidCulTimes = cul_times;
            askCulTimes = cul_times;
        }
        lastABSSpread = bidAkdSpread;
    }
}
//将投资者持仓信息写入文件保存
int CTraderSpi::storeInvestorPosition(CUstpFtdcRspInvestorPositionField *pInvestorPosition)
{
    ///合约代码
    char	*InstrumentID = pInvestorPosition->InstrumentID;
    ///经纪公司代码
    char	*BrokerID = pInvestorPosition->BrokerID;
    ///交易所代码
    TUstpFtdcExchangeIDType	ExchangeID ;
    strcpy( ExchangeID,pInvestorPosition->ExchangeID);
    ///投资者代码
    char	*InvestorID = pInvestorPosition->InvestorID;
    ///客户代码
    TUstpFtdcClientIDType	ClientID;
    strcpy(ClientID,pInvestorPosition->ClientID);
    ///持仓多空方向
    TUstpFtdcDirectionType	dir = pInvestorPosition->Direction;
    char PosiDirection[] = {dir,'\0'};
    ///投机套保标志
    TUstpFtdcHedgeFlagType	flag = pInvestorPosition->HedgeFlag;
    char HedgeFlag[] = {flag,'\0'};
    ///昨持仓量
    TUstpFtdcVolumeType	ydPosition = pInvestorPosition->YdPosition;
    char YdPosition[100];
    sprintf(YdPosition,"%d",ydPosition);
    ///昨日持仓成本
    TUstpFtdcMoneyType	YdPositionCost = pInvestorPosition->YdPositionCost;
    char char_ydPstCost[100];
    sprintf(char_ydPstCost,"%f",YdPositionCost);
    ///今日持仓
    TUstpFtdcVolumeType	position = pInvestorPosition->Position;
    char Position[100];
    sprintf(Position,"%d",position);
    ///今日持仓成本
    TUstpFtdcMoneyType	positionCost = pInvestorPosition->PositionCost;
    char PositionCost[100];
    sprintf(PositionCost,"%f",positionCost);
    ///占用的保证金
    TUstpFtdcMoneyType	UseMargin = pInvestorPosition->UsedMargin;
    char char_useMargin[100] ;
    sprintf(char_useMargin,"%f",UseMargin);
    ///冻结的保证金
    TUstpFtdcMoneyType	FrozenMargin = pInvestorPosition->FrozenMargin;
    char char_frozenMargin[100] ;
    sprintf(char_frozenMargin,"%f",FrozenMargin);
    ///开仓冻结持仓
    TUstpFtdcVolumeType	FrozenPosition = pInvestorPosition->FrozenPosition;
    char char_FrozenPosition[100] ;
    sprintf(char_FrozenPosition,"%d",FrozenPosition);
    ///平仓冻结持仓
    TUstpFtdcVolumeType	FrozenClosing = pInvestorPosition->FrozenClosing;
    char char_FrozenClosing[100] ;
    sprintf(char_FrozenClosing,"%d",FrozenClosing);
    ///冻结的权利金
    TUstpFtdcMoneyType	FrozenPremium = pInvestorPosition->FrozenPremium;
    char char_FrozenPremium[100] ;
    sprintf(char_FrozenPremium,"%d",FrozenPremium);

    string sInvestorInfo;
    sInvestorInfo.append("position:");
    sInvestorInfo.append("InstrumentID=").append(InstrumentID);sInvestorInfo.append("\t");
    sInvestorInfo.append("BrokerID=").append(BrokerID);sInvestorInfo.append("\t");
    sInvestorInfo.append("ExchangeID=").append(ExchangeID);sInvestorInfo.append("\t");
    sInvestorInfo.append("InvestorID=").append(InvestorID);sInvestorInfo.append("\t");
    sInvestorInfo.append("ClientID=").append(ClientID);sInvestorInfo.append("\t");
    sInvestorInfo.append("PosiDirection=").append(PosiDirection);sInvestorInfo.append("\t");
    sInvestorInfo.append("HedgeFlag=").append(HedgeFlag);sInvestorInfo.append("\t");

    sInvestorInfo.append("YdPosition=").append(YdPosition);sInvestorInfo.append("\t");
    sInvestorInfo.append("YdPositionCost=").append(char_ydPstCost);sInvestorInfo.append("\t");
    sInvestorInfo.append("Position=").append(Position);sInvestorInfo.append("\t");
    sInvestorInfo.append("PositionCost=").append(PositionCost);sInvestorInfo.append("\t");
    sInvestorInfo.append("UsedMargin=").append(char_useMargin);sInvestorInfo.append("\t");
    sInvestorInfo.append("FrozenMargin=").append(char_frozenMargin);sInvestorInfo.append("\t");

    sInvestorInfo.append("FrozenPosition=").append(char_FrozenPosition);sInvestorInfo.append("\t");
    sInvestorInfo.append("FrozenClosing=").append(char_FrozenClosing);sInvestorInfo.append("\t");
    sInvestorInfo.append("FrozenPremium=").append(char_FrozenPremium);sInvestorInfo.append("\t");

    LOG(INFO)<<sInvestorInfo;
    return 0;
}
//将投资者成交信息写入文件保存
string CTraderSpi::storeInvestorTrade(CUstpFtdcTradeField *pTrade)
{
    string tradeInfo;
    ///买卖方向
    TUstpFtdcDirectionType	direction = pTrade->Direction;
    char Direction[]={direction,'\0'};
    //sprintf(Direction,"%s",direction);
    ///开平标志
    TUstpFtdcOffsetFlagType	offsetFlag = pTrade->OffsetFlag;
    char OffsetFlag[]={offsetFlag,'\0'};
    //sprintf(OffsetFlag,"%s",offsetFlag);
    ///经纪公司代码
    char	*BrokerID = pTrade->BrokerID;
    ///投资者代码
    char	*InvestorID = pTrade->InvestorID;
    ///合约代码
    char	*InstrumentID =pTrade->InstrumentID;
    ///报单引用
    //char	*UserOrderLocalID = pTrade->UserOrderLocalID;
    ///用户代码
    char	*UserID = pTrade->UserID;
    ///交易所代码
    char	*ExchangeID =pTrade->ExchangeID;
    ///成交编号
    char *	TradeID = pTrade->TradeID;

    ///报单编号
    char	*OrderSysID = pTrade->OrderSysID;
    ///会员代码
    //TUstpFtdcParticipantIDType	ParticipantID;
    ///客户代码
    char	*ClientID = pTrade->ClientID;
    ///交易角色
    //TUstpFtdcTradingRoleType	TradingRole;
    ///合约在交易所的代码
    //TUstpFtdcExchangeInstIDType	ExchangeInstID;

    ///投机套保标志
    TUstpFtdcHedgeFlagType	hedgeFlag = pTrade->HedgeFlag;
    char HedgeFlag[]={hedgeFlag,'\0'};
    //sprintf(HedgeFlag,"%s",hedgeFlag);
    ///价格
    TUstpFtdcPriceType	price = pTrade->TradePrice;
    char Price[100];
    sprintf(Price,"%f",price);
    ///数量
    TUstpFtdcVolumeType	volume = pTrade->TradeVolume;
    char Volume[100];
    sprintf(Volume,"%d",volume);
    ///成交时期
    //TUstpFtdcDateType	TradeDate;
    ///成交时间
    char	*TradeTime = pTrade->TradeTime;
    ///成交类型
//    TUstpFtdcTradeTypeType	tradeType = pTrade->tr;
//    char TradeType[]={tradeType,'\0'};
    //sprintf(TradeType,"%s",tradeType);
    ///成交价来源
    //TUstpFtdcPriceSourceType	PriceSource;
    ///交易所交易员代码
    //TUstpFtdcTraderIDType	TraderID;
    ///本地报单编号
    char	*OrderLocalID = pTrade->UserOrderLocalID;
    ///结算会员编号
    //TUstpFtdcParticipantIDType	ClearingPartID;
    ///业务单元
    //TUstpFtdcBusinessUnitType	BusinessUnit;
    ///序号
    //TUstpFtdcSequenceNoType	SequenceNo;
    ///交易日
    char	*TradingDay = pTrade->TradingDay;
    ///结算编号
    //TUstpFtdcSettlementIDType	SettlementID;
    ///经纪公司报单编号
    //TUstpFtdcSequenceNoType	BrokerOrderSeq;
    tradeInfo.append("BrokerID=").append(BrokerID);tradeInfo.append("\t");
    tradeInfo.append("InvestorID=").append(InvestorID);tradeInfo.append("\t");
    tradeInfo.append("InstrumentID=").append(InstrumentID);tradeInfo.append("\t");
    //tradeInfo.append("OrderRef=").append(OrderRef);tradeInfo.append("\t");
    tradeInfo.append("UserID=").append(UserID);tradeInfo.append("\t");
    tradeInfo.append("ExchangeID=").append(ExchangeID);tradeInfo.append("\t");
    tradeInfo.append("Direction=").append(Direction);tradeInfo.append("\t");
    tradeInfo.append("ClientID=").append(ClientID);tradeInfo.append("\t");
    tradeInfo.append("OffsetFlag=").append(OffsetFlag);tradeInfo.append("\t");
    tradeInfo.append("HedgeFlag=").append(HedgeFlag);tradeInfo.append("\t");
    tradeInfo.append("Price=").append(Price);tradeInfo.append("\t");
    tradeInfo.append("Volume=").append(Volume);tradeInfo.append("\t");
    tradeInfo.append("TradeTime=").append(TradeTime);tradeInfo.append("\t");
    //tradeInfo.append("TradeType=").append(TradeType);tradeInfo.append("\t");
    tradeInfo.append("OrderLocalID=").append(OrderLocalID);tradeInfo.append("\t");
    tradeInfo.append("TradingDay=").append(TradingDay);tradeInfo.append("\t");
    tradeInfo.append("ordersysid=").append(OrderSysID);tradeInfo.append("\t");
    tradeInfo.append("tradeid=").append(TradeID);tradeInfo.append("\t");
    return tradeInfo;
}
//提取投资者报单信息
string CTraderSpi::getInvestorOrderInsertInfo(CUstpFtdcInputOrderField *order)
{
    ///经纪公司代码
    char	*BrokerID = order->BrokerID;
    ///投资者代码
    char	*InvestorID = order->InvestorID;
    ///合约代码
    char	*InstrumentID = order->InstrumentID;
    ///报单引用
    char	*UserOrderLocalID = order->UserOrderLocalID;
    ///用户代码
    char	*UserID = order->UserID;
    ///报单价格条件
    char	OrderPriceType = order->OrderPriceType;
    ///买卖方向
    char	Direction[] = {order->Direction,'\0'};
    ///组合开平标志
    char	*CombOffsetFlag =&order->OffsetFlag;
    ///组合投机套保标志
    char	*CombHedgeFlag = &order->HedgeFlag;
    ///价格
    TUstpFtdcPriceType	limitPrice = order->LimitPrice;
    char LimitPrice[100];
    sprintf(LimitPrice,"%f",limitPrice);
    ///数量
    TUstpFtdcVolumeType	volumeTotalOriginal = order->Volume;
    char VolumeTotalOriginal[100];
    sprintf(VolumeTotalOriginal,"%d",volumeTotalOriginal);
    ///有效期类型
    TUstpFtdcTimeConditionType	TimeCondition = order->TimeCondition;
    ///GTD日期
    //TUstpFtdcDateType	GTDDate = order->GTDDate;
    ///成交量类型
    TUstpFtdcVolumeConditionType	VolumeCondition[] = {order->VolumeCondition,'\0'};
    ///最小成交量
    TUstpFtdcVolumeType	MinVolume = order->MinVolume;
    ///触发条件
//    TUstpFtdcContingentConditionType	ContingentCondition = order->ContingentCondition;
    ///止损价
    TUstpFtdcPriceType	StopPrice = order->StopPrice;
    ///强平原因
    TUstpFtdcForceCloseReasonType	ForceCloseReason = order->ForceCloseReason;
    ///自动挂起标志
    TUstpFtdcBoolType	IsAutoSuspend = order->IsAutoSuspend;
    ///业务单元
    //TUstpFtdcBusinessUnitType	BusinessUnit = order->BusinessUnit;
    ///请求编号
//    TUstpFtdcRequestIDType	requestID = order->RequestID;
//    char RequestID[100];
//    sprintf(RequestID,"%d",requestID);
    ///用户强评标志
//    TUstpFtdcBoolType	UserForceClose = order->;

    string ordreInfo;
    ordreInfo.append("BrokerID=").append(BrokerID);ordreInfo.append("\t");
    ordreInfo.append("InvestorID=").append(InvestorID);ordreInfo.append("\t");
    ordreInfo.append("InstrumentID=").append(InstrumentID);ordreInfo.append("\t");
    ordreInfo.append("UserOrderLocalID=").append(UserOrderLocalID);ordreInfo.append("\t");
    ordreInfo.append("UserID=").append(UserID);ordreInfo.append("\t");
    ordreInfo.append("Direction=").append(Direction);ordreInfo.append("\t");
    ordreInfo.append("CombOffsetFlag=").append(CombOffsetFlag);ordreInfo.append("\t");
    ordreInfo.append("CombHedgeFlag=").append(CombHedgeFlag);ordreInfo.append("\t");
    ordreInfo.append("LimitPrice=").append(LimitPrice);ordreInfo.append("\t");
    ordreInfo.append("VolumeTotalOriginal=").append(VolumeTotalOriginal);ordreInfo.append("\t");
    ordreInfo.append("VolumeCondition=").append(VolumeCondition);ordreInfo.append("\t");
//    ordreInfo.append("RequestID").append(RequestID);ordreInfo.append("\t");
    return ordreInfo;
}
//获取交易所回报响应
string CTraderSpi::getRtnOrder(CUstpFtdcOrderField *pOrder)
{
    ///经纪公司代码
    char	*BrokerID = pOrder->BrokerID;
    ///投资者代码
    char	*InvestorID = pOrder->InvestorID;
    ///合约代码
    char	*InstrumentID = pOrder->InstrumentID;
    ///报单引用
   // char	*UserOrderLocalID = pOrder->UserOrderLocalID;
    ///用户代码
    TUstpFtdcUserIDType	UserID;
    ///报单价格条件
    TUstpFtdcOrderPriceTypeType	OrderPriceType;
    ///买卖方向
    TUstpFtdcDirectionType	Direction = pOrder->Direction;
    ///组合开平标志
    TUstpFtdcOffsetFlagType	OffsetFlag;
    ///组合投机套保标志
    TUstpFtdcHedgeFlagType	HedgeFlag;
    ///价格
    TUstpFtdcPriceType	LimitPrice;
    ///数量
    TUstpFtdcVolumeType	VolumeTotalOriginal;
    ///有效期类型
    TUstpFtdcTimeConditionType	TimeCondition;
    ///GTD日期
    TUstpFtdcDateType	GTDDate;
    ///成交量类型
    TUstpFtdcVolumeConditionType	VolumeCondition;
    ///最小成交量
    TUstpFtdcVolumeType	MinVolume;
    ///触发条件
   // TUstpFtdcContingentConditionType	ContingentCondition;
    ///止损价
    TUstpFtdcPriceType	StopPrice;
    ///强平原因
    TUstpFtdcForceCloseReasonType	ForceCloseReason;
    ///自动挂起标志
    TUstpFtdcBoolType	IsAutoSuspend;
    ///业务单元
    TUstpFtdcBusinessUnitType	BusinessUnit;
    ///请求编号
//    TUstpFtdcRequestIDType	RequestID = pOrder->RequestID;
//    char cRequestId[100];
//    sprintf(cRequestId,"%d",RequestID);
    ///本地报单编号
    char	*OrderLocalID = pOrder->OrderLocalID;
    ///交易所代码
    TUstpFtdcExchangeIDType	ExchangeID;
    strcpy(ExchangeID,pOrder->ExchangeID);
    ///会员代码
    TUstpFtdcParticipantIDType	ParticipantID;
    ///客户代码
    char	*ClientID = pOrder->ClientID;
    ///合约在交易所的代码
   // TUstpFtdcExchangeInstIDType	ExchangeInstID;
    ///交易所交易员代码
    //TUstpFtdcTraderIDType	TraderID;
    ///安装编号
    //TUstpFtdcInstallIDType	InstallID;
    ///报单提交状态
//    char	OrderSubmitStatus = pOrder->OrderSubmitStatus;
//    char cOrderSubmitStatus[] = {OrderSubmitStatus,'\0'};
    //sprintf(cOrderSubmitStatus,"%s",OrderSubmitStatus);
    ///报单状态
    TUstpFtdcOrderStatusType	OrderStatus = pOrder->OrderStatus;
    char cOrderStatus[] = {OrderStatus,'\0'};
    //sprintf(cOrderStatus,"%s",OrderStatus);
    ///报单提示序号
    TUstpFtdcSequenceNoType	NotifySequence;
    ///交易日
    TUstpFtdcDateType	TradingDay;
    strcpy(TradingDay,pOrder->TradingDay);
    ///结算编号
    TUstpFtdcSettlementIDType	SettlementID;
    ///报单编号
    char	*OrderSysID = pOrder->OrderSysID;
    string str_ordersysid = boost::trim_copy(string(OrderSysID));

    ///报单来源
    TUstpFtdcOrderSourceType	OrderSource;
    ///报单类型
    //TUstpFtdcOrderTypeType	OrderType;
    ///今成交数量
    TUstpFtdcVolumeType	VolumeTraded = pOrder->VolumeTraded;
    char cVolumeTraded[100];
    sprintf(cVolumeTraded,"%d",VolumeTraded);
    ///剩余数量
    /*TUstpFtdcVolumeType	VolumeTotal = pOrder->VolumeTotal;
    char iVolumeTotal[100];
    sprintf(iVolumeTotal,"%d",VolumeTotal);*/
    ///报单日期
    TUstpFtdcDateType	InsertDate;
    ///委托时间
    TUstpFtdcTimeType	InsertTime;
    ///激活时间
    TUstpFtdcTimeType	ActiveTime;
    ///挂起时间
    TUstpFtdcTimeType	SuspendTime;
    ///最后修改时间
    TUstpFtdcTimeType	UpdateTime;
    ///撤销时间
    TUstpFtdcTimeType	CancelTime;
    ///最后修改交易所交易员代码
    //TUstpFtdcTraderIDType	ActiveTraderID;
    ///结算会员编号
    TUstpFtdcParticipantIDType	ClearingPartID;
    ///序号
    TUstpFtdcSequenceNoType	SequenceNo;
    ///前置编号
    //TUstpFtdcFrontIDType	FrontID;
    ///会话编号
    //TUstpFtdcSessionIDType	SessionID;
    ///用户端产品信息
    TUstpFtdcProductInfoType	UserProductInfo;
    ///状态信息
    //char	*StatusMsg = pOrder->StatusMsg;
    ///用户强评标志
    TUstpFtdcBoolType	UserForceClose;
    ///操作用户代码
    TUstpFtdcUserIDType	ActiveUserID;
    ///经纪公司报单编号
    TUstpFtdcSequenceNoType	BrokerOrderSeq;

//    char char_ordref[21] = {'\0'};
//    sprintf(char_ordref,"%s",pOrder->OrderRef);
//    string str_orderk = str_front_id+str_sessioin_id+string(char_ordref);
//    //处理报单回报
//    boost::recursive_mutex::scoped_lock SLock(order_mtx);
//    if(seq_map_orderref.find(str_orderk)==seq_map_orderref.end()){
//        string tmp_msg = "error:查询不到"+str_orderk+"对应的报单信息";
//        LOG(INFO)<<tmp_msg;
//    }else{
//        unordered_map<string,unordered_map<string,int64_t>>::iterator tmpit = seq_map_orderref.find(str_orderk);
//        int64_t ist_time = tmpit->second["ist_time"];
//        int64_t tmpendtime = GetSysTimeMicros();
//        //auto tmpendtime = boost::chrono::high_resolution_clock::now();
//        //没有对应关系
//        if(strlen(OrderSysID)  != 0 && seq_map_ordersysid.find(str_ordersysid)==seq_map_ordersysid.end()){
//            seq_map_ordersysid[str_ordersysid] = str_orderk;
//            LOG(INFO)<<"ordersysid="+str_ordersysid;
//        }
//        double elapse = boost::chrono::duration<double>(tmpendtime - ist_time).count();
//        if(strlen(OrderSysID) == 0){//CTP返回
//            tmpit->second["ctp_rtnorder"] = tmpendtime;
//        }else if(strcmp(cOrderStatus,"3") == 0){//未成交
//            tmpit->second["exchange_rtnorder_untrade"] = tmpendtime;
//        }else if(strcmp(cOrderStatus,"5") == 0){//已经撤单
//            tmpit->second["exchange_rtnorder_action"] =tmpendtime;
//        }else if(strcmp(cOrderStatus,"0") == 0){//全部成交
//            tmpit->second["exchange_rtnorder_alltrade"] = tmpendtime;
//        }
//    }
    string info;
    info.append(BrokerID);info.append("\t");
    info.append(InvestorID);info.append("\t");
    info.append(InstrumentID);info.append("\t");
    info.append(OrderLocalID);info.append("\t");
//    info.append(cRequestId);info.append("\t");
    info.append(ClientID);info.append("\t");
    info.append(OrderSysID);info.append("\t");
    info.append(cVolumeTraded);info.append("\t");
//    info.append(iVolumeTotal);info.append("\t");
//    info.append(StatusMsg);info.append("\t");
//    info.append(cOrderSubmitStatus);info.append("\t");
    info.append(cOrderStatus);info.append("\t");
    return info;
}
