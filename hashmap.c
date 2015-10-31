/*
 * File: cmap.c
 * Author: Thomas Lau
 * ----------------------
 *
 */

#include "cmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <string.h>

// a suggested value to use when given capacity_hint is 0
#define DEFAULT_CAPACITY 199
#define LOAD_FACTOR 1.5

/* Type: struct CMapImplementation
 * -------------------------------
 * This definition completes the CMap type that was declared in
 * cmap.h. You fill in the struct with your chosen fields.
 */
struct CMapImplementation {
    //Function Pointers for Cleanup Function
    CleanupValueFn cleanupFunction;

    //Local Variables for CMap
    int sizeOfElements; //the size of each value element
    int numberOfBuckets; //the number of buckets currently in the map
    int numberOfElements; //the number of elements (keys and values) current in the map
    void** buckets; //array to store the pointers to each linkedlist of buffers
};

/* Function: hash
 * --------------
 * This function adapted from Eric Roberts' _The Art and Science of C_
 * It takes a string and uses it to derive a "hash code," which
 * is an integer in the range [0..nbuckets-1]. The hash code is computed
 * using a method called "linear congruence." A similar function using this
 * method is described on page 144 of Kernighan and Ritchie. The choice of
 * the value for the multiplier can have a significant effort on the
 * performance of the algorithm, but not on its correctness.
 * The computed hash value is stable, e.g. passing the same string and
 * nbuckets to function again will always return the same code.
 * The hash is case-sensitive, "ZELENSKI" and "Zelenski" are
 * not guaranteed to hash to same code.
 */
static int hash(const char *s, int nbuckets)
{
   const unsigned long MULTIPLIER = 2630849305L; // magic number
   unsigned long hashcode = 0;
   for (int i = 0; s[i] != '\0'; i++)
      hashcode = hashcode * MULTIPLIER + s[i];
   return hashcode % nbuckets;
}
//empty clean up function to assign to the function pointer if we are passed NULL for our CleanupValueFn
static void emptyCleanUpFunction(void *addr) {}

//given a void* node, perform pointer arthemetic to return a pointer to the start of the key in the node
static void* getKeyFromNode(const void* node)
{
    return (char*)node + sizeof(void*);
}

//given a void* node, perform pointer arthemetic to return a pointer to the start of the value in the node
static void* getValueFromNode(const void* node)
{
    return (char*)node + sizeof(void*) + strlen((char*)getKeyFromNode(node)) + 1;
}

//returns a void** pointing to map's index-th bucket
static void** getBucketAtIndex(const CMap* map, const int index)
{
    return (map->buckets)+index;
}

CMap *cmap_create(int valuesz, int capacity_hint, CleanupValueFn fn)
{
    //make sure that we're given valid parameters
    assert(valuesz > 0);
    assert(capacity_hint >= 0);

    //if we're given 0 for our capacity_hint, use the DEFAULT_CAPACITY
    if(capacity_hint == 0) 
        capacity_hint = DEFAULT_CAPACITY;

    CMap* map = malloc(sizeof(CMap));
    assert(map != NULL);

    //instansiate our variables
    map->sizeOfElements = valuesz;
    map->numberOfBuckets = capacity_hint;
    map->numberOfElements = 0;
    map->buckets = malloc(sizeof(void**)*map->numberOfBuckets);

    //init all of the buckets to NULL
    for(int x = 0; x < map->numberOfBuckets; x++)
        *getBucketAtIndex(map,x) = NULL;

    //instansiate the cleanup function
    if(fn == NULL) 
        map->cleanupFunction = emptyCleanUpFunction;
    else
        map->cleanupFunction = fn;

    return map;
}

void cmap_dispose(CMap *cm)
{
    void** tempArray[cm->numberOfElements];
    int elemCount = 0;

    //loop through all the buckets and store the pointers in an array so that we don't have to deal with the messiness of additional pointers while we cleanup and free
    for(int x = 0; x < cm->numberOfBuckets; x++)
    {
        void** tempBucket = cm->buckets + x; //our current xth bucket
        while(*tempBucket != NULL)
        {
            tempArray[elemCount] = *tempBucket;
            tempBucket = *tempBucket;
            elemCount++;
        }
    }

    for(int x = 0; x < elemCount; x++)
    {
        cm->cleanupFunction(getValueFromNode(tempArray[x]));
        free(tempArray[x]);
    }

    free(cm->buckets);
    free(cm);
}

int cmap_count(const CMap *cm)
{
    return cm->numberOfElements;
}
//returns the address to a newly created node with key and addr, pointing to NULL as the next node
static void* createNode(const CMap* cm, const char* key, const void* addr)
{
    void* node = malloc(sizeof(void**)+(strlen(key)+1)+cm->sizeOfElements); //malloc the size that we need for the void** pointer, the key + '\0', and the value

    void** nextNodeToPointTo = malloc(sizeof(void**)); //malloc our pointer to the next element in the array
    *nextNodeToPointTo = NULL;

    //copy over our values into the memory allocated to the node
    memcpy(node,nextNodeToPointTo,sizeof(void*));
    strcpy(getKeyFromNode(node),key);
    memcpy(getValueFromNode(node),addr,cm->sizeOfElements);
    free(nextNodeToPointTo);

    return node;
}

