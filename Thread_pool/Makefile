Reuzel: test.o Thread.o ThreadPool.o CountDownLatch.o
	g++ -o Reuzel test.o Thread.o ThreadPool.o CountDownLatch.o -lpthread
	rm *.o
test.o: ./test/test.cpp
	g++ -c -std=c++11 ./test/test.cpp
Thread.o: ./src/Thread.cpp
	g++ -c -std=c++11 ./src/Thread.cpp
ThreadPool.o: ./src/ThreadPool.cpp
	g++ -c -std=c++11 ./src/ThreadPool.cpp
CountDownLatch.o: ./src/CountDownLatch.cpp
	g++ -c -std=c++11 ./src/CountDownLatch.cpp
