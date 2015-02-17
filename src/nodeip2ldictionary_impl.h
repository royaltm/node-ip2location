#ifndef HAVE_NODEIP2L_DICTIONARY_H
# error 'nodeip2ldictionary_impl.h' is not supposed to be included directly.
#endif

/* IP2LDictionaryElement */

IP2LDictionaryElement::IP2LDictionaryElement(const char *name_)
    : name( strdup(name_) ) {}

IP2LDictionaryElement::~IP2LDictionaryElement()
{
  free(name);
}

NAN_INLINE const char* IP2LDictionaryElement::Name() const
{
  return name;
}

/* IP2LDictionary */

IP2LDictionary::IP2LDictionary() {}

IP2LDictionary::~IP2LDictionary()
{
  for( Map<IP2LDictionaryElement>::type::iterator it = children.begin();
                                      it != children.end(); ++it ) {
    delete it->second;
  }
  children.clear();
}

IP2LDictionaryCountry *IP2LDictionary::FindOrAddDictionaryCountry(
                                                              const char *name)
{
  return FindOrAddDictionaryElementMap<IP2LDictionaryCountry>(name, children);
}

template <class DictElement>
NAN_INLINE DictElement *IP2LDictionary::FindOrAddDictionaryElementMap(
                                        const char *name,
                                        Map<IP2LDictionaryElement>::type &map)
{
  if ( name[0] == '-' ) return NULL;

  Map<IP2LDictionaryElement>::type::iterator it = map.find(name);
  if ( it != map.end() ) {
    return (DictElement *) it->second;
  }

  DictElement *elem = new DictElement(name);

  map.insert( Map<IP2LDictionaryElement>::type::value_type(
                                                        elem->Name(), elem) );

  return elem;
}

NAN_INLINE const Map<IP2LDictionaryElement>::type &IP2LDictionary::Children() const
{
  return children;
}

/* IP2LDictionaryBranch */

IP2LDictionaryBranch::IP2LDictionaryBranch(const char *name)
    : IP2LDictionaryElement(name), IP2LDictionary() {}

void IP2LDictionaryBranch::AddUniqueDictionaryElement(const char *name)
{
  IP2LDictionary::FindOrAddDictionaryElementMap<IP2LDictionaryElement>(
                                              name, IP2LDictionary::children);
}

/* IP2LDictionaryCountry */

IP2LDictionaryCountry::IP2LDictionaryCountry(const char *name)
    : IP2LDictionaryElement(name), second_name(NULL) {}

IP2LDictionaryCountry::~IP2LDictionaryCountry()
{
  free(second_name);
  for( int i = 0; i <= IP2L_DICT_TYPE_MAX; ++i ) {
    Map<IP2LDictionaryElement>::type &child = children[i];
    for( Map<IP2LDictionaryElement>::type::iterator it = child.begin();
                                        it != child.end(); ++it ) {
      delete it->second;
    }
    child.clear();
  }
}

NAN_INLINE const char* IP2LDictionaryCountry::SecondName() const
{
  return second_name;
}

NAN_INLINE bool IP2LDictionaryCountry::NoSecondName() const
{
  return second_name == NULL;
}

void IP2LDictionaryCountry::SecondName(const char *name)
{
  free(second_name);
  second_name = strdup(name);
}

NAN_INLINE const Map<IP2LDictionaryElement>::type &IP2LDictionaryCountry::Children(
                                                    IP2L_DICT_TYPE type) const
{
  return children[(int) type];
}

void IP2LDictionaryCountry::AddUniqueDictionaryElement(char *name,
                                                       IP2L_DICT_TYPE type)
{
  IP2LDictionary::FindOrAddDictionaryElementMap<IP2LDictionaryElement>(
                                                    name, children[(int)type]);
}

IP2LDictionaryBranch *IP2LDictionaryCountry::FindOrAddDictionaryBranch(
                                                           char *name,
                                                           IP2L_DICT_TYPE type)
{
  return IP2LDictionary::FindOrAddDictionaryElementMap<IP2LDictionaryBranch>(
                                                    name, children[(int)type]);
}
