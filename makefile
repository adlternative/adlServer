
CPP = g++
RM = rm -rf

SRC =

SRC += src/main.cpp
SRC += src/base/currentThread.cpp
SRC += src/base/timeStamp.cpp
SRC += src/log/asyncLogging.cpp
SRC += src/log/fileUtil.cpp
SRC += src/log/Logging.cpp
SRC += src/log/logStream.cpp
SRC += src/log/logFile.cpp
SRC += src/net/tcpServer.cpp
SRC += src/net/tcpConnection.cpp
SRC += src/net/EventLoop.cpp
SRC += src/net/EventLoopThread.cpp
SRC += src/net/EventLoopThreadPool.cpp
SRC += src/net/Epoller.cpp
SRC += src/net/Acceptor.cpp
SRC += src/net/InetAddress.cpp
SRC += src/net/Socket.cpp
SRC += src/net/Channel.cpp
SRC += src/net/netBuffer.cpp

OBJS := $(SRC:.cpp=.o)


all: build
# build: OBJ
# 	$(CPP) $(OBJS) -lpthread -o a.out
# OBJ:
# 	$(CPP) -c $(SRC) -o $(OBJS)
build :
	$(CPP) $(SRC) -g -lpthread -o a.out

