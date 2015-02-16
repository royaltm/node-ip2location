#ifndef HAVE_NODEIP2LOCATION_H
#define HAVE_NODEIP2LOCATION_H

#include <nan.h>
#include <v8.h>
#include <node.h>
#include <string.h>

extern "C" {
#  include "ip2location.h"
}

#include "nodeip2ldictionary.h"

using namespace node;
using namespace v8;

#define LOCATION_DBMODE_FILE   "file"
#define LOCATION_DBMODE_MMAP   "mmap"
#define LOCATION_DBMODE_SHARED "shared"
#define LOCATION_DBMODE_CACHE  "cache"
#define LOCATION_DBMODE_CLOSED "closed"

#define LOCATION_ALL ((uint32_t)((1UL << IP2L_INDEX_MAX) - 1))

class Location: public ObjectWrap {
  public:
    IP2Location *iplocdb;
    const char *dbmode;
    static Persistent<FunctionTemplate> constructor;

    static void Init(Handle<Object> exports);
    Location(char *locdbpath, IP2LOCATION_ACCESS_TYPE mtype, char *shared);
    ~Location();
    void Close();
    static NAN_METHOD(New);
    static NAN_GETTER(GetDbMode);
    static NAN_GETTER(GetIsOpen);
    static NAN_METHOD(CloseDatabase);
    static NAN_METHOD(DeleteShared);
    static NAN_METHOD(CreateDictionary);
    static NAN_METHOD(GetDbInfo);
    static NAN_METHOD(Query);
  private:
    static void MakeDictionaryItem(IP2Location *loc,
                                  uint32_t rowoffset,
                                  uint32_t mask,
                                  Map<IP2LDictionary>::type &dict);
    static void MakeDictionary(Map<IP2LDictionary>::type &dict,
                              IP2Location *loc,
                              uint32_t mask);
    static void FreeDictionary(Map<IP2LDictionary>::type &dict);
    static Local<Object> CreateDictionaryResult(Map<IP2LDictionary>::type &dict, uint32_t mask);
    static Local<Array> CreateArrayResult(Map<IP2LDictionary>::type &dict);
};

#endif /* HAVE_NODEIP2LOCATION_H */
