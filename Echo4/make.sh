#!/bin/bash
	g++ -std=c++11 ./Echoserver2.cpp ./src/*.cpp -o server -pthread
	g++ -std=c++11 ./Echoclient.cpp ./src/*.cpp -o client -pthread

