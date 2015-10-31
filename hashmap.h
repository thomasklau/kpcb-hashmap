/* File: hashmap.h
 * ------------
 * Defines the interface for the CMap type.
 *
 * The CMap manages a collection of key-value pairs or "entries". The keys
 * are always of type char *, but the values can be of any desired type. The
 * main operations are to associate a value with a key and to retrieve
 * the value associated with a key.  In order to work for all types of values,
 * all CMap values must be passed and returned via (void *) pointers.
 * Given its extensive use of untyped pointers, the CMap is a bit tricky
 * to use correctly as a client. Be diligent!
 *
 */

#ifndef _cmap_h
#define _cmap_h

class HashMap{
public:
  HashMap(int size);
  void cmap_dispose();
  void cmap_put(const char *key, const void *addr);
  void *cmap_get(const char *key);
  void cmap_remove(const char *key);
  int cmap_count();
  const char *cmap_first();
  const char *cmap_next(const char *prevkey);

private:
  void** getBucketAtIndex(const int index);
  static int hash(const char *s, int nbuckets);
  static void emptyCleanUpFunction(void *addr);
  static void* getKeyFromNode(const void* node);
  static void* getValueFromNode(const void* node);
  void* createNode(const char* key, const void* addr);
  void** findKey(const char *key, int returnPrevFind, int* foundKey);


  //Local Variables for CMap
  int sizeOfElements; //the size of each value element
  int numberOfBuckets; //the number of buckets currently in the map
  int numberOfElements; //the number of elements (keys and values) current in the map
  void** buckets; //array to store the pointers to each linkedlist of buffers

};
 /**
  * Type: CleanupValueFn
  * --------------------
  * CleanupValueFn is the typename for a pointer to a client-supplied
  * cleanup function. The client passes a cleanup function to cmap_create
  * and the CMap will apply that function to a value that is being
  * removed/replaced/disposed. The cleanup function takes one void* pointer
  * that points to the value.
  *
  * The typedef allows the nickname "CleanupValueFn" to stand in for the
  * longer declaration.  CleanupValueFn can be used as the declared type
  * for a variable, parameter, struct field, and so on.
  */
typedef void (*CleanupValueFn)(void *addr);

#endif
