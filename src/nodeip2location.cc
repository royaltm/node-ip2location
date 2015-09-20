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

NAN_MODULE_INIT(Location::Init)
{
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName( Nan::New<String>("Location").ToLocalChecked() );

  Local<ObjectTemplate> i_t = tpl->InstanceTemplate();
  i_t->SetInternalFieldCount(1);

  Nan::SetAccessor( i_t, Nan::New<String>("mode").ToLocalChecked(), GetDbMode );
  Nan::SetAccessor( i_t, Nan::New<String>("opened").ToLocalChecked(), GetIsOpen );

  Nan::SetAccessor( tpl->PrototypeTemplate(), Nan::New<String>("ipv6").ToLocalChecked(), HasIpv6 );

  for(int index = 0; index <= IP2L_INDEX_MAX; ++index) {
    Nan::SetTemplate(tpl,
              Nan::New<String>(LOCATION_CONST_KEYS[index]).ToLocalChecked(),
              Nan::New<Int32>(1 << index),
              static_cast<PropertyAttribute>(ReadOnly | DontDelete) );
  }

  Nan::SetTemplate(tpl, Nan::New<String>("ALL").ToLocalChecked(),
    Nan::New<Int32>(LOCATION_ALL),
    static_cast<PropertyAttribute>(ReadOnly | DontDelete) );

  Nan::SetPrototypeMethod(tpl, "query", Query);
  Nan::SetPrototypeMethod(tpl, "close", CloseDatabase);
  Nan::SetPrototypeMethod(tpl, "info", GetDbInfo);
  Nan::SetPrototypeMethod(tpl, "deleteShared", DeleteShared);
  Nan::SetPrototypeMethod(tpl, "createDictionary", CreateDictionary);
  Nan::SetPrototypeMethod(tpl, "getAll", GetAll);

  constructor.Reset( Nan::GetFunction(tpl).ToLocalChecked() );
  Nan::Set(target, Nan::New<v8::String>("Location").ToLocalChecked(),
                   Nan::GetFunction(tpl).ToLocalChecked());
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

  if ( info.IsConstructCall() ) {

    Nan::Utf8String locdbpath( info[0] );
    Location* location;
    IP2LOCATION_ACCESS_TYPE mtype(IP2LOCATION_FILE_IO);

    const char *dbmode = LOCATION_DBMODE_FILE;

    if ( info.Length() > 1 ) {
      char *shared = NULL;
      Nan::Utf8String stype( info[1] );
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
        return Nan::ThrowError("unknown IP2LOCATION access mode, should be "
                             "\"file\", \"cache\", \"mmap\" or \"shared\"");
      }

      location = new Location(*locdbpath, mtype, shared);
    } else {
      location = new Location(*locdbpath, mtype);
    }

    if ( ! location->iplocdb ) {
      delete location;
      return Nan::ThrowError("could not open IP2LOCATION database");
    }

    location->dbmode = dbmode;
    location->Wrap( info.This() );
    info.GetReturnValue().Set( info.This() );

  } else {

    int argc = info.Length();
    if (argc > 2) argc = 2;
    Local<Value> argv[2];
    for (int i = 0; i < argc; i++) {
      argv[i] = info[i];
    }
    Local<Function> cons = Nan::New<Function>(constructor);
    info.GetReturnValue().Set( Nan::NewInstance(cons, argc, &argv[0]).ToLocalChecked() );
  }
}

NAN_GETTER(Location::GetDbMode)
{
  Location *location = Nan::ObjectWrap::Unwrap<Location>( info.This() );
  info.GetReturnValue().Set( Nan::New<String>(location->dbmode).ToLocalChecked() );
}

NAN_GETTER(Location::GetIsOpen)
{
  Location *location = Nan::ObjectWrap::Unwrap<Location>( info.This() );
  info.GetReturnValue().Set( location->iplocdb != NULL );
}

NAN_GETTER(Location::HasIpv6)
{
  Location *location = Nan::ObjectWrap::Unwrap<Location>( info.This() );
  info.GetReturnValue().Set( IP2LocationDBhasIPV6(location->iplocdb) != 0 );
}

