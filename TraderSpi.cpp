// TraderSpi.cpp: implementation of the CTraderSpi class.
//
//////////////////////////////////////////////////////////////////////

#include "TraderSpi.h"
#include <unistd.h>
#include <iostream>
#include "globalutil.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace std;

// UserApi对象
extern CUstpFtdcTraderApi* pUserApi;
extern FILE * g_fpRecv;
extern int orderref;
// 请求编号
extern int iRequestID;
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
    sprintf(ORDER_REF,"%d",orderref);
    cout<<"------->"<<ORDER_REF<<endl;
    orderref++;
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
//    string str_osk = str_front_id + str_sessioin_id + string(req.OrderRef);
//    unordered_map<string,int64_t> tmpmap ;
//    tmpmap["ist_time"] = ist_time;
//    seq_map_orderref[str_osk] = tmpmap;
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
	CUstpFtdcReqUserLoginField reqUserLogin;
	memset(&reqUserLogin,0,sizeof(CUstpFtdcReqUserLoginField));		
    strcpy(reqUserLogin.BrokerID,BROKER_ID);
    strcpy(reqUserLogin.UserID, INVESTOR_ID);
    strcpy(reqUserLogin.Password, PASSWORD);
	strcpy(reqUserLogin.UserProductInfo,g_pProductInfo);		
	m_pUserApi->ReqUserLogin(&reqUserLogin, g_nOrdLocalID);	
	
    printf("请求登录，BrokerID=[%s]UserID=[%s]\n",BROKER_ID,INVESTOR_ID);
#ifdef WIN32
	Sleep(1000);
#else
	usleep(1000);
#endif
}
void CTraderSpi:: OnFrontDisconnected(int nReason)
{
    cerr << "--->>> " << "OnFrontDisconnected" << endl;
    cerr << "--->>> Reason = " << nReason << endl;
}

void CTraderSpi::OnRspUserLogin(CUstpFtdcRspUserLoginField *pRspUserLogin, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cerr << "--->>> " << "OnRspUserLogin" << endl;
    printf("登录失败...错误原因：%s\n",pRspInfo->ErrorMsg);
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("登录失败...错误原因：%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	g_nOrdLocalID=atoi(pRspUserLogin->MaxOrderLocalID)+1;
 	printf("-----------------------------\n");
 	printf("登录成功，最大本地报单号:%d\n",g_nOrdLocalID);
 	printf("-----------------------------\n");

 	StartAutoOrder();
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

void CTraderSpi::OnRspQryInvestorPosition(CUstpFtdcRspInvestorPositionField *pRspInvestorPosition, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("查询投资者持仓 错误原因：%s\n",pRspInfo->ErrorMsg);
		return;
	}
	
	if (pRspInvestorPosition==NULL)
	{
		printf("没有查询到投资者持仓\n");
		return ;
	}
	printf("-----------------------------\n");
	printf("交易所代码=[%s]\n",pRspInvestorPosition->ExchangeID);
	printf("经纪公司编号=[%s]\n",pRspInvestorPosition->BrokerID);
	printf("投资者编号=[%s]\n",pRspInvestorPosition->InvestorID);
	printf("客户代码=[%s]\n",pRspInvestorPosition->ClientID);
	printf("合约代码=[%s]\n",pRspInvestorPosition->InstrumentID);
	printf("买卖方向=[%c]\n",pRspInvestorPosition->Direction);
	printf("今持仓量=[%d]\n",pRspInvestorPosition->Position);
	printf("-----------------------------\n");
	return ;

}

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
    char	*OrderRef = order->UserOrderLocalID;
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
    TUstpFtdcRequestIDType	requestID = order->RequestID;
    char RequestID[100];
    sprintf(RequestID,"%d",requestID);
    ///用户强评标志
    TUstpFtdcBoolType	UserForceClose = order->ForceCloseReason;

    string ordreInfo;
    ordreInfo.append("BrokerID=").append(BrokerID);ordreInfo.append("\t");
    ordreInfo.append("InvestorID=").append(InvestorID);ordreInfo.append("\t");
    ordreInfo.append("InstrumentID=").append(InstrumentID);ordreInfo.append("\t");
    ordreInfo.append("OrderRef=").append(OrderRef);ordreInfo.append("\t");
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
