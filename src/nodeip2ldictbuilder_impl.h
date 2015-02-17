#ifdef HAVE_NODEIP2LDICTBUILDER_IMPL
# error 'nodeip2ldictbuilder_impl.h' is supposed to be included once.
#endif
#define HAVE_NODEIP2LDICTBUILDER_IMPL
/*
result = {
  _index: ['AA', ..., 'ZZ']
  'usagetype'
  'netspeed'
  'AA': {
    'country_long': "AAAAaaaaa",
    'country_short': "AA",
    'region': {
      _index: ['Region1', 'RegionN'],
      Region1: ['City1', 'City2'],
    }
    'isp': ['isp1', 'isp2'],
    'domain': ['domain1', 'domain2'],
    'zipcode': ['zipcode1', 'zipcode2'],
    'timezone': ['timezone1', 'timezone2'],
    'iddcode': [],
    'areacode': [],
    'weatherstationcode': {
      'weatherstationname': []
    }
    'mcc', {
      _index: ['mcc1', 'mcc2'],
      mcc1: {
        mnc: [mnc1, mnc2]
      }
    }
    'mobilebrand': []
  }
}

*/

void Location::BuildDictionary(IP2LDictionary &dict,
                              IP2Location *loc,
                              uint32_t mask)
{
  uint32_t rowoffset = loc->databaseaddr;
  uint32_t columnsize = loc->databasecolumn * 4;
  uint32_t dbcount = loc->databasecount - 2;

  uint32_t index;

  for (index = 0; index < dbcount; ++index) {
    BuildDictionaryItem(loc, rowoffset, mask, dict);
    rowoffset += columnsize;
  }

  if ( loc->v6databasecount >= 2 ) {
    rowoffset = loc->v6databaseaddr + 12;
    columnsize = 16 + ( (loc->databasecolumn - 1) * 4 );
    dbcount = loc->v6databasecount - 2;
    for (index = 0; index < dbcount; ++index) {
      BuildDictionaryItem(loc, rowoffset, mask, dict);
      rowoffset += columnsize;
    }
  }

}

void Location::BuildDictionaryItem(IP2Location *loc,
                                  uint32_t rowoffset,
                                  uint32_t mask,
                                  IP2LDictionary &dict)
{
  char name[257];
  if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_COUNTRY_SHORT_INDEX, rowoffset, name) ) {
    IP2LDictionaryCountry *country = dict.FindOrAddDictionaryCountry(name);

    if ( country == NULL )
      return;

    if ( country->NoSecondName() && ((mask & IP2L_COUNTRY_LONG_MASK) != 0) ) {
      if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_COUNTRY_LONG_INDEX, rowoffset, name) ) {
        country->SecondName(name);
      }
    }

    BuildDictionaryBranchItem<IP2L_REGION_INDEX,
                         IP2L_DICT_REGION_CITY,
                         IP2L_CITY_INDEX>(loc, rowoffset, mask, country, name);

    BuildDictionaryBranchItem<IP2L_WEATHERSTATIONNAME_INDEX,
                         IP2L_DICT_WEATHERSTATION,
                         IP2L_WEATHERSTATIONCODE_INDEX>(
                                          loc, rowoffset, mask, country, name);

    if ( (mask & ( IP2L_WEATHERSTATIONCODE_MASK|
                   IP2L_WEATHERSTATIONNAME_MASK
                   )) == IP2L_WEATHERSTATIONCODE_MASK ) {
      if ( IP2L_DATA_STRING == IP2LocationRowString(loc,
                            IP2L_WEATHERSTATIONCODE_INDEX, rowoffset, name) ) {
        country->AddUniqueDictionaryElement(name, IP2L_DICT_WEATHERSTATION);
      }
    }

    BuildDictionaryBranchItem<IP2L_MCC_INDEX,
                          IP2L_DICT_MCC_MNC,
                          IP2L_MNC_INDEX>(loc, rowoffset, mask, country, name);

    if ( (mask & IP2L_ISP_MASK) != 0 ) {
      if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_ISP_INDEX,
                                                           rowoffset, name) ) {
        country->AddUniqueDictionaryElement(name, IP2L_DICT_ISP);
      }
    }
    if ( (mask & IP2L_DOMAIN_MASK) != 0 ) {
      if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_DOMAIN_INDEX,
                                                           rowoffset, name) ) {
        country->AddUniqueDictionaryElement(name, IP2L_DICT_DOMAIN);
      }
    }
    if ( (mask & IP2L_ZIPCODE_MASK) != 0 ) {
      if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_ZIPCODE_INDEX,
                                                           rowoffset, name) ) {
        country->AddUniqueDictionaryElement(name, IP2L_DICT_ZIPCODE);
      }
    }
    if ( (mask & IP2L_IDDCODE_MASK) != 0 ) {
      if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_IDDCODE_INDEX,
                                                           rowoffset, name) ) {
        country->AddUniqueDictionaryElement(name, IP2L_DICT_IDDCODE);
      }
    }
    if ( (mask & IP2L_AREACODE_MASK) != 0 ) {
      if ( IP2L_DATA_STRING == IP2LocationRowString(loc, IP2L_AREACODE_INDEX,
                                                           rowoffset, name) ) {
        country->AddUniqueDictionaryElement(name, IP2L_DICT_AREACODE);
      }
    }
    if ( (mask & IP2L_MOBILEBRAND_MASK) != 0 ) {
      if ( IP2L_DATA_STRING == IP2LocationRowString(loc,
                                   IP2L_MOBILEBRAND_INDEX, rowoffset, name) ) {
        country->AddUniqueDictionaryElement(name, IP2L_DICT_MOBILEBRAND);
      }
    }
  }
}

