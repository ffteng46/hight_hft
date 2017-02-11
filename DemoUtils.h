/*!
 * @file DemoUtils.h
 *
 * 
 * @author shuaiw
 */

#ifndef DEMOUTILS_H_
#define DEMOUTILS_H_
#include <stddef.h>
#include <iostream>
#include <iomanip>
#include <XeleMdFtdcUserApiStruct.h>
/*
 * 求结构体成员字节大小
 */
#define MEMB_SIZEOF(st_type, member) sizeof(((st_type*)0)->member)
/*
 * 求结构体成员字节偏移
 */
#define MEMB_CHARPTR(ptr, st_type, member) ((char*)ptr + offsetof(st_type, member))
/*
 * 辅助宏: 打印marketData域字段
 */
#define M_INPUT(ptr, CField, member, src) memcpy(MEMB_CHARPTR(ptr, CField, member), src, MEMB_SIZEOF(CField, member))
#define S_INPUT(ptr, CField, member, src) strncpy(MEMB_CHARPTR(ptr, CField, member), src, MEMB_SIZEOF(CField, member))
#define STR(x) #x

#define S_OUTPUT(ptr, CField, member) do{ \
    cout << setw(20)<<STR(CField)<<"->"<<STR(member)<<right<<setw(20)<<" :"<< (ptr)->member << endl; \
}while(0)

#define FORMAT_OUT(out, item) do{ \
            out<<setw(20)<<STR(item)<<":"<<right<<setw(20)<< pDepthMarketData->item ;\
            out<< endl;\
}while(0)

#define FORMAT_OUT2(out, ptr, item) do{ \
            out<<setw(20)<<STR(item)<<":"<<right<<setw(20)<< ptr->item ;\
            out<< endl;\
}while(0)



inline std::ostream& _hexdump_oneline2(std::ostream &os,
                                       const void* vbuf,
                                       int sz)
{
    char * const str = new char[sz * 4 + 1];
    char* wp = str;

    int cur(0);
    unsigned char* buf = (unsigned char*) vbuf;

    while (cur < sz)
    {
        snprintf(wp, 3, "%02hhx", buf[cur++]);
        wp += 2;
    }
    *wp++ = '\0';
    os.write(str, wp - str);
    os << std::endl;
    delete[] str;
    return os;
}

void printXeleShfeHighLevelOneMarketData(ostream &log,
                                         const char *title,
                                         CXeleShfeHighLevelOneMarketData *pDataType1) {
    log << "< " << title << " >" << endl;

    FORMAT_OUT2(log, pDataType1, Instrument);
    FORMAT_OUT2(log, pDataType1, UpdateTime);
    FORMAT_OUT2(log, pDataType1, UpdateMillisec);
    FORMAT_OUT2(log, pDataType1, Volume);
    FORMAT_OUT2(log, pDataType1, LastPrice);
    FORMAT_OUT2(log, pDataType1, Turnover);
    FORMAT_OUT2(log, pDataType1, OpenInterest);
    FORMAT_OUT2(log, pDataType1, BidPrice);
    FORMAT_OUT2(log, pDataType1, AskPrice);
    FORMAT_OUT2(log, pDataType1, BidVolume);
    FORMAT_OUT2(log, pDataType1, AskVolume);


}



void printXeleShfeDepthMarketData(ostream &log,
                                  const char *title,
                                  CXeleShfeDepthMarketData *pDataType2) {
    log << "< " << title << " >" << endl;

    FORMAT_OUT2(log, pDataType2, Instrument);
    FORMAT_OUT2(log, pDataType2, Direction);
    FORMAT_OUT2(log, pDataType2, UpdateTime);
    FORMAT_OUT2(log, pDataType2, UpdateMillisec);
    FORMAT_OUT2(log, pDataType2, Volume1);
    FORMAT_OUT2(log, pDataType2, Price1);
    FORMAT_OUT2(log, pDataType2, Volume2);
    FORMAT_OUT2(log, pDataType2, Price2);
    FORMAT_OUT2(log, pDataType2, Volume3);
    FORMAT_OUT2(log, pDataType2, Price3);
    FORMAT_OUT2(log, pDataType2, Volume4);
    FORMAT_OUT2(log, pDataType2, Price4);
    FORMAT_OUT2(log, pDataType2, Volume5);
    FORMAT_OUT2(log, pDataType2, Price5);

}

void printXeleShfeLowLevelOneMarketData(ostream &log,
                                        const char *title,
                                        CXeleShfeLowLevelOneMarketData *pData)
{
    log << "< " << title << " >" << endl;

    FORMAT_OUT2(log, pData, Instrument);
    FORMAT_OUT2(log, pData, UpdateTime);
    FORMAT_OUT2(log, pData, OpenPrice);
    FORMAT_OUT2(log, pData, HighestPrice);
    FORMAT_OUT2(log, pData, LowestPrice);
    FORMAT_OUT2(log, pData, ClosePrice);
    FORMAT_OUT2(log, pData, UpperLimitPrice);
    FORMAT_OUT2(log, pData, LowerLimitPrice);
    FORMAT_OUT2(log, pData, SettlementPrice);
    FORMAT_OUT2(log, pData, CurrDelta);
}

void printDepthMarketData(ostream& log,
                          const char* title,
                          CXeleMdFtdcDepthMarketDataField *pDepthMarketData)
{
    log << "< " << title << " >" << endl;

    FORMAT_OUT(log, LastPrice);

    FORMAT_OUT(log, OpenPrice);
    FORMAT_OUT(log, HighestPrice);
    FORMAT_OUT(log, LowestPrice);
    FORMAT_OUT(log, Volume);
    FORMAT_OUT(log, Turnover);
    FORMAT_OUT(log, OpenInterest);
    FORMAT_OUT(log, ClosePrice);
    FORMAT_OUT(log, SettlementPrice);
    FORMAT_OUT(log, UpperLimitPrice);
    FORMAT_OUT(log, LowerLimitPrice);
    FORMAT_OUT(log, CurrDelta);
    FORMAT_OUT(log, BidPrice1);
    FORMAT_OUT(log, BidVolume1);
    FORMAT_OUT(log, AskPrice1);
    FORMAT_OUT(log, AskVolume1);
#if 0
    FORMAT_OUT(log, BidPrice2);
    FORMAT_OUT(log, BidVolume2);
    FORMAT_OUT(log, AskPrice2);
    FORMAT_OUT(log, AskVolume2);
    FORMAT_OUT(log, BidPrice3);
    FORMAT_OUT(log, BidVolume3);
    FORMAT_OUT(log, AskPrice3);
    FORMAT_OUT(log, AskVolume3);
    FORMAT_OUT(log, BidPrice4);
    FORMAT_OUT(log, BidVolume4);
    FORMAT_OUT(log, AskPrice4);
    FORMAT_OUT(log, AskVolume4);
    FORMAT_OUT(log, BidPrice5);
    FORMAT_OUT(log, BidVolume5);
    FORMAT_OUT(log, AskPrice5);
    FORMAT_OUT(log, AskVolume5);
#endif
    FORMAT_OUT(log, InstrumentID);
    FORMAT_OUT(log, UpdateTime);
    FORMAT_OUT(log, UpdateMillisec);
}

#endif /* DEMOUTILS_H_ */
