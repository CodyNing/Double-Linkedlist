/**
 * Sample test routine for executing each function at least once.
 * Copyright Brian Fraser, 2020
 */

#include "list.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

// Macro for custom testing; does exit(1) on failure.
#define CHECK(condition) do{ \
    if (!(condition)) { \
        printf("ERROR: %s (@%d): failed condition \"%s\"\n", __func__, __LINE__, #condition); \
        exit(1);\
    }\
} while(0)


static void s_shuffle(size_t * pArr, size_t size){
    for(size_t i = 0; i < size - 1; ++i){
        size_t j = i + rand() / (RAND_MAX / (size - i) + 1);
        size_t val = pArr[i];
        pArr[i] = pArr[j];
        pArr[j] = val;
    }
}

static void s_free_do_nothing(void* item){

}

// For checking the "free" function called
static int complexTestFreeCounter = 0;
static void complexTestFreeFn(void* pItem) 
{
    CHECK(pItem != NULL);
    complexTestFreeCounter++;
}

// For searching
static bool itemEquals(void* pItem, void* pArg) 
{
    return (pItem == pArg);
}

static void testComplex()
{
    // Empty list
    List* pList = List_create();
    CHECK(pList != NULL);
    CHECK(List_count(pList) == 0);

    // Add 
    int added = 41;
    CHECK(List_add(pList, &added) == 0);
    CHECK(List_count(pList) == 1);
    CHECK(List_curr(pList) == &added);

    // Insert
    int inserted = 42;
    CHECK(List_insert(pList, &inserted) == 0);
    CHECK(List_count(pList) == 2);
    CHECK(List_curr(pList) == &inserted);
    
    // Prepend
    int prepended = 43;
    CHECK(List_prepend(pList, &prepended) == 0);
    CHECK(List_count(pList) == 3);
    CHECK(List_curr(pList) == &prepended);
    
    // Append
    int appended = 44;
    CHECK(List_append(pList, &appended) == 0);
    CHECK(List_count(pList) == 4);
    CHECK(List_curr(pList) == &appended);
    
    // Next through it all (from before list)
    CHECK(List_first(pList) == &prepended);
    CHECK(List_prev(pList) == NULL);
    CHECK(List_next(pList) == &prepended);
    CHECK(List_next(pList) == &inserted);
    CHECK(List_next(pList) == &added);
    CHECK(List_next(pList) == &appended);
    CHECK(List_next(pList) == NULL);
    CHECK(List_next(pList) == NULL);

    // Prev through it all
    //   starting from past end
    CHECK(List_last(pList) == &appended);
    CHECK(List_next(pList) == NULL);
    CHECK(List_prev(pList) == &appended);
    CHECK(List_prev(pList) == &added);
    CHECK(List_prev(pList) == &inserted);
    CHECK(List_prev(pList) == &prepended);
    CHECK(List_prev(pList) == NULL);
    CHECK(List_prev(pList) == NULL);

    // Remove first
    CHECK(List_first(pList) == &prepended);
    CHECK(List_remove(pList) == &prepended);
    CHECK(List_curr(pList) == &inserted);

    // Trim last
    CHECK(List_trim(pList) == &appended);
    CHECK(List_curr(pList) == &added);

    // Free remaining 2 elements
    complexTestFreeCounter = 0;
    List_free(pList, complexTestFreeFn);
    CHECK(complexTestFreeCounter == 2);


    // Concat
    int one = 1;
    int two = 2;
    int three = 3;
    int four = 4;
    List* pList1 = List_create();
    List_add(pList1, &one);
    List_add(pList1, &two);
    List* pList2 = List_create();
    List_add(pList2, &three);
    List_add(pList2, &four);
    
    List_concat(pList1, pList2);
    CHECK(List_count(pList1) == 4);
    CHECK(List_first(pList1) == &one);
    CHECK(List_last(pList1) == &four);


    // Search
    List_first(pList1);
    CHECK(List_search(pList1, itemEquals, &two) == &two);
    CHECK(List_search(pList1, itemEquals, &two) == &two);
    CHECK(List_search(pList1, itemEquals, &one) == NULL);

    List_free(pList1, s_free_do_nothing);
}

