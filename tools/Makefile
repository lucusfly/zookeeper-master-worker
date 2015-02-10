CFLAG=/usr/local/lib/libzookeeper_mt.a /usr/local/lib/libboost_system.a /usr/local/lib/libboost_thread.a -lpthread -lrt
CFLAG2=/usr/local/lib/libzookeeper_mt.a -lpthread -DTHREADED

INC=-I../common -I../lib
SRC=../lib/zookeeper.cpp

clearDir:clearDir.cpp
	g++ -o clearDir clearDir.cpp $(SRC) $(CFLAG) $(INC)

clean:
	rm clearDir
