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
      Local<ObjectTemplate> i_t = tpl->InstanceTemplate();
      i_t->SetInternalFieldCount(1);
      i_t->SetAccessor(String::NewSymbol("mode"), GetDbMode);
      i_t->SetAccessor(String::NewSymbol("opened"), GetIsOpen);

      tpl->Set(String::NewSymbol("COUNTRYSHORT"), Int32::New(COUNTRYSHORT), ReadOnly);
      tpl->Set(String::NewSymbol("COUNTRYLONG"), Int32::New(COUNTRYLONG), ReadOnly);
      tpl->Set(String::NewSymbol("REGION"), Int32::New(REGION), ReadOnly);
      tpl->Set(String::NewSymbol("CITY"), Int32::New(CITY), ReadOnly);
      tpl->Set(String::NewSymbol("ISP"), Int32::New(ISP), ReadOnly);
      tpl->Set(String::NewSymbol("LATITUDE"), Int32::New(LATITUDE), ReadOnly);
      tpl->Set(String::NewSymbol("LONGITUDE"), Int32::New(LONGITUDE), ReadOnly);
      tpl->Set(String::NewSymbol("DOMAIN"), Int32::New(DOMAIN), ReadOnly);
      tpl->Set(String::NewSymbol("ZIPCODE"), Int32::New(ZIPCODE), ReadOnly);
      tpl->Set(String::NewSymbol("TIMEZONE"), Int32::New(TIMEZONE), ReadOnly);
      tpl->Set(String::NewSymbol("NETSPEED"), Int32::New(NETSPEED), ReadOnly);
      tpl->Set(String::NewSymbol("IDDCODE"), Int32::New(IDDCODE), ReadOnly);
      tpl->Set(String::NewSymbol("AREACODE"), Int32::New(AREACODE), ReadOnly);
      tpl->Set(String::NewSymbol("WEATHERSTATIONCODE"), Int32::New(WEATHERSTATIONCODE), ReadOnly);
      tpl->Set(String::NewSymbol("WEATHERSTATIONNAME"), Int32::New(WEATHERSTATIONNAME), ReadOnly);
      tpl->Set(String::NewSymbol("MCC"), Int32::New(MCC), ReadOnly);
      tpl->Set(String::NewSymbol("MNC"), Int32::New(MNC), ReadOnly);
      tpl->Set(String::NewSymbol("MOBILEBRAND"), Int32::New(MOBILEBRAND), ReadOnly);
      tpl->Set(String::NewSymbol("ELEVATION"), Int32::New(ELEVATION), ReadOnly);
      tpl->Set(String::NewSymbol("USAGETYPE"), Int32::New(USAGETYPE), ReadOnly);
      tpl->Set(String::NewSymbol("ALL"), Int32::New(ALL), ReadOnly);

      NODE_SET_PROTOTYPE_METHOD(tpl, "query", Query);
      NODE_SET_PROTOTYPE_METHOD(tpl, "close", CloseDatabase);
      NODE_SET_PROTOTYPE_METHOD(tpl, "info", GetDbInfo);
      NODE_SET_PROTOTYPE_METHOD(tpl, "deleteShared", DeleteShared);

      constructor = Persistent<Function>::New(tpl->GetFunction());
      exports->Set(String::NewSymbol("Location"), constructor);
    }

    Location(char *locdbpath, enum IP2Location_mem_type mtype, char *shared) {
      iplocdb = IP2Location_open(locdbpath, mtype, shared);
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
        Location* location;
        enum IP2Location_mem_type mtype(IP2LOCATION_FILE_IO);
        const char *dbmode(LOCATION_DBMODE_FILE);
        if (args.Length() > 1) {
          char *shared = NULL;
          String::Utf8Value stype(args[1]->ToString());
          if (strncmp(*stype, LOCATION_DBMODE_SHARED, sizeof(LOCATION_DBMODE_SHARED)) == 0) {
            mtype = IP2LOCATION_SHARED_MEMORY;
            dbmode = LOCATION_DBMODE_SHARED;
          } else if (strncmp(*stype, LOCATION_DBMODE_CACHE, sizeof(LOCATION_DBMODE_CACHE)) == 0) {
            mtype = IP2LOCATION_CACHE_MEMORY;
            dbmode = LOCATION_DBMODE_CACHE;
          } else if (strncmp(*stype, "/", 1) == 0){
            mtype = IP2LOCATION_SHARED_MEMORY;
            dbmode = LOCATION_DBMODE_SHARED;
            shared = *stype;
          }
          location = new Location(*locdbpath, mtype, shared);
        } else {
          location = new Location(*locdbpath, mtype, NULL);
        }
        if (!location->iplocdb) {
          ThrowException(Exception::Error(String::New("could not open IP2LOCATION database")));
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
      Location* location = ObjectWrap::Unwrap<Location>(args.This());
      IP2Location_delete_shm(location->iplocdb);
      return scope.Close(Undefined());
    }

    static Handle<Value> GetDbInfo(const Arguments& args) {
      HandleScope scope;
      Location* location = ObjectWrap::Unwrap<Location>(args.This());
      IP2Location *iplocdb = location->iplocdb;
      Local<Object> info = Object::New();
      if (iplocdb) {
        info->Set(String::NewSymbol("databasetype"), Integer::New(iplocdb->databasetype));
        info->Set(String::NewSymbol("databasecolumn"), Integer::New(iplocdb->databasecolumn));
        info->Set(String::NewSymbol("databaseyear"), Integer::New(iplocdb->databaseyear));
        info->Set(String::NewSymbol("databaseyear"), Integer::New(iplocdb->databaseyear));
        info->Set(String::NewSymbol("databasemonth"), Integer::New(iplocdb->databasemonth));
        info->Set(String::NewSymbol("databaseday"), Integer::New(iplocdb->databaseday));
        info->Set(String::NewSymbol("databasecount"), Integer::New(iplocdb->databasecount));
        info->Set(String::NewSymbol("databaseaddr"), Integer::New(iplocdb->databaseaddr));
        info->Set(String::NewSymbol("ipversion"), Integer::New(iplocdb->ipversion));
        if (iplocdb->shm_node != NULL) {
          info->Set(String::NewSymbol("shared"), String::New(iplocdb->shm_node->name));
        }
        return scope.Close(info);
      } else {
        return scope.Close(Null());
      }
    }

    static Handle<Value> Query(const Arguments& args) {
      HandleScope scope;
      uint32_t mode(ALL);
      String::Utf8Value ip(args[0]->ToString());
      if (args.Length() > 1) {
        mode = args[1]->Uint32Value();
      }
      Location *location = ObjectWrap::Unwrap<Location>(args.This());
      if (!location->iplocdb) {
        ThrowException(Exception::Error(String::New("IP2LOCATION database closed")));
        return scope.Close(Undefined());
      }
      IP2LocationRecord *record = IP2Location_get_mode(location->iplocdb, *ip, mode);
      Local<Object> result = Object::New();
      if (record != NULL) {
        if (record->country_short != NULL) {
          result->Set(String::NewSymbol("country_short"), String::New(record->country_short));
        }
        if (record->country_long != NULL) {
          result->Set(String::NewSymbol("country_long"), String::New(record->country_long));
        }
        if (record->region != NULL) {
          result->Set(String::NewSymbol("region"), String::New(record->region));
        }
        if (record->city != NULL) {
          result->Set(String::NewSymbol("city"), String::New(record->city));
        }
        if (record->isp != NULL) {
          result->Set(String::NewSymbol("isp"), String::New(record->isp));
        }
        if (mode & LATITUDE) {
          result->Set(String::NewSymbol("latitude"), Number::New(record->latitude));
        }
        if (mode & LONGITUDE) {
          result->Set(String::NewSymbol("longitude"), Number::New(record->longitude));
        }
        if (record->domain != NULL) {
          result->Set(String::NewSymbol("domain"), String::New(record->domain));
        }
        if (record->zipcode != NULL) {
          result->Set(String::NewSymbol("zipcode"), String::New(record->zipcode));
        }
        if (record->timezone != NULL) {
          result->Set(String::NewSymbol("timezone"), String::New(record->timezone));
        }
        if (record->netspeed != NULL) {
          result->Set(String::NewSymbol("netspeed"), String::New(record->netspeed));
        }
        if (record->iddcode != NULL) {
          result->Set(String::NewSymbol("iddcode"), String::New(record->iddcode));
        }
        if (record->areacode != NULL) {
          result->Set(String::NewSymbol("areacode"), String::New(record->areacode));
        }
        if (record->weatherstationcode != NULL) {
          result->Set(String::NewSymbol("weatherstationcode"), String::New(record->weatherstationcode));
        }
        if (record->weatherstationname != NULL) {
          result->Set(String::NewSymbol("weatherstationname"), String::New(record->weatherstationname));
        }
        if (record->mcc != NULL) {
          result->Set(String::NewSymbol("mcc"), String::New(record->mcc));
        }
        if (record->mnc != NULL) {
          result->Set(String::NewSymbol("mnc"), String::New(record->mnc));
        }
        if (record->mobilebrand != NULL) {
          result->Set(String::NewSymbol("mobilebrand"), String::New(record->mobilebrand));
        }
        if (mode & ELEVATION) {
          result->Set(String::NewSymbol("elevation"), Number::New(record->elevation));
        }
        if (record->usagetype != NULL) {
          result->Set(String::NewSymbol("usagetype"), String::New(record->usagetype));
        }
        IP2Location_free_record(record);
      }
      return scope.Close(result);
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
