/* -------------------------------------------------------------------------- *
 *                                MapTest                                     *
 * -------------------------------------------------------------------------- *
 * This program tests all of the major features of HashMap. For more          *
 * information about HashMap, please see the README.                          *
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
 
#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>

using namespace std;

static int numDeleteCalled = 0; //used to make sure that complexStructDelete is
                                //actually being called
/**
 * insert_test()
 * ----------------------------------------------------------------------------
 * Tests the HashMap's ability to insert large amounts of keys into the
 * HashMap.
 */
 void insert_test()
 {
    printf("Testing Insert...\n");
    HashMap map1(100, sizeof(int));
    for(int x = 0; x < 100000; x++)
    {
        char* key = (char*)to_string(x).c_str();
        int tempInt = 1;
        map1.set(key,&tempInt);

        assert(map1.getSize() == x+1);
    }
    assert(map1.getLoadFactor() == (float)(100000/100));

    struct bogusStruct{
        int integerOne;
        int integerTwo;
    };

    HashMap map2(100, sizeof(bogusStruct));
    for(int x = 0; x < 100000; x++)
    {
        bogusStruct temp;
        temp.integerOne = x;
        temp.integerTwo = x+1;

        char* key = (char*)to_string(x).c_str();
        map2.set(key, &temp);

        assert(map2.getSize() == x+1);
    }
    assert(map2.getLoadFactor() == (float)(100000/100));
 }
 /**
 * consistency_test()
 * ----------------------------------------------------------------------------
 * Tests the HashMap's ability to consistently reproduce the same value for a
 * given key.
 */
void consistency_test()
{
    printf("Testing Consistency...\n");
    HashMap map1(100, sizeof(int));
    for(int x = 0; x < 100000; x++)
    {
        char* key = (char*)to_string(x).c_str();
        int tempInt = rand() % 1000;
        map1.set(key,&tempInt);
    }
    vector<char*> keys;
    for (char *key = map1.firstNode(); key != NULL; key = map1.nextNode(key))
    {
        keys.push_back(key);
    }
    for(int x = 0; x < keys.size(); x++)
    {
        int firstValue = *(int*)map1.get(keys[x]);
        for(int y = 0; y < 10; y++)
            assert(*(int*)map1.get(keys[x]) == firstValue);
    }
    
}
 /**
 * delete_test()
 * ----------------------------------------------------------------------------
 * Tests the HashMap's ability to remove keys in the HashMap
 */
void delete_test()
{
    printf("Testing Delete...\n");
    HashMap map(1000, sizeof(int));
    for(int x = 0; x < 10000; x++)
    {
        char* key = (char*)to_string(x).c_str();
        int tempInt = 1;
        map.set(key,&tempInt);
    }
    vector<char*> keys;
    for (char *key = map.firstNode(); key != NULL; key = map.nextNode(key))
    {
        keys.push_back(key);
    }
    for(int x = 0; x < keys.size(); x++)
        map.remove(keys[x]);

    assert(map.getSize() == 0);
}
 /**
 * complexStruct
 * ----------------------------------------------------------------------------
 * Used to test HashMap's ability to call appropriate cleanup functions for
 * dynamically allocated variables inside storage elements.
 */
struct complexStruct{
    int* integer;
};
/**
 * complexStructDelete
 * ----------------------------------------------------------------------------
 * Used to test HashMap's ability to call appropriate cleanup functions for
 * dynamically allocated variables inside storage elements.
 */
void complexStructDelete(void* cStruct)
{
    free((*(complexStruct*)cStruct).integer);
    numDeleteCalled++;
}
 /**
 * complex_delete_test()
 * ----------------------------------------------------------------------------
 * Used to test HashMap's ability to call appropriate cleanup functions for
 * dynamically allocated variables inside storage elements.
 */
void complex_delete_test()
{
    printf("Testing Complex Delete...\n");
    HashMap map(1000, sizeof(complexStruct),complexStructDelete);
    for(int x = 0; x < 10000; x++)
    {
        char* key = (char*)to_string(x).c_str();
        complexStruct cs;
        int* tempInt = new int;
        cs.integer = tempInt;

        map.set(key,&cs);
    }
    vector<char*> keys;
    for (char *key = map.firstNode(); key != NULL; key = map.nextNode(key))
    {
        keys.push_back(key);
    }
    for(int x = 0; x < keys.size(); x++)
        map.remove(keys[x]);

    assert(map.getSize() == 0);
    assert(numDeleteCalled == 10000);
}
/**
 * update_test()
 * ----------------------------------------------------------------------------
 * Tests the HashMap's ability to consistently update/set the existing keys
 * that are in the HashMap. Requires kcpb.txt.
 */
void update_test()
{
    printf("Testing Update...\n");
    HashMap map(100,sizeof(int));
    typedef istreambuf_iterator<char> buf_iter;
    fstream file("kpcb.txt");
    if(!file)
        cout << "Please put kpcb.txt in the same folder as maptest!" << endl;

    for(buf_iter i(file), e; i != e; ++i){
        char c = *i;
        char* cStar = new char[2];
        cStar[0] = c;
        cStar[1] = '\0';

        int* intPointer = (int*)map.get(cStar);
        int intValue = 0;

        if(intPointer != NULL)
            intValue = *intPointer;
        intValue++;

        map.set(cStar,&intValue);
    }

    int totalCharacterCount = 0;
    for (char *key = map.firstNode(); key != NULL; key = map.nextNode(key))
    {
        totalCharacterCount += *(int*)map.get(key);
    }
    assert(totalCharacterCount == 5667);
}
int main(int argc, char *argv[])
{
    insert_test();
    consistency_test();
    update_test();
    delete_test();
    complex_delete_test();
    printf("All tests pass!\n");
    return 0;
}
