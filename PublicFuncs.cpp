// PublicFuncs.cpp: implementation of the PublicFuncs class.
//
//////////////////////////////////////////////////////////////////////

#include "PublicFuncs.h"
#include <unistd.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PublicFuncs::PublicFuncs()
{

}

PublicFuncs::~PublicFuncs()
{

}

typedef struct
{
    int nordspeed;
    int nordloop;
} stordspeed;

/*---------------ȫ�ֱ�����---------------*/

const char* TEST_API_INI_NAME="./config/TestApi.ini";
char* g_pFlowPath="./flow/";
char* g_pProductInfo="ceshi";
char* g_pProgramName="TestApi";
int g_choose;

extern int g_nSwitch;
extern FILE * g_fpSend;

//1.��¼��Ϣ
///���͹�˾����
TUstpFtdcBrokerIDType g_BrokerID;
///�����û�����
TUstpFtdcUserIDType	g_UserID;
///����
TUstpFtdcPasswordType	g_Password;

//2.��ַ��Ϣ
char g_frontaddr[BUFLEN];

//3.������Ϣ
extern int g_nOrdLocalID;
CUstpFtdcTraderApi * g_puserapi=NULL;


void StartQueryFee()
{
    printf("�ڲ�ѯ��������\n");
    CUstpFtdcQryInvestorFeeField InvestorFee;
    memset(&InvestorFee,0,sizeof(CUstpFtdcQryInvestorFeeField));
    strcpy(InvestorFee.BrokerID,g_BrokerID);
    strcpy(InvestorFee.UserID,g_UserID);
    g_puserapi->ReqQryInvestorFee(&InvestorFee,g_nOrdLocalID++);
}

void StartQueryMargin()
{
    printf("�ڲ�ѯ��֤����\n");
    CUstpFtdcQryInvestorMarginField InvestorMargin;
    memset(&InvestorMargin,0,sizeof(CUstpFtdcQryInvestorMarginField));
    strcpy(InvestorMargin.BrokerID,g_BrokerID);
    strcpy(InvestorMargin.UserID,g_UserID);
    g_puserapi->ReqQryInvestorMargin(&InvestorMargin,g_nOrdLocalID++);
}

void StartQueryComplianceParam()
{
    printf("�ڲ�ѯ�Ϲ����\n");
    CUstpFtdcQryComplianceParamField ComplianceParam;
    memset(&ComplianceParam,0,sizeof(CUstpFtdcQryComplianceParamField));
    strcpy(ComplianceParam.BrokerID,g_BrokerID);
    strcpy(ComplianceParam.UserID,g_UserID);
    printf("��Ͷ���߱�ţ�");
    scanf("%s",(ComplianceParam.InvestorID));
    g_puserapi->ReqQryComplianceParam(&ComplianceParam,g_nOrdLocalID++);
}

