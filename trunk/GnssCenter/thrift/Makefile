
BOOST_DIR  = /usr/local/include
THRIFT_DIR = /usr/local/include/thrift
LIB_DIR    = /usr/local/lib
DEFS       = -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H

INC = -I$(THRIFT_DIR) -I$(BOOST_DIR)

GEN_SRC = client.cpp \
          gen-cpp/RtnetData.cpp \
          gen-cpp/rtnet_constants.cpp gen-cpp/rtnet_types.cpp \
          gen-cpp/rtnet_data_constants.cpp gen-cpp/rtnet_data_types.cpp

GEN_OBJ = $(patsubst %.cpp,%.o, $(GEN_SRC))

.PHONY: all clean

all: gen-cpp client

gen-cpp: rtnet.thrift
	thrift -r -gen cpp rtnet.thrift

%.o: %.cpp
	$(CXX) -Wall -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H $(INC) -c $< -o $@

## client: client.o $(GEN_OBJ)
## 	$(CXX) $^ -o $@ -L/usr/local/lib -lthrift 

client: client.o $(GEN_OBJ)
	$(CXX) $^ -o $@ /usr/local/lib/libthrift.a

clean:
	$(RM) *.o gen-cpp/*.o client