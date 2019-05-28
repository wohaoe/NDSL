#include"BpTree.h"

Container::Pool Container::ppool = Container::Pool();


Node::Node() : Container(CLASS_NODE)
{
    pairs.clear();
    sp_ptr = NULL;
    parent = NULL;
    left_ptr = NULL;
    right_ptr = NULL;
    node_id = 0;
    nodeType = ' ';
    key_node_pair.clear();
}

Node::Node(BpTree *_tree) : Container(CLASS_NODE)
{
    pairs.clear();
    sp_ptr = NULL;
    parent = NULL;
    left_ptr = NULL;
    right_ptr = NULL;
    tree = _tree;
    tree->node_number += 1;
    node_id = 0;
    nodeType = ' ';
    key_node_pair.clear();
}

//创建新root时使用
Node::Node(BpTree *_tree, int _key, Node *_left, Node *_right) : Container(CLASS_NODE)
{
    pairs.clear();
    pairs.push_back(make_pair(_key, dynamic_cast<Container *>(_left)));
    sp_ptr = _right;

    _left->parent = this;//将新建立的root赋值给子节点的父指针
    _right->parent = this;
    
    left_ptr = NULL;
    right_ptr = NULL;

    _right->becomeRightSibling(_left);

    tree = _tree;
    tree->node_number += 1;
    node_id = 0;
    nodeType = ' ';
    key_node_pair.clear();
}

Node::~Node()
{
    pairs.clear();
    key_node_pair.clear();
}

bool Node::isLeaf()
{
    return(tree->node_number == 1) || (pairs[0].second->getType() == CLASS_VALUE);
}

char Node::getType()
{
    if(isLeaf())
        return 'L';
    else    
        return 'I';
}

void Node::setNextLeaf(Node *_next_leaf)
{
    sp_ptr = _next_leaf;
}

void Node::becomeRightSibling(Node *_left)
{
    if(_left != NULL)
        _left->right_ptr = this;
    this->left_ptr = _left;
}

void Node::becomeLeftSibling(Node *_right)
{
    if(_right != NULL)
        _right->left_ptr = this;
    this->right_ptr = _right;
}

Node *Node::getNextLeaf()
{
    return sp_ptr;
}

void Node::printKeys()
{
    printf("[");
    for(auto it = pairs.begin(); it != pairs.end(); it++)
    {
        if(it == pairs.begin())
            printf("%d", it->first);
        else
            printf(",%d", it->first);
    }
    printf("]");
    printf(" ");
}

void Node::printValues()
{
    printf("[");
    for(auto it = pairs.begin(); it != pairs.end(); it++)
    {
        if(it == pairs.begin())
            printf("%s",((Value *)it->second)->getValue().c_str());
        else
            printf(",%s", ((Value *)it->second)->getValue().c_str());
    }
    printf("]");
}

Container *Node::findChild(int _key)
{
    for(auto it = pairs.begin(); it != pairs.end(); it++)
    {
        if(it->first > _key)
        {
            return it->second;
        }
    }
    return sp_ptr;
}

Container *Node::findValue(int _key)
{
    for(auto it = pairs.begin(); it != pairs.end(); it++)
    {
        if(it->first == _key)
        {
            return it->second;
        }
    }
    return NULL;
}

Container *Node::findLeftMostChild()
{
    if(pairs.size() > 0)
    {
        return pairs.front().second;
    }
    else
    {
        return NULL;
    }
}

int Node::insert(int _key, Container *_con)
{
    if((int)pairs.size() < tree->key_num)
    {
        return forceInsert(_key, _con);
    }
    else
    {
        return FULL;
    }
}       

int Node::forceInsert(int _key, Container *_con)//分裂时暂时破坏B+树结构，强制插入
{
    for(auto it = pairs.begin(); it != pairs.end(); it++)
    {
        if(_key < it->first)
        {
            pairs.insert(it, make_pair(_key, _con));
            if(!isLeaf())
                ((Node *) _con)->parent = this;
            return SUCCESS;
        }
        else if(_key == it->first)
        {
            return SAME_KEY;
        }
    }
    pairs.push_back(make_pair(_key, _con));
    if(!isLeaf())
        (dynamic_cast<Node *>(_con))->parent = this;
    return SUCCESS;
}

int Node::saveData(int _key, Container *_con)
{
    for(auto it = pairs.begin(); it != pairs.end(); it++)
    {
        if(_key < it->first)
        {
            pairs.insert(it, make_pair(_key, _con));
            return SUCCESS;
        }
        else if(_key == it->first)
        {
            return SAME_KEY;
        }
    }
    pairs.push_back(make_pair(_key, _con));
    return SUCCESS;
}

