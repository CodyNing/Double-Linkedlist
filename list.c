#include <assert.h>
#include <stddef.h>
#include "list.h"

//static allocated head array
static List s_heads[LIST_MAX_NUM_HEADS];
//static allocated node array
static Node s_nodes[LIST_MAX_NUM_NODES];
//stack head to the head array
static List *s_pFreeHead = s_heads;
//stack head to the node array
static Node *s_pFreeNode = s_nodes;
//static boolean to indicate the whether stack has been init'd
static bool s_hasInit = 0;

//init the stack
static void s_init()
{
    //push all head into the head stack
    //and set data to inital value
    for (size_t i = 0; i < LIST_MAX_NUM_HEADS; ++i)
    {
        s_heads[i].head = NULL;
        s_heads[i].tail = NULL;
        s_heads[i].cur = NULL;
        s_heads[i].isBeforeHead = 1;
        s_heads[i].length = 0;
        s_heads[i].stackNext = s_heads + i + 1;
        s_heads[i].isFree = true;
    }
    s_heads[LIST_MAX_NUM_HEADS - 1].stackNext = NULL;
    ///push all node into the node stack
    //and set data to inital value
    for (size_t i = 0; i < LIST_MAX_NUM_NODES; ++i)
    {
        s_nodes[i].data = NULL;
        s_nodes[i].listPrev = NULL;
        s_nodes[i].listNext = NULL;
        s_nodes[i].stackNext = s_nodes + i + 1;
        s_nodes[i].isFree = true;
    }
    s_nodes[LIST_MAX_NUM_NODES - 1].stackNext = NULL;
}

//push a head into the head stack
static void s_push_free_head(List *head)
{
    //stack guard to prevent pushing existing node
    //which will corrupt the linked list linkage
    if (head->isFree)
    {
        return;
    }
    //erase data just to be safe
    head->head = NULL;
    head->tail = NULL;
    head->cur = NULL;
    head->isBeforeHead = true;
    head->length = 0;
    head->isFree = true;
    head->stackNext = s_pFreeHead;
    s_pFreeHead = head;
}

//pop a head out of head stack
static List *s_pop_free_head()
{
    List *free = s_pFreeHead;
    if (free != NULL)
    {
        s_pFreeHead = s_pFreeHead->stackNext;
        free->isFree = false;
        free->stackNext = NULL;
    }
    return free;
}

//push a node into the node stack
static void s_push_free_node(Node *node)
{
    //stack guard to prevent pushing existing node
    //which will corrupt the linked list linkage
    if (node->isFree)
    {
        return;
    }
    //erase the data just to be safe
    node->data = NULL;
    node->listNext = NULL;
    node->listPrev = NULL;
    node->isFree = true;
    node->stackNext = s_pFreeNode;
    s_pFreeNode = node;
}

//pop a node out of node stack
static Node *s_pop_free_node()
{
    Node *free = s_pFreeNode;
    if (free != NULL)
    {
        s_pFreeNode = s_pFreeNode->stackNext;
        free->isFree = false;
        free->stackNext = NULL;
    }
    return free;
}

//when adding or inserting to a list with null cur,
//do a special insert logic
static void s_special_insert(List *pList, Node *new)
{
    //calling this function when cur is not null is not allowed
    assert(pList->cur == NULL);

    //if before head, add the item to start
    if (pList->isBeforeHead)
    {
        new->listNext = pList->head;
        new->listPrev = NULL;

        //if there was a head, head should point back to the added item
        if (pList->head)
        {
            pList->head->listPrev = new;
        }
        //there was no head, the list was empty
        //the new item should also be the tail
        else
        {
            pList->tail = new;
        }

        //let the item be the new head
        pList->head = new;
    }
    //current is NULL and not before head, means it should be after tail
    //add it to the tail
    else
    {
        new->listPrev = pList->tail;
        new->listNext = NULL;

        //if there was a tail, tail should point to the added item
        if (pList->tail)
        {
            pList->tail->listNext = new;
        }
        //there was no tail, the list was empty
        //the new item should also be the head
        else
        {
            pList->head = new;
        }

        //let the item be the new tail
        pList->tail = new;
    }
}

//centralized assert
static void s_List_assert(List *pList){
    //the given pointer must not be NULL or in the pool
    assert(pList != NULL && !pList->isFree);
}

// Makes a new, empty list, and returns its reference on success.
// Returns a NULL pointer on failure.
List *List_create()
{
    //O(n) set-up at the very first time
    if (!s_hasInit)
    {
        s_init();
        s_hasInit = true;
    }

    //return the top of the head stack
    return s_pop_free_head();
}