//returns a void** pointer to the node with key in CMap if the key is found in the map, changes foundKey to 1 if we find a key, returns the previous node if returnPrevFind is set to 1
static void** findKey(const CMap *cm, const char *key, int returnPrevFind, int* foundKey)
{
    int keyHash = hash(key, cm->numberOfBuckets);

    //keep track of two nodes at a time so that remove also works
    void** prevKeyBucket = NULL;
    void** keyBucket = getBucketAtIndex(cm,keyHash);

    //iterate through the linked list in the bucket until we reach the end
    while(*keyBucket != NULL)
    {
        if(strcmp((char*)getKeyFromNode(*keyBucket),key) == 0) //if we find a match
        {
            *foundKey = 1;
            break;
        }
        //increment both of our nodes
        prevKeyBucket = keyBucket;
        keyBucket = *keyBucket;
    }

    if(returnPrevFind)
    {
        return prevKeyBucket;
    }
    return keyBucket;
}

//checks the load capacity of the map and rehashes the map if needed
static void checkForRehash(CMap* map)
{
    double loadCapacity = (double)map->numberOfElements/map->numberOfBuckets;
    if(loadCapacity > LOAD_FACTOR) //if we've exceeded a load capacity of LOAD_FACTOR
    {   
        //set the map to have more buckets and store the old buckets temporarily
        int oldNumBuckets = map->numberOfBuckets;
        map->numberOfBuckets = map->numberOfBuckets * 3 + 1; //then increase the number of buckets

        void** newBuckets = malloc(sizeof(void**)*map->numberOfBuckets);
        void** oldBuckets = map->buckets;
        map->buckets = newBuckets;

        //initialize the new buckets to all point to NULL
        for(int x = 0; x < map->numberOfBuckets; x++)
        {
            *getBucketAtIndex(map,x) = NULL;
        }

        //rewire the nodes in the old buckets to the new buckets
        for(int x = 0; x < oldNumBuckets; x++) //go through every bucket
        {
            void** tempBucket = oldBuckets + x; //our current xth bucket
            while(*tempBucket != NULL)
            {
                int found = 0;
                void** nodePlacement = findKey(map, (char*)(*tempBucket) + sizeof(void*), 0, &found); //find the place to put the key in the new buckets

                void** nextNodeToPointTo = malloc(sizeof(void**)); //malloc our pointer to the next element in the array
                *nextNodeToPointTo = NULL;

                *nodePlacement = *tempBucket; //make nodePlacement now point to the linked list cell of tempBucket
                void* nextNode = *(void**)*tempBucket; //make tempBucket increment to the next node
                memcpy(*nodePlacement,nextNodeToPointTo,sizeof(void*)); //copy over the NULL pointer
                free(nextNodeToPointTo); //then free it
                tempBucket = &nextNode;
            }
        }
        free(oldBuckets);
    }
}

void cmap_put(CMap *cm, const char *key, const void *addr)
{   
    int foundKey = 0;
    void** nodePointer = findKey(cm, key, 0,&foundKey);

    if(*nodePointer != NULL) //if the key already exists in the map, just clean up and copy over the value
    {
        cm->cleanupFunction(getValueFromNode(*nodePointer));
        memcpy(getValueFromNode(*nodePointer),addr,cm->sizeOfElements);
    }
    else //the key doesn't exist in the map so create a new node
    {
        void* node = createNode(cm, key, addr);
        assert(node != NULL);
        *nodePointer = node;
        cm->numberOfElements++;
    }

    checkForRehash(cm);
}

void *cmap_get(const CMap *cm, const char *key)
{
    int foundKey = 0;
    void** nodePointer = findKey(cm, key, 0,&foundKey);
    if(foundKey)
        return getValueFromNode(*nodePointer);
    return NULL;
}

void cmap_remove(CMap *cm, const char *key)
{
    int foundKey = 0;
    void** prevNodePointer = findKey(cm, key, 1, &foundKey);

    if(foundKey)//if we find the key in the map
    {
        if(prevNodePointer == NULL) //the element is at index 0 of the bucket
        {
            int keyHash = hash(key, cm->numberOfBuckets);
            void** bucketPointer = getBucketAtIndex(cm,keyHash);
            void* toReplacePointer = *(void**)(*bucketPointer); //temp pointer so that we can set our bucket to the next index

            //cleanup and free the node
            cm->cleanupFunction(getValueFromNode(*bucketPointer));
            free(*bucketPointer);

            //set the node to the next node
            *bucketPointer = toReplacePointer;
        }else{ //the element is not at index 0
            void** toRemovePointer = *prevNodePointer; //pointer to the node that we need to remove
            *prevNodePointer = *(void**)(*toRemovePointer); //set the next element of the previous node to the next element of the to remove node

            cm->cleanupFunction(getValueFromNode(*toRemovePointer));
            free(*toRemovePointer);
        }
        cm->numberOfElements--;
    }
}

const char *cmap_first(const CMap *cm)
{
    //loop over all of the buckets and return the first node we find
    for(int x = 0; x < cm->numberOfBuckets; x ++) //look in all the buckets
        if(*getBucketAtIndex(cm,x) != NULL)
            return (char*)getKeyFromNode(*getBucketAtIndex(cm,x));
    return NULL;
}

const char *cmap_next(const CMap *cm, const char *prevkey)
{
    void** currentNodePointer = (void**)(prevkey-sizeof(void**)); //go backwards to get the pointer to the next node
    if((*currentNodePointer)==NULL) //if we're at the last element of the linked list
    {
        int bucketNumber = hash(prevkey, cm->numberOfBuckets);
        if(bucketNumber >= cm->numberOfBuckets-1) //if there are no more elements left to iterate over
            return NULL;

        for(int x = bucketNumber+1; x < cm->numberOfBuckets; x ++) //continue searching in the buckets ahead of us
            if(*getBucketAtIndex(cm,x) != NULL)
                return (char*)getKeyFromNode(*getBucketAtIndex(cm,x));

        return NULL;
    }
    //else just get the next key in the linked list
    return (char*)getKeyFromNode(*currentNodePointer);
}
