LIB=-lpthread /usr/local/lib/libzookeeper_mt.a /usr/local/lib/libboost_system.a /usr/local/lib/libboost_thread.a -lpthread -lrt -DTHREADED
INC=-I../lib/ -I../common/

SRC=$(wildcard ../lib/*.cpp ./*.cpp ../common/*.cpp)
OBJ=$(patsubst %.cpp, %.o,$(SRC))

.cpp.o:
	g++ -o $@ -c $< $(INC)

.c.o:
	g++ -o $@ -c $< $(INC)

master:$(OBJ)
	g++ -o master $(OBJ) $(LIB) $(INC)

clean:
	rm $(OBJ)
