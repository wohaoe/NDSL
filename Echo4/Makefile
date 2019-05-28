Reuzel: Echoserver1.o Thread.o ThreadPool.o CountDownLatch.o
	g++ -o server Echoserver1.o Thread.o ThreadPool.o CountDownLatch.o -lpthread
	rm *.o
Echoserver1.o: ./Echoserver1.cpp
	g++ -c -std=c++11 ./Echoserver1.cpp
Thread.o: ./src/Thread.cpp
	g++ -c -std=c++11 ./src/Thread.cpp
ThreadPool.o: ./src/ThreadPool.cpp
	g++ -c -std=c++11 ./src/ThreadPool.cpp
CountDownLatch.o: ./src/CountDownLatch.cpp
	g++ -c -std=c++11 ./src/CountDownLatch.cpp
