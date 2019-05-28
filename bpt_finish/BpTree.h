#ifndef BTREE_H_
#define BPTREE_H_

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<iostream>
#include<string>
#include<vector>
#include<memory>
#include<utility>
#include<queue>
#include<set>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include<wait.h>
#include<semaphore.h>
#include<sys/types.h>
#include<map>
#include<sys/mman.h>

#define SUCCESS       0
#define UNKNOWN_ERROR 1
#define FULL          2
#define SAME_KEY      3
#define KEY_NOT_FOUND 4
#define TOO_FEW_KEYS  5

#define CLASS_VALUE   1
#define CLASS_NODE    2

#define FLOOR(x, y) int( (x) / (y) )
#define CEIL(x, y)  int( (x - 1) / (y) + 1 )

#define KEY_NUM 10
#define NODE_NUMBER 50*50*50

#define SIZE NODE_NUMBER*4


using namespace std;
class Container;
class BpTree;

class Container
{
private:
    class Pool
    {
    private:
        set<Container *> pool;
        bool is_access;
    public:
        Pool()
        {
            is_access = true;
        }
        ~Pool()
        {
            is_access = false;
            for(auto it = pool.begin(); it != pool.end(); it++)
            {
                delete(*it);
            }
        }

        void insert(Container *_con)
        {
            if(is_access)
                pool.insert(_con);
        }
        void erase(Container *_con)
        {
            if(is_access)
                pool.erase(_con);
        }
    };
    static Pool ppool;//静态内部类，外部类第一次被实例化即构造出来，直到全部外部类实例引用断开时才析构
    int type;
public:
    Container(int _type)
    {
        type = _type;
        ppool.insert(this);
    }
    virtual ~Container()
    {
        ppool.erase(this);
    }
    int getType()
    {
        return type;
    }
};

class Value : public Container
{
private:
    string value;
public:
    Value(string _value) : Container(CLASS_VALUE)
    {
        value = _value;
    }
    string getValue()
    {
        return value;
    }
};

class Node : public Container
{
public:
    vector<pair<int, Container *>> pairs;   //vector存放在堆中

    //指针
    Node *sp_ptr;
    Node *parent;
    BpTree *tree;
    Node *left_ptr;
    Node *right_ptr;

    //存入磁盘时的节点id
    int node_id;
    char nodeType;
    vector<pair<int, Node *>> key_node_pair;

    Node();
    Node(BpTree *_tree);
    
    //新建根节点
    Node(BpTree *_tree, int _key, Node *_left, Node *_right);
    ~Node();

    int insert(int _key, Container *_con);
    int forceInsert(int _key, Container *_con);

    int saveData(int _key, Container *_con);

    pair<int, Container *> split(int _key, Container *_con);

    Container *findChild(int _key);
    Container *findValue(int _key);
    Container *findLeftMostChild();

    bool removeValue(int _key);
    void forceRemove();
    int remove(int _key);

    static bool Merge(Node *_left, Node *_right, bool merge_to_right = false);
    static bool borrow(Node *_left, Node *_right, bool right_to_left = false);
    static bool isSibling(Node *_left, Node *_right);

    int numOfContainer();
    bool hasEnoughKeys();
    bool hasExtraKeys();
    bool isLeaf();

    char getType();
    Node *getNextLeaf();
    void setNextLeaf(Node *_next_leaf);
    void becomeRightSibling(Node *_left);
    void becomeLeftSibling(Node *_right);

    void printKeys();
    void printValues();
};

class BpTree
{
private:
    Node *root;
    int height;

    //分裂时产生新根节点
    void makeNewRoot(int _key, Node *_left, Node *_right)
    {
        root = new Node(this, _key, _left, _right);
        height += 1;
    }

    //合并时减少高度，删除空的节点
    void deleteEmptyNode(Node *current_node)
    {
        if(isRoot(current_node) && height > 1)
        {
            root = current_node->sp_ptr;
            root->parent = NULL;
            height -= 1;
        }
    }
public:
    bool isRoot(Node *_root)
    {
        return (_root == root);
    }

    int node_number;
    int key_num;

    BpTree(int _key_num);
    BpTree();
    BpTree(const BpTree &_copy);
    ~BpTree();

    //重载
    BpTree &operator =(const BpTree &_other);

    int insert(int _key, string &_value,sem_t *mutex);
    int remove(int _key, sem_t *mutex);
    string find(int _key, sem_t *mutex);

    void printKeys();
    void printValues();
    void save();
    void load();
};


#endif
