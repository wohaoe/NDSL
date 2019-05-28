#include"PrintMessage.h"

bool isTraceEnabled() {
        char* envValue = getenv("NO_TRACE");
        return envValue == NULL || envValue[0] == '0';
    }
    
    void printMessage(size_t size) {
        if (!isTraceEnabled()) {
            return;
        }
        int pow = 1;
        size_t pow10 = 10;
        while(pow10 <= size) {
            pow++;
            pow10 *= 10;    
        }
        char* message = (char*) mmap(NULL, pow, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (size == 0) {
            *message = '0';
    } else {
        int i = pow - 1;
        while (size > 0) {
            message[i] = '0' + size % 10;
            i--;
            size /= 10;
        }
    }
    write(2, message, pow);
    munmap(message, pow);
}

void printMessage(void* pointer) {
    printMessage((size_t) pointer);
}

void printMessage(const char* message) {
    if (!isTraceEnabled()) {
        return;
    }
    write(2, message, strlen(message));
}

void printMessageln(const char* message) {
    printMessage(message);
    printMessage("\n");
}

void printMessageln(const char* message, size_t size) {
    printMessage(message);
    printMessage(" ");
    printMessage(size);
    printMessage("\n");
}

void printMessageln(const char* message, void* pointer) {
    printMessage(message);
    printMessage(" ");
    printMessage(pointer);
    printMessage("\n");
}

void printMessageln(const char* message, size_t size, void* pointer) {
    printMessage(message);
    printMessage(" ");
    printMessage(size);
    printMessage(" ");
    printMessage(pointer);
    printMessage("\n");
}

void printMessageln(const char* message, size_t size, size_t size2, void* pointer) {
    printMessage(message);
    printMessage(" ");
    printMessage(size);

    printMessage(" ");
    printMessage(size2);
    printMessage(" ");
    printMessage(pointer);
    printMessage("\n");
}

void printMessageln(const char* message, size_t size, void* pointer, void* pointer2) {
    printMessage(message);
    printMessage(" ");
    printMessage(size);
    printMessage(" ");
    printMessage(pointer);
    printMessage(" ");
    printMessage(pointer2);
    printMessage("\n");
}
