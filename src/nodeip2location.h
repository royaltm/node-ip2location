#include <nan.h>
#include <v8.h>
#include <node.h>
#include <string.h>

extern "C" {
#  include "ip2location.h"
}

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
    static NAN_METHOD(GetDbInfo);
    static NAN_METHOD(Query);
};

