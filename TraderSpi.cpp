// TraderSpi.cpp: implementation of the CTraderSpi class.
//
//////////////////////////////////////////////////////////////////////

#include "TraderSpi.h"
#include <unistd.h>
#include <iostream>
#include "globalutil.h"

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
extern char  FRONT_ADDR;// 前置地址 新湖期货
// 请求编号
extern int iRequestID;
extern int g_nOrdLocalID;
extern double tick;
extern int isclose;
extern int offset_flag;
extern int limit_volume;
extern bool isrtntradeprocess;
extern boost::lockfree::queue<LogMsg*> logqueue;
extern unordered_map<string,unordered_map<string,int64_t>> seq_map_orderref;
extern unordered_map<string,string> seq_map_ordersysid;
//positionmap可重入锁
boost::recursive_mutex pst_mtx;
//orderinsertkey与ordersysid对应关系锁
boost::recursive_mutex order_mtx;
//记录时间
int ret = 0;
int start_process = 0;
CTraderSpi::CTraderSpi(CUstpFtdcTraderApi *pTrader):m_pUserApi(pTrader)
{

}

CTraderSpi::~CTraderSpi()
{

}

//记录时间
int CTraderSpi::md_orderinsert(double price,char *dir,char *offset,char * ins,int ordervolume){
    TUstpFtdcUserOrderLocalIDType ORDER_REF;
    char InstrumentID[31];
    strcpy(InstrumentID,ins);
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
    sprintf(ORDER_REF,"%d",g_nOrdLocalID);
    cout<<"------->"<<ORDER_REF<<endl;
    g_nOrdLocalID++;
    //报单结构体
    CUstpFtdcInputOrderField req;
    ///经纪公司代码
    strcpy(req.BrokerID, BROKER_ID);
    ///投资者代码
    strcpy(req.InvestorID, INVESTOR_ID);
    ///合约代码// UserApi对象
    CUstpFtdcTraderApi* pUserApi;
    strcpy(req.InstrumentID, InstrumentID);
    ///报单引用
    strcpy(req.UserOrderLocalID, ORDER_REF);

    ///用户代码
    //	TUstpFtdcUserIDType	UserID;
    strcpy(req.UserID,INVESTOR_ID);
    ///报单价格条件: 限价
    req.OrderPriceType = USTP_FTDC_OPT_LimitPrice;
    ///买卖方向:
    strcpy(&req.Direction,dir);
    ///组合开平标志: 开仓
    strcpy(&req.OffsetFlag ,offset);
    ///组合投机套保标志
    strcpy(&req.HedgeFlag,HedgeFlag);
    ///价格
    req.LimitPrice = Price;
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

    int nRequestID = ++iRequestID;
    char char_order_index[10]={'\0'};
    sprintf(char_order_index,"%d",nRequestID);
    //req.RequestID = nRequestID;

    nRequestID = ++iRequestID;
    char char_query_index[10]={'\0'};
    sprintf(char_query_index,"%d",nRequestID);

    //下单开始时间
//    int64_t ist_time = GetSysTimeMicros();
    //orderinsertkey
//    string str_osk = str_front_id + str_sessioin_id + string(req.g_nOrdLocalID);
//    unordered_map<string,int64_t> tmpmap ;
//    tmpmap["ist_time"] = ist_time;
//    seq_map_g_nOrdLocalID[str_osk] = tmpmap;
    //委托类操作，使用客户端定义的请求编号格式
    int iResult = pUserApi->ReqOrderInsert(&req,nRequestID);
    cerr << "--->>> ReqOrderInsert:" << ((iResult == 0) ? "success" : "failed") << endl;
    string msg = "order_requestid=" + string(char_order_index) + ";orderinsert_requestid=" + string(char_query_index);
    //LogMsg lmsg;
    //lmsg.setMsg(msg);;
    //logqueue.push(&lmsg);
    LOG(INFO) << msg;
    return 0;
}
void CTraderSpi::OnFrontConnected()
{
    cerr << "--->>> " << "OnFrontConnected" << endl;
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

    cerr << "hello" <<endl;
    CUstpFtdcReqUserLoginField reqUserLogin;
    memset(&reqUserLogin,0,sizeof(CUstpFtdcReqUserLoginField));
    strcpy(reqUserLogin.BrokerID,BROKER_ID);
    strcpy(reqUserLogin.UserID, INVESTOR_ID);
    strcpy(reqUserLogin.Password, PASSWORD);
    strcpy(reqUserLogin.UserProductInfo,"");
    int iResult = m_pUserApi->ReqUserLogin(&reqUserLogin, ++iRequestID);
//    m_pUserApi->ReqUserLogin(&reqUserLogin, g_nOrdLocalID);
//    CThostFtdcReqUserLoginField req;
//    //memset(&req, 0, sizeof(req));
//    strcpy(req.BrokerID, BROKER_ID);
//    strcpy(req.UserID, INVESTOR_ID);
//    strcpy(req.Password, PASSWORD);
//    int iResult = pUserApi->ReqUserLogin(&req, ++iRequestID);
    printf("请求登录，BrokerID=[%s]UserID=[%s]\n",BROKER_ID,INVESTOR_ID);
    cerr << "--->>> 发送用户登录请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
#ifdef WIN32
    Sleep(1000);
#else
    usleep(1000);
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
    sleep(200);
}
void CTraderSpi::OnRspUserLogin(CUstpFtdcRspUserLoginField *pRspUserLogin, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "--->>> " << "OnRspUserLogin" << endl;
    printf("登录失败...错误原因：%s\n",pRspInfo->ErrorMsg);
    if (bIsLast && !IsErrorRspInfo(pRspInfo))
    //if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
        g_nOrdLocalID=atoi(pRspUserLogin->MaxOrderLocalID)+1;
        printf("-----------------------------\n");
        printf("登录成功，最大本地报单号:%d\n",g_nOrdLocalID);
        printf("-----------------------------\n");
        //请求响应日志
        char char_msg[1024] = {'\0'};
        sprintf(char_msg,"--->>>登陆成功， 获取当前交易日 = %s,获取当前报单引用 =%d",pUserApi->GetTradingDay(),g_nOrdLocalID );
        string msg(char_msg);
        LOG(INFO)<<msg;
        //ReqQryInvestorPosition();
        ///投资者结算结果确认;
        cout<<msg<<endl;
        ReqQryInvestorPosition();
    }else{
        printf("-----------------------------\n");
        printf("登录失败...错误原因：%s\n",pRspInfo->ErrorMsg);
        printf("-----------------------------\n");
        return;
    }

    StartAutoOrder();
}
void CTraderSpi::ReqQryInvestorPosition()
{
    CUstpFtdcQryInvestorPositionField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, BROKER_ID);
    strcpy(req.InvestorID, INVESTOR_ID);
    //strcpy(req.InstrumentID, INSTRUMENT_ID);
    int iResult = pUserApi->ReqQryInvestorPosition(&req, ++g_nOrdLocalID);

    cerr << "--->>> 请求查询投资者持仓: " << ((iResult == 0) ? "成功" : "失败") << endl;
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
        for(;tmpit != positionmap.end();tmpit ++){
            string str_instrument = tmpit->first;
            unordered_map<string,int> tmppst = tmpit->second;
            int longpst = tmppst["longTotalPosition"];
            int shortpst = tmppst["shortTotalPosition"];
            char char_longpst[12] = {'\0'};
            char char_shortpst[12] = {'\0'};
            sprintf(char_longpst,"%d",longpst);
            sprintf(char_shortpst,"%d",shortpst);
            string pst_msg = "持仓结构:"+str_instrument + ",多头持仓量=" + string(char_longpst) + ",空头持仓量=" + string(char_shortpst) ;
            cout<<pst_msg<<endl;
            LOG(INFO)<<pst_msg;
        }
        cout<<"是否启动策略程序?0 否，1是"<<endl;
        cin>>isbeginmk;
        if(isbeginmk == 1){
            start_process = 1;
            isrtntradeprocess = true;
            // 初始化UserApi
//            mduserapi = CThostFtdcMdApi::CreateFtdcMdApi();			// 创建UserApi
//            CThostFtdcMdSpi* pUserSpi = new CMdSpi();
//            mduserapi->RegisterSpi(pUserSpi);						// 注册事件类
//            mduserapi->RegisterFront(MD_FRONT_ADDR);					// connect
//            mduserapi->Init();
        }
    }
}
void CTraderSpi::OnRspOrderInsert(CUstpFtdcInputOrderField *pInputOrder, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
        printf("order insert failed! the reason is：%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if(pInputOrder==NULL)
	{
        printf("no order info!\n");
		return;
	}
	printf("-----------------------------\n");
    printf("order insert success!!\n");
	printf("-----------------------------\n");
	return ;
	
}


void CTraderSpi::OnRtnTrade(CUstpFtdcTradeField *pTrade)
{
	printf("-----------------------------\n");
    printf("received rtn trade\n");
	Show(pTrade);
	printf("-----------------------------\n");
	return;
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
	printf("-----------------------------\n");
	printf("收到报单回报\n");
	Show(pOrder);
	printf("-----------------------------\n");
	return ;
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
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("查询投资者账户失败 错误原因：%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	
	if (pRspInvestorAccount==NULL)
	{
		printf("没有查询到投资者账户\n");
		return ;
	}
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
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("查询交易编码失败 错误原因：%s\n",pRspInfo->ErrorMsg);
		return;
	}
	
	if (pRspInstrument==NULL)
	{
		printf("没有查询到合约数据\n");
		return ;
	}
	
	Show(pRspInstrument);
	return ;
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
	
	printf("-----------------------------\n");
	printf("交易所代码=[%s]\n",pInstrumentStatus->ExchangeID);
	printf("品种代码=[%s]\n",pInstrumentStatus->ProductID);
	printf("品种名称=[%s]\n",pInstrumentStatus->ProductName);
	printf("合约代码=[%s]\n",pInstrumentStatus->InstrumentID);
	printf("合约名称=[%s]\n",pInstrumentStatus->InstrumentName);
	printf("交割年份=[%d]\n",pInstrumentStatus->DeliveryYear);
	printf("交割月=[%d]\n",pInstrumentStatus->DeliveryMonth);
	printf("限价单最大下单量=[%d]\n",pInstrumentStatus->MaxLimitOrderVolume);
	printf("限价单最小下单量=[%d]\n",pInstrumentStatus->MinLimitOrderVolume);
	printf("市价单最大下单量=[%d]\n",pInstrumentStatus->MaxMarketOrderVolume);
	printf("市价单最小下单量=[%d]\n",pInstrumentStatus->MinMarketOrderVolume);
	
	printf("数量乘数=[%d]\n",pInstrumentStatus->VolumeMultiple);
	printf("报价单位=[%lf]\n",pInstrumentStatus->PriceTick);
	printf("币种=[%c]\n",pInstrumentStatus->Currency);
	printf("多头限仓=[%d]\n",pInstrumentStatus->LongPosLimit);
	printf("空头限仓=[%d]\n",pInstrumentStatus->ShortPosLimit);
	printf("跌停板价=[%lf]\n",pInstrumentStatus->LowerLimitPrice);
	printf("涨停板价=[%lf]\n",pInstrumentStatus->UpperLimitPrice);
	printf("昨结算=[%lf]\n",pInstrumentStatus->PreSettlementPrice);
	printf("合约交易状态=[%c]\n",pInstrumentStatus->InstrumentStatus);
	
	printf("创建日=[%s]\n",pInstrumentStatus->CreateDate);
	printf("上市日=[%s]\n",pInstrumentStatus->OpenDate);
	printf("到期日=[%s]\n",pInstrumentStatus->ExpireDate);
	printf("开始交割日=[%s]\n",pInstrumentStatus->StartDelivDate);
	printf("最后交割日=[%s]\n",pInstrumentStatus->EndDelivDate);
	printf("挂牌基准价=[%lf]\n",pInstrumentStatus->BasisPrice);
	printf("当前是否交易=[%d]\n",pInstrumentStatus->IsTrading);
	printf("基础商品代码=[%s]\n",pInstrumentStatus->UnderlyingInstrID);
	printf("持仓类型=[%c]\n",pInstrumentStatus->PositionType);
	printf("执行价=[%lf]\n",pInstrumentStatus->StrikePrice);
	printf("期权类型=[%c]\n",pInstrumentStatus->OptionsType);

	printf("-----------------------------\n");
	printf("[%d]",icount);
	return ;

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
    char	*g_nOrdLocalID = order->UserOrderLocalID;
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
    ordreInfo.append("g_nOrdLocalID=").append(g_nOrdLocalID);ordreInfo.append("\t");
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
    cout<<"iserror"<<bResult<<" "<< pRspInfo<<endl;
//    string errmsg = boosttoolsnamespace::CBoostTools::gbktoutf8(pRspInfo->ErrorMsg);

    char char_msg[1024]={'\0'};
    if (bResult){
        string errmsg =pRspInfo->ErrorMsg;
        if(pRspInfo->ErrorID == 22){//重复的ref
            g_nOrdLocalID += 1000;
            cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << errmsg <<",g_nOrdLocalID增加="<<g_nOrdLocalID<<endl;
            sprintf(char_msg, "--->>> ErrorID=%d,ErrorMsg=%s,g_nOrdLocalID增加=%d",pRspInfo->ErrorID,  pRspInfo->ErrorMsg,g_nOrdLocalID);

        }else if(pRspInfo->ErrorID == 31){//资金不足
            isclose = 2;
            cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << errmsg <<",isclose平仓开仓方式修改为:"<<isclose<<endl;
            sprintf(char_msg, "--->>> ErrorID=%d,ErrorMsg=%s,isclose平仓开仓方式修改为:%d",pRspInfo->ErrorID,  pRspInfo->ErrorMsg,isclose);
        }else if(pRspInfo->ErrorID == 30){//平仓量超过持仓量
            isclose = 1;
            cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << errmsg <<",isclose平仓开仓方式修改为:"<<isclose<<endl;
            sprintf(char_msg, "--->>> ErrorID=%d,ErrorMsg=%s,isclose平仓开仓方式修改为:%d",pRspInfo->ErrorID,  pRspInfo->ErrorMsg,isclose);
        }else if(pRspInfo->ErrorID == 51){//平昨仓位不足
            offset_flag = 3;
            cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << errmsg <<",平仓方式修改为平今:"<<offset_flag<<endl;
            sprintf(char_msg, "--->>> ErrorID=%d,ErrorMsg=%s,平仓方式修改为平今:%d",pRspInfo->ErrorID,  pRspInfo->ErrorMsg,offset_flag);
        }else if(pRspInfo->ErrorID == 50){//平今仓位不足
            isclose = 1;
            cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << errmsg <<",isclose平仓开仓方式修改为:"<<isclose<<endl;
            sprintf(char_msg, "--->>> ErrorID=%d,ErrorMsg=%s,isclose平仓开仓方式修改为:%d",pRspInfo->ErrorID,  pRspInfo->ErrorMsg,isclose);
        }else if(pRspInfo->ErrorID == 3){//不合法的登录
            cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << errmsg <<endl;
            sprintf(char_msg, "--->>> ErrorID=%d,ErrorMsg=%s",pRspInfo->ErrorID,errmsg);
        }else{
            cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << boosttoolsnamespace::CBoostTools::gbktoutf8(pRspInfo->ErrorMsg) <<endl;

            sprintf(char_msg, "--->>> ErrorID=%d,ErrorMsg=%s",pRspInfo->ErrorID,  boosttoolsnamespace::CBoostTools::gbktoutf8(pRspInfo->ErrorMsg));
        }
        string msg(char_msg);
        cout<<"----------"<<msg<<endl;
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
        if("2" == str_dir){//买
            //多头
            tmpmap["longTotalPosition"] = position;
            //空头
            tmpmap["shortTotalPosition"] = 0;
        }else if("3" == str_dir){//空
            //空头
            tmpmap["shortTotalPosition"] = position;
            tmpmap["longTotalPosition"] = 0;
        }else{
            cout<<InstrumentID<<";error:持仓类型无法判断PosiDirection="<<str_dir<<endl;
            exit;
        }
        positionmap[str_instrumentid] = tmpmap;
    }else{
        unordered_map<string,unordered_map<string,int>>::iterator tmpmap  = positionmap.find(str_instrumentid);
        //对应的反方向应该已经存在，这里后续需要确认
        if("2" == str_dir){//多
            //多头
            tmpmap->second["longTotalPosition"] = position + tmpmap->second["longTotalPosition"];
        }else if("3" == str_dir){//空
            //空头
            tmpmap->second["shortTotalPosition"] = position + tmpmap->second["shortTotalPosition"] ;
        }else{
            cout<<InstrumentID<<";error:持仓类型无法判断PosiDirection="<<str_dir<<endl;
            exit;
        }
    }
    storeInvestorPosition(pInvestorPosition);
}
//将投资者持仓信息写入文件保存
int CTraderSpi::storeInvestorPosition(CUstpFtdcRspInvestorPositionField *pInvestorPosition)
{

    ///合约代码
    char	*InstrumentID = pInvestorPosition->InstrumentID;
    ///经纪公司代码
    char	*BrokerID = pInvestorPosition->BrokerID;
    ///投资者代码
    char	*InvestorID = pInvestorPosition->InvestorID;
    ///持仓多空方向
    TUstpFtdcDirectionType	dir = pInvestorPosition->Direction;
    char PosiDirection[] = {dir,'\0'};
    ///投机套保标志
    TUstpFtdcHedgeFlagType	flag = pInvestorPosition->HedgeFlag;
    char HedgeFlag[] = {flag,'\0'};
    ///持仓日期
//    TUstpFtdcPositionDateType	positionDate = pInvestorPosition->PositionDate;
//    char PositionDate[] = {positionDate,'\0'};
    ///上日持仓
    TUstpFtdcVolumeType	ydPosition = pInvestorPosition->YdPosition;
    char YdPosition[100];
    sprintf(YdPosition,"%d",ydPosition);
    ///今日持仓
    TUstpFtdcVolumeType	position = pInvestorPosition->Position;
    char Position[100];
    sprintf(Position,"%d",position);
    ///多头冻结
    //TUstpFtdcVolumeType	LongFrozen = pInvestorPosition->LongFrozen;
    ///空头冻结
    //TUstpFtdcVolumeType	ShortFrozen = pInvestorPosition->ShortFrozen;
    ///开仓冻结金额
    //TUstpFtdcMoneyType	LongFrozenAmount = pInvestorPosition->LongFrozenAmount;
    ///开仓冻结金额
    //TUstpFtdcMoneyType	ShortFrozenAmount = pInvestorPosition->ShortFrozenAmount;
    ///开仓量
    //TUstpFtdcVolumeType	openVolume = pInvestorPosition->OpenVolume;
//    char OpenVolume[100] ;
//    sprintf(OpenVolume,"%d",openVolume);
    ///平仓量
    //TUstpFtdcVolumeType	closeVolume = pInvestorPosition->CloseVolume;
    //char CloseVolume[100];
    //sprintf(CloseVolume,"%d",closeVolume);
    ///开仓金额
    //TUstpFtdcMoneyType	OpenAmount = pInvestorPosition->OpenAmount;
    ///平仓金额
    //TUstpFtdcMoneyType	CloseAmount = pInvestorPosition->CloseAmount;
    ///持仓成本
    TUstpFtdcMoneyType	positionCost = pInvestorPosition->PositionCost;
    char PositionCost[100];
    sprintf(PositionCost,"%f",positionCost);
    ///上次占用的保证金
   // TUstpFtdcMoneyType	PreMargin = pInvestorPosition->PreMargin;
    ///占用的保证金
    //TUstpFtdcMoneyType	UseMargin = pInvestorPosition->UseMargin;
    ///冻结的保证金
    TUstpFtdcMoneyType	FrozenMargin = pInvestorPosition->FrozenMargin;
    ///冻结的资金
    //TUstpFtdcMoneyType	FrozenCash = pInvestorPosition->FrozenCash;
    ///冻结的手续费
    //TUstpFtdcMoneyType	FrozenCommission = pInvestorPosition->FrozenCommission;
    ///资金差额
    //TUstpFtdcMoneyType	CashIn = pInvestorPosition->CashIn;
    ///手续费
    //TUstpFtdcMoneyType	Commission = pInvestorPosition->Commission;
    ///平仓盈亏
    //TUstpFtdcMoneyType	CloseProfit = pInvestorPosition->CloseProfit;
    ///持仓盈亏
    //TUstpFtdcMoneyType	PositionProfit = pInvestorPosition->PositionProfit;
    ///上次结算价
    //TUstpFtdcPriceType	preSettlementPrice = pInvestorPosition->PreSettlementPrice;
    //char PreSettlementPrice[100];
    //sprintf(PreSettlementPrice,"%f",preSettlementPrice);
    ///本次结算价
    //TUstpFtdcPriceType	SettlementPrice = pInvestorPosition->PreSettlementPrice;
    ///交易日
    //char	*TradingDay = pInvestorPosition->TradingDay;
    ///结算编号
    TUstpFtdcSettlementIDType	SettlementID;
    ///开仓成本
    //TUstpFtdcMoneyType	openCost = pInvestorPosition->OpenCost;
    //char OpenCost[100] ;
    //sprintf(OpenCost,"%f",openCost);
    ///交易所保证金
    //TUstpFtdcMoneyType	exchangeMargin = pInvestorPosition->ExchangeMargin;
    //char ExchangeMargin[100];
    //sprintf(ExchangeMargin,"%f",exchangeMargin);
    ///组合成交形成的持仓
    TUstpFtdcVolumeType	CombPosition;
    ///组合多头冻结
    TUstpFtdcVolumeType	CombLongFrozen;
    ///组合空头冻结
    TUstpFtdcVolumeType	CombShortFrozen;
    ///逐日盯市平仓盈亏
    //TUstpFtdcMoneyType	CloseProfitByDate = pInvestorPosition->CloseProfitByDate;
    ///逐笔对冲平仓盈亏
    //TUstpFtdcMoneyType	CloseProfitByTrade = pInvestorPosition->CloseProfitByTrade;
    ///今日持仓
    //TUstpFtdcVolumeType	todayPosition = pInvestorPosition->TodayPosition;
   // char TodayPosition[100] ;
    //sprintf(TodayPosition,"%d",todayPosition);
    ///保证金率
    //TUstpFtdcRatioType	marginRateByMoney = pInvestorPosition->MarginRateByMoney;
    //char MarginRateByMoney[100];
    //sprintf(MarginRateByMoney,"%f",marginRateByMoney);
    ///保证金率(按手数)
    //TUstpFtdcRatioType	marginRateByVolume = pInvestorPosition->MarginRateByVolume;
    //char MarginRateByVolume[100];
    //sprintf(MarginRateByVolume,"%f",marginRateByVolume);
    string sInvestorInfo;
    sInvestorInfo.append("InstrumentID=").append(InstrumentID);sInvestorInfo.append("\t");
    sInvestorInfo.append("BrokerID=").append(BrokerID);sInvestorInfo.append("\t");
    sInvestorInfo.append("InvestorID=").append(InvestorID);sInvestorInfo.append("\t");
    sInvestorInfo.append("PosiDirection=").append(PosiDirection);sInvestorInfo.append("\t");
    sInvestorInfo.append("HedgeFlag=").append(HedgeFlag);sInvestorInfo.append("\t");
    //sInvestorInfo.append("PositionDate=").append(PositionDate);sInvestorInfo.append("\t");
    sInvestorInfo.append("YdPosition=").append(YdPosition);sInvestorInfo.append("\t");
    sInvestorInfo.append("Position=").append(Position);sInvestorInfo.append("\t");
   // sInvestorInfo.append("OpenVolume=").append(OpenVolume);sInvestorInfo.append("\t");
    //sInvestorInfo.append("CloseVolume=").append(CloseVolume);sInvestorInfo.append("\t");
    sInvestorInfo.append("PositionCost=").append(PositionCost);sInvestorInfo.append("\t");
   // sInvestorInfo.append("PreSettlementPrice=").append(PreSettlementPrice);sInvestorInfo.append("\t");

    //sInvestorInfo.append("TradingDay=").append(TradingDay);sInvestorInfo.append("\t");
    //sInvestorInfo.append("OpenCost=").append(OpenCost);sInvestorInfo.append("\t");
    //sInvestorInfo.append("ExchangeMargin=").append(ExchangeMargin);sInvestorInfo.append("\t");
    //sInvestorInfo.append("TodayPosition=").append(TodayPosition);sInvestorInfo.append("\t");
    //sInvestorInfo.append("MarginRateByMoney=").append(MarginRateByMoney);sInvestorInfo.append("\t");
    //sInvestorInfo.append("MarginRateByVolume=").append(MarginRateByVolume);sInvestorInfo.append("\t");
    LOG(INFO)<<sInvestorInfo;
    return 0;
}
