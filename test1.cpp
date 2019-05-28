#include<iostream>
#include<queue>

using namespace std;

enum COLOR{RED, BLACK};

class Node
{
public:
    int val;
    COLOR color;
    Node *left, *right, *parent;

    Node(int val) : val(val)
    {
        parent = left = right = NULL;
        color = RED;
    }

    Node *uncle()
    {
        if(parent == NULL or parent->parent == NULL)
            return NULL;
        
        if(parent->isOnLeft())
            return parent->parent->right;
        else
            return parent->parent->left;
    }

    bool isOnLeft()
    {
        return this == parent->left;
    }

    Node *sibling()
    {
        if(parent == NULL)
            return NULL;

        if(isOnLeft())
            return parent->right;
        
        return parent->left; 
    }

    void moveDown(Node *nParent)
    {
        if(parent != NULL)
        {
            if(isOnLeft())
            {
                parent->left = nParent
            }
            else
            {
                parent->right = nParent;
            }

            nParent->parent = parent;

            parent = nParent;
        }
    }

    bool hasRedChild()
    {
        return ((left != NULL and left->color ==RED) or (right != NULL and right->color ==RED));
    }
};

class RBTree
{
private:
    Node *root;

    void leftRotate(Node *x)
    {
        Node *nparent = x->right;

        if(x == root)
        {
            root = nparent;
        }

        x->moveDown(nparent);

        x->right = nparent->left;

        if(nparent->left != NULL)
        {
            nparent->left->parent = x;
        }

        nparent->left = x;
    }

    void rightRotate(Node *x)
    {
        Node *nparent = x->left;

        if(x == root)
        {
            root = nparent;
        }

        x->moveDown(nparent);

        x->left = nparent->right;

        if(nparent->right != NULL)
        {
            nparent->right->parent = x;
        }

        nparent->right = x;
    }

    void swapColor(Node *x1, Node *x2)
    {
        COLOR temp;
        temp = x1->color;
        x1->color = x2->color;
        x2->color = temp;
    }

    void swapVal(Node *u, Node *v)
    {
        int temp;
        temp = u->val;
        u->val = v->val;
        v->val = temp;
    }

    //被插入的节点的父节点为红色，判断uncle的颜色，并分别处理
    void fixRedRed(Node *x)
    {
        if(x == root)
        {
            x->color = BLACK;
            return;
        }

        //initialize parent, grandparent, uncle
        Node *parent = x->parent, *grandparent = parent->parent, *uncle = x->uncle();

        if(parent->color != BLOCK)
        {
            if(uncle != NULL && uncle->color == RED)
            {
                parent->color = BLACK;
                uncle->color = BLACK;
                grandparent->color = RED;
                fixRedRed(grandparent);
            }
            else
            {
                //else perform LR, LL, RL, RR
                if(parent->isOnLeft())
                {
                    if(x->isOnLeft())   //LL
                    {
                        swapColor(parent, grandparent);
                    }
                    else    //LR
                    {
                        leftRotate(parent);
                        swapColor(x, grandparent);
                    }

                    //for LL, LR
                    rightRotate(grandparent);
                }

                else
                {
                    if(x->isOnLeft())   //RL
                    {
                        rightRotate(parent);
                        swapColor(x, grandparent);
                    }    
                    else
                    {
                        swapColor(parent, grandparent); //RR
                    }
                    
                    leftRotate(grandparent);
                }
            }
            
        }
    }

    Node *successor(Node *x)
    {
        Node *temp = x;
        while(temp->left != NULL)
            temp = temp->left;
        return temp;
    }

    Node *BSTreplace(Node *x)
    {
        if(x->left != NULL and x->right != NULL)
        {
            return successor(x->right);
        }

        if(x->left == NULL and x->right == NULL)
        {
            return NULL;
        }

        if(x->left != NULL)
        {
            return x->left;
        }
        else
        {
            return x->right;
        }
    }

    
};