// Returns the number of items in pList.
int List_count(List *pList)
{
    s_List_assert(pList);
    return pList->length;
}

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void *List_first(List *pList)
{
    s_List_assert(pList);
    //head is not null, then return head
    if (pList->head)
    {
        pList->cur = pList->head;
        return pList->cur->data;
    }
    //head is null means list is empty
    //then cur should be before head
    else
    {
        pList->cur = NULL;
        pList->isBeforeHead = true;
        return NULL;
    }
}

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void *List_last(List *pList)
{
    s_List_assert(pList);

    //no matter what, cur should no longer before head
    pList->isBeforeHead = false;
    //tail is not null return tail
    if (pList->tail)
    {
        pList->cur = pList->tail;
        return pList->cur->data;
    }
    //else list is empty,
    else
    {
        pList->cur = NULL;
        return NULL;
    }
}

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer
// is returned and the current item is set to be beyond end of pList.
void *List_next(List *pList)
{
    s_List_assert(pList);
    //if cur is not empty, return next
    if (pList->cur)
    {
        pList->cur = pList->cur->listNext;
        pList->isBeforeHead = false;
    }
    //if current is empty, need to know if current is beyond head or after tail
    else
    {
        //if beyond head, next should be point to head
        if (pList->isBeforeHead)
        {
            pList->cur = pList->head;
            pList->isBeforeHead = false;
        }
        //current is empty and not beyond head, means it should be after tail
        else
        {
            return NULL;
        }
    }
    return List_curr(pList);
}

// Backs up pList's current item by one, and returns a pointer to the new current item.
// If this operation backs up the current item beyond the start of the pList, a NULL pointer
// is returned and the current item is set to be before the start of pList.
void *List_prev(List *pList)
{
    s_List_assert(pList);
    //if cur is not empty, return prev
    if (pList->cur)
    {
        //if cur is head, the calling prev will point cur to before head
        if (pList->cur == pList->head)
        {
            pList->isBeforeHead = true;
        }
        pList->cur = pList->cur->listPrev;
    }
    //if current is NULL, need to know if current is beyond head or after tail
    else
    {
        //if beyond head, do nothing
        if (pList->isBeforeHead)
        {
            return NULL;
        }
        //current is NULL and not beyond head, means it should be after tail
        //so prev should be tail
        else
        {
            pList->cur = pList->tail;
        }
    }
    return List_curr(pList);
}

// Returns a pointer to the current item in pList.
void *List_curr(List *pList)
{
    s_List_assert(pList);
    //return data if cur is not null
    if (pList->cur)
    {
        return pList->cur->data;
    }
    else
    {
        return NULL;
    }
}

// Adds the new item to pList directly after the current item, and makes item the current item.
// If the current pointer is before the start of the pList, the item is added at the start. If
// the current pointer is beyond the end of the pList, the item is added at the end.
// Returns 0 on success, -1 on failure.
int List_add(List *pList, void *pItem)
{
    s_List_assert(pList);
    //if no free node, insert fail
    if (!s_pFreeNode)
    {
        return -1;
    }

    //pop the top of the node stack
    Node *new = s_pop_free_node();
    //insert the data
    new->data = pItem;

    //cur is not null, perform normal double linked list insert
    if (pList->cur)
    {
        Node *next = pList->cur->listNext;

        //1. connect new node with next node
        new->listNext = next;

        //2. connect cur node with new node
        pList->cur->listNext = new;

        //3. connect new node with cur node
        new->listPrev = pList->cur;

        //if next node is not null
        if (next)
        {
            //4. connect next node with new node
            next->listPrev = new;
        }
        //otherwise, cur is the tail
        else
        {
            //5. make new node the new tail
            pList->tail = new;
        }
    }
    //if current is NULL, do special insert logic
    else
    {
        s_special_insert(pList, new);
    }
    pList->cur = new;
    ++(pList->length);

    return 0;
}

// Adds item to pList directly before the current item, and makes the new item the current one.
// If the current pointer is before the start of the pList, the item is added at the start.
// If the current pointer is beyond the end of the pList, the item is added at the end.
// Returns 0 on success, -1 on failure.
int List_insert(List *pList, void *pItem)
{
    s_List_assert(pList);
    //if no free node, insert fail
    if (!s_pFreeNode)
    {
        return -1;
    }

    //pop the top of the node stack
    Node *new = s_pop_free_node();
    //insert the data
    new->data = pItem;

    //cur is not null, perform normal double linked list insert
    if (pList->cur)
    {
        Node *prev = pList->cur->listPrev;

        //1. connect new node with cur node
        new->listNext = pList->cur;

        //2. connect cur node with new node
        pList->cur->listPrev = new;

        //3. connect new node with prev node
        new->listPrev = prev;

        //if prev node is not null
        if (prev)
        {
            //4. connect prev node with new node
            prev->listNext = new;
        }
        //otherwise, cur is the head
        else
        {
            //5. make new node the new head
            pList->head = new;
        }
    }
    //if current is NULL, do special insert logic
    else
    {
        s_special_insert(pList, new);
    }
    pList->cur = new;
    ++(pList->length);

    return 0;
}

