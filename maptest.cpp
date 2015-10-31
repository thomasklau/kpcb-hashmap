/* File: maptest.c
* ----------------
* A small program to exercise some basic functionality of the CMap. You should
* supplement with additional tests of your own.  You may change/extend/replace
* this test program in any way you see fit.
*/

#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <assert.h>

using namespace std;

/* Function: verify_int
* ---------------------
* Used to compare a given result with what was expected and report on whether
* passed/failed.
*/
static void verify_int(int expected, int found, string msg)
{
    printf("%s expect: %d found: %d. %s\n", msg.c_str(), expected, found,
        (expected == found) ? "Seems ok." : "##### PROBLEM HERE #####");
}

static void verify_ptr(void *expected, void *found, string msg)
{
    printf("%s expect: %p found: %p. %s\n", msg.c_str(), expected, found,
        (expected == found) ? "Seems ok." : "##### PROBLEM HERE #####");
}

static void verify_int_ptr(int expected, int *found, string msg)
{
    if (found == NULL)
        printf("%s found: %p %s\n", msg.c_str(), found, "##### PROBLEM HERE #####");
    else
        verify_int(expected, *found, msg);
}

void simple_test()
{
    char *words[] = {"apple", "pear", "banana", "cherry", "kiwi", "melon", "grape", "plum"};
    char *extra = "strawberry";
    int len, nwords = sizeof(words)/sizeof(words[0]);
    HashMap<int> cm(100);

    printf("\n----------------- Testing simple ops ------------------ \n");
    printf("Created empty CMap.\n");
    verify_int(0, cm.getSize(), "cmap_count");
    verify_ptr(NULL, cm.get("nonexistent"), "cmap_get(\"nonexistent\")");

    printf("\nAdding %d keys to CMap.\n", nwords);
    for (int i = 0; i < nwords; i++) {
        len = strlen(words[i]);
        cm.set(words[i], &len); // associate word w/ its strlen
    }
    verify_int(nwords, cm.getSize(), "cmap_count");
    verify_int_ptr(strlen(words[0]), (int*)cm.get(words[0]), "cmap_get(\"apple\")");

    printf("\nAdd one more key to CMap.\n");
    len = strlen(extra);
    cm.set(extra, &len);
    verify_int(nwords+1, cm.getSize(), "cmap_count");
    verify_int_ptr(strlen(extra), (int*)cm.get(extra), "cmap_get(\"strawberry\")");

    printf("\nReplace existing key in CMap.\n");
    len = 2*strlen(extra);
    cm.set(extra, &len);
    verify_int(nwords+1, cm.getSize(), "cmap_count");
    verify_int_ptr(len, (int*)cm.get(extra), "cmap_get(\"strawberry\")");

    cm.remove(words[0]);
    printf("\nRemove key from CMap.\n");
    verify_int(nwords, cm.getSize(), "cmap_count");
    verify_ptr(NULL, cm.get(words[0]), "cmap_get(\"apple\")");

    printf("\nUse iterator to count keys.\n");
    int nkeys = 0;
    for (const char *key = cm.cmap_first(); key != NULL; key = cm.cmap_next(key))
    {
        printf("%s\n",key);
        nkeys++;
    }
    verify_int(cm.getSize(), nkeys, "Number of keys");
}


/* Function: frequency_test
* -------------------------
* Runs a test of the CMap to count letter frequencies from a file.
* Each key is single-char string, value is count (int).
* Reads file char-by-char, updates map, prints total when done.
*/
/*static void frequency_test()
{
    printf("\n----------------- Testing frequency ------------------ \n");
    CMap *counts = cmap_create(sizeof(int), 10, NULL);

    int val, zero = 0;
    char buf[2];
    buf[1] = '\0';

   // initialize map to have entries for all lowercase letters, count = 0
    for (char ch = 'a'; ch <= 'z'; ch++) {
        buf[0] = ch;
        cmap_put(counts, buf, &zero);
    }

    FILE *fp = fopen("cmap.h", "r"); // open CMap header file
    assert(fp != NULL);
    while ((val = getc(fp)) != EOF) {
        if (isalpha(val)) { // only count letters
            buf[0] = tolower(val);
            (*(int *)cmap_get(counts, buf))++;
        }
    }

    for (char ch = 'a'; ch <= 'z'; ch*=5) {
        buf[0] = ch;
        cmap_remove(counts, buf);
        cmap_put(counts,buf,&zero);

    }

    fclose(fp);

    int total = 0;
    for (const char *key = cmap_first(counts); key != NULL; key = cmap_next(counts, key))
    {
        total += *(int *)cmap_get(counts, key);
    }
    printf("Total of all frequencies = %d\n", total);
    // correct count should agree with shell command
    // tr -c -d "[:alpha:]" < cmap.h | wc -c
    cmap_dispose(counts);
}*/


int main(int argc, char *argv[])
{
    simple_test();
    //frequency_test();
    return 0;
}

