CONFIG += debug

HEADERS += Arbiter.h \
           MainWindow.h \
           MapControl.h \
           MapView.h \
           Player.h \
           SimplePlayer.h \
           SimplePlayer2.h \
           SimplePlayer3.h \
           GuiPlayer.h \
           world.h
SOURCES += Arbiter.cpp \
           MainWindow.cpp \
           MapControl.cpp \
           MapView.cpp \
           SimplePlayer.cpp \
           SimplePlayer2.cpp \
           SimplePlayer3.cpp \
           GuiPlayer.cpp \
           world.cpp \
           main.cpp

QMAKE_CXXFLAGS_DEBUG += -D_GLIBCXX_DEBUG