// Adds item to the end of pList, and makes the new item the current one.
// Returns 0 on success, -1 on failure.
int List_append(List *pList, void *pItem)
{
    s_List_assert(pList);

    //make cur the tail
    //then reuse the add logic
    pList->cur = pList->tail;
    return List_add(pList, pItem);
}

// Adds item to the front of pList, and makes the new item the current one.
// Returns 0 on success, -1 on failure.
int List_prepend(List *pList, void *pItem)
{
    s_List_assert(pList);

    //make cur the head
    //then reuse the insert logic
    pList->cur = pList->head;
    return List_insert(pList, pItem);
}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void *List_remove(List *pList)
{
    s_List_assert(pList);

    // nothing to remove
    if (!pList->cur)
    {
        return NULL;
    }

    void *data = pList->cur->data;

    //if there is a next, link it back to the prev
    if (pList->cur->listNext)
    {
        pList->cur->listNext->listPrev = pList->cur->listPrev;
    }
    //if not, cur is the tail, so prev will be the new tail
    else
    {
        pList->tail = pList->cur->listPrev;
    }

    //if there is a prev, link it to the next
    if (pList->cur->listPrev)
    {
        pList->cur->listPrev->listNext = pList->cur->listNext;
    }
    //if not, cur is the head, so next will be the new head
    else
    {
        pList->head = pList->cur->listNext;
    }

    //point cur to next before erasing the data
    Node *cur = pList->cur;
    pList->cur = pList->cur->listNext;

    s_push_free_node(cur);
    --pList->length;

    return data;
}

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1.
// pList2 no longer exists after the operation; its head is available
// for future operations.
void List_concat(List *pList1, List *pList2)
{
    s_List_assert(pList1);
    s_List_assert(pList2);

    //only perform concat if list 2 is not empty
    if (pList2->head)
    {
        //if list 1 is not empty
        if (pList1->tail)
        {
            //link list 1 tail to list 2 head
            pList1->tail->listNext = pList2->head;
            pList1->tail = pList2->tail;
        }
        //if list 1 is empty
        //list 1's head and tail should be list 2's head and tail
        else
        {
            pList1->head = pList2->head;
            pList1->tail = pList2->tail;
        }
    }

    //add up the length
    pList1->length += pList2->length;

    //directly push list 2 back to the stack for future reuse (no need to remove)
    s_push_free_head(pList2);
}

// Delete pList. itemFree is a pointer to a routine that frees an item.
// It should be invoked (within List_free) as: (*pItemFree)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and s_nodes are
// available for future operations.
void List_free(List *pList, FREE_FN pItemFreeFn)
{
    s_List_assert(pList);
    assert(pItemFreeFn != NULL);

    //set the cur to head, so we can loop through
    pList->cur = pList->head;

    //while list is not empty
    while (pList->length)
    {
        //free the data
        void *data = pList->cur->data;
        (*pItemFreeFn)(data);
        //remove the node
        List_remove(pList);
    }

    //recycle the head
    s_push_free_head(pList);
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void *List_trim(List *pList)
{
    s_List_assert(pList);

    //make cur the tail
    pList->cur = pList->tail;
    //remove cur = remove tail
    void *pop = List_remove(pList);
    //make cur the new tail
    pList->cur = pList->tail;
    return pop;
}

// Search pList, starting at the current item, until the end is reached or a match is found.
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match,
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator.
//
// If a match is found, the current pointer is left at the matched item and the pointer to
// that item is returned. If no match is found, the current pointer is left beyond the end of
// the list and a NULL pointer is returned.
void *List_search(List *pList, COMPARATOR_FN pComparator, void *pComparisonArg)
{
    s_List_assert(pList);

    if(pList->isBeforeHead){
        List_next(pList);
    }

    //while cur is not NULL
    while (pList->cur)
    {
        //compare data in cur with compartor and arg
        //if equal then return
        if (pComparator(pList->cur->data, pComparisonArg))
        {
            return pList->cur->data;
        }
        //not equal, make cur the next
        List_next(pList);
    }
    // not found
    return NULL;
}
