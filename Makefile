all: loopback

loopback:
	g++ -o loopback -D_FILE_OFFSET_BITS=64 -std=c++0x *.cpp -Iinclude -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux -fPIC -O3 -Wall -lfuse
