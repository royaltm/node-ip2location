#ifdef HAVE_NODEIP2LDICT_RESULT_IMPL
# error 'nodeip2ldict_result_impl.h' is supposed to be included once.
#endif
#define HAVE_NODEIP2LDICT_RESULT_IMPL
/*
result = {
  _index: ['AA', ..., 'ZZ']
  'usagetype'
  'netspeed'
  'AA': {
    'country_long': "AAAAaaaaa",
    'country_short': "AA",
    'regions': {
      _index: ['Region1', 'RegionN'],
      Region1: ['City1', 'City2'],
    }
    'isp': ['isp1', 'isp2'],
    'domain': ['domain1', 'domain2'],
    'zipcode': ['zipcode1', 'zipcode2'],
    'timezone': ['timezone1', 'timezone2'],
    'iddcode'
    'areacode',
    'weatherstations': {
      
    }
    'mcc', {
      _index: ['mcc1', 'mcc2'],
      mcc1: {
        mnc: [mnc1, mnc2]
      }
    }
  }
}

*/

void Location::MakeDictionaryItem(IP2Location *loc,
                                  uint32_t rowoffset,
                                  uint32_t mask,
                                  Map<IP2LDictionary>::type &dict)
{
  char name[257];
  if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_COUNTRY_SHORT_INDEX, rowoffset, name) ) {
    IP2LDictionary *entry = IP2LDictionary::FindOrAddDictionaryElementMap(name, dict);

    if ( entry == NULL )
      return;

    if ( entry->SecondName() == NULL && ((mask & IP2L_COUNTRY_LONG_MASK) != 0) ) {
      if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_COUNTRY_LONG_INDEX, rowoffset, name) ) {
        entry->SecondName(name);
      }
    }
    if ( (mask & IP2L_REGION_MASK) != 0 ) {
      if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_REGION_INDEX, rowoffset, name) ) {
        IP2LDictionary *region = entry->FindOrAddDictionaryElement(name, IP2L_DICT_REGION_CITY);
        if ( region != NULL ) {
          if ( (mask & IP2L_CITY_MASK) != 0 ) {
            if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_CITY_INDEX, rowoffset, name) ) {
              region->FindOrAddDictionaryElement(name, IP2L_DICT_REGION_CITY);
            }
          }
        }
      }
    }
    if ( (mask & IP2L_ISP_MASK) != 0 ) {
      if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_ISP_INDEX, rowoffset, name) ) {
        entry->FindOrAddDictionaryElement(name, IP2L_DICT_ISP);
      }
    }
    if ( (mask & IP2L_DOMAIN_MASK) != 0 ) {
      if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_DOMAIN_INDEX, rowoffset, name) ) {
        entry->FindOrAddDictionaryElement(name, IP2L_DICT_DOMAIN);
      }
    }
    if ( (mask & IP2L_ZIPCODE_MASK) != 0 ) {
      if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_ZIPCODE_INDEX, rowoffset, name) ) {
        entry->FindOrAddDictionaryElement(name, IP2L_DICT_ZIPCODE);
      }
    }
    if ( (mask & IP2L_IDDCODE_MASK) != 0 ) {
      if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_IDDCODE_INDEX, rowoffset, name) ) {
        entry->FindOrAddDictionaryElement(name, IP2L_DICT_IDDCODE);
      }
    }
    if ( (mask & IP2L_AREACODE_MASK) != 0 ) {
      if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_AREACODE_INDEX, rowoffset, name) ) {
        entry->FindOrAddDictionaryElement(name, IP2L_DICT_AREACODE);
      }
    }
  }
}

void Location::MakeDictionary(Map<IP2LDictionary>::type &dict,
                              IP2Location *loc,
                              uint32_t mask)
{
  uint32_t rowoffset = loc->databaseaddr;
  uint32_t columnsize = loc->databasecolumn * 4;
  uint32_t dbcount = loc->databasecount - 2;

  uint32_t index;

  for (index = 0; index < dbcount; ++index) {
    MakeDictionaryItem(loc, rowoffset, mask, dict);
    rowoffset += columnsize;
  }

  if ( loc->v6databasecount >= 2 ) {
    rowoffset = loc->v6databaseaddr + 12;
    columnsize = 16 + ( (loc->databasecolumn - 1) * 4 );
    dbcount = loc->v6databasecount - 2;
    for (index = 0; index < dbcount; ++index) {
      MakeDictionaryItem(loc, rowoffset, mask, dict);
      rowoffset += columnsize;
    }
  }

}

