#include <v8.h>
#include <node.h>
#include <string.h>
#include "IP2Location.h"

using namespace node;
using namespace v8;

#define LOCATION_DBMODE_FILE "file"
#define LOCATION_DBMODE_SHARED "shared"
#define LOCATION_DBMODE_CACHE "cache"
#define LOCATION_DBMODE_CLOSED "closed"

class Location: public node::ObjectWrap {
  public:
    IP2Location *iplocdb;
    const char *dbmode;
    static Persistent<Function> constructor;
    static void Init(Handle<Object> exports) {
      Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
      tpl->SetClassName(String::NewSymbol("Location"));
      tpl->InstanceTemplate()->SetInternalFieldCount(1);
      tpl->InstanceTemplate()->SetAccessor(String::New("mode"), GetDbMode);
      tpl->InstanceTemplate()->SetAccessor(String::New("opened"), GetIsOpen);

      NODE_SET_PROTOTYPE_METHOD(tpl, "getRecord", GetRecord);
      NODE_SET_PROTOTYPE_METHOD(tpl, "close", CloseDatabase);
      NODE_SET_METHOD(tpl, "deleteShared", DeleteShared);

      constructor = Persistent<Function>::New(tpl->GetFunction());
      exports->Set(String::NewSymbol("Location"), constructor);
    }

    Location(char *locdbpath, enum IP2Location_mem_type mtype) {
      iplocdb = IP2Location_open(locdbpath);
      if (IP2Location_open_mem(iplocdb, mtype) == -1) {
        Close();
      }
    }

    ~Location() {
      Close();
    }

    void Close() {
      if(iplocdb != NULL){
        IP2Location_close(iplocdb);
        iplocdb = NULL;
        dbmode = LOCATION_DBMODE_CLOSED;
      }
    }

    static Handle<Value> New(const Arguments& args) {
      HandleScope scope;

      if (args.IsConstructCall()) {
        String::Utf8Value locdbpath(args[0]->ToString());
        enum IP2Location_mem_type mtype(IP2LOCATION_FILE_IO);
        const char *dbmode(LOCATION_DBMODE_FILE);
        if (args.Length() > 1) {
          String::Utf8Value stype(args[1]->ToString());
          if (strncmp(*stype, LOCATION_DBMODE_SHARED, sizeof(LOCATION_DBMODE_SHARED)) == 0) {
            mtype = IP2LOCATION_SHARED_MEMORY;
            dbmode = LOCATION_DBMODE_SHARED;
          } else if (strncmp(*stype, LOCATION_DBMODE_CACHE, sizeof(LOCATION_DBMODE_CACHE)) == 0) {
            mtype = IP2LOCATION_CACHE_MEMORY;
            dbmode = LOCATION_DBMODE_CACHE;
          }
        }
        Location* location = new Location(*locdbpath, mtype);
        if (!location->iplocdb) {
          ThrowException(Exception::Error(String::New("error in opening database")));
        }
        location->dbmode = dbmode;
        location->Wrap(args.This());
        return args.This();
      } else {
        const int argc = args.Length();
        Local<Value> argv[argc];
        for (int i = 0; i < argc; i++) {
          argv[i] = args[i];
        }
        return scope.Close(constructor->NewInstance(argc, argv));
      }
    }

    static Handle<Value> GetDbMode(Local<String> property, const AccessorInfo& info) {
      Location *location = ObjectWrap::Unwrap<Location>(info.This());
      return String::New(location->dbmode);
    }

    static Handle<Value> GetIsOpen(Local<String> property, const AccessorInfo& info) {
      Location *location = ObjectWrap::Unwrap<Location>(info.This());
      return location->iplocdb ? True() : False();
    }

    static Handle<Value> CloseDatabase(const Arguments& args){
      HandleScope scope;
      Location* location = ObjectWrap::Unwrap<Location>(args.This());
      location->Close();
      return scope.Close(Undefined());
    }

    static Handle<Value> DeleteShared(const Arguments& args) {
      HandleScope scope;
      IP2Location_DB_del_shm();
      return scope.Close(Undefined());
    }

    static Handle<Value> GetRecord(const Arguments& args)
    {
      HandleScope scope;
      String::Utf8Value query(args[0]->ToString());
      Location *location = ObjectWrap::Unwrap<Location>(args.This());
      if (!location->iplocdb) {
        ThrowException(Exception::Error(String::New("connection already closed")));
        return scope.Close(Undefined());
      }
      IP2LocationRecord *record = IP2Location_get_record(location->iplocdb, *query,
        COUNTRYSHORT | REGION | CITY | LATITUDE | LONGITUDE);
      Local<Object> result = Object::New();
      result->Set(
            String::NewSymbol("latitude"),
            Number::New(record->latitude)
          );
      result->Set(
            String::NewSymbol("longitude"),
            Number::New(record->longitude)
          );
      if(record->country_short != NULL){
        result->Set(
              String::NewSymbol("country_short"),
              String::New(record->country_short)
            );
      }
      if(record->region != NULL){
        result->Set(
              String::NewSymbol("region"),
              String::New(record->region)
            );
      }
      if(record->city != NULL){
        result->Set(
              String::NewSymbol("city"),
              String::New(record->city)
            );
      }
      IP2Location_free_record(record);
      return result;
    }
};

Persistent<Function> Location::constructor;

extern "C" {
  static void init(Handle<Object> exports)
  {
    Location::Init(exports);
  }
  NODE_MODULE(nodeip2location, init)
}
