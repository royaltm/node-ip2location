#include <nan.h>
#include <v8.h>
#include <node.h>
#include <string.h>

extern "C" {
#  include "ip2location.h"
}

using namespace node;
using namespace v8;

#define LOCATION_DBMODE_FILE "file"
#define LOCATION_DBMODE_MMAP "mmap"
#define LOCATION_DBMODE_SHARED "shared"
#define LOCATION_DBMODE_CACHE "cache"
#define LOCATION_DBMODE_CLOSED "closed"

class Location: public ObjectWrap {
  public:
    IP2Location *iplocdb;
    const char *dbmode;
    static Persistent<FunctionTemplate> constructor;

    static void Init(Handle<Object> exports)
    {
      Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
      NanAssignPersistent( constructor, tpl );
      tpl->SetClassName( NanNew<String>("Location") );

      Local<ObjectTemplate> i_t = tpl->InstanceTemplate();
      i_t->SetInternalFieldCount(1);
      i_t->SetAccessor( NanNew<String>("mode"), GetDbMode );
      i_t->SetAccessor( NanNew<String>("opened"), GetIsOpen );

      tpl->Set( NanNew<String>("COUNTRYSHORT"), NanNew<Int32>(COUNTRYSHORT),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("COUNTRYLONG"), NanNew<Int32>(COUNTRYLONG),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("REGION"), NanNew<Int32>(REGION),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("CITY"), NanNew<Int32>(CITY),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("ISP"), NanNew<Int32>(ISP),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("LATITUDE"), NanNew<Int32>(LATITUDE),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("LONGITUDE"), NanNew<Int32>(LONGITUDE),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("DOMAIN"), NanNew<Int32>(DOMAIN),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("ZIPCODE"), NanNew<Int32>(ZIPCODE),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("TIMEZONE"), NanNew<Int32>(TIMEZONE),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("NETSPEED"), NanNew<Int32>(NETSPEED),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("IDDCODE"), NanNew<Int32>(IDDCODE),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("AREACODE"), NanNew<Int32>(AREACODE),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("WEATHERSTATIONCODE"), NanNew<Int32>(WEATHERSTATIONCODE),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("WEATHERSTATIONNAME"), NanNew<Int32>(WEATHERSTATIONNAME),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("MCC"), NanNew<Int32>(MCC),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("MNC"), NanNew<Int32>(MNC),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("MOBILEBRAND"), NanNew<Int32>(MOBILEBRAND),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("ELEVATION"), NanNew<Int32>(ELEVATION),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("USAGETYPE"), NanNew<Int32>(USAGETYPE),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
      tpl->Set( NanNew<String>("ALL"), NanNew<Int32>(ALL),
        static_cast<PropertyAttribute>(ReadOnly | DontDelete) );

      NODE_SET_PROTOTYPE_METHOD(tpl, "query", Query);
      NODE_SET_PROTOTYPE_METHOD(tpl, "close", CloseDatabase);
      NODE_SET_PROTOTYPE_METHOD(tpl, "info", GetDbInfo);
      NODE_SET_PROTOTYPE_METHOD(tpl, "deleteShared", DeleteShared);
      NODE_SET_PROTOTYPE_METHOD(tpl, "createDictionary", CreateDictionary);

      exports->Set( NanNew<String>("Location"), NanNew<FunctionTemplate>(constructor)->GetFunction() );
    }

    Location(char *locdbpath, IP2LOCATION_ACCESS_TYPE mtype, char *shared)
    {
      iplocdb = IP2LocationOpen(locdbpath, mtype, shared);
    }

    ~Location()
    {
      Close();
    }

    void Close()
    {
      if (iplocdb != NULL) {
        IP2LocationClose(iplocdb);
        iplocdb = NULL;
        dbmode = LOCATION_DBMODE_CLOSED;
      }
    }

    static NAN_METHOD(New)
    {
      NanScope();

      if ( args.IsConstructCall() ) {
        NanUtf8String locdbpath( args[0] );
        Location* location;
        IP2LOCATION_ACCESS_TYPE mtype(IP2LOCATION_FILE_IO);
        const char *dbmode(LOCATION_DBMODE_FILE);
        if (args.Length() > 1) {
          char *shared = NULL;
          NanUtf8String stype( args[1] );
          if (strncmp(*stype, LOCATION_DBMODE_SHARED, sizeof(LOCATION_DBMODE_SHARED)) == 0) {
            mtype = IP2LOCATION_SHARED_MEMORY;
            dbmode = LOCATION_DBMODE_SHARED;
          } else if (strncmp(*stype, LOCATION_DBMODE_CACHE, sizeof(LOCATION_DBMODE_CACHE)) == 0) {
            mtype = IP2LOCATION_CACHE_MEMORY;
            dbmode = LOCATION_DBMODE_CACHE;
          } else if (strncmp(*stype, LOCATION_DBMODE_MMAP, sizeof(LOCATION_DBMODE_MMAP)) == 0) {
            mtype = IP2LOCATION_FILE_MMAP;
            dbmode = LOCATION_DBMODE_MMAP;
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
          return NanThrowError("could not open IP2LOCATION database");
        }
        location->dbmode = dbmode;
        location->Wrap( args.This() );
        NanReturnValue( args.This() );
      } else {
        int argc = args.Length();
        if (argc > 2) argc = 2;
        Local<Value> argv[2];
        for (int i = 0; i < argc; i++) {
          argv[i] = args[i];
        }
        NanReturnValue(
          NanNew<FunctionTemplate>(constructor)->GetFunction()->NewInstance(argc, &argv[0]) );
      }
    }

    static NAN_GETTER(GetDbMode)
    {
      NanScope();
      Location *location = ObjectWrap::Unwrap<Location>( args.This() );
      NanReturnValue( NanNew<String>(location->dbmode) );
    }

    static NAN_GETTER(GetIsOpen)
    {
      NanScope();
      Location *location = ObjectWrap::Unwrap<Location>( args.This() );
      NanReturnValue( NanNew<Boolean>( location->iplocdb != NULL ) );
    }

    static NAN_METHOD(CloseDatabase)
    {
      NanScope();
      Location* location = ObjectWrap::Unwrap<Location>(args.This());
      location->Close();
      NanReturnUndefined();
    }

    static NAN_METHOD(DeleteShared)
    {
      NanScope();
      Location* location = ObjectWrap::Unwrap<Location>(args.This());
      switch(IP2LocationDeleteShared(location->iplocdb)) {
      case 0:
        NanReturnValue( NanTrue() );
      case -1:
        NanReturnValue( NanFalse() );
      default:
        NanReturnNull();
      }
    }

    static NAN_METHOD(CreateDictionary)
    {
      NanScope();
      Location* location = ObjectWrap::Unwrap<Location>(args.This());
      if (!location->iplocdb) {
        return NanThrowError("IP2LOCATION database is closed");
      }
      if (args.Length() < 1) {
        return NanThrowTypeError("missing directory name");
      }
      int ret = IP2LocationMakeDictionary( location->iplocdb, *NanUtf8String( args[0] ) );
      if (ret < 0) {
        switch(ret) {
          case -1:
            return NanThrowError("couldn't create file or directory");
          default:
            return NanThrowError("error saving dictionary");
        }
      }
      NanReturnValue(NanNew<Number>(ret));
    }

    static NAN_METHOD(GetDbInfo)
    {
      NanScope();
      Location* location = ObjectWrap::Unwrap<Location>( args.This() );
      IP2Location *iplocdb = location->iplocdb;
      Local<Object> info = NanNew<Object>();
      if (iplocdb) {
        info->Set( NanNew<String>("ipversion"), NanNew<Int32>(iplocdb->ipversion) );
        info->Set( NanNew<String>("filename"), NanNew<String>(iplocdb->filename) );
        info->Set( NanNew<String>("databasetype"), NanNew<Int32>(iplocdb->databasetype) );
        info->Set( NanNew<String>("databasecolumn"), NanNew<Int32>(iplocdb->databasecolumn) );
        info->Set( NanNew<String>("databaseyear"), NanNew<Int32>(iplocdb->databaseyear) );
        info->Set( NanNew<String>("databasemonth"), NanNew<Int32>(iplocdb->databasemonth) );
        info->Set( NanNew<String>("databaseday"), NanNew<Int32>(iplocdb->databaseday) );
        info->Set( NanNew<String>("databasecount"), NanNew<Int32>(iplocdb->databasecount) );
        info->Set( NanNew<String>("databaseaddr"), NanNew<Int32>(iplocdb->databaseaddr) );
        if (iplocdb->mml_node != NULL) {
          info->Set(NanNew<String>("cacheoccupants"), NanNew<Int32>(iplocdb->mml_node->count));
          info->Set(NanNew<String>("cachesize"), NanNew<Number>( (double) iplocdb->mml_node->mem_size) );
          if (iplocdb->mml_node->type == MEMMAP_TYPE_SHARED) {
            info->Set(NanNew<String>("sharedname"), NanNew<String>(iplocdb->mml_node->name));
          }
        }
        NanReturnValue(info);
      } else {
        NanReturnNull();
      }
    }

    static NAN_METHOD(Query)
    {
      NanScope();

      uint32_t mode(ALL);

      if (args.Length() < 1) {
        return NanThrowError("IP address is required");
      }
      NanUtf8String ip( args[0] );

      if (args.Length() > 1) {
        mode = args[1]->Uint32Value();
      }

      Location *location = ObjectWrap::Unwrap<Location>( args.This() );

      if (!location->iplocdb) {
        return NanThrowError("IP2LOCATION database is closed");
      }

      IP2LocationRecord *record = IP2LocationQuery(location->iplocdb, *ip, mode);

      if ( record != NULL ) {
        Local<Object> result = NanNew<Object>();
        if (record->country_short != NULL) {
          result->Set( NanNew<String>("country_short"), NanNew<String>(record->country_short) );
        }
        if (record->country_long != NULL) {
          result->Set( NanNew<String>("country_long"), NanNew<String>(record->country_long) );
        }
        if (record->region != NULL) {
          result->Set( NanNew<String>("region"), NanNew<String>(record->region) );
        }
        if (record->city != NULL) {
          result->Set( NanNew<String>("city"), NanNew<String>(record->city) );
        }
        if (record->isp != NULL) {
          result->Set( NanNew<String>("isp"), NanNew<String>(record->isp) );
        }
        if (mode & LATITUDE) {
          result->Set( NanNew<String>("latitude"), NanNew<Number>(record->latitude) );
        }
        if (mode & LONGITUDE) {
          result->Set( NanNew<String>("longitude"), NanNew<Number>(record->longitude) );
        }
        if (record->domain != NULL) {
          result->Set( NanNew<String>("domain"), NanNew<String>(record->domain) );
        }
        if (record->zipcode != NULL) {
          result->Set( NanNew<String>("zipcode"), NanNew<String>(record->zipcode) );
        }
        if (record->timezone != NULL) {
          result->Set( NanNew<String>("timezone"), NanNew<String>(record->timezone) );
        }
        if (record->netspeed != NULL) {
          result->Set( NanNew<String>("netspeed"), NanNew<String>(record->netspeed) );
        }
        if (record->iddcode != NULL) {
          result->Set( NanNew<String>("iddcode"), NanNew<String>(record->iddcode) );
        }
        if (record->areacode != NULL) {
          result->Set( NanNew<String>("areacode"), NanNew<String>(record->areacode) );
        }
        if (record->weatherstationcode != NULL) {
          result->Set( NanNew<String>("weatherstationcode"), NanNew<String>(record->weatherstationcode) );
        }
        if (record->weatherstationname != NULL) {
          result->Set( NanNew<String>("weatherstationname"), NanNew<String>(record->weatherstationname) );
        }
        if (record->mcc != NULL) {
          result->Set( NanNew<String>("mcc"), NanNew<String>(record->mcc) );
        }
        if (record->mnc != NULL) {
          result->Set( NanNew<String>("mnc"), NanNew<String>(record->mnc) );
        }
        if (record->mobilebrand != NULL) {
          result->Set( NanNew<String>("mobilebrand"), NanNew<String>(record->mobilebrand) );
        }
        if (mode & ELEVATION) {
          result->Set( NanNew<String>("elevation"), NanNew<Number>(record->elevation) );
        }
        if (record->usagetype != NULL) {
          result->Set( NanNew<String>("usagetype"), NanNew<String>(record->usagetype) );
        }
        IP2LocationFreeRecord(record);
        NanReturnValue(result);
      }
      NanReturnValue( NanFalse() );
    }
};

Persistent<FunctionTemplate> Location::constructor;

extern "C" {
  static void init(Handle<Object> exports)
  {
    Location::Init(exports);
  }
  NODE_MODULE(nodeip2location, init)
}
