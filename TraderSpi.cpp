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

// UserApi����
extern CUstpFtdcTraderApi* pUserApi;
extern FILE * g_fpRecv;
extern int orderref;
// ������
extern int iRequestID;
CTraderSpi::CTraderSpi(CUstpFtdcTraderApi *pTrader):m_pUserApi(pTrader)
{

}

CTraderSpi::~CTraderSpi()
{

}

//��¼ʱ��
int CTraderSpi::md_orderinsert(double price,char *dir,char *offset,char * ins,int ordervolume){
    TUstpFtdcUserOrderLocalIDType ORDER_REF;
    char InstrumentID[31];
    strcpy(InstrumentID,ins);
    char Direction[2];
    strcpy(Direction,dir);
    ///Ͷ�� '1';�ױ�'3'
    char HedgeFlag[]="1";
    //��Ͽ�ƽ��־: ���� '0';ƽ�� '1';ƽ�� '3';ƽ�� '4';ǿƽ '2'
    char OffsetFlag[2];
    strcpy(OffsetFlag,offset);
    ///�۸� double
    TUstpFtdcPriceType Price = price;
    //��������
    int Volume = ordervolume;
    //�������ñ��
    sprintf(ORDER_REF,"%d",orderref);
    cout<<"------->"<<ORDER_REF<<endl;
    orderref++;
    //�����ṹ��
    CUstpFtdcInputOrderField req;
    ///���͹�˾����
    strcpy(req.BrokerID, BROKER_ID);
    ///Ͷ���ߴ���
    strcpy(req.InvestorID, INVESTOR_ID);
    ///��Լ����// UserApi����
    CUstpFtdcTraderApi* pUserApi;
    strcpy(req.InstrumentID, InstrumentID);
    ///��������
    strcpy(req.UserOrderLocalID, ORDER_REF);

    ///�û�����
    //	TUstpFtdcUserIDType	UserID;
    strcpy(req.UserID,INVESTOR_ID);
    ///�����۸�����: �޼�
    req.OrderPriceType = USTP_FTDC_OPT_LimitPrice;
    ///��������:
    strcpy(&req.Direction,dir);
    ///��Ͽ�ƽ��־: ����
    strcpy(&req.OffsetFlag ,offset);
    ///���Ͷ���ױ���־
    strcpy(&req.HedgeFlag,HedgeFlag);
    ///�۸�
    req.LimitPrice = Price;
    ///����: 1
    req.Volume = Volume;
    ///��Ч������: ������Ч
    //req.TimeCondition = THOST_FTDC_TC_GFD;
    req.TimeCondition = USTP_FTDC_TC_IOC;
    ///GTD����
    //TUstpFtdcDateType	GTDDate;
    strcpy(req.GTDDate,"");
    ///�ɽ�������: �κ�����
    req.VolumeCondition = USTP_FTDC_VC_AV;
    ///��// ������
    //int iRequestID = 0;
    //С�ɽ���: 1
    req.MinVolume = 1;
    ///��������: ����
    //req.ContingentCondition = THOST_FTDC_CC_Immediately;
    ///ֹ���
    //TUstpFtdcPriceType	StopPrice;
    req.StopPrice = 0;
    ///ǿƽԭ��: ��ǿƽ
    req.ForceCloseReason = USTP_FTDC_FCR_NotForceClose;
    ///�Զ������־: ��
    req.IsAutoSuspend = 0;
    ///ҵ��Ԫ
    //	TUstpFtdcBusinessUnitType	BusinessUnit;
    strcpy(req.BusinessUnit,"");
    ///������
    //	TUstpFtdcRequestIDType	RequestID;
    ///�û�ǿ����־: ��
    //req.UserForceClose = 0;///���͹�˾����
    //

    int nRequestID = ++iRequestID;
    char char_order_index[10]={'\0'};
    sprintf(char_order_index,"%d",nRequestID);
    //req.RequestID = nRequestID;

    nRequestID = ++iRequestID;
    char char_query_index[10]={'\0'};
    sprintf(char_query_index,"%d",nRequestID);

    //�µ���ʼʱ��
//    int64_t ist_time = GetSysTimeMicros();
    //orderinsertkey
//    string str_osk = str_front_id + str_sessioin_id + string(req.OrderRef);
//    unordered_map<string,int64_t> tmpmap ;
//    tmpmap["ist_time"] = ist_time;
//    seq_map_orderref[str_osk] = tmpmap;
    //ί���������ʹ�ÿͻ��˶���������Ÿ�ʽ
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
	
    printf("�����¼��BrokerID=[%s]UserID=[%s]\n",BROKER_ID,INVESTOR_ID);
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
    printf("��¼ʧ��...����ԭ��%s\n",pRspInfo->ErrorMsg);
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("��¼ʧ��...����ԭ��%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	g_nOrdLocalID=atoi(pRspUserLogin->MaxOrderLocalID)+1;
 	printf("-----------------------------\n");
 	printf("��¼�ɹ�����󱾵ر�����:%d\n",g_nOrdLocalID);
 	printf("-----------------------------\n");

 	StartAutoOrder();
}

void CTraderSpi::OnRspOrderInsert(CUstpFtdcInputOrderField *pInputOrder, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
        printf("order insert failed! the reason is��%s\n",pRspInfo->ErrorMsg);
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
	printf("����������=[%s]\n",pOrder->ExchangeID);
	printf("������=[%s]\n",pOrder->TradingDay);
	printf("��Ա���=[%s]\n",pOrder->ParticipantID);
	printf("�µ�ϯλ��=[%s]\n",pOrder->SeatID);
	printf("Ͷ���߱��=[%s]\n",pOrder->InvestorID);
	printf("�ͻ���=[%s]\n",pOrder->ClientID);
	printf("ϵͳ�������=[%s]\n",pOrder->OrderSysID);
	printf("���ر������=[%s]\n",pOrder->OrderLocalID);
	printf("�û����ر�����=[%s]\n",pOrder->UserOrderLocalID);
	printf("��Լ����=[%s]\n",pOrder->InstrumentID);
	printf("�����۸�����=[%c]\n",pOrder->OrderPriceType);
	printf("��������=[%c]\n",pOrder->Direction);
	printf("��ƽ��־=[%c]\n",pOrder->OffsetFlag);
	printf("Ͷ���ױ���־=[%c]\n",pOrder->HedgeFlag);
	printf("�۸�=[%lf]\n",pOrder->LimitPrice);
	printf("����=[%d]\n",pOrder->Volume);
	printf("������Դ=[%c]\n",pOrder->OrderSource);
	printf("����״̬=[%c]\n",pOrder->OrderStatus);
	printf("����ʱ��=[%s]\n",pOrder->InsertTime);
	printf("����ʱ��=[%s]\n",pOrder->CancelTime);
	printf("��Ч������=[%c]\n",pOrder->TimeCondition);
	printf("GTD����=[%s]\n",pOrder->GTDDate);
	printf("��С�ɽ���=[%d]\n",pOrder->MinVolume);
	printf("ֹ���=[%lf]\n",pOrder->StopPrice);
	printf("ǿƽԭ��=[%c]\n",pOrder->ForceCloseReason);
	printf("�Զ������־=[%d]\n",pOrder->IsAutoSuspend);
	printf("-----------------------------\n");
	return ;
}

void CTraderSpi::OnRtnOrder(CUstpFtdcOrderField *pOrder)
{
	printf("-----------------------------\n");
	printf("�յ������ر�\n");
	Show(pOrder);
	printf("-----------------------------\n");
	return ;
}

void CTraderSpi::OnRspOrderAction(CUstpFtdcOrderActionField *pOrderAction, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("����ʧ�� ����ԭ��%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if(pOrderAction==NULL)
	{
		printf("û�г�������\n");
		return;
	}
	printf("-----------------------------\n");
	printf("�����ɹ�\n");
	printf("-----------------------------\n");
	return ;
}
void CTraderSpi::OnRspUserPasswordUpdate(CUstpFtdcUserPasswordUpdateField *pUserPasswordUpdate, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("�޸�����ʧ�� ����ԭ��%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if(pUserPasswordUpdate==NULL)
	{
		printf("û���޸���������\n");
		return;
	}
	printf("-----------------------------\n");
	printf("�޸�����ɹ�\n");
	printf("-----------------------------\n");
	return ;
}

void CTraderSpi::OnErrRtnOrderInsert(CUstpFtdcInputOrderField *pInputOrder, CUstpFtdcRspInfoField *pRspInfo)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("��������ر�ʧ�� ����ԭ��%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if(pInputOrder==NULL)
	{
		printf("û������\n");
		return;
	}
	printf("-----------------------------\n");
	printf("��������ر�\n");
	printf("-----------------------------\n");
	return ;
}
void CTraderSpi::OnErrRtnOrderAction(CUstpFtdcOrderActionField *pOrderAction, CUstpFtdcRspInfoField *pRspInfo)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("��������ر�ʧ�� ����ԭ��%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if(pOrderAction==NULL)
	{
		printf("û������\n");
		return;
	}
	printf("-----------------------------\n");
	printf("��������ر�\n");
	printf("-----------------------------\n");
	return ;
}

void CTraderSpi::OnRspQryOrder(CUstpFtdcOrderField *pOrder, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("��ѯ����ʧ�� ����ԭ��%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if(pOrder==NULL)
	{
		printf("û�в�ѯ����������\n");
		return;
	}
	Show(pOrder);
	return ;
}
void CTraderSpi::Show(CUstpFtdcTradeField *pTrade)
{
	printf("-----------------------------\n");
	printf("����������=[%s]\n",pTrade->ExchangeID);
	printf("������=[%s]\n",pTrade->TradingDay);
	printf("��Ա���=[%s]\n",pTrade->ParticipantID);
	printf("�µ�ϯλ��=[%s]\n",pTrade->SeatID);
	printf("Ͷ���߱��=[%s]\n",pTrade->InvestorID);
	printf("�ͻ���=[%s]\n",pTrade->ClientID);
	printf("�ɽ����=[%s]\n",pTrade->TradeID);

	printf("�û����ر�����=[%s]\n",pTrade->UserOrderLocalID);
	printf("��Լ����=[%s]\n",pTrade->InstrumentID);
	printf("��������=[%c]\n",pTrade->Direction);
	printf("��ƽ��־=[%c]\n",pTrade->OffsetFlag);
	printf("Ͷ���ױ���־=[%c]\n",pTrade->HedgeFlag);
	printf("�ɽ��۸�=[%lf]\n",pTrade->TradePrice);
	printf("�ɽ�����=[%d]\n",pTrade->TradeVolume);
	printf("�����Ա���=[%s]\n",pTrade->ClearingPartID);
	
	printf("-----------------------------\n");
	return ;
}
void CTraderSpi::OnRspQryTrade(CUstpFtdcTradeField *pTrade, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("��ѯ�ɽ�ʧ�� ����ԭ��%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if(pTrade==NULL)
	{
		printf("û�в�ѯ���ɽ�����");
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
		printf("��ѯ������ʧ�� ����ԭ��%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	if (pRspExchange==NULL)
	{
		printf("û�в�ѯ����������Ϣ\n");
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
		printf("��ѯͶ�����˻�ʧ�� ����ԭ��%s\n",pRspInfo->ErrorMsg);
		printf("-----------------------------\n");
		return;
	}
	
	if (pRspInvestorAccount==NULL)
	{
		printf("û�в�ѯ��Ͷ�����˻�\n");
		return ;
	}
	printf("-----------------------------\n");
	printf("Ͷ���߱��=[%s]\n",pRspInvestorAccount->InvestorID);
	printf("�ʽ��ʺ�=[%s]\n",pRspInvestorAccount->AccountID);
	printf("�ϴν���׼����=[%lf]\n",pRspInvestorAccount->PreBalance);
	printf("�����=[%lf]\n",pRspInvestorAccount->Deposit);
	printf("������=[%lf]\n",pRspInvestorAccount->Withdraw);
	printf("����ı�֤��=[%lf]\n",pRspInvestorAccount->FrozenMargin);
	printf("����������=[%lf]\n",pRspInvestorAccount->FrozenFee);
	printf("������=[%lf]\n",pRspInvestorAccount->Fee);
	printf("ƽ��ӯ��=[%lf]\n",pRspInvestorAccount->CloseProfit);
	printf("�ֲ�ӯ��=[%lf]\n",pRspInvestorAccount->PositionProfit);
	printf("�����ʽ�=[%lf]\n",pRspInvestorAccount->Available);
	printf("��ͷ����ı�֤��=[%lf]\n",pRspInvestorAccount->LongFrozenMargin);
	printf("��ͷ����ı�֤��=[%lf]\n",pRspInvestorAccount->ShortFrozenMargin);
	printf("��ͷ��֤��=[%lf]\n",pRspInvestorAccount->LongMargin);
	printf("��ͷ��֤��=[%lf]\n",pRspInvestorAccount->ShortMargin);
	printf("-----------------------------\n");

}

void CTraderSpi::OnRspQryUserInvestor(CUstpFtdcRspUserInvestorField *pUserInvestor, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("-----------------------------\n");
		printf("��ѯ����Ͷ����ʧ�� ����ԭ��%s\n",pRspInfo->ErrorMsg);
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
	printf("����������=[%s]\n",pRspInstrument->ExchangeID);
	printf("Ʒ�ִ���=[%s]\n",pRspInstrument->ProductID);
	printf("Ʒ������=[%s]\n",pRspInstrument->ProductName);
	printf("��Լ����=[%s]\n",pRspInstrument->InstrumentID);
	printf("��Լ����=[%s]\n",pRspInstrument->InstrumentName);
	printf("�������=[%d]\n",pRspInstrument->DeliveryYear);
	printf("������=[%d]\n",pRspInstrument->DeliveryMonth);
	printf("�޼۵�����µ���=[%d]\n",pRspInstrument->MaxLimitOrderVolume);
	printf("�޼۵���С�µ���=[%d]\n",pRspInstrument->MinLimitOrderVolume);
	printf("�м۵�����µ���=[%d]\n",pRspInstrument->MaxMarketOrderVolume);
	printf("�м۵���С�µ���=[%d]\n",pRspInstrument->MinMarketOrderVolume);
	
	printf("��������=[%d]\n",pRspInstrument->VolumeMultiple);
	printf("���۵�λ=[%lf]\n",pRspInstrument->PriceTick);
	printf("����=[%c]\n",pRspInstrument->Currency);
	printf("��ͷ�޲�=[%d]\n",pRspInstrument->LongPosLimit);
	printf("��ͷ�޲�=[%d]\n",pRspInstrument->ShortPosLimit);
	printf("��ͣ���=[%lf]\n",pRspInstrument->LowerLimitPrice);
	printf("��ͣ���=[%lf]\n",pRspInstrument->UpperLimitPrice);
	printf("�����=[%lf]\n",pRspInstrument->PreSettlementPrice);
	printf("��Լ����״̬=[%c]\n",pRspInstrument->InstrumentStatus);
	
	printf("������=[%s]\n",pRspInstrument->CreateDate);
	printf("������=[%s]\n",pRspInstrument->OpenDate);
	printf("������=[%s]\n",pRspInstrument->ExpireDate);
	printf("��ʼ������=[%s]\n",pRspInstrument->StartDelivDate);
	printf("��󽻸���=[%s]\n",pRspInstrument->EndDelivDate);
	printf("���ƻ�׼��=[%lf]\n",pRspInstrument->BasisPrice);
	printf("��ǰ�Ƿ���=[%d]\n",pRspInstrument->IsTrading);
	printf("������Ʒ����=[%s]\n",pRspInstrument->UnderlyingInstrID);
	printf("�ֲ�����=[%c]\n",pRspInstrument->PositionType);
	printf("ִ�м�=[%lf]\n",pRspInstrument->StrikePrice);
	printf("��Ȩ����=[%c]\n",pRspInstrument->OptionsType);
	printf("-----------------------------\n");
	
}
void CTraderSpi::OnRspQryInstrument(CUstpFtdcRspInstrumentField *pRspInstrument, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("��ѯ���ױ���ʧ�� ����ԭ��%s\n",pRspInfo->ErrorMsg);
		return;
	}
	
	if (pRspInstrument==NULL)
	{
		printf("û�в�ѯ����Լ����\n");
		return ;
	}
	
	Show(pRspInstrument);
	return ;
}

void CTraderSpi::OnRspQryTradingCode(CUstpFtdcRspTradingCodeField *pTradingCode, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("��ѯ���ױ���ʧ�� ����ԭ��%s\n",pRspInfo->ErrorMsg);
		return;
	}
	
	if (pTradingCode==NULL)
	{
		printf("û�в�ѯ�����ױ���\n");
		return ;
	}
	printf("-----------------------------\n");
	printf("����������=[%s]\n",pTradingCode->ExchangeID);
	printf("���͹�˾���=[%s]\n",pTradingCode->BrokerID);
	printf("Ͷ���߱��=[%s]\n",pTradingCode->InvestorID);
	printf("�ͻ�����=[%s]\n",pTradingCode->ClientID);
	printf("�ͻ�����Ȩ��=[%d]\n",pTradingCode->ClientRight);
	printf("�Ƿ��Ծ=[%c]\n",pTradingCode->IsActive);
	printf("-----------------------------\n");
	return ;
}

void CTraderSpi::OnRspQryInvestorPosition(CUstpFtdcRspInvestorPositionField *pRspInvestorPosition, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("��ѯͶ���ֲ߳� ����ԭ��%s\n",pRspInfo->ErrorMsg);
		return;
	}
	
	if (pRspInvestorPosition==NULL)
	{
		printf("û�в�ѯ��Ͷ���ֲ߳�\n");
		return ;
	}
	printf("-----------------------------\n");
	printf("����������=[%s]\n",pRspInvestorPosition->ExchangeID);
	printf("���͹�˾���=[%s]\n",pRspInvestorPosition->BrokerID);
	printf("Ͷ���߱��=[%s]\n",pRspInvestorPosition->InvestorID);
	printf("�ͻ�����=[%s]\n",pRspInvestorPosition->ClientID);
	printf("��Լ����=[%s]\n",pRspInvestorPosition->InstrumentID);
	printf("��������=[%c]\n",pRspInvestorPosition->Direction);
	printf("��ֲ���=[%d]\n",pRspInvestorPosition->Position);
	printf("-----------------------------\n");
	return ;

}

	///Ͷ�����������ʲ�ѯӦ��
void CTraderSpi::OnRspQryInvestorFee(CUstpFtdcInvestorFeeField *pInvestorFee, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("��ѯͶ������������ʧ�� ����ԭ��%s\n",pRspInfo->ErrorMsg);
		return;
	}
	
	if (pInvestorFee==NULL)
	{
		printf("û�в�ѯ��Ͷ������������\n");
		return ;
	}
	printf("-----------------------------\n");
	printf("���͹�˾���=[%s]\n",pInvestorFee->BrokerID);
	printf("��Լ����=[%s]\n",pInvestorFee->InstrumentID);
	printf("�ͻ�����=[%s]\n",pInvestorFee->ClientID);
	printf("���������Ѱ�����=[%f]\n",pInvestorFee->OpenFeeRate);
	printf("���������Ѱ�����=[%f]\n",pInvestorFee->OpenFeeAmt);
	printf("ƽ�������Ѱ�����=[%f]\n",pInvestorFee->OffsetFeeRate);
	printf("ƽ�������Ѱ�����=[%f]\n",pInvestorFee->OffsetFeeAmt);
	printf("ƽ��������Ѱ�����=[%f]\n",pInvestorFee->OTFeeRate);
	printf("ƽ��������Ѱ�����=[%f]\n",pInvestorFee->OTFeeAmt);
	printf("-----------------------------\n");
	return ;

}

	///Ͷ���߱�֤���ʲ�ѯӦ��
void CTraderSpi::OnRspQryInvestorMargin(CUstpFtdcInvestorMarginField *pInvestorMargin, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("��ѯͶ���߱�֤����ʧ�� ����ԭ��%s\n",pRspInfo->ErrorMsg);
		return;
	}
	
	if (pInvestorMargin==NULL)
	{
		printf("û�в�ѯ��Ͷ���߱�֤����\n");
		return ;
	}
	printf("-----------------------------\n");
	printf("���͹�˾���=[%s]\n",pInvestorMargin->BrokerID);
	printf("��Լ����=[%s]\n",pInvestorMargin->InstrumentID);
	printf("�ͻ�����=[%s]\n",pInvestorMargin->ClientID);
	printf("��ͷռ�ñ�֤�𰴱���=[%f]\n",pInvestorMargin->LongMarginRate);
	printf("��ͷ��֤������=[%f]\n",pInvestorMargin->LongMarginAmt);
	printf("��ͷռ�ñ�֤�𰴱���=[%f]\n",pInvestorMargin->ShortMarginRate);
	printf("��ͷ��֤������=[%f]\n",pInvestorMargin->ShortMarginAmt);
	printf("-----------------------------\n");
	return ;

}


void CTraderSpi::OnRspQryComplianceParam(CUstpFtdcRspComplianceParamField *pRspComplianceParam, CUstpFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if (pRspInfo!=NULL&&pRspInfo->ErrorID!=0)
	{
		printf("��ѯ�Ϲ����ʧ�� ����ԭ��%s\n",pRspInfo->ErrorMsg);
		return;
	}
	
	if (pRspComplianceParam==NULL)
	{
		printf("û�в�ѯ���Ϲ����\n");
		return ;
	}
	printf("-----------------------------\n");
	printf("���͹�˾���=[%s]\n",pRspComplianceParam->BrokerID);
	printf("�ͻ�����=[%s]\n",pRspComplianceParam->ClientID);
	printf("ÿ����󱨵���=[%d]\n",pRspComplianceParam->DailyMaxOrder);
	printf("ÿ����󳷵���=[%d]\n",pRspComplianceParam->DailyMaxOrderAction);
	printf("ÿ��������=[%d]\n",pRspComplianceParam->DailyMaxErrorOrder);
	printf("ÿ����󱨵���=[%d]\n",pRspComplianceParam->DailyMaxOrderVolume);
	printf("ÿ����󳷵���=[%d]\n",pRspComplianceParam->DailyMaxOrderActionVolume);
	printf("-----------------------------\n");
	return ;


}


int icount=0;
void CTraderSpi::OnRtnInstrumentStatus(CUstpFtdcInstrumentStatusField *pInstrumentStatus)
{
	if (pInstrumentStatus==NULL)
	{
		printf("û�к�Լ״̬��Ϣ\n");
		return ;
	}
	icount++;
	
	printf("-----------------------------\n");
	printf("����������=[%s]\n",pInstrumentStatus->ExchangeID);
	printf("Ʒ�ִ���=[%s]\n",pInstrumentStatus->ProductID);
	printf("Ʒ������=[%s]\n",pInstrumentStatus->ProductName);
	printf("��Լ����=[%s]\n",pInstrumentStatus->InstrumentID);
	printf("��Լ����=[%s]\n",pInstrumentStatus->InstrumentName);
	printf("�������=[%d]\n",pInstrumentStatus->DeliveryYear);
	printf("������=[%d]\n",pInstrumentStatus->DeliveryMonth);
	printf("�޼۵�����µ���=[%d]\n",pInstrumentStatus->MaxLimitOrderVolume);
	printf("�޼۵���С�µ���=[%d]\n",pInstrumentStatus->MinLimitOrderVolume);
	printf("�м۵�����µ���=[%d]\n",pInstrumentStatus->MaxMarketOrderVolume);
	printf("�м۵���С�µ���=[%d]\n",pInstrumentStatus->MinMarketOrderVolume);
	
	printf("��������=[%d]\n",pInstrumentStatus->VolumeMultiple);
	printf("���۵�λ=[%lf]\n",pInstrumentStatus->PriceTick);
	printf("����=[%c]\n",pInstrumentStatus->Currency);
	printf("��ͷ�޲�=[%d]\n",pInstrumentStatus->LongPosLimit);
	printf("��ͷ�޲�=[%d]\n",pInstrumentStatus->ShortPosLimit);
	printf("��ͣ���=[%lf]\n",pInstrumentStatus->LowerLimitPrice);
	printf("��ͣ���=[%lf]\n",pInstrumentStatus->UpperLimitPrice);
	printf("�����=[%lf]\n",pInstrumentStatus->PreSettlementPrice);
	printf("��Լ����״̬=[%c]\n",pInstrumentStatus->InstrumentStatus);
	
	printf("������=[%s]\n",pInstrumentStatus->CreateDate);
	printf("������=[%s]\n",pInstrumentStatus->OpenDate);
	printf("������=[%s]\n",pInstrumentStatus->ExpireDate);
	printf("��ʼ������=[%s]\n",pInstrumentStatus->StartDelivDate);
	printf("��󽻸���=[%s]\n",pInstrumentStatus->EndDelivDate);
	printf("���ƻ�׼��=[%lf]\n",pInstrumentStatus->BasisPrice);
	printf("��ǰ�Ƿ���=[%d]\n",pInstrumentStatus->IsTrading);
	printf("������Ʒ����=[%s]\n",pInstrumentStatus->UnderlyingInstrID);
	printf("�ֲ�����=[%c]\n",pInstrumentStatus->PositionType);
	printf("ִ�м�=[%lf]\n",pInstrumentStatus->StrikePrice);
	printf("��Ȩ����=[%c]\n",pInstrumentStatus->OptionsType);

	printf("-----------------------------\n");
	printf("[%d]",icount);
	return ;

}


void CTraderSpi::OnRtnInvestorAccountDeposit(CUstpFtdcInvestorAccountDepositResField *pInvestorAccountDepositRes)
{
	if (pInvestorAccountDepositRes==NULL)
	{
		printf("û���ʽ�������Ϣ\n");
		return ;
	}

	printf("-----------------------------\n");
	printf("���͹�˾���=[%s]\n",pInvestorAccountDepositRes->BrokerID);
	printf("�û����룽[%s]\n",pInvestorAccountDepositRes->UserID);
	printf("Ͷ���߱��=[%s]\n",pInvestorAccountDepositRes->InvestorID);
	printf("�ʽ��˺�=[%s]\n",pInvestorAccountDepositRes->AccountID);
	printf("�ʽ���ˮ�ţ�[%s]\n",pInvestorAccountDepositRes->AccountSeqNo);
	printf("��[%s]\n",pInvestorAccountDepositRes->Amount);
	printf("�������[%s]\n",pInvestorAccountDepositRes->AmountDirection);
	printf("�����ʽ�[%s]\n",pInvestorAccountDepositRes->Available);
	printf("����׼����[%s]\n",pInvestorAccountDepositRes->Balance);
	printf("-----------------------------\n");
	return ;

}
//��ȡͶ���߱�����Ϣ
string getInvestorOrderInsertInfo(CUstpFtdcInputOrderField *order)
{
    ///���͹�˾����
    char	*BrokerID = order->BrokerID;
    ///Ͷ���ߴ���
    char	*InvestorID = order->InvestorID;
    ///��Լ����
    char	*InstrumentID = order->InstrumentID;
    ///��������
    char	*OrderRef = order->UserOrderLocalID;
    ///�û�����
    char	*UserID = order->UserID;
    ///�����۸�����
    char	OrderPriceType = order->OrderPriceType;
    ///��������
    char	Direction[] = {order->Direction,'\0'};
    ///��Ͽ�ƽ��־
    char	*CombOffsetFlag =&order->OffsetFlag;
    ///���Ͷ���ױ���־
    char	*CombHedgeFlag = &order->HedgeFlag;
    ///�۸�
    TUstpFtdcPriceType	limitPrice = order->LimitPrice;
    char LimitPrice[100];
    sprintf(LimitPrice,"%f",limitPrice);
    ///����
    TUstpFtdcVolumeType	volumeTotalOriginal = order->Volume;
    char VolumeTotalOriginal[100];
    sprintf(VolumeTotalOriginal,"%d",volumeTotalOriginal);
    ///��Ч������
    TUstpFtdcTimeConditionType	TimeCondition = order->TimeCondition;
    ///GTD����
    //TUstpFtdcDateType	GTDDate = order->GTDDate;
    ///�ɽ�������
    TUstpFtdcVolumeConditionType	VolumeCondition[] = {order->VolumeCondition,'\0'};
    ///��С�ɽ���
    TUstpFtdcVolumeType	MinVolume = order->MinVolume;
    ///��������
    //TUstpFtdcContingentConditionType	ContingentCondition = order->ContingentCondition;
    ///ֹ���
    TUstpFtdcPriceType	StopPrice = order->StopPrice;
    ///ǿƽԭ��
    TUstpFtdcForceCloseReasonType	ForceCloseReason = order->ForceCloseReason;
    ///�Զ������־
    TUstpFtdcBoolType	IsAutoSuspend = order->IsAutoSuspend;
    ///ҵ��Ԫ
    //TUstpFtdcBusinessUnitType	BusinessUnit = order->BusinessUnit;
    ///������
    TUstpFtdcRequestIDType	requestID = order->RequestID;
    char RequestID[100];
    sprintf(RequestID,"%d",requestID);
    ///�û�ǿ����־
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