NAN_METHOD(Location::CloseDatabase)
{
  Location* location = Nan::ObjectWrap::Unwrap<Location>(info.This());
  location->Close();
  info.GetReturnValue().SetUndefined();
}

NAN_METHOD(Location::DeleteShared)
{

  Location* location = Nan::ObjectWrap::Unwrap<Location>(info.This());

  switch(IP2LocationDeleteShared(location->iplocdb)) {
  case 0:
    info.GetReturnValue().Set(true);
  case -1:
    info.GetReturnValue().Set(false);
  default:
    info.GetReturnValue().SetNull();
  }
}

NAN_METHOD(Location::CreateDictionary)
{

  Location* location = Nan::ObjectWrap::Unwrap<Location>(info.This());
  if ( ! location->iplocdb ) {
    return Nan::ThrowError("IP2LOCATION database is closed");
  }
  if ( location->iplocdb->mml_node == NULL ) {
    return Nan::ThrowError("IP2LOCATION database should be in cache, mmap or shared mode");
  }

  uint32_t mode = LOCATION_ALL;

  if ( info.Length() > 0 ) {
    mode = Nan::To<uint32_t>(info[0]).FromMaybe(mode);
  }

  IP2LDictionary dict;

  BuildDictionary(dict, location->iplocdb, mode);

  Local<Object> result = CreateDictionaryResult(dict, mode);

  info.GetReturnValue().Set(result);
}

NAN_METHOD(Location::GetDbInfo)
{

  Location* location = Nan::ObjectWrap::Unwrap<Location>( info.This() );

  IP2Location *iplocdb = location->iplocdb;
  Local<Object> result = Nan::New<Object>();

  if (iplocdb) {
    Nan::Set(result, Nan::New<String>("filename").ToLocalChecked(),
                   Nan::New<String>(iplocdb->filename).ToLocalChecked() );
    Nan::Set(result, Nan::New<String>("filesize").ToLocalChecked(),
                   Nan::New<Uint32>( (uint32_t) iplocdb->filesize) );
    Nan::Set(result, Nan::New<String>("databasetype").ToLocalChecked(),
                   Nan::New<Int32>(iplocdb->databasetype) );
    Nan::Set(result, Nan::New<String>("databasetype").ToLocalChecked(),
                   Nan::New<Int32>(iplocdb->databasetype) );
    Nan::Set(result, Nan::New<String>("databasecolumn").ToLocalChecked(),
                   Nan::New<Int32>(iplocdb->databasecolumn) );
    Nan::Set(result, Nan::New<String>("databaseyear").ToLocalChecked(),
                   Nan::New<Int32>(iplocdb->databaseyear) );
    Nan::Set(result, Nan::New<String>("databasemonth").ToLocalChecked(),
                   Nan::New<Int32>(iplocdb->databasemonth) );
    Nan::Set(result, Nan::New<String>("databaseday").ToLocalChecked(),
                   Nan::New<Int32>(iplocdb->databaseday) );
    Nan::Set(result, Nan::New<String>("databasecount").ToLocalChecked(),
                   Nan::New<Int32>(iplocdb->databasecount) );
    Nan::Set(result, Nan::New<String>("databaseaddr").ToLocalChecked(),
                   Nan::New<Int32>(iplocdb->databaseaddr) );
    Nan::Set(result, Nan::New<String>("v6databasecount").ToLocalChecked(),
                   Nan::New<Int32>(iplocdb->v6databasecount) );
    Nan::Set(result, Nan::New<String>("v6databaseaddr").ToLocalChecked(),
                   Nan::New<Int32>(iplocdb->v6databaseaddr) );

    if (iplocdb->mml_node != NULL) {
      Nan::Set(result, Nan::New<String>("cacheoccupants").ToLocalChecked(),
                     Nan::New<Int32>(iplocdb->mml_node->count));
      Nan::Set(result, Nan::New<String>("cachesize").ToLocalChecked(),
                     Nan::New<Uint32>( (uint32_t) iplocdb->mml_node->mem_size) );
      Nan::Set(result, Nan::New<String>("copybythisprocess").ToLocalChecked(),
                     Nan::New<Boolean>( iplocdb->mml_node->copybythisprocess != 0 ) );
      if (iplocdb->mml_node->type == MEMMAP_TYPE_SHARED) {
        Nan::Set(result, Nan::New<String>("sharedname").ToLocalChecked(),
                       Nan::New<String>(iplocdb->mml_node->name).ToLocalChecked());
      }
    }

    return info.GetReturnValue().Set(result);
  }

  info.GetReturnValue().SetNull();
}

