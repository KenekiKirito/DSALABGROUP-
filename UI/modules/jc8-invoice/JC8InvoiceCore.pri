# Include this file from the team's qmake .pro file:
#     include(path/to/JC8InvoiceCore.pri)

CONFIG += c++17

INCLUDEPATH += $$PWD/src

HEADERS += \
    $$PWD/src/core/Types.h \
    $$PWD/src/core/InvoiceList.h \
    $$PWD/src/core/Stack.h \
    $$PWD/src/core/TransactionManager.h

SOURCES += \
    $$PWD/src/core/Types.cpp \
    $$PWD/src/core/InvoiceList.cpp \
    $$PWD/src/core/TransactionManager.cpp