pair<int, Container *> Node::split(int _key, Container *_con)
{
    int insert_status = forceInsert(_key, _con);
    if(insert_status == SAME_KEY)
        return pair<int, Container *>(-1,NULL);
    
    Node *left_node = new Node(tree);   //为分裂节点分配新的节点
    Node *temp_left_ptr = left_ptr;     //当前节点的左兄弟
    left_node->becomeRightSibling(temp_left_ptr);
    becomeRightSibling(left_node);

    int new_key;    //分裂后移到父节点的关键字
    if(isLeaf())
    {
        if(temp_left_ptr != NULL)
            temp_left_ptr->setNextLeaf(left_node);
        left_node->setNextLeaf(this);
        new_key = pairs[(pairs.size()+1)/2].first;  //pairs从0凯撒
    }
    else
    {
        new_key = pairs[pairs.size()/2].first;    //非叶子节点不用复制关键字
    }
    
    for(auto it = pairs.begin(); it != pairs.end(); it++)
    {
        if(it->first != new_key)
        {
            left_node->pairs.push_back(*it);//将要移到父节点之前的关键字移到左边节点
            if(!isLeaf())
                ((Node *)it->second)->parent = left_node;   //将父节点分裂的键值对对应的子节点的父节点设为新分裂的节点
        }
        else
        {
            if(!isLeaf())
            {
                left_node->sp_ptr = (Node *)(it->second);//第二次循环，it已经移到父节点，设置父节点中新分裂的节点的sp_ptr，指向关键字为new_key的it的子节点
                left_node->sp_ptr->parent = left_node;//反向指向
                it++;//内部节点不用中间节点的复制
            }
            pairs.erase(pairs.begin(), it);//删去当前节点中移到左边的节点
            break;
        }
    }
    return make_pair(new_key, (Container *)left_node);//将要移到父节点的关键字和新分配的节点组成键值对
}

int Node::numOfContainer()
{
    if(isLeaf())
    {
        return (pairs.size());
    }
    else
    {
        return (pairs.size()+(int)(sp_ptr != NULL));
    }
}

bool Node::hasEnoughKeys()
{
    if(isLeaf())
    {
        return (numOfContainer() >= FLOOR(tree->key_num +1, 2));
    }
    else
    {
        return (numOfContainer() >= CEIL(tree->key_num +1, 2));
    }
}

bool Node::hasExtraKeys()
{
    if(isLeaf())
    {
        return (numOfContainer() > FLOOR(tree->key_num +1, 2));
    }
    else
    {
        return (numOfContainer() > CEIL(tree->key_num +1, 2));
    }    
}

bool Node::isSibling(Node *_left, Node *_right)
{
    if(_left == NULL || _right == NULL)
        return false;
    return (_left->parent == _right->parent);
}

bool Node::removeValue(int _key)
{
    auto it = pairs.begin();
    while(it != pairs.end())
    {
        if(it->first == _key)
        {
            pairs.erase(it);//将内存归还
            return true;
        }
        it++;
    }
    return false;
}

int Node::remove(int _key)
{
    if(isLeaf() && removeValue(_key) == false)
    {
        return KEY_NOT_FOUND;
    }

    if(!hasEnoughKeys())
        return TOO_FEW_KEYS;
    return SUCCESS;
}

bool Node::borrow(Node *_left, Node *_right, bool right_to_left)
{
    if(_left->parent != _right->parent)
        return false;
    auto parent = _right->parent;

    if(_left->isLeaf() && _right->isLeaf())
    {
        if(right_to_left)
        {
            auto container = _right->pairs.front();
            _right->pairs.erase(_right->pairs.begin());
            _left->pairs.push_back(container);
        }
        else
        {
            auto container = _left->pairs.back();
            _left->pairs.pop_back();
            _right->pairs.insert(_right->pairs.begin(), container);
        }

        for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++)
        {
            if(it->second == _left)
            {
                it->first = _right->pairs.front().first;
            }
        }
    }
    else    //非叶子节点
    {
        int key = -1;
        for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++)
        {
            if(it->second == _left)
            {
                key = it->first;
            }
        }

        if(right_to_left)
        {
            auto container = _right->pairs.front();

            auto new_container = make_pair(key, _left->getNextLeaf());
            _left->pairs.push_back(new_container);  //将父节点的关键字，子节点的container组成的键值对插入左节点尾部
            _right->pairs.erase(_right->pairs.begin());//删除右兄弟的第一个元素

            //修改指针
            ((Node *)container.second)->parent = _left; //将右节点的子节点的父节点指针赋给左节点，因为右节点少了一个元素，左节点借到了一个元素，所以对应的子节点指针也要改变
            _left->setNextLeaf((Node *)container.second);

            for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++)
            {
                if(it->second == _left)  //右节点key上移到父节点
                {
                    it->first = container.first;
                }
            }
        }
        else
        {
            auto container = _left->pairs.back();
            auto new_container = make_pair(key, _left->getNextLeaf());

            _left->setNextLeaf((Node *)container.second);//修改左节点的sp_ptr
            _right->pairs.insert(_right->pairs.begin(), new_container);
        
            for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++)
            {
                if(it->second == _left)
                {
                    it->first = container.first;//左节点的最后一个元素借到了右节点，父节点对应的key也要改变
                }
            }
        }
    }
    return true;
}

