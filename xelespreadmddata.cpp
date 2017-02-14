/**
 *	@file reference.cpp
 *  @author shuaiw
 */
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "TraderSpi.h"
#include <sstream>
#include <list>
#include <string.h>
#include <stdlib.h>
#include "globalutil.h"
#include <unordered_map>
#include <boost/chrono.hpp>

using namespace std;

#include <XeleFtdcMduserApi.h>
#include "DemoUtils.h"
#include <pthread.h>
#include <signal.h>

/*
 * 辅助函数: 读取配置文件refer.ini
 */

static char USERID[MEMB_SIZEOF(CXeleMdFtdcReqUserLoginField, UserID)];
static char PASSWD[MEMB_SIZEOF(CXeleMdFtdcReqUserLoginField, Password)];
static char FRONTADDRESS[40];
static char MCASTADDRESS[40];
static char NIC[9];
extern boost::thread_group thread_log_group;
extern CTraderSpi* pUserSpi;
extern unordered_map<string,unordered_map<string,int>> positionmap;
//价格变动单位
extern double tick;
//跌停价格
extern double min_price;
//涨停价格
extern double max_price;
//报单触发信号
extern int cul_times;
extern int default_volume;
// USER_API参数
CXeleMdApi* mduserapi;
static void loadConfigFile(char *iniName) {
    if (iniName == NULL || iniName[0] == 0) {
    }
    memset(USERID, 0, sizeof(USERID));
    memset(PASSWD, 0, sizeof(PASSWD));
    memset(FRONTADDRESS, 0, sizeof(FRONTADDRESS));
    /*
     * load USERID
     */
    FILE *ini = popen("sed 's/USERID=\\(.*\\)/\\1/g;tx;d;:x' config/refer.ini", "r");
    fscanf(ini, "%s\n", USERID);
    pclose(ini);
    /*
     * load PASSWD
     */
    ini = popen("sed 's/PASSWD=\\(.*\\)/\\1/g;tx;d;:x' config/refer.ini", "r");
    fscanf(ini, "%s\n", PASSWD);
    pclose(ini);
    /*
     * load FRONTADDRESS
     */
    ini = popen("sed 's/FRONTADDRESS=\\(.*\\)/\\1/g;tx;d;:x' config/refer.ini", "r");
    fscanf(ini, "%s\n", FRONTADDRESS);
    pclose(ini);
    /*
     * load MCASTADDRESS
     */
    ini = popen("sed 's/MCASTADDRESS=\\(.*\\)/\\1/g;tx;d;:x' config/refer.ini", "r");
    fscanf(ini, "%s\n", MCASTADDRESS);
    pclose(ini);
    /*
     * load NIC
     */
    ini = popen("sed 's/NIC=\\(.*\\)/\\1/g;tx;d;:x' config/refer.ini", "r");
    fscanf(ini, "%s\n", NIC);
    pclose(ini);


    fprintf(stderr, "USERID:%s\n", USERID);
    fprintf(stderr, "PASSWD:%s\n", PASSWD);
    fprintf(stderr, "FRONTADDRESS:%s\n", FRONTADDRESS);
    fprintf(stderr, "MCASTADDRESS:%s\n", MCASTADDRESS);
    fprintf(stderr, "NIC:%s\n", NIC);

}


/*
 *
 * 辅助函数: 填写login域
 */
static void fill_userlogin(CXeleMdFtdcReqUserLoginField *req) {
    S_INPUT(req, CXeleMdFtdcReqUserLoginField, UserID, USERID);
    S_INPUT(req, CXeleMdFtdcReqUserLoginField, Password, PASSWD);
    S_INPUT(req, CXeleMdFtdcReqUserLoginField, ProtocolInfo, "protocol");
}

/*
 * 示例Spi
 */
struct CXeleFtdcMdApi : public CXeleMdSpi {
public:

    virtual void OnFrontDisconnected(int nReason) {
        (cout) << "<" << __FUNCTION__ << ">" << " Errcode:" << nReason << endl;
    }

};

/*
 * 主函数:
 */

int g_md_switch = 1;

void *job_recv_market_data() {
    cerr << "--->>> " << "job_recv_market_data"  << endl;
    int handle = mduserapi->GetHandle();
    CXeleShfeMarketDataUnion mdtick;
    ofstream log("RecvMarketDataTick.log");
    while (g_md_switch) {
        if (RecvShfeMarketDataTick(handle, &mdtick)) {
            if (mdtick.md_type[0] == 'M') {
                //printXeleShfeHighLevelOneMarketData(log, "ShfeHighLevelOneMarketData", &mdtick.type_high);
                OnRtnSHFEMarketData(&mdtick.type_high);
            }
            else if (mdtick.md_type[0] == 'S') {
                printXeleShfeLowLevelOneMarketData(log, "ShfeLowLevelOneMarketData", &mdtick.type_low);
                //OnRtnSHFEMarketData(&mdtick.type_low);
            }
            else if (mdtick.md_type[0] == 'Q') {
                printXeleShfeDepthMarketData(log, "ShfeDepthMarketData", &mdtick.type_depth);
            }
        }
    }
}

int initMarketDataApi() {
    cerr << "--->>> " << "initMarketDataApi"  << endl;
    if(tick == 0 || min_price == 0||max_price == 0||cul_times == 0 ||default_volume == 0){
//        char char_tick[10]={'\0'};
//        sprintf(char_tick,"%f",tick);
//        char char_min_price[10]={'\0'};
//        sprintf(char_min_price,"%f",min_price);
//        char char_max_price[10]={'\0'};
//        sprintf(char_max_price,"%f",max_price);
//        char char_cul_times[10]={'\0'};
//        sprintf(char_cul_times,"%f",cul_times);
//        char char_default_volume[10]={'\0'};
//        sprintf(char_default_volume,"%f",default_volume);
        char msg[1024]={'\0'};
        sprintf(msg,"error:tick=%f,min_price=%f,max_price=%f,cul_time=%f,default_volume=%f must not be 0!!!!!!!!!!!",
               tick,min_price,max_price,cul_times,default_volume );
        //cout<<"error:tick,min_price,max_price,cul_time,default_volume must not be 0!!!!!!!!!!!";
        cout<<msg<<endl;
        exit(0);
    }
    /*
     * 读取refer.ini
     */
    loadConfigFile(NULL);

    std::string msg;

    /*
     * 创建对象
     */
    CXeleFtdcMdApi spi;
    mduserapi = CXeleMdApi::CreateMdApi(&spi);

    /*
     * 准备login的结构体
     */
    CXeleMdFtdcReqUserLoginField login_info;
    fill_userlogin(&login_info);

    /*
     * 开始登录
     */
    fprintf(stdout, "%s\n", mduserapi->GetVersion());

    int status = mduserapi->LoginInit(FRONTADDRESS, MCASTADDRESS, NIC, &login_info);
    if (status == XELEAPI_SUCCESS) {
        cout << "XELEAPI_SUCCESS" << endl;
        initPriceGap();
    }else {
        mduserapi->Release();
        cout << "LoginInit fail. Exit." << endl;
        return 1;
    }
    /*
     * 创建线程, 获取数据
     */

    //pthread_t md_thread;
    g_md_switch = 1;
    thread_log_group.create_thread(job_recv_market_data);
    //pthread_create(&md_thread, NULL, job_recv_market_data, mduserapi);
//    do {
//        cout << "Input 'q' to disconnect API:";
//        getline(cin, msg);
//    } while (msg != "q");
//    g_md_switch = 0;
//    pthread_join(md_thread, NULL);
//    mduserapi->Release();
//    cerr << "API release done. Exit Demo." << endl;
}

