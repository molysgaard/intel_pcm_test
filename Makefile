test: test.cpp
	g++ -std=c++11 -O0 IntelPCM/pci.cpp IntelPCM/msr.cpp IntelPCM/client_bw.cpp IntelPCM/cpucounters.cpp test.cpp -o test -lrt -lpthread
