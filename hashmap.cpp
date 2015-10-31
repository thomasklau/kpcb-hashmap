/* -------------------------------------------------------------------------- *
 *                                HashMap                                     *
 * -------------------------------------------------------------------------- *
 * This project is a fixed-size hash map implementation that associates       *
 * string keys with data object references (void*). This implementation uses  *
 * open hashing (seperate chaining) which is probabilistically gives a better *
 * runtime than open addressing. For more information, please refer to the    *
 * README.                                                                    *
 *                                                                            *
 * Authors: Thomas Lau                                                        *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <string.h>

//TODO: change to void*
// templatize

namespace{
    int DEFAULT_CAPACITY = 199; //default fixed size of the map
}

HashMap::HashMap(){
    numberOfBuckets = DEFAULT_CAPACITY;
    numberOfElements = 0;
    buckets = (void**)new char[sizeof(void**) * numberOfBuckets];

    //init all of the buckets to NULL
    for(int x = 0; x < numberOfBuckets; x++)
        *getBucketAtIndex(x) = NULL;
}

HashMap::HashMap(int size){
    //make sure that we're given valid parameters
    assert(size > 0);

    //if we're given 0 for our size, use the DEFAULT_CAPACITY
    if(size == 0) 
        size = DEFAULT_CAPACITY;

    numberOfBuckets = size;
    numberOfElements = 0;
    buckets = (void**)new char[sizeof(void**)*numberOfBuckets];

    //init all of the buckets to NULL
    for(int x = 0; x < numberOfBuckets; x++)
        *getBucketAtIndex(x) = NULL;

}

/**
 * Function: cmap_dispose
 * Usage: cmap_dispose(m)
 * ----------------------
 * Disposes of the CMap. Calls the client's cleanup function on all values
 * and deallocates memory used for the CMap's storage, including the keys
 * that were copied. Operates in linear-time.
 */
HashMap::~HashMap()
{
    void** tempArray[numberOfElements];
    int elemCount = 0;

    //loop through all the buckets and store the pointers in an array so that we don't have to deal with the messiness of additional pointers while we cleanup and free
    for(int x = 0; x < numberOfBuckets; x++)
    {
        void** tempBucket = buckets + x; //our current xth bucket
        while(*tempBucket != NULL)
        {
            tempArray[elemCount] = (void**)*tempBucket;
            tempBucket = (void**)*tempBucket;
            elemCount++;
        }
    }

    for(int x = 0; x < elemCount; x++)
    {
        //cleanupFunction(getValueFromNode(tempArray[x]));
        free(tempArray[x]);
    }

    free(buckets);
}
/**
 * Function: cmap_put
 * Usage: cmap_put(m, "CS107", &val)
 * ---------------------------------
 * Associates the given key with a new value in the CMap. If there is an
 * existing value for the key, it is replaced with the new value. Before
 * being overwritten, the client's cleanup function is called on the old value.
 * addr is expected to be a valid pointer and the new value is copied
 * from the memory pointed to by addr. The key string is copied and stored
 * internally by the CMap. Note that keys are compared case-sensitively,
 * e.g. "binky" is not the same key as "BinKy". The capacity is enlarged
 * if necessary. An assert is raised on allocation failure. Operates in
 * constant-time (amortized).
 */
void HashMap::set(const char *key, const void *addr)
{   
    int foundKey = 0;
    void** nodePointer = findKey(key, 0,&foundKey);

    if(*nodePointer != NULL) //if the key already exists in the map, just clean up and copy over the value
    {
        //cleanupFunction(getValueFromNode(*nodePointer));
        memcpy(getValueFromNode(*nodePointer),&addr,sizeof(void*));
    }
    else //the key doesn't exist in the map so create a new node
    {
        void* node = createNode(key, addr);
        assert(node != NULL);
        *nodePointer = node;
        numberOfElements++;
    }
}
/**
 * Function: cmap_get
 * Usage: int val = *(int *)cmap_get(m, "CS107")
 * ---------------------------------------------
 * Searches the CMap for an entry with the given key and if found, returns
 * a pointer to its associated value.  If key is not found, then NULL is
 * returned as a sentinel.  Note this function returns a pointer into the
 * CMap's storage, so the pointer should be used with care. In particular,
 * the pointer returned by cmap_get can become invalid during a call that adds
 * or removes entries within the CMap.  Note that keys are compared
 * case-sensitively,  e.g. "binky" is not the same key as "BinKy".
 * Operates in constant-time.
 */
void* HashMap::get(const char *key)
{
    int foundKey = 0;
    void** nodePointer = findKey(key, 0,&foundKey);
    if(foundKey)
        return getValueFromNode(*nodePointer);
    return NULL;
}
/**
 * Function: cmap_remove
 * Usage: cmap_remove(m, "CS107")
 * ------------------------------
 * Searches the CMap for an entry with the given key and if found, removes that
 * key and its associated value.  If key is not found, no changes are made.
 * The client's cleanup function is called on the removed value and the copy of the
 * key string is deallocated. Note that keys are compared case-sensitively,
 * e.g. "binky" is not the same key as "BinKy". Operates in constant-time.
 */
