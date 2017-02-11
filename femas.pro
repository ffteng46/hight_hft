TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    TraderSpi.cpp \
    testapi.cpp \
    PublicFuncs.cpp \
    globalutil.cpp \
    boost_tools.cpp \
    Trade.cpp \
    main.cpp \
    userdemo_shfe.cpp \
    MdSpi.cpp
INCLUDEPATH += ../../boost_1_61_0
LIBS += -L../../boost_1_61_0/stage/lib
LIBS += -L../../glog_0_3_3
LIBS += -lboost_system -lboost_thread  -lpthread -lboost_chrono -lglog -lboost_locale
LIBS += -L/home/tff/software/develop/femas -lUSTPtraderapi
LIBS +=  -lpthread
HEADERS += \
    TraderSpi.h \
    PublicFuncs.h \
    USTPFtdcUserApiStruct.h \
    USTPFtdcUserApiDataType.h \
    USTPFtdcTraderApi.h \
    globalutil.h \
    boost_tools.h \
    Trade.h \
    XeleFtdcMduserApi.h \
    XeleMdFtdcUserApiDataType.h \
    XeleMdFtdcUserApiStruct.h \
    DemoUtils.h \
    MdSpi.h

