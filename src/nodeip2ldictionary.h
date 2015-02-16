#ifndef HAVE_NODEIP2L_DICTIONARY_H
#define HAVE_NODEIP2L_DICTIONARY_H

#include <string.h>
#include <map>

typedef enum {
  IP2L_DICT_REGION_CITY = 0,
  IP2L_DICT_ISP         = 1,
  IP2L_DICT_DOMAIN      = 2,
  IP2L_DICT_ZIPCODE     = 3,
  IP2L_DICT_IDDCODE     = 4,
  IP2L_DICT_AREACODE    = 5,
  IP2L_DICT_TYPE_MAX    = IP2L_DICT_AREACODE
} IP2L_DICT_TYPE;

struct char_cmp {
  bool operator () (const char *a, const char *b) const
  {
    return strcmp(a,b) < 0;
  }
};

template <class T>
struct Map {
  typedef std::map<const char *, T *, char_cmp> type;
};

class IP2LDictionary {
  public:
    IP2LDictionary(const char *name, const unsigned int level);
    virtual ~IP2LDictionary(void);
    IP2LDictionary *FindOrAddDictionaryElement(char *name, IP2L_DICT_TYPE type);
    static IP2LDictionary *FindOrAddDictionaryElementMap(char *name,
                                          Map<IP2LDictionary>::type &map,
                                          unsigned int level = 0);

    const char* Name(void);
    const char* SecondName(void);
    void SecondName(const char *value);
    unsigned int Level(void);
    const Map<IP2LDictionary>::type &Children(IP2L_DICT_TYPE type);

  private:
    char* name;
    char* second_name;
    unsigned int level;
    Map<IP2LDictionary>::type children[IP2L_DICT_TYPE_MAX + 1];

    IP2LDictionary(const IP2LDictionary&);
    void operator=(const IP2LDictionary&);
};

#endif /* HAVE_NODEIP2L_DICTIONARY_H */