void HashMap::remove(const char *key)
{
    int foundKey = 0;
    void** prevNodePointer = findKey(key, 1, &foundKey);

    if(foundKey)//if we find the key in the map
    {
        if(prevNodePointer == NULL) //the element is at index 0 of the bucket
        {
            int keyHash = hash(key, numberOfBuckets);
            void** bucketPointer = getBucketAtIndex(keyHash);
            void* toReplacePointer = *(void**)(*bucketPointer); //temp pointer so that we can set our bucket to the next index

            //cleanup and free the node
            //cleanupFunction(getValueFromNode(*bucketPointer));
            free(*bucketPointer);

            //set the node to the next node
            *bucketPointer = toReplacePointer;
        }else{ //the element is not at index 0
            void** toRemovePointer = (void**)*prevNodePointer; //pointer to the node that we need to remove
            *prevNodePointer = *(void**)(*toRemovePointer); //set the next element of the previous node to the next element of the to remove node

            //cleanupFunction(getValueFromNode(*toRemovePointer));
            free(*toRemovePointer);
        }
        numberOfElements--;
    }
}
/**
 * Function: cmap_count
 * Usage: int count = cmap_count(m)
 * --------------------------------
 * Returns the number of entries currently stored in the CMap. Operates in
 * constant-time.
 */
int HashMap::getSize()
{
    return numberOfElements;
}
/**
 * Function: getLoadFactor
 * Usage: int count = cmap_count(m)
 * --------------------------------
 * Returns the number of entries currently stored in the CMap. Operates in
 * constant-time.
 */
float HashMap::getLoadFactor()
{
    return (double)numberOfElements/numberOfBuckets;
}
/**
 * Functions: cmap_first, cmap_next
 * Usage: for (const char *key = cmap_first(m); key != NULL; key = cmap_next(m, key))
 * ----------------------------------------------------------------------------------
 * These functions provide iteration over the CMap keys. The client
 * starts an iteration with a call to cmap_first which returns the first
 * key of the CMap or NULL if the CMap is empty. The client loop calls
 * cmap_next passing the previous key and receives the next key in the iteration
 * or NULL if there are no more keys. Keys are iterated in arbitrary order.
 * The argument to cmap_next is expected to be a valid pointer to a key string
 * as returned by a previous call to cmap_first/cmap_next. The CMap supports
 * multiple iterations at same time without interference, however, the client
 * should not add/remove/rearrange CMap entries in the midst of iterating.
 */

const char* HashMap::cmap_first()
{
    //loop over all of the buckets and return the first node we find
    for(int x = 0; x < numberOfBuckets; x ++) //look in all the buckets
        if(*getBucketAtIndex(x) != NULL)
            return (char*)getKeyFromNode(*getBucketAtIndex(x));
    return NULL;
}

const char* HashMap::cmap_next(const char *prevkey)
{
    void** currentNodePointer = (void**)(prevkey-sizeof(void**)); //go backwards to get the pointer to the next node
    if((*currentNodePointer)==NULL) //if we're at the last element of the linked list
    {
        int bucketNumber = hash(prevkey, numberOfBuckets);
        if(bucketNumber >= numberOfBuckets-1) //if there are no more elements left to iterate over
            return NULL;

        for(int x = bucketNumber+1; x < numberOfBuckets; x ++) //continue searching in the buckets ahead of us
            if(*getBucketAtIndex(x) != NULL)
                return (char*)getKeyFromNode(*getBucketAtIndex(x));

        return NULL;
    }
    //else just get the next key in the linked list
    return (char*)getKeyFromNode(*currentNodePointer);
}

//returns a void** pointing to map's index-th bucket
void** HashMap::getBucketAtIndex(const int index)
{
    return (void**)(buckets+index);
}
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
int HashMap::hash(const char *s, int nbuckets)
{
   const unsigned long MULTIPLIER = 2630849305L; // magic number
   unsigned long hashcode = 0;
   for (int i = 0; s[i] != '\0'; i++)
      hashcode = hashcode * MULTIPLIER + s[i];
    return hashcode % nbuckets;
}

//empty clean up function to assign to the function pointer if we are passed NULL for our CleanupValueFn
void HashMap::emptyCleanUpFunction(void *addr) {}

//given a void* node, perform pointer arthemetic to return a pointer to the start of the key in the node
void* HashMap::getKeyFromNode(const void* node)
{
    return (char*)node + sizeof(void*);
}

//given a void* node, perform pointer arthemetic to return a pointer to the start of the value in the node
void* HashMap::getValueFromNode(const void* node)
{
    return (char*)node + sizeof(void**) + strlen((char*)getKeyFromNode(node)) + 1;
}

//layout of a node: NEXT_PTR/STRING/ELEMENT_ADDRESS
//returns the address to a newly created node with key and addr, pointing to NULL as the next node
void* HashMap::createNode(const char* key, const void* addr)
{
    void* node = new char[sizeof(void**)+(strlen(key)+1)+sizeof(void*)]; //malloc the size that we need for the void** pointer, the key + '\0', and the value

    void** nextNodeToPointTo = (void**)new char[sizeof(void**)]; //malloc our pointer to the next element in the array
    *nextNodeToPointTo = NULL;

    //copy over our values into the memory allocated to the node
    memcpy(node,nextNodeToPointTo,sizeof(void**));
    strcpy((char*)getKeyFromNode(node),key);
    memcpy(getValueFromNode(node),&addr,sizeof(void*));
    free(nextNodeToPointTo);

    return node;
}

//returns a void** pointer to the node with key in CMap if the key is found in the map, changes foundKey to 1 if we find a key, returns the previous node if returnPrevFind is set to 1
void** HashMap::findKey(const char *key, int returnPrevFind, int* foundKey)
{
    int keyHash = hash(key, numberOfBuckets);

    //keep track of two nodes at a time so that remove also works
    void** prevKeyBucket = NULL;
    void** keyBucket = getBucketAtIndex(keyHash);

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
        keyBucket = (void**)*keyBucket;
    }

    if(returnPrevFind)
    {
        return prevKeyBucket;
    }
    return keyBucket;
}


