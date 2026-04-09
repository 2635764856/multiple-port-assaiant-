QT       += core gui network serialport serialbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    modbusmasterdialog.cpp \
    portsettingform.cpp \
    receviersettingform.cpp \
    sendersettingform.cpp \
    setlastbitedialog.cpp \
    statusbarform.cpp

HEADERS += \
    mainwindow.h \
    modbusmasterdialog.h \
    portsettingform.h \
    receviersettingform.h \
    sendersettingform.h \
    setlastbitedialog.h \
    statusbarform.h

FORMS += \
    mainwindow.ui \
    modbusmasterdialog.ui \
    portsettingform.ui \
    receviersettingform.ui \
    sendersettingform.ui \
    setlastbitedialog.ui \
    statusbarform.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    langRes.qrc \
    pic.qrc
TRANSLATIONS = Language/linguist_cn.ts Language/linguist_en.ts