template <IP2LOCATION_DATA_INDEX branch_index,
          IP2L_DICT_TYPE branch_type,
          IP2LOCATION_DATA_INDEX leaf_index>
NAN_INLINE void Location::BuildDictionaryBranchItem(
                                                IP2Location *loc,
                                                uint32_t rowoffset,
                                                uint32_t mask,
                                                IP2LDictionaryCountry *country,
                                                char * const name)
{
  if ( (mask & IP2L_MASK(branch_index)) != 0 ) {
    if ( IP2L_DATA_STRING == IP2LocationRowString(loc, branch_index, rowoffset, name) ) {
      if ( (mask & IP2L_MASK(leaf_index)) != 0 ) {
        IP2LDictionaryBranch *branch = country->FindOrAddDictionaryBranch(name, branch_type);
        if ( branch != NULL ) {
          if ( IP2L_DATA_STRING == IP2LocationRowString(loc, leaf_index, rowoffset, name) ) {
            branch->AddUniqueDictionaryElement(name);
          }
        }
      } else {
        country->AddUniqueDictionaryElement(name, branch_type);
      }
    }
  }
}


Local<Object> Location::CreateDictionaryResult( const IP2LDictionary &dict,
                                                const uint32_t mask )
{
  NanEscapableScope();
  Local<Object> result( NanNew<Object>() );
  Local<String> _index( NanNew<String>("_index")),
                country_long( NanNew<String>("country_long") ),
                country_short( NanNew<String>("country_short") ),
                region( NanNew<String>("region") ),
                isp( NanNew<String>("isp") ),
                domain( NanNew<String>("domain") ),
                zipcode( NanNew<String>("zipcode") ),
                iddcode( NanNew<String>("iddcode") ),
                areacode( NanNew<String>("areacode") ),
                weatherstationcode( NanNew<String>("weatherstationcode") ),
                weatherstationname( NanNew<String>("weatherstationname") ),
                mcc( NanNew<String>("mcc") ),
                mnc( NanNew<String>("mnc") ),
                mobilebrand( NanNew<String>("mobilebrand") );

  const Map<IP2LDictionaryElement>::type &dict_map = dict.Children();
  Local<Array> index_array = CreateArrayResult(dict_map);
  result->Set( _index, index_array );

  uint32_t country_index = 0;

  for( Map<IP2LDictionaryElement>::type::const_iterator it = dict_map.begin();
                                                it != dict_map.end(); ++it ) {
    const IP2LDictionaryCountry *country = static_cast<
                                    const IP2LDictionaryCountry *>(it->second);
    Local<Object> country_result( NanNew<Object>() );
    Local<String> short_name = index_array->Get(country_index++).As<String>();
    country_result->Set( country_short, short_name );

    if ( (mask & IP2L_COUNTRY_LONG_MASK) != 0 ) {
      if ( country->SecondName() != NULL ) {
        country_result->Set( country_long,
                             NanNew<String>( country->SecondName() ) );
      } else {
        country_result->Set( country_long, NanNull() );
      }
    }

    CreateDictionaryResultBranch( mask, IP2L_REGION_MASK, IP2L_CITY_MASK,
                                  country->Children(IP2L_DICT_REGION_CITY),
                                  _index, region, country_result );

    CreateDictionaryResultBranch( mask, IP2L_WEATHERSTATIONNAME_MASK,
                                  IP2L_WEATHERSTATIONCODE_MASK,
                                  country->Children(IP2L_DICT_WEATHERSTATION),
                                  _index, weatherstationname, country_result );

    if ( (mask & ( IP2L_WEATHERSTATIONCODE_MASK|
                   IP2L_WEATHERSTATIONNAME_MASK
                   )) == IP2L_WEATHERSTATIONCODE_MASK ) {
      country_result->Set(weatherstationcode, CreateArrayResult(
                                country->Children(IP2L_DICT_WEATHERSTATION)) );
    }

    CreateDictionaryResultBranch( mask, IP2L_MCC_MASK, IP2L_MNC_MASK,
                                  country->Children(IP2L_DICT_MCC_MNC),
                                  _index, mcc, country_result );

    CreateDictionaryResultElement( mask, IP2L_ISP_MASK,
                                   country->Children(IP2L_DICT_ISP),
                                   isp, country_result );

    CreateDictionaryResultElement( mask, IP2L_DOMAIN_MASK,
                                   country->Children(IP2L_DICT_DOMAIN),
                                   domain, country_result );

    CreateDictionaryResultElement( mask, IP2L_ZIPCODE_MASK,
                                   country->Children(IP2L_DICT_ZIPCODE),
                                   zipcode, country_result );

    CreateDictionaryResultElement( mask, IP2L_IDDCODE_MASK,
                                   country->Children(IP2L_DICT_IDDCODE),
                                   iddcode, country_result );

    CreateDictionaryResultElement( mask, IP2L_AREACODE_MASK,
                                   country->Children(IP2L_DICT_AREACODE),
                                   areacode, country_result );

    CreateDictionaryResultElement( mask, IP2L_MOBILEBRAND_MASK,
                                   country->Children(IP2L_DICT_MOBILEBRAND),
                                   mobilebrand, country_result );

    result->Set( short_name, country_result );
  }

  return NanEscapeScope(result);
}

