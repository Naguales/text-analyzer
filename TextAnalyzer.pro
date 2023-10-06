QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets charts

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TARGET = TextAnalyzer
TEMPLATE = app

SOURCES += \
    Workers/FileReader.cpp \
    Workers/TextAnalyzer.cpp \
    Widgets/GlowedButton.cpp \
    Table/ColorItemDelegate.cpp \
    Table/LetterCombinationsModel.cpp \
    Table/LetterCombinationsTableView.cpp \
    TextAnalyzerWindow.cpp \
    main.cpp

HEADERS += \
    CommonData.h \
    Workers/FileReader.h \
    Workers/TextAnalyzer.h \
    Widgets/GlowedButton.h \
    Table/ColorItemDelegate.h \
    Table/LetterCombinationsModel.h \
    Table/LetterCombinationsTableView.h \
    TextAnalyzerWindow.h

CONFIG(debug, debug|release) {
    win32: DESTDIR = Debug
} else {
    win32: DESTDIR = Release
}

OBJECTS_DIR = $${DESTDIR}/obj
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Resources/TextAnalyzerWindow.qrc
