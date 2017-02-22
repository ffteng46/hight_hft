TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    TraderSpi.cpp \
    globalutil.cpp \
    boost_tools.cpp \
    Trade.cpp \
    main.cpp \
    xelespreadmddata.cpp
INCLUDEPATH += ../../boost_1_61_0
LIBS += -L../../boost_1_61_0/stage/lib
LIBS += -L../../glog_0_3_3
LIBS += -lboost_system -lboost_thread  -lpthread -lboost_chrono -lglog -lboost_locale
LIBS += -Lconfig -lUSTPtraderapi -lXeleMdAPI64
LIBS +=  -lpthread -lrt
HEADERS += \
    TraderSpi.h \
    globalutil.h \
    boost_tools.h \
    Trade.h \
    XeleFtdcMduserApi.h \
    XeleMdFtdcUserApiDataType.h \
    XeleMdFtdcUserApiStruct.h \
    DemoUtils.h \
    USTPFtdcTraderApi.h \
    USTPFtdcUserApiDataType.h \
    USTPFtdcUserApiStruct.h \
    PublicFuncs.h


