QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Configuration de base
TARGET = REEMaker
VERSION = 6.0.0.0

# Informations de l'application
QMAKE_TARGET_COMPANY = "WENTZEL Gr\351gory"
QMAKE_TARGET_PRODUCT = "REEMaker"
QMAKE_TARGET_DESCRIPTION = "Assistant de foliotage de proc\351dure"
QMAKE_TARGET_COPYRIGHT = "Copyright \251 2023"
RC_ICONS = ReeMaker.ico

# Sources
SOURCES += \
    BlocEditeur.cpp \
    PdgHelper.cpp \
    archivers.cpp \
    bloc.cpp \
    blocQuestion.cpp \
    customHeader.cpp \
    customPushButton.cpp \
    customlistitemwidget.cpp \
    imageconverter.cpp \
    imageutils.cpp \
    main.cpp \
    mainwindow.cpp \
    overlaywidget.cpp \
    pressepapier.cpp \
    textutils.cpp

# Headers
HEADERS += \
    BlocEditeur.h \
    PdgHelper.h \
    QComboBox_custom.h \
    archivers.h \
    bloc.h \
    blocQuestion.h \
    customHeader.h \
    customPushButton.h \
    custom_qlistwidget.h \
    customlistitemwidget.h \
    imageconverter.h \
    imageutils.h \
    mainwindow.h \
    overlaywidget.h \
    pressepapier.h \
    textutils.h

# Forms
FORMS += \
    mainwindow.ui

# Resources
RESOURCES += \
    Resource.qrc

# Chemins d'inclusion
INCLUDEPATH += $$PWD/include
INCLUDEPATH += "framelesswindow"
DEPENDPATH += $$PWD/include

# ===========================================
# BIBLIOTHÈQUES - ORDRE CRITIQUE RESPECTÉ
# ===========================================

# Chemin des bibliothèques GCC
LIBS += -L$$PWD/lib/

# BIBLIOTHÈQUES PRINCIPALES PODOFO (après toutes les sous-librairies à linker)
LIBS += -lpodofo -lpodofo_private -lpodofo_3rdparty #3rd party pour cfwBegSet, cfwBegFont, cfwEndFont venant du CFF writer d’Adobe AFDKO

# # 1. BIBLIOTHÈQUES DE TEXTE ET POLICES
LIBS += -lfreetype

# # 2. INTERNATIONALISATION
LIBS += -lutf8proc


# # 3. XML ET PARSING
LIBS += -lxml2

# # 4. IMAGES (ordre de dépendance respecté)
LIBS += -ltiff
LIBS += -ljpeg
LIBS += -lpng

# # 3. COMPRESSION (ordre de dépendance respecté)
LIBS += -lbrotlienc -lbrotlidec -lbrotlicommon
LIBS += -llzma -lbz2
LIBS += -lzstd -lxxhash
LIBS += -lz #(inflate/deflate...)



# # 2. CRYPTOGRAPHIE
LIBS += -lssl -lcrypto

# 1. BIBLIOTHÈQUES SYSTÈME SPÉCIFIQUES À LA PLATEFORME
win32 {
    # Bibliothèques Windows essentielles
    LIBS += -lVersion -luuid -lws2_32 -lgdi32 -lShcore

    # # Cryptographie Windows
    LIBS += -lbcrypt #BCryptGenRandom
    LIBS += -lcrypt32 #CertEnumCertificatesInStore
}

# Librairie incluse de bas en haut

# Déploiement
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


win32 {
    # Ceci est la méthode moderne pour la prise en charge DPI par moniteur
    DEFINES += QT_DPI_AWARENESS=1
    DEFINES += QT_DPI_AWARENESS_PER_MONITOR_V2=1
}
# Options de debug pour le linker (décommentez si nécessaire)
# QMAKE_LFLAGS += -v
# win32: QMAKE_LFLAGS += -Wl,--verbose

# DISTFILES += \
#     include/zstd/decompress/huf_decompress_amd64.S