NAN_METHOD(Location::Query)
{

  if ( info.Length() < 1 ) {
    return Nan::ThrowError("IP address is required");
  }

  Location *location = Nan::ObjectWrap::Unwrap<Location>( info.This() );

  if ( ! location->iplocdb ) {
    return Nan::ThrowError("IP2LOCATION database is closed");
  }

  uint32_t mode = location->iplocdb->mode_mask;

  if ( info.Length() > 1 ) {
    mode &= Nan::To<uint32_t>(info[1]).FromMaybe(0xFFFFFFFF);
  }

  uint32_t dboffset;

  if ( Buffer::HasInstance(info[0]) ) {
    Local<Object> ipbuff = info[0].As<Object>();
    dboffset = IP2LocationFindRow2( location->iplocdb,
                        (void *)Buffer::Data(ipbuff), (uint32_t)Buffer::Length(ipbuff) );
  } else {
    dboffset = IP2LocationFindRow( location->iplocdb, *Nan::Utf8String(info[0]) );
  }

  if ( dboffset != IP2L_NOT_FOUND ) {
    Local<Object> result = Nan::New<Object>();

    for ( uint8_t index = 0; mode != 0; ++index ) {
      const unsigned char *data;
      if ( (mode & 1) != 0 ) {
        Local<Value> value;
        switch ( IP2LocationRowData(location->iplocdb,
                                    static_cast<IP2LOCATION_DATA_INDEX>(index),
                                    dboffset,
                                    (const void **)&data ) ) {
          case IP2L_DATA_STRING:
            value = Nan::New<String>( (const char *)data + 1, data[0] ).ToLocalChecked();
            break;
          case IP2L_DATA_FLOAT:
            value = Nan::New<Number>( (const double)(*(const float *)data) );
            break;
          default:
            ;
        }
        if ( ! value.IsEmpty() )
          Nan::Set(result, Nan::New<String>(LOCATION_RESULT_KEYS[index]).ToLocalChecked(),
                           value );
      }

      mode >>= 1;
    }

    return info.GetReturnValue().Set(result);
  }

  info.GetReturnValue().SetNull();
}

void Location::SetResultErrorStatus(Local<Object> &result,
                                         const char * const status, bool setIP)
{

  Local<String> value( Nan::New<String>("?").ToLocalChecked() );

  if (setIP) {
    Nan::Set(result, Nan::New<String>("ip").ToLocalChecked(), value );
    Nan::Set(result, Nan::New<String>("ip_no").ToLocalChecked(), value );
  }

  for (uint8_t index = 0; index <= IP2L_INDEX_MAX; ++index ) {
    Nan::Set(result, Nan::New<String>(LOCATION_RESULT_KEYS[index]).ToLocalChecked(), value );
  }

  Nan::Set(result, Nan::New<String>("status").ToLocalChecked(),
                   Nan::New<String>(status).ToLocalChecked() );
}

