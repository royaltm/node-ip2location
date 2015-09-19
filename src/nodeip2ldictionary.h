#ifndef HAVE_NODEIP2L_DICTIONARY_H
#define HAVE_NODEIP2L_DICTIONARY_H

#include <stdlib.h>
#include <string.h>
#include <map>

typedef enum {
  IP2L_DICT_REGION_CITY    = 0,
  IP2L_DICT_WEATHERSTATION = 1,
  IP2L_DICT_MCC_MNC        = 2,
  IP2L_DICT_ISP            = 3,
  IP2L_DICT_DOMAIN         = 4,
  IP2L_DICT_ZIPCODE        = 5,
  IP2L_DICT_IDDCODE        = 6,
  IP2L_DICT_AREACODE       = 7,
  IP2L_DICT_MOBILEBRAND    = 8,
  IP2L_DICT_TYPE_MAX       = IP2L_DICT_MOBILEBRAND
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

class IP2LDictionaryElement {
  public:
    IP2LDictionaryElement(const char *name);
    virtual ~IP2LDictionaryElement(void);

    NAN_INLINE virtual const char* Name(void) const;

  private:
    char* name;

    IP2LDictionaryElement(const IP2LDictionaryElement&);
    void operator=(const IP2LDictionaryElement&);
};

class IP2LDictionaryCountry;
class IP2LDictionary {
  public:
    IP2LDictionary(void);
    virtual ~IP2LDictionary(void);
    NAN_INLINE virtual const ::Map<IP2LDictionaryElement>::type &Children(void)
                                                                         const;
    IP2LDictionaryCountry *FindOrAddDictionaryCountry(const char *name);

    template <class DictElement>
    NAN_INLINE static DictElement *FindOrAddDictionaryElementMap(
                                        const char *name,
                                        ::Map<IP2LDictionaryElement>::type &map);

  protected:
    ::Map<IP2LDictionaryElement>::type children;

  private:

    IP2LDictionary(const IP2LDictionary&);
    void operator=(const IP2LDictionary&);
};

class IP2LDictionaryBranch : public IP2LDictionaryElement,
                             public IP2LDictionary {
  public:
    IP2LDictionaryBranch(const char *name);
    void AddUniqueDictionaryElement(const char *name);

  private:

    IP2LDictionaryBranch(const IP2LDictionaryBranch&);
    void operator=(const IP2LDictionaryBranch&);
};

class IP2LDictionaryCountry : public IP2LDictionaryElement {
  public:
    IP2LDictionaryCountry(const char *name);
    virtual ~IP2LDictionaryCountry(void);
    void AddUniqueDictionaryElement(char *name, IP2L_DICT_TYPE type);
    IP2LDictionaryBranch *FindOrAddDictionaryBranch(char *name,
                                                          IP2L_DICT_TYPE type);
    NAN_INLINE const ::Map<IP2LDictionaryElement>::type &Children(
                                                    IP2L_DICT_TYPE type) const;
    NAN_INLINE const char* SecondName(void) const;
    NAN_INLINE bool NoSecondName(void) const;
    void SecondName(const char *value);

  private:
    char* second_name;
    ::Map<IP2LDictionaryElement>::type children[IP2L_DICT_TYPE_MAX + 1];

    IP2LDictionaryCountry(const IP2LDictionaryCountry&);
    void operator=(const IP2LDictionaryCountry&);
};

#include "nodeip2ldictionary_impl.h"

#endif /* HAVE_NODEIP2L_DICTIONARY_H */
