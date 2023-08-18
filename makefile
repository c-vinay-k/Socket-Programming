CC=g++

all: client-phase1 client-phase2 client-phase3 client-phase4 client-phase5

client-phase1: client-phase1.cpp
	$(CC) -o $@ $^ -pthread

client-phase2: client-phase2.cpp
	$(CC) -o $@ $^ -pthread

client-phase3: client-phase3.cpp
	$(CC) -o $@ $^ -pthread -lssl -lcrypto

client-phase4: client-phase4.cpp
	$(CC) -o $@ $^ -pthread

client-phase5: client-phase5.cpp
	$(CC) -o $@ $^ -pthread -lssl -lcrypto