void Node::forceRemove()
{
    this->pairs.clear();
    this->tree->node_number--;
    this->key_node_pair.clear();
    this->left_ptr = NULL;
    this->right_ptr = NULL;
}

bool Node::Merge(Node *_left, Node *_right, bool merge_to_right)   //合并
{
    if(_left->parent != _right->parent)
        return false;

    auto parent = _left->parent;

    if(_left->isLeaf() && _right->isLeaf())
    {
        if(merge_to_right)
        {
            for(auto it = _left->pairs.begin(); it != _left->pairs.end(); it++)
            {
                _right->pairs.insert(_right->pairs.begin(), *it);
            }

            _right->becomeRightSibling(_left->left_ptr);
            if(_left->getNextLeaf() != NULL)
            {
                _left->getNextLeaf()->setNextLeaf(_right);
            }

            _left->forceRemove();    //清空节点

            for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++)
            {
                if(it->second == _left)
                {
                    parent->pairs.erase(it);    //删除父节点对应左节点的key
                    break;
                }
            }
        }
        else
        {
            for (auto it = _right->pairs.begin(); it != _right->pairs.end(); ++it)
            {
                _left->pairs.push_back(*it);
            }

            _left->becomeLeftSibling(_right->right_ptr);
            _left->setNextLeaf(_right->getNextLeaf());
            _right->forceRemove();

            int key = -1;
            for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++)
            {
                if(it->second == _right)
                {
                    key = it->first; //保留key;
                    parent->pairs.erase(it);
                    break;
                }
            }

            if(parent->getNextLeaf() != _right && key != -1)
            {
                for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++)
                {
                    if(it->second == _left)
                        it->first = key;//替换key
                }
            }
            else
            {
                parent->pairs.pop_back();//不用替换，直接删除父节点最后一个关键字
                parent->setNextLeaf(_left);
            }
        }
    }
    else//非叶子节点
    {
        int key = -1;
        for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++)
        {
            if(it->second == _left)
            {
                key = it->first;//保存父节点对应左节点的key
            }
        }

        if(merge_to_right)
        {
            //父节点key下移到右节点，和左节点合并一起组成新的节点
            auto new_container = make_pair(key, _left->getNextLeaf());

            ((Node *)new_container.second)->parent = _right;
            _right->pairs.insert(_right->pairs.begin(), new_container);

            for (auto it = _left->pairs.end(); it != _left->pairs.begin(); --it)
            {
                ((Node *)it->second)->parent = _right;
                _right->pairs.insert(_right->pairs.begin(), new_container);
            }

            _right->becomeRightSibling(_left->left_ptr);
            _left->forceRemove();

            for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++)
            {
                if(it->second == _left)
                {
                    parent->pairs.erase(it);
                    break;
                }
            }
        }
        else
        {
            auto new_container = make_pair(key, _left->getNextLeaf());
            _left->pairs.push_back(new_container);

            for (auto it = _right->pairs.begin(); it != _right->pairs.end(); ++it)
            {
                ((Node *)it->second)->parent = _left;
                _left->pairs.push_back(*it);
            }

            auto container = _right->getNextLeaf();
            container->parent = _left;
            
            _left->setNextLeaf(container);

            _left->becomeLeftSibling(_right->right_ptr);
            _right->forceRemove();

            key = -1;
            for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++) 
            {
                if(it->second == _right)
                {
                    key = it->first;
                    parent->pairs.erase(it);// 删除父节点中key
                    break;
                }
            }

            if(parent->getNextLeaf() != _right && key != -1)
            {
                for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++) 
                {
                    if(it->second == _left) //将父节点中对应左节点的key替换
                        it->first = key;
                }   
            }
            else
            {
                parent->pairs.pop_back();
                parent->setNextLeaf(_left);
            }
        }
    }
    return true;
}

BpTree::BpTree(int _key_num)
{
    node_number = 0;
    root = new Node(this);
    height = 1;
    key_num = _key_num;
}

BpTree::BpTree()
{
    node_number = 0;
    root = nullptr;
    height = 0;
    key_num = 0;
}

BpTree::BpTree(const BpTree &_copy)
{
    (*this) = _copy;
}

BpTree::~BpTree()
{

}

BpTree& BpTree::operator=(const BpTree &_other)
{
    root = _other.root;
    height = _other.height;
    key_num = _other.key_num;
    node_number = _other.node_number;
    return *this;
}

