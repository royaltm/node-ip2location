#include "nodeip2ldictionary.h"

#include <stdlib.h>
#include <string.h>

IP2LDictionary::IP2LDictionary(const char *name_, const unsigned int level_)
    : name( strdup(name_) ), second_name(NULL), level(level_) {}

IP2LDictionary::~IP2LDictionary()
{
  free(name);
  free(second_name);
  for( int i = 0; i <= IP2L_DICT_TYPE_MAX; ++i ) {
    Map<IP2LDictionary>::type &child = children[i];
    for( Map<IP2LDictionary>::type::iterator it = child.begin();
                                        it != child.end(); ++it ) {
      delete it->second;
    }
    child.clear();
  }
}

IP2LDictionary *IP2LDictionary::FindOrAddDictionaryElement(char *name,
                                                           IP2L_DICT_TYPE type)
{
  return FindOrAddDictionaryElementMap(name, children[(int)type], level + 1);
}

IP2LDictionary *IP2LDictionary::FindOrAddDictionaryElementMap(char *name,
                                          Map<IP2LDictionary>::type &map,
                                          unsigned int level)
{
  if (name[0] == '-') return NULL;

  Map<IP2LDictionary>::type::iterator it = map.find(name);
  if ( it != map.end() ) {
    return it->second;
  }

  IP2LDictionary *elem = new IP2LDictionary(name, level);

  map.insert( Map<IP2LDictionary>::type::value_type(elem->name, elem) );

  return elem;
}

const char* IP2LDictionary::Name(void)
{
  return name;
}

const char* IP2LDictionary::SecondName(void)
{
  return second_name;
}

void IP2LDictionary::SecondName(const char *value)
{
  free(second_name);
  second_name = strdup(value);
}

unsigned int IP2LDictionary::Level(void)
{
  return level;
}

const Map<IP2LDictionary>::type &IP2LDictionary::Children(IP2L_DICT_TYPE type)
{
  return children[(int) type];
}
