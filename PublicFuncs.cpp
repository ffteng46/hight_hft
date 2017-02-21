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

/*---------------全局变量区---------------*/

const char* TEST_API_INI_NAME="./config/TestApi.ini";
char* g_pFlowPath="./flow/";
char* g_pProductInfo="ceshi";
char* g_pProgramName="TestApi";
int g_choose;

extern int g_nSwitch;
extern FILE * g_fpSend;

//1.登录信息
///经纪公司代码
TUstpFtdcBrokerIDType g_BrokerID;
///交易用户代码
TUstpFtdcUserIDType	g_UserID;
///密码
TUstpFtdcPasswordType	g_Password;

//2.地址信息
char g_frontaddr[BUFLEN];

//3.报单信息
extern int g_nOrdLocalID;
CUstpFtdcTraderApi * g_puserapi=NULL;


void StartQueryFee()
{
    printf("在查询手续费率\n");
    CUstpFtdcQryInvestorFeeField InvestorFee;
    memset(&InvestorFee,0,sizeof(CUstpFtdcQryInvestorFeeField));
    strcpy(InvestorFee.BrokerID,g_BrokerID);
    strcpy(InvestorFee.UserID,g_UserID);
    g_puserapi->ReqQryInvestorFee(&InvestorFee,g_nOrdLocalID++);
}

void StartQueryMargin()
{
    printf("在查询保证金率\n");
    CUstpFtdcQryInvestorMarginField InvestorMargin;
    memset(&InvestorMargin,0,sizeof(CUstpFtdcQryInvestorMarginField));
    strcpy(InvestorMargin.BrokerID,g_BrokerID);
    strcpy(InvestorMargin.UserID,g_UserID);
    g_puserapi->ReqQryInvestorMargin(&InvestorMargin,g_nOrdLocalID++);
}

void StartQueryComplianceParam()
{
    printf("在查询合规参数\n");
    CUstpFtdcQryComplianceParamField ComplianceParam;
    memset(&ComplianceParam,0,sizeof(CUstpFtdcQryComplianceParamField));
    strcpy(ComplianceParam.BrokerID,g_BrokerID);
    strcpy(ComplianceParam.UserID,g_UserID);
    printf("请投资者编号：");
    scanf("%s",(ComplianceParam.InvestorID));
    g_puserapi->ReqQryComplianceParam(&ComplianceParam,g_nOrdLocalID++);
}

