#!/bin/bash
	g++ -g condition.cpp Echoserver2.cpp threadpool.cpp -o server -pthread
	g++ -g Echoclient.cpp -o client
