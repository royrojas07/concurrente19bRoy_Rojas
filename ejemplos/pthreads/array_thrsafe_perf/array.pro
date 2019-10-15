TEMPLATE = app
QT -= core
CONFIG += console c11
CONFIG -= qt app_bundle

QMAKE_CFLAGS += -pthread
LIBS += -pthread

SOURCES += main.c \
    array_mutex.c \
    array_rwlock.c \

HEADERS += \
    array_mutex.h \
    array_rwlock.h \
