#include"Heap.h"
#include"Block.h"
#include"SuperBlock.h"
#include"SingleBlock.h"
#include"PrintMessage.h"
#include"Lockable.h"
#include<memory.h>
#include<string.h>
#include<unistd.h>
#include<sys/mman.h>
#include<map>
#include <vector>
#include <map>

using namespace std;

void test1() {
    const int count = 10000;
    const int maxSize = 10;
    
    vector<void*> ptrs;
    vector<int> sizes;
    map<void*, int> idByPtr;
    int nextID = 1;
    bool flag = true;
    for (int i = 0; i < count && flag; i++) {
        for (int j = 0; j < count && flag; j++) {
            int size = rand() % maxSize + 1;
            void* ptr = poolMalloc(size, 8);
            printMessageln("malloc", size, ptr);
            memset(ptr, 239, size);
            if (idByPtr.find(ptr) == idByPtr.end()) {
                idByPtr[ptr] = nextID;
                nextID++;
            } else {
                //cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
                flag = false;
            }
            //cout << size << " alloc " << ptr << " " << idByPtr[ptr] << endl;
            ptrs.push_back(ptr);
            sizes.push_back(size);
        }

        cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"<<endl;
        for (int j = 0; j < count / 2; j++) {
            int randI = rand() % ptrs.size();
            int size = sizes[randI];
            void* ptr = ptrs[randI];
            //cout << size << " deall " << ptr << " " << idByPtr[ptr] << endl;
            sizes.erase(sizes.begin() + randI);
            ptrs.erase(ptrs.begin() + randI);
            poolFree(ptr);
            //cout<<"!!!!!!!!!!!!!!!!!!!!!!!!\n"<<endl;
        }
    }
    cout.flush();
}

int main(int argc, char** argv) {
    test1();
    return 0;
}

