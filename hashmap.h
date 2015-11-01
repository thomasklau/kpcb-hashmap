/* -------------------------------------------------------------------------- *
 *                                HashMap                                     *
 * -------------------------------------------------------------------------- *
 * This project is a fixed-size hash map implementation that associates       *
 * string keys with data object references (void*). This implementation uses  *
 * open hashing (seperate chaining) which is probabilistically gives a better *
 * runtime than open addressing. To use HashMap, simply include "hashmap.h"   *
 * in the top of your project. For more information, please refer to the      *
 * README.                                                                    *
 *                                                                            *
 * Author: Thomas Lau                                                         *
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

#ifndef _hashmap_h
#define _hashmap_h

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <string.h>

namespace{ //local namespace variables
    int DEFAULT_SIZE = 100;
}

class HashMap{
public:
    ///////////////////////////////////
    // CONSTRUCTORS AND DESTRUCTORS
    ///////////////////////////////////
    HashMap();
    HashMap(int size);
    ~HashMap();

    ///////////////////////////////////
    // DATA STRUCTURE ACCESS METHODS
    ///////////////////////////////////
    bool set(const char *key, const void *addr);
    void* get(const char *key);
    void* remove(const char *key);

    ///////////////////////////////////
    // DATA STRUCTURE PROPERTIES
    ///////////////////////////////////
    int getSize();
    float getLoadFactor();

    ///////////////////////////////////
    // ITERATOR METHODS
    ///////////////////////////////////
    const char *firstNode();
    const char *nextNode(const char *prevkey);

private:
    ///////////////////////////////////
    // PRIVATE HELPER METHODS
    ///////////////////////////////////
    void** getBucketAtIndex(const int index);
    static int hash(const char *s, int nbuckets);
    static void emptyCleanUpFunction(void *addr);
    static void* getKeyFromNode(const void* node);
    static void* getValueFromNode(const void* node);
    void* createNode(const char* key, const void* addr);
    void** findKey(const char *key, int returnPrevFind, int* foundKey);

    ///////////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////////
    int sizeOfElements; //the size of each value element
    int numberOfBuckets; //the number of buckets currently in the HashMap
    int numberOfElements; //the number of elements current in the HashMap
    void** buckets; //array to store the pointers to each LinkedList of buffers

};
///////////////////////////////////
// CONSTRUCTORS AND DESTRUCTORS
///////////////////////////////////
/**
 * HashMap()
 * ----------------------------------------------------------------------------
 * Creates a fixed-size HashMap of DEFAULT_SIZE. All buckets are initialized
 * to point to NULL.
 * ----------------------------------------------------------------------------
 * Runtime: O(k); k = DEFAULT_SIZE
 */
HashMap::HashMap(){
    numberOfBuckets = DEFAULT_SIZE;
    numberOfElements = 0;
    buckets = (void**)new char[sizeof(void**) * numberOfBuckets];

    //init all of the buckets to NULL
    for(int x = 0; x < numberOfBuckets; x++)
        *getBucketAtIndex(x) = NULL;
}
/**
 * HashMap()
 * ----------------------------------------------------------------------------
 * Creates a fixed-size HashMap of size. All buckets are initialized to point
 * to NULL.
 * ----------------------------------------------------------------------------
 * Runtime: O(k); k = size of HashMap
 */
HashMap::HashMap(int size){
    //make sure that we're given valid parameters
    assert(size > 0);

    //if we're given 0 for our size, use the DEFAULT_SIZE
    if(size == 0) 
        size = DEFAULT_SIZE;

    numberOfBuckets = size;
    numberOfElements = 0;
    buckets = (void**)new char[sizeof(void**)*numberOfBuckets];

    //init all of the buckets to NULL
    for(int x = 0; x < numberOfBuckets; x++)
        *getBucketAtIndex(x) = NULL;

}
/**
 * ~HashMap()
 * ----------------------------------------------------------------------------
 * Destructor for HashMap -- called when HashMap is explicitly deleted or goes
 * out of scope. The destructor uses the previously specified class T to delete
 * the T* pointers in each node of the HashMap to avoid memory leaks.
 * ----------------------------------------------------------------------------
 * Runtime: O(k); k = number of nodes (keys and values)
 */
HashMap::~HashMap()
{
    void** tempArray[numberOfElements];
    int elemCount = 0;

    // loop through all the buckets and store the pointers in an array so that
    // we don't have to deal with freeing in place
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
        free(tempArray[x]);
    }

    free(buckets);
}
///////////////////////////////////
// DATA STRUCTURE ACCESS METHODS
///////////////////////////////////
/**
 * set(char* key, void* addr)
 * ----------------------------------------------------------------------------
 * Associates a char* key to a void* pointer to an outside data element. If the
 * key already exists in the HashMap, it is replaced with the new pointer value
 * provided.
 * ----------------------------------------------------------------------------
 * Runtime: O(1) (amortized)
 */