void Location::FreeDictionary(Map<IP2LDictionary>::type &dict)
{
  for( Map<IP2LDictionary>::type::iterator it = dict.begin();
                                      it != dict.end(); ++it ) {
    delete it->second;
  }
  dict.clear();
}

Local<Object> Location::CreateDictionaryResult(const Map<IP2LDictionary>::type &dict, const uint32_t mask)
{
  NanEscapableScope();
  Local<Object> result( NanNew<Object>() );
  Local<String> _index( NanNew<String>("_index")),
                country_long( NanNew<String>("country_long") ),
                country_short( NanNew<String>("country_short") ),
                regions( NanNew<String>("regions") ),
                isp( NanNew<String>("isp") ),
                domain( NanNew<String>("domain") ),
                zipcode( NanNew<String>("zipcode") ),
                iddcode( NanNew<String>("iddcode") ),
                areacode( NanNew<String>("areacode") );

  Local<Array> index_array = CreateArrayResult(dict);
  result->Set( _index, index_array );

  uint32_t country_index = 0;

  for( Map<IP2LDictionary>::type::const_iterator it = dict.begin();
                                      it != dict.end(); ++it ) {
    IP2LDictionary *country = it->second;
    Local<Object> country_result( NanNew<Object>() );
    Local<String> short_name = index_array->Get(country_index++).As<String>();
    country_result->Set( country_short, short_name );

    if ( (mask & IP2L_COUNTRY_LONG_MASK) != 0 ) {
      if ( country->SecondName() != NULL ) {
        country_result->Set( country_long, NanNew<String>( country->SecondName() ) );
      } else {
        country_result->Set( country_long, NanNull() );
      }
    }

    if ( (mask & IP2L_REGION_MASK) != 0 ) {
      const Map<IP2LDictionary>::type &region_map = country->Children(IP2L_DICT_REGION_CITY);
      Local<Array> region_array = CreateArrayResult(region_map);

      if ( (mask & IP2L_CITY_MASK) != 0 ) {
        Local<Object> region_result( NanNew<Object>() );
        region_result->Set( _index, region_array );
        uint32_t region_index = 0;
        for( Map<IP2LDictionary>::type::const_iterator itr = region_map.begin();
                                            itr != region_map.end(); ++itr ) {
          region_result->Set(
                              region_array->Get(region_index++),
                              CreateArrayResult(itr->second->Children(IP2L_DICT_REGION_CITY)) );
        }
        country_result->Set( regions, region_result );
      } else {
        country_result->Set( regions, region_array );
      }
    }

    if ( (mask & IP2L_ISP_MASK) != 0 ) {
      country_result->Set(
                        isp,
                        CreateArrayResult(country->Children(IP2L_DICT_ISP)) );
    }

    if ( (mask & IP2L_DOMAIN_MASK) != 0 ) {
      country_result->Set(
                        domain,
                        CreateArrayResult(country->Children(IP2L_DICT_DOMAIN)) );
    }

    if ( (mask & IP2L_ZIPCODE_MASK) != 0 ) {
      country_result->Set(
                        zipcode,
                        CreateArrayResult(country->Children(IP2L_DICT_ZIPCODE)) );
    }

    if ( (mask & IP2L_IDDCODE_MASK) != 0 ) {
      country_result->Set(
                        iddcode,
                        CreateArrayResult(country->Children(IP2L_DICT_IDDCODE)) );
    }

    if ( (mask & IP2L_AREACODE_MASK) != 0 ) {
      country_result->Set(
                        areacode,
                        CreateArrayResult(country->Children(IP2L_DICT_AREACODE)) );
    }

    result->Set( short_name, country_result );
  }

  return NanEscapeScope(result);
}

Local<Array> Location::CreateArrayResult(const Map<IP2LDictionary>::type &dict)
{
  NanEscapableScope();

  Local<Array> result( NanNew<Array>( (uint32_t) dict.size() ) );

  uint32_t count = 0;

  for( Map<IP2LDictionary>::type::const_iterator it = dict.begin();
                                      it != dict.end(); ++it ) {
    result->Set( count++, NanNew<String>( it->second->Name() ) );
  }
  return NanEscapeScope(result);
}
