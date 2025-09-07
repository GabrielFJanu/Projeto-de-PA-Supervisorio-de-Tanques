QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../MySocket/mysocket.cpp \
    supcliente.cpp \
    supcliente_main_qt.cpp \
    supcliente_qt.cpp \
    supimg.cpp \
    suplogin.cpp

HEADERS += \
    ../MySocket/mysocket.h \
    supcliente.h \
    supcliente_qt.h \
    supdados.h \
    supimg.h \
    suplogin.h \
    tanques-param.h

FORMS += \
    supcliente_qt.ui \
    suplogin.ui

LIBS   += \
    -lWs2_32

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