bool HashMap::set(const char* key, const void* addr)
{   
    int foundKey = 0;
    void** nodePointer = findKey(key, 0,&foundKey);

    if(*nodePointer != NULL) //if the key already exists in the map, copy over
    {
        memcpy(getValueFromNode(*nodePointer),&addr,sizeof(void*));
    }
    else //the key doesn't exist in the map so we create a new node
    {
        void* node = createNode(key, addr);
        if(node == NULL) //on allocation failure, return false
            return false;
        *nodePointer = node;
        numberOfElements++;
    }
    return true;
}
/**
 * get(const char* key)
 * ----------------------------------------------------------------------------
 * Searches the HashMap for the given key and if found, returns a pointer to
 * the appropriate value. If the key is not found, NULL is returned. To look
 * for the correct key value, get() uses the hash() function that is specified
 * by the HashMap.
 * ----------------------------------------------------------------------------
 * Runtime: O(1) (amortized)
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
 * remove(const char* key)
 * ----------------------------------------------------------------------------
 * Searches the HashMap for the given key and if found, removes the associated
 * key and value from the HashMap; returning the value associated with the
 * provided key. If the key is not found in the HashMap, NULL is returned.
 * ----------------------------------------------------------------------------
 * Runtime: O(1) (amortized)
 */
void* HashMap::remove(const char *key)
{
    int foundKey = 0;
    void** prevNodePointer = findKey(key, 1, &foundKey);
    void* value = NULL;
    if(foundKey)//if we find the key in the map
    {
        if(prevNodePointer == NULL) //the element is at index 0 of the bucket
        {
            int keyHash = hash(key, numberOfBuckets);
            void** bucketPointer = getBucketAtIndex(keyHash);
            value = getValueFromNode(*bucketPointer);
            void* toReplacePointer = *(void**)(*bucketPointer); //temp pointer so that we can set our bucket to the next index

            free(*bucketPointer);

            //set the node to the next node
            *bucketPointer = toReplacePointer;
        }else{ //the element is not at index 0
            void** toRemovePointer = (void**)*prevNodePointer; //pointer to the node that we need to remove
            value = getValueFromNode(*toRemovePointer);
            *prevNodePointer = *(void**)(*toRemovePointer); //set the next element of the previous node to the next element of the to remove node

            //cleanupFunction(getValueFromNode(*toRemovePointer));
            free(*toRemovePointer);
        }
        numberOfElements--;
    }
    return value;
}
///////////////////////////////////
// DATA STRUCTURE PROPERTIES
///////////////////////////////////
/**
 * getSize()
 * ----------------------------------------------------------------------------
 * Returns the number of elements currently in the HashMap.
 * ----------------------------------------------------------------------------
 * Runtime: O(1)
 */
int HashMap::getSize()
{
    return numberOfElements;
}
/**
 * getLoadFactor()
 * ----------------------------------------------------------------------------
 * Returns the load factor of the HashMap. The load factor is calculate via
 * (#items in map/#size of hash map).
 */
float HashMap::getLoadFactor()
{
    return (double)numberOfElements/numberOfBuckets;
}
///////////////////////////////////
// ITERATOR METHODS
///////////////////////////////////
/**
 * firstNode(), nextNode(const char* prevKey)
 * ----------------------------------------------------------------------------
 * These functions allow iteration over the nodes (key/value pairs) in the
 * HashMap. firstNode() returns the first key found in the map. nextNode(prev)
 * returns the next key found in the map after the specified key, prev.
 */
const char* HashMap::firstNode()
{
    //loop over all of the buckets and return the first node we find
    for(int x = 0; x < numberOfBuckets; x ++) //look in all the buckets
        if(*getBucketAtIndex(x) != NULL)
            return (char*)getKeyFromNode(*getBucketAtIndex(x));
    return NULL;
}
const char* HashMap::nextNode(const char* prevKey)
{
    void** currentNodePointer = (void**)(prevKey-sizeof(void**)); //go backwards to get the pointer to the next node
    if((*currentNodePointer)==NULL) //if we're at the last element of the linked list
    {
        int bucketNumber = hash(prevKey, numberOfBuckets);
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
///////////////////////////////////
// PRIVATE HELPER METHODS
///////////////////////////////////
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
// given a void* node, perform pointer arthemetic to return a pointer to the
// start of the key in the node
void* HashMap::getKeyFromNode(const void* node)
{
    return (char*)node + sizeof(void*);
}
// given a void* node, perform pointer arthemetic to return a pointer to the
// start of the value in the node
void* HashMap::getValueFromNode(const void* node)
{
    return (char*)node + sizeof(void**) + strlen((char*)getKeyFromNode(node)) + 1;
}
// layout of a node: NEXT_PTR/STRING/ELEMENT_ADDRESS
// returns the address to a newly created node with key and addr, pointing to
// NULL as the next node
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
// returns a void** pointer to the node with key in CMap if the key is found in
// the map, changes foundKey to 1 if we find a key, returns the previous node
// if returnPrevFind is set to 1
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

#endif
