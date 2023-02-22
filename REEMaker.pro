QT       += core gui
# QT += openglwidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    BlocEditeur.cpp \
    DarkStyle.cpp \
    PdgHelper.cpp \
    bloc.cpp \
    blocQuestion.cpp \
    customHeader.cpp \
    customPushButton.cpp \
    framelesswindow/framelesswindow.cpp \
    framelesswindow/windowdragger.cpp \
    main.cpp \
    mainwindow.cpp \
    pressepapier.cpp

HEADERS += \
    BlocEditeur.h \
    DarkStyle.h \
    PdgHelper.h \
    bloc.h \
    blocQuestion.h \
    customHeader.h \
    customPushButton.h \
    framelesswindow/framelesswindow.h \
    framelesswindow/windowdragger.h \
    mainwindow.h \
    pressepapier.h

FORMS += \
    framelesswindow/framelesswindow.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Les caractères spéciaux sont en notation octal \ooo   par exemple é en ascii est 233 et en octal c'est donc 351 d'ou \351.
VERSION = 5.2.0.0
QMAKE_TARGET_COMPANY = "WENTZEL Gr\351gory"
QMAKE_TARGET_PRODUCT = "REEMaker"
QMAKE_TARGET_DESCRIPTION = "Assistant de foliotage de proc\351dure"
QMAKE_TARGET_COPYRIGHT = "Copyright \251 2023"

RC_ICONS = ReeMaker.ico


unix|win32: LIBS += -L$$PWD/lib/ -llibpodofo.dll
LIBS += -lVersion
INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

INCLUDEPATH +="framelesswindow"

RESOURCES += \
    Resource.qrc \
    darkstyle.qrc \
    framelesswindow.qrc