NAN_METHOD(Location::GetAll)
{

  Local<Object> result = Nan::New<Object>();

  Location *location = Nan::ObjectWrap::Unwrap<Location>( info.This() );

  if ( ! location->iplocdb ) {
    SetResultErrorStatus(result, "MISSING_FILE");
    return info.GetReturnValue().Set(result);
  }

  Local<Value> ip;

  if ( info.Length() < 1 ) {
    ip = Nan::Undefined();
  } else {
    ip = info[0];
  }

  ipv6le128_t ipaddr;
  int iptype;

  if ( Buffer::HasInstance(ip) ) {
    Local<Object> ipbuff = ip.As<Object>();
    iptype = IP2LocationIPBin2No(
               (void *)Buffer::Data(ipbuff), (uint32_t)Buffer::Length(ipbuff), &ipaddr );
  } else {
    iptype = IP2LocationIP2No( *Nan::Utf8String(ip), &ipaddr );
  }

  uint32_t dboffset;

  switch( iptype ) {
    case 6:
      if ( IP2LocationDBhasIPV6(location->iplocdb) ) {
        char ipnostr[IP2L_ULONG128_DECIMAL_SIZE];
        int ipnolen = IP2LocationULong128ToDecimal(ipaddr.ui32, ipnostr);
        Nan::Set(result, Nan::New<String>("ip").ToLocalChecked(), ip );
        Nan::Set(result, Nan::New<String>("ip_no").ToLocalChecked(),
                         Nan::New<String>((const char *)ipnostr, ipnolen).ToLocalChecked() );
        dboffset = IP2LocationFindRowIPV6(location->iplocdb, &ipaddr);
      } else {
        SetResultErrorStatus(result, "IPV6_NOT_SUPPORTED");
        return info.GetReturnValue().Set(result);
      }

      break;

    case 4:
      {
        char ipstr[16];
        int iplen = IP2LocationIPv4Str(&ipaddr, ipstr);
        Nan::Set(result, Nan::New<String>("ip").ToLocalChecked(),
                         Nan::New<String>((const char *)ipstr, iplen).ToLocalChecked() );
        Nan::Set(result, Nan::New<String>("ip_no").ToLocalChecked(),
                         Nan::New<Uint32>(ipaddr.ipv4.addr) );
        dboffset = IP2LocationFindRowIPV4(location->iplocdb, ipaddr.ipv4.addr);
      }
      break;

    default:
      SetResultErrorStatus(result, "INVALID_IP_ADDRESS");
      return info.GetReturnValue().Set(result);
  }

  if ( dboffset == IP2L_NOT_FOUND ) {

    SetResultErrorStatus(result, "IP_ADDRESS_NOT_FOUND", false);

  } else {

    uint32_t mode = location->iplocdb->mode_mask;

    Local<String> nosupport( Nan::New<String>(LOCATION_MSG_NOT_SUPPORTED).ToLocalChecked() );

    for ( uint8_t index = 0; index <= IP2L_INDEX_MAX; ++index ) {
      const unsigned char *data;
      Local<Value> value;

      if ( (mode & 1) != 0 ) {
        switch ( IP2LocationRowData(location->iplocdb,
                                    static_cast<IP2LOCATION_DATA_INDEX>(index),
                                    dboffset,
                                    (const void **)&data ) ) {
          case IP2L_DATA_STRING:
            value = Nan::New<String>( (const char *)data + 1, data[0] ).ToLocalChecked();
            break;
          case IP2L_DATA_FLOAT:
            value = Nan::New<Number>( (const double)(*(const float *)data) );
            break;
          default:
            ;
        }
      } else {
        switch( static_cast<IP2LOCATION_DATA_INDEX>(index) ) {
          case IP2L_LATITUDE_INDEX:
          case IP2L_LONGITUDE_INDEX:
          case IP2L_ELEVATION_INDEX:
            value = Nan::New<Number>(0.0);
            break;
          default:
            ;
        }
      }

      if ( value.IsEmpty() ) {
        Nan::Set(result, Nan::New<String>(LOCATION_RESULT_KEYS[index]).ToLocalChecked(), nosupport );
      } else {
        Nan::Set(result, Nan::New<String>(LOCATION_RESULT_KEYS[index]).ToLocalChecked(), value );
      }

      mode >>= 1;
    }

    Nan::Set(result, Nan::New<String>("status").ToLocalChecked(),
                     Nan::New<String>("OK").ToLocalChecked() );
  }

  info.GetReturnValue().Set(result);
}

Nan::Persistent<Function> Location::constructor;

NODE_MODULE(nodeip2location, Location::Init)
