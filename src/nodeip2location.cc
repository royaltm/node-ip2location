#include "nodeip2location.h"

using namespace node;
using namespace v8;

static const char * const LOCATION_RESULT_KEYS[IP2L_INDEX_MAX + 1] = {
  "country_short",
  "country_long",
  "region",
  "city",
  "isp",
  "latitude",
  "longitude",
  "domain",
  "zipcode",
  "timezone",
  "netspeed",
  "iddcode",
  "areacode",
  "weatherstationcode",
  "weatherstationname",
  "mcc",
  "mnc",
  "mobilebrand",
  "elevation",
  "usagetype"
};

static const char * const LOCATION_CONST_KEYS[IP2L_INDEX_MAX + 1] = {
  "COUNTRYSHORT",
  "COUNTRYLONG",
  "REGION",
  "CITY",
  "ISP",
  "LATITUDE",
  "LONGITUDE",
  "DOMAIN",
  "ZIPCODE",
  "TIMEZONE",
  "NETSPEED",
  "IDDCODE",
  "AREACODE",
  "WEATHERSTATIONCODE",
  "WEATHERSTATIONNAME",
  "MCC",
  "MNC",
  "MOBILEBRAND",
  "ELEVATION",
  "USAGETYPE"
};

void Location::Init(Handle<Object> exports)
{
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
  NanAssignPersistent( constructor, tpl );
  tpl->SetClassName( NanNew<String>("Location") );

  Local<ObjectTemplate> i_t = tpl->InstanceTemplate();
  i_t->SetInternalFieldCount(1);

  i_t->SetAccessor( NanNew<String>("mode"), GetDbMode );
  i_t->SetAccessor( NanNew<String>("opened"), GetIsOpen );

  for(int index = 0; index < IP2L_INDEX_MAX; ++index) {
    tpl->Set( NanNew<String>(LOCATION_CONST_KEYS[index]),
              NanNew<Int32>(1 << index),
              static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
  }

  tpl->Set( NanNew<String>("ALL"), NanNew<Int32>(LOCATION_ALL),
    static_cast<PropertyAttribute>(ReadOnly | DontDelete) );

  NODE_SET_PROTOTYPE_METHOD(tpl, "query", Query);
  NODE_SET_PROTOTYPE_METHOD(tpl, "close", CloseDatabase);
  NODE_SET_PROTOTYPE_METHOD(tpl, "info", GetDbInfo);
  NODE_SET_PROTOTYPE_METHOD(tpl, "deleteShared", DeleteShared);
  // NODE_SET_PROTOTYPE_METHOD(tpl, "createDictionary", CreateDictionary);
  // NODE_SET_PROTOTYPE_METHOD(tpl, "createISPDictionary", CreateSimpleDictionary);

  exports->Set( NanNew<String>("Location"), NanNew<FunctionTemplate>(constructor)->GetFunction() );
}

Location::Location(char *locdbpath, IP2LOCATION_ACCESS_TYPE mtype, char *shared)
{
  iplocdb = IP2LocationOpen(locdbpath, mtype, shared);
}

Location::~Location()
{
  Close();
}

void Location::Close()
{
  if (iplocdb != NULL) {
    IP2LocationClose(iplocdb);
    iplocdb = NULL;
    dbmode = LOCATION_DBMODE_CLOSED;
  }
}

NAN_METHOD(Location::New)
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

NAN_GETTER(Location::GetDbMode)
{
  NanScope();
  Location *location = ObjectWrap::Unwrap<Location>( args.This() );
  NanReturnValue( NanNew<String>(location->dbmode) );
}

NAN_GETTER(Location::GetIsOpen)
{
  NanScope();
  Location *location = ObjectWrap::Unwrap<Location>( args.This() );
  NanReturnValue( NanNew<Boolean>( location->iplocdb != NULL ) );
}

NAN_METHOD(Location::CloseDatabase)
{
  NanScope();
  Location* location = ObjectWrap::Unwrap<Location>(args.This());
  location->Close();
  NanReturnUndefined();
}

NAN_METHOD(Location::DeleteShared)
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

/*    NAN_METHOD(Location::CreateDictionary)
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

NAN_METHOD(Location::CreateSimpleDictionary)
{
  NanScope();
  Location* location = ObjectWrap::Unwrap<Location>(args.This());
  if (!location->iplocdb) {
    return NanThrowError("IP2LOCATION database is closed");
  }
  if (args.Length() < 1) {
    return NanThrowTypeError("missing directory name");
  }
  int ret = IP2LocationMakeSimpleDictionary(location->iplocdb, *NanUtf8String( args[0] ), IP2L_ISP);
  if (ret < 0) {
    switch(ret) {
    case -1:
      return NanThrowError("couldn't create file");
    default:
      return NanThrowError("error saving dictionary");
    }
  }
  NanReturnValue(NanNew<Number>(ret));
}

*/
NAN_METHOD(Location::GetDbInfo)
{
  NanScope();
  Location* location = ObjectWrap::Unwrap<Location>( args.This() );
  IP2Location *iplocdb = location->iplocdb;
  Local<Object> info = NanNew<Object>();
  if (iplocdb) {
    info->Set( NanNew<String>("filename"), NanNew<String>(iplocdb->filename) );
    info->Set( NanNew<String>("databasetype"), NanNew<Int32>(iplocdb->databasetype) );
    info->Set( NanNew<String>("databasecolumn"), NanNew<Int32>(iplocdb->databasecolumn) );
    info->Set( NanNew<String>("databaseyear"), NanNew<Int32>(iplocdb->databaseyear) );
    info->Set( NanNew<String>("databasemonth"), NanNew<Int32>(iplocdb->databasemonth) );
    info->Set( NanNew<String>("databaseday"), NanNew<Int32>(iplocdb->databaseday) );
    info->Set( NanNew<String>("databasecount"), NanNew<Int32>(iplocdb->databasecount) );
    info->Set( NanNew<String>("databaseaddr"), NanNew<Int32>(iplocdb->databaseaddr) );
    info->Set( NanNew<String>("v6databasecount"), NanNew<Int32>(iplocdb->v6databasecount) );
    info->Set( NanNew<String>("v6databaseaddr"), NanNew<Int32>(iplocdb->v6databaseaddr) );
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

NAN_METHOD(Location::Query)
{
  NanScope();

  uint32_t mode(LOCATION_ALL);

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

  uint32_t dboffset = IP2LocationFindRow(location->iplocdb, *ip);

  if ( dboffset != IP2L_NOT_FOUND ) {
    Local<Object> result = NanNew<Object>();
    uint8_t index = 0;
    uint32_t mask = 1;

    for ( ; index <= IP2L_INDEX_MAX; ++index ) {
      const unsigned char *data;
      if ( (mode & mask) != 0 ) {
        Local<Value> value;
        switch ( IP2LocationRowData(location->iplocdb,
                                    static_cast<IP2LOCATION_OFFSET_INDEX>(index),
                                    dboffset,
                                    (const void **)&data ) ) {
          case IP2L_DATA_STRING:
            value = NanNew<String>( (const char *)data + 1, data[0] );
            break;
          case IP2L_DATA_FLOAT:
            value = NanNew<Number>( (double)(*(float *)data) );
            break;
          default:
            value = NanFalse();
        }
        result->Set( NanNew<String>(LOCATION_RESULT_KEYS[index]), value );
      }
      if ( (mode &= ~mask) == 0 )
        break;
      mask <<= 1;
    }
    NanReturnValue(result);
  }

  NanReturnValue( NanFalse() );
}

Persistent<FunctionTemplate> Location::constructor;

extern "C" {
  static void init(Handle<Object> exports)
  {
    Location::Init(exports);
  }
  NODE_MODULE(nodeip2location, init)
}
