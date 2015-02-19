#include "nodeip2location.h"

using namespace node;
using namespace v8;

#include "nodeip2ldictbuilder_impl.h"

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
  "COUNTRY_SHORT",
  "COUNTRY_LONG",
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

  Local<ObjectTemplate> proto = tpl->PrototypeTemplate();
  proto->SetAccessor( NanNew<String>("ipv6"), HasIpv6 );

  for(int index = 0; index <= IP2L_INDEX_MAX; ++index) {
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
  NODE_SET_PROTOTYPE_METHOD(tpl, "createDictionary", CreateDictionary);
  NODE_SET_PROTOTYPE_METHOD(tpl, "getAll", GetAll);

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

    const char *dbmode = LOCATION_DBMODE_FILE;

    if ( args.Length() > 1 ) {
      char *shared = NULL;
      NanUtf8String stype( args[1] );
      if ( ! strcmp(*stype, LOCATION_DBMODE_SHARED) ) {
        mtype = IP2LOCATION_SHARED_MEMORY;
        dbmode = LOCATION_DBMODE_SHARED;
      } else if ( ! strcmp(*stype, LOCATION_DBMODE_CACHE) ) {
        mtype = IP2LOCATION_CACHE_MEMORY;
        dbmode = LOCATION_DBMODE_CACHE;
      } else if ( ! strcmp(*stype, LOCATION_DBMODE_MMAP) ) {
        mtype = IP2LOCATION_FILE_MMAP;
        dbmode = LOCATION_DBMODE_MMAP;
      } else if ( ! strncmp(*stype, "/", 1) ) {
        mtype = IP2LOCATION_SHARED_MEMORY;
        dbmode = LOCATION_DBMODE_SHARED;
        shared = *stype;
      } else if ( strcmp(*stype, LOCATION_DBMODE_FILE) ) {
        return NanThrowError("unknown IP2LOCATION access mode, should be "
                             "\"file\", \"cache\", \"mmap\" or \"shared\"");
      }

      location = new Location(*locdbpath, mtype, shared);
    } else {
      location = new Location(*locdbpath, mtype);
    }

    if ( ! location->iplocdb ) {
      delete location;
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

NAN_GETTER(Location::HasIpv6)
{
  NanScope();
  Location *location = ObjectWrap::Unwrap<Location>( args.This() );
  NanReturnValue( NanNew<Boolean>( IP2LocationDBhasIPV6(location->iplocdb) != 0 ) );
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

NAN_METHOD(Location::CreateDictionary)
{
  NanScope();

  Location* location = ObjectWrap::Unwrap<Location>(args.This());
  if ( ! location->iplocdb ) {
    return NanThrowError("IP2LOCATION database is closed");
  }
  if ( location->iplocdb->mml_node == NULL ) {
    return NanThrowError("IP2LOCATION database should be in cache, mmap or shared mode");
  }

  uint32_t mode = LOCATION_ALL;

  if ( args.Length() > 0 ) {
    mode = args[0]->Uint32Value();
  }

  IP2LDictionary dict;

  BuildDictionary(dict, location->iplocdb, mode);

  Local<Object> result = CreateDictionaryResult(dict, mode);

  NanReturnValue(result);
}

NAN_METHOD(Location::GetDbInfo)
{
  NanScope();

  Location* location = ObjectWrap::Unwrap<Location>( args.This() );

  IP2Location *iplocdb = location->iplocdb;
  Local<Object> info = NanNew<Object>();

  if (iplocdb) {
    info->Set( NanNew<String>("filename"), NanNew<String>(iplocdb->filename) );
    info->Set( NanNew<String>("filesize"), NanNew<Uint32>( (uint32_t) iplocdb->filesize) );
    info->Set( NanNew<String>("databasetype"), NanNew<Int32>(iplocdb->databasetype) );
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
      info->Set(NanNew<String>("cachesize"), NanNew<Uint32>( (uint32_t) iplocdb->mml_node->mem_size) );
      info->Set(NanNew<String>("copybythisprocess"), NanNew<Boolean>( iplocdb->mml_node->copybythisprocess != 0 ) );
      if (iplocdb->mml_node->type == MEMMAP_TYPE_SHARED) {
        info->Set(NanNew<String>("sharedname"), NanNew<String>(iplocdb->mml_node->name));
      }
    }

    NanReturnValue(info);

  }

  NanReturnNull();
}

NAN_METHOD(Location::Query)
{
  NanScope();

  if ( args.Length() < 1 ) {
    return NanThrowError("IP address is required");
  }

  Location *location = ObjectWrap::Unwrap<Location>( args.This() );

  if ( ! location->iplocdb ) {
    return NanThrowError("IP2LOCATION database is closed");
  }

  NanUtf8String ip( args[0] );

  uint32_t mode = location->iplocdb->mode_mask;

  if ( args.Length() > 1 ) {
    mode &= args[1]->Uint32Value();
  }

  uint32_t dboffset = IP2LocationFindRow(location->iplocdb, *ip);

  if ( dboffset != IP2L_NOT_FOUND ) {
    Local<Object> result = NanNew<Object>();

    for ( uint8_t index = 0; mode != 0; ++index ) {
      const unsigned char *data;
      if ( (mode & 1) != 0 ) {
        Local<Value> value;
        switch ( IP2LocationRowData(location->iplocdb,
                                    static_cast<IP2LOCATION_DATA_INDEX>(index),
                                    dboffset,
                                    (const void **)&data ) ) {
          case IP2L_DATA_STRING:
            value = NanNew<String>( (const char *)data + 1, data[0] );
            break;
          case IP2L_DATA_FLOAT:
            value = NanNew<Number>( (const double)(*(const float *)data) );
            break;
          default:
            ;
        }
        if ( ! value.IsEmpty() )
          result->Set( NanNew<String>(LOCATION_RESULT_KEYS[index]), value );
      }

      mode >>= 1;
    }

    NanReturnValue(result);
  }

  NanReturnNull();
}

void Location::SetResultErrorStatus(Handle<Object> &result,
                                         const char * const status, bool setIP)
{
  NanScope();

  Local<String> value( NanNew<String>("?") );

  if (setIP) {
    result->Set( NanNew<String>("ip"), value );
    result->Set( NanNew<String>("ip_no"), value );
  }

  for (uint8_t index = 0; index <= IP2L_INDEX_MAX; ++index ) {
    result->Set( NanNew<String>(LOCATION_RESULT_KEYS[index]), value );
  }

  result->Set( NanNew<String>("status"), NanNew<String>(status) );
}

NAN_METHOD(Location::GetAll)
{
  NanScope();

  Local<Object> result = NanNew<Object>();

  Location *location = ObjectWrap::Unwrap<Location>( args.This() );

  if ( ! location->iplocdb ) {
    SetResultErrorStatus(result, "MISSING_FILE");
    NanReturnValue(result);
  }

  Local<Value> ip;

  if ( args.Length() < 1 ) {
    ip = NanUndefined();
  } else {
    ip = args[0];
  }

  ipv6le128_t ipaddr;

  int iptype = IP2LocationIP2No( *NanUtf8String(ip), &ipaddr );

  uint32_t dboffset;

  switch( iptype ) {
    case 6:
      if ( IP2LocationDBhasIPV6(location->iplocdb) ) {
        char ipnostr[IP2L_ULONG128_DECIMAL_SIZE];
        int ipnolen = IP2LocationULong128ToDecimal(ipaddr.ui32, ipnostr);
        result->Set( NanNew<String>("ip"), ip );
        result->Set( NanNew<String>("ip_no"), NanNew(ipnostr, ipnolen) );
        dboffset = IP2LocationFindRowIPV6(location->iplocdb, &ipaddr);
      } else {
        SetResultErrorStatus(result, "IPV6_NOT_SUPPORTED");
        NanReturnValue(result);
      }

      break;

    case 4:
      {
        char ipstr[16];
        int iplen = IP2LocationIPv4Str(&ipaddr, ipstr);
        result->Set( NanNew<String>("ip"), NanNew(ipstr, iplen) );
        result->Set( NanNew<String>("ip_no"), NanNew<Uint32>(ipaddr.ipv4.addr) );
        dboffset = IP2LocationFindRowIPV4(location->iplocdb, ipaddr.ipv4.addr);
      }
      break;

    default:
      SetResultErrorStatus(result, "INVALID_IP_ADDRESS");
      NanReturnValue(result);
  }

  if ( dboffset == IP2L_NOT_FOUND ) {

    SetResultErrorStatus(result, "IP_ADDRESS_NOT_FOUND", false);

  } else {

    uint32_t mode = location->iplocdb->mode_mask;

    Local<String> nosupport( NanNew<String>(LOCATION_MSG_NOT_SUPPORTED) );

    for ( uint8_t index = 0; index <= IP2L_INDEX_MAX; ++index ) {
      const unsigned char *data;
      Local<Value> value;

      if ( (mode & 1) != 0 ) {
        switch ( IP2LocationRowData(location->iplocdb,
                                    static_cast<IP2LOCATION_DATA_INDEX>(index),
                                    dboffset,
                                    (const void **)&data ) ) {
          case IP2L_DATA_STRING:
            value = NanNew<String>( (const char *)data + 1, data[0] );
            break;
          case IP2L_DATA_FLOAT:
            value = NanNew<Number>( (const double)(*(const float *)data) );
            break;
          default:
            ;
        }
      } else {
        switch( static_cast<IP2LOCATION_DATA_INDEX>(index) ) {
          case IP2L_LATITUDE_INDEX:
          case IP2L_LONGITUDE_INDEX:
          case IP2L_ELEVATION_INDEX:
            value = NanNew<Number>(0.0);
            break;
          default:
            ;
        }
      }

      if ( value.IsEmpty() ) {
        result->Set( NanNew<String>(LOCATION_RESULT_KEYS[index]), nosupport );
      } else {
        result->Set( NanNew<String>(LOCATION_RESULT_KEYS[index]), value );
      }

      mode >>= 1;
    }

    result->Set( NanNew<String>("status"), NanNew<String>("OK") );
  }

  NanReturnValue(result);
}

Persistent<FunctionTemplate> Location::constructor;

extern "C" {
  static void init(Handle<Object> exports)
  {
    Location::Init(exports);
  }
  NODE_MODULE(nodeip2location, init)
}