int node_size = 0;
int BpTree::insert(int _key, string &_value, sem_t *mutex)
{
    //while(1)
    //{
      //  if(node_size > 10)
        //{
          //  break;
        //}
        sem_wait(mutex);
        Node *current_node = root;//根节点设为当前节点
        while(!current_node->isLeaf())
        {
            current_node = (Node *)current_node->findChild(_key);//找到关键字所在节点
        }

        int key = _key;
        Container *container = new Value(_value);//创建新的container
        int insert_status;

        while((insert_status = current_node->insert(key, container)) = FULL)
        {
            auto p = current_node->split(key, container);  //return make_pair(new_key, (Container *)left_node);
            if(p.second == NULL)
                break;
            
            key = p.first;
            container = p.second;

            if(current_node->parent == NULL)
            {
                makeNewRoot(key, (Node *)container, current_node);//树高度加一
                insert_status = SUCCESS;
                break;
            }
            else
            {
                current_node = current_node->parent;//分裂后如果有父节点，则将父节点设为当前节点
            }
        }
        sem_post(mutex);
        return(insert_status == SUCCESS);
    //}
}

int BpTree::remove(int _key, sem_t *mutex)
{
    sem_wait(mutex);
    Node *current_node = root;
    while(!current_node->isLeaf())
    {
        current_node = (Node *)current_node->findChild(_key);
    }

    int delete_status;
    while( (delete_status = current_node->remove(_key)) == TOO_FEW_KEYS )
    {
        if( current_node->left_ptr != NULL&&Node::isSibling(current_node->left_ptr, current_node) && 
        current_node->left_ptr->hasExtraKeys())
        {
            Node::borrow(current_node->left_ptr, current_node);//向左兄弟借    
        }
        else if(current_node->right_ptr != NULL&&Node::isSibling(current_node, current_node->right_ptr) &&
        current_node->right_ptr->hasExtraKeys())
        {
            Node::borrow(current_node, current_node->right_ptr, true);
        }
        else if(current_node->left_ptr != NULL&&Node::isSibling(current_node->left_ptr, current_node)&&
        !current_node->left_ptr->hasExtraKeys())
        {
            Node::Merge(current_node->left_ptr, current_node);
        }
        else if(current_node->right_ptr != NULL&&Node::isSibling(current_node, current_node->right_ptr)&&
        !current_node->right_ptr->hasExtraKeys())
        {
            Node::Merge(current_node, current_node->right_ptr);
        }
        else
        {

        }

        if(current_node->parent != NULL)
        {
            current_node = current_node->parent;
        }
        else
        {
            deleteEmptyNode(current_node);
            break;
        }
    }
    if(delete_status == KEY_NOT_FOUND)
    {
        return false;
    }
    sem_post(mutex);
    return(delete_status == SUCCESS);
}

string BpTree::find(int _key, sem_t *mutex)
{
    sem_wait(mutex);
    Node *current_node = root;
    while(!current_node->isLeaf())
    {
        current_node = (Node *)current_node->findChild(_key);
    }
    Container *container = current_node->findValue(_key);

    if(container == NULL)
    {
        sem_post(mutex);
        return string("");
    }
    else
    {
        sem_post(mutex);
        return((Value*)container)->getValue();
    }
    sem_post(mutex);
}

void BpTree::printKeys() {
    queue< pair<int, Node *> > que;
    que.push( make_pair(1, root) ); 
    int cur_level = 1;
    while ( !que.empty() ) 
    {    
        auto cur = que.front();
        que.pop();
        if ( !cur.second->isLeaf() ) 
        {
            for (auto it = cur.second->pairs.begin(); it != cur.second->pairs.end(); it++) 
            { 
                que.push( make_pair(cur.first + 1, (Node *)it->second) );
            }
            que.push( make_pair(cur.first + 1, (Node *)cur.second->sp_ptr) );
        }
        if (cur.first > cur_level) 
        {
            printf("\n");
            cur_level = cur.first;
        }
        cur.second->printKeys();
    }
    printf("\n");
}

void BpTree::printValues() {
    Node * current_node = root;
    while ( !current_node->isLeaf() ) 
    {
        current_node = (Node *)current_node->findLeftMostChild();
    }
    
    do{
        current_node->printValues();
        current_node = current_node->sp_ptr;
    }
    while ( current_node );

    printf("\n");  
}

/*int shareMomery()
{
    int shm;
    shm = shm_open("/shm", O_CREAT | O_RDWR, S_IRWXU | S_IRWXO);
    if(shm == -1)
    {
        perror("can not shm_open");
        return 1;
    }
    ftruncate(shm, SIZE);
    mem = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
    if (mem == MAP_FAILED) {
        printf("Could not map shared memory, errno: %d\n", errno);
        return 1;
    }
}*/

