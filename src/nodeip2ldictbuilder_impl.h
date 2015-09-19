#ifdef HAVE_NODEIP2LDICTBUILDER_IMPL
# error 'nodeip2ldictbuilder_impl.h' is supposed to be included once.
#endif
#define HAVE_NODEIP2LDICTBUILDER_IMPL
/*

dict = {
  _index: [country1, ..., countryN]
  country1: {
    'country_long': "Full Country Name",
    'country_short': country1,
    'region': {
      '_index': [region1, ..., regionN],
      region1: [city1, ..., cityN],
      ...
      regionN: [...]
    }
    'isp': [...],
    'domain': [...],
    'zipcode': [...],
    'timezone': [...],
    'iddcode': [...],
    'areacode': [...],
    'weatherstationname': [...], // or
    'weatherstationname': {
      '_index': [weatherstationname1, ..., weatherstationnameN]
      weatherstationname1: [weatherstationcode],
      ...
      weatherstationnameN: [weatherstationcode]
    },
    'weatherstationcode': [...] // alternative if only WEATHERSTATIONCODE is provided
    'mcc': [...], // or
    'mcc': {
      _index: [mcc1, ..., mccN],
      mcc1: [mnc1, ..., mncN],
      ...
      mccN: [...],
    }
    'mobilebrand': [...]
  },
  ...
  countryN: {...}
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
  Nan::EscapableHandleScope scope;
  Local<Object> result( Nan::New<Object>() );
  Local<String> _index( Nan::New<String>("_index").ToLocalChecked() ),
                country_long( Nan::New<String>("country_long").ToLocalChecked()  ),
                country_short( Nan::New<String>("country_short").ToLocalChecked()  ),
                region( Nan::New<String>("region").ToLocalChecked()  ),
                isp( Nan::New<String>("isp").ToLocalChecked()  ),
                domain( Nan::New<String>("domain").ToLocalChecked()  ),
                zipcode( Nan::New<String>("zipcode").ToLocalChecked()  ),
                iddcode( Nan::New<String>("iddcode").ToLocalChecked()  ),
                areacode( Nan::New<String>("areacode").ToLocalChecked()  ),
                weatherstationcode( Nan::New<String>("weatherstationcode").ToLocalChecked()  ),
                weatherstationname( Nan::New<String>("weatherstationname").ToLocalChecked()  ),
                mcc( Nan::New<String>("mcc").ToLocalChecked()  ),
                mobilebrand( Nan::New<String>("mobilebrand").ToLocalChecked()  );

  const ::Map<IP2LDictionaryElement>::type &dict_map = dict.Children();
  Local<Array> index_array = CreateArrayResult(dict_map);
  Nan::Set(result, _index, index_array );

  uint32_t country_index = 0;

  for( ::Map<IP2LDictionaryElement>::type::const_iterator it = dict_map.begin();
                                                it != dict_map.end(); ++it ) {
    const IP2LDictionaryCountry *country = static_cast<
                                    const IP2LDictionaryCountry *>(it->second);
    Local<Object> country_result( Nan::New<Object>() );
    Local<String> short_name = Nan::Get(index_array, country_index++).ToLocalChecked().As<String>();
    Nan::Set(country_result, country_short, short_name );

    if ( (mask & IP2L_COUNTRY_LONG_MASK) != 0 ) {
      if ( country->SecondName() != NULL ) {
        Nan::Set(country_result, country_long,
                             Nan::New<String>( country->SecondName() ).ToLocalChecked() );
      } else {
        Nan::Set(country_result, country_long, Nan::Null() );
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
      Nan::Set(country_result, weatherstationcode, CreateArrayResult(
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

    Nan::Set(result, short_name, country_result );
  }

  return scope.Escape(result);
}

NAN_INLINE void Location::CreateDictionaryResultBranch(
                            const uint32_t mask,
                            const uint32_t branch_mask,
                            const uint32_t leaf_mask,
                            const ::Map<IP2LDictionaryElement>::type &branch_map,
                            const Handle<String> &indexLabel,
                            const Handle<String> &label,
                            Handle<Object> &result)
{
  if ( (mask & branch_mask) != 0 ) {
    Local<Array> branch_array = CreateArrayResult(branch_map);

    if ( (mask & leaf_mask) != 0 ) {
      Local<Object> branch_result( Nan::New<Object>() );
      Nan::Set(branch_result, indexLabel, branch_array );
      uint32_t branch_index = 0;
      for( ::Map<IP2LDictionaryElement>::type::const_iterator it = branch_map.begin();
                                          it != branch_map.end(); ++it ) {
        Nan::Set(branch_result,
                      Nan::Get(branch_array, branch_index++).ToLocalChecked(),
                      CreateArrayResult(
                        static_cast<const IP2LDictionaryBranch *>(it->second)->Children()) );
      }
      Nan::Set(result, label, branch_result );
    } else {
      Nan::Set(result, label, branch_array );
    }
  }
}

NAN_INLINE void Location::CreateDictionaryResultElement(
                            const uint32_t mask,
                            const uint32_t leaf_mask,
                            const ::Map<IP2LDictionaryElement>::type &leaf_map,
                            const Handle<String> &label,
                            Handle<Object> &result)
{
  if ( (mask & leaf_mask) != 0 ) {
    Nan::Set(result, label, CreateArrayResult(leaf_map) );
  }
}

Local<Array> Location::CreateArrayResult(
                              const ::Map<IP2LDictionaryElement>::type &dict_map)
{
  Nan::EscapableHandleScope scope;

  Local<Array> result( Nan::New<Array>( (uint32_t) dict_map.size() ) );

  uint32_t index = 0;

  for( ::Map<IP2LDictionaryElement>::type::const_iterator it = dict_map.begin();
                                      it != dict_map.end(); ++it ) {
    Nan::Set(result, index++, Nan::New<String>( it->second->Name() ).ToLocalChecked() );
  }
  return scope.Escape(result);
}
