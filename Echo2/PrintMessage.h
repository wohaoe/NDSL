#ifndef PRINTMESSAGE_H_

#include<cstddef>
#include<cstdlib>
#include<unistd.h>
#include<sys/mman.h>
#include<cstring>

bool isTraceEnabled();
void printMessage(void* pointer);

void printMessage(const char* message);

void printMessageln(const char* message);
void printMessageln(const char* message, size_t size);
void printMessageln(const char* message, void* pointer);

void printMessageln(const char* message, size_t size, void* pointer);
void printMessageln(const char* message, size_t size, size_t size2, void* pointer);
void printMessageln(const char* message, size_t size, void* pointer, void* pointer2);
#endif