Local<Array> Location::CreateArrayResult(
                              const Map<IP2LDictionaryElement>::type &dict_map)
{
  NanEscapableScope();

  Local<Array> result( NanNew<Array>( (uint32_t) dict_map.size() ) );

  uint32_t count = 0;

  for( Map<IP2LDictionaryElement>::type::const_iterator it = dict_map.begin();
                                      it != dict_map.end(); ++it ) {
    result->Set( count++, NanNew<String>( it->second->Name() ) );
  }
  return NanEscapeScope(result);
}

NAN_INLINE void Location::CreateDictionaryResultBranch(
                            const uint32_t mask,
                            const uint32_t branch_mask,
                            const uint32_t leaf_mask,
                            const Map<IP2LDictionaryElement>::type &branch_map,
                            Handle<String> &indexLabel,
                            Handle<String> &label,
                            Handle<Object> &result)
{
  if ( (mask & branch_mask) != 0 ) {
    Local<Array> branch_array = CreateArrayResult(branch_map);

    if ( (mask & leaf_mask) != 0 ) {
      Local<Object> branch_result( NanNew<Object>() );
      branch_result->Set( indexLabel, branch_array );
      uint32_t branch_index = 0;
      for( Map<IP2LDictionaryElement>::type::const_iterator it = branch_map.begin();
                                          it != branch_map.end(); ++it ) {
        branch_result->Set(
                      branch_array->Get(branch_index++),
                      CreateArrayResult(
                        static_cast<const IP2LDictionaryBranch *>(it->second)->Children()) );
      }
      result->Set( label, branch_result );
    } else {
      result->Set( label, branch_array );
    }
  }
}

NAN_INLINE void Location::CreateDictionaryResultElement(
                            const uint32_t mask,
                            const uint32_t leaf_mask,
                            const Map<IP2LDictionaryElement>::type &leaf_map,
                            Handle<String> &label,
                            Handle<Object> &result)
{
  if ( (mask & leaf_mask) != 0 ) {
    result->Set(label, CreateArrayResult(leaf_map) );
  }
}