static void s_test(){
    //shuffle the shared head pool and node pool
    size_t shuffleHead[LIST_MAX_NUM_HEADS];
    List * myHeads[LIST_MAX_NUM_HEADS];

    //put 4 kind of insert function in array;
    int (*insertFn[4])(List*, void*);
    insertFn[0] = List_add;
    insertFn[1] = List_insert;
    insertFn[2] = List_prepend;
    insertFn[3] = List_append;
    
    //init the queue and get one list
    List * pList = List_create();
    CHECK(pList != NULL);

    //insert in random order
    for(size_t i = 0; i < LIST_MAX_NUM_NODES; ++i){
        size_t fnI = rand() % 4;

        //insert should success;
        CHECK((*insertFn[fnI])(pList, &i) == 0);
    }

    //all insert should fail on stack empty
    CHECK(List_add(pList, shuffleHead) == -1);
    CHECK(List_insert(pList, shuffleHead) == -1);
    CHECK(List_append(pList, shuffleHead) == -1);
    CHECK(List_prepend(pList, shuffleHead) == -1);

    //set cur to head
    //and check the first entry
    CHECK(List_first(pList) != NULL);

    //free the list
    //will remove the randomly inserted node
    //effectly shuffled the shared node queue
    List_free(pList, s_free_do_nothing);

    //create maximum number of List
    for(size_t i = 0; i < LIST_MAX_NUM_HEADS; ++i){
        //create should success
        myHeads[i] = List_create(); 
        CHECK(myHeads[i] != NULL);
        //insert all the index to an array
        shuffleHead[i] = i;
    }

    //should no long be able to create
    CHECK(List_create() == NULL);

    //shuffle index array
    s_shuffle(shuffleHead, LIST_MAX_NUM_HEADS);
    
    //use the shuffled index to free all the created heads
    //effectively shuffled the head queue
    for(size_t i = 0; i < LIST_MAX_NUM_HEADS; ++i){
        size_t freeI = shuffleHead[i];
        List_free(myHeads[freeI], s_free_do_nothing);
    }

    int one = 1, 
        two = 2, 
        three = 3, 
        four = 4,
        five = 5;

    pList = List_create();
    List *pList2 = List_create();

    //check all non insert function when list is empty
    //concat two empty list
    List_concat(pList, pList2);
    CHECK(pList != NULL);
    CHECK(List_count(pList) == 0);
    CHECK(List_first(pList) == NULL);
    CHECK(List_last(pList) == NULL);
    CHECK(List_prev(pList) == NULL);
    CHECK(List_next(pList) == NULL);
    CHECK(List_curr(pList) == NULL);
    CHECK(List_remove(pList) == NULL);
    CHECK(List_trim(pList) == NULL);
    CHECK(List_search(pList, itemEquals, &one) == NULL);

    //when single item, it should be both head and tail
    CHECK(List_add(pList, &one) == 0);
    CHECK(List_last(pList) == &one);
    CHECK(List_next(pList) == NULL);
    CHECK(List_first(pList) == &one);
    //set cur to before head
    CHECK(List_prev(pList) == NULL);

    //test add on before head
    CHECK(List_add(pList, &two) == 0);
    CHECK(List_curr(pList) == &two);
    CHECK(List_first(pList) == &two);

    //set cur to before head
    CHECK(List_prev(pList) == NULL);

    //test insert on before head
    CHECK(List_insert(pList, &three) == 0);
    CHECK(List_curr(pList) == &three);
    CHECK(List_first(pList) == &three);

    //set cur to after tail
    CHECK(List_last(pList) == &one);
    CHECK(List_next(pList) == NULL);

    //test add on before head
    CHECK(List_add(pList, &four) == 0);
    CHECK(List_curr(pList) == &four);
    CHECK(List_last(pList) == &four);

    //set cur to before head
    CHECK(List_next(pList) == NULL);

    //test insert on before head
    CHECK(List_insert(pList, &five) == 0);
    CHECK(List_curr(pList) == &five);
    CHECK(List_last(pList) == &five);

    CHECK(List_count(pList) == 5);

    //searching at the end
    CHECK(List_search(pList, itemEquals, &five) == &five);
    CHECK(List_search(pList, itemEquals, &one) == NULL);
    CHECK(List_search(pList, itemEquals, &two) == NULL);
    CHECK(List_search(pList, itemEquals, &three) == NULL);
    CHECK(List_search(pList, itemEquals, &four) == NULL);
    CHECK(List_search(pList, itemEquals, &five) == NULL);

    //set cur to before head
    CHECK(List_first(pList) == &three);
    CHECK(List_prev(pList) == NULL);

    //search before head
    CHECK(List_search(pList, itemEquals, &three) == &three);

    pList2 = List_create();
    //concat empty list with non empty
    List_concat(pList2, pList);
    CHECK(List_count(pList2) == 5);
    CHECK(List_curr(pList2) == NULL);
    CHECK(List_prev(pList2) == NULL);
    CHECK(List_next(pList2) == &three);
    CHECK(List_first(pList2) == &three);
    CHECK(List_last(pList2) == &five);
    CHECK(List_next(pList2) == NULL);
    CHECK(List_next(pList2) == NULL);

    pList = List_create();
    //concat non empty list with empty
    List_concat(pList2, pList);
    CHECK(List_count(pList2) == 5);
    CHECK(List_curr(pList2) == NULL);
    CHECK(List_next(pList2) == NULL);
    CHECK(List_prev(pList2) == &five);
    CHECK(List_last(pList2) == &five);
    CHECK(List_first(pList2) == &three);
    CHECK(List_prev(pList2) == NULL);
    CHECK(List_prev(pList2) == NULL);

    //prepend and append
    pList = List_create();
    CHECK(List_prepend(pList, &five) == 0);
    CHECK(List_count(pList) == 1);
    CHECK(List_curr(pList) == &five);
    CHECK(List_last(pList) == &five);
    CHECK(List_first(pList) == &five);
    
    CHECK(List_append(pList, &three) == 0);
    CHECK(List_count(pList) == 2);
    CHECK(List_curr(pList) == &three);
    CHECK(List_last(pList) == &three);
    CHECK(List_first(pList) == &five);
    
}

int main(int argCount, char *args[]) 
{
    testComplex();

    s_test();


    // We got here?!? PASSED!
    printf("********************************\n");
    printf("           PASSED\n");
    printf("********************************\n");
    return 0;
}