CONFIG += debug

QMAKE_CXXFLAGS += -std=c++0x
QMAKE_CXXFLAGS_DEBUG += -D_GLIBCXX_DEBUG

HEADERS += Arbiter.h \
           MainWindow.h \
           MapControl.h \
           MapView.h \
           Player.h \
           SimplePlayer.h \
           SimplePlayer2.h \
           SimplePlayer3.h \
           LineReader.h \
           GuiPlayer.h \
           world.h
SOURCES += Arbiter.cpp \
           LineReader.cpp \
           MainWindow.cpp \
           MapControl.cpp \
           MapView.cpp \
           SimplePlayer.cpp \
           SimplePlayer2.cpp \
           SimplePlayer3.cpp \
           GuiPlayer.cpp \
           world.cpp \
           main.cpp
