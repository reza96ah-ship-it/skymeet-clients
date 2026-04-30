QT += core gui qml quick network websockets quickcontrols2

CONFIG += c++17

TEMPLATE = app
TARGET = skymeet_qml_client_qt515

SOURCES += \
    src/main.cpp \
    src/ApiClient.cpp \
    src/EventSocket.cpp

HEADERS += \
    src/ApiClient.h \
    src/EventSocket.h

RESOURCES += qml.qrc

win32: CONFIG += windows
