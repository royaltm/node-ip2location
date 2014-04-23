
typedef struct IP2LDictionary {
  struct IP2LDictionary* next;
  struct IP2LDictionary* child;
  char* name;
  unsigned int level;
} IP2LDictionary;


IP2LDictionary *IP2LFindOrAddDictionaryElement(char *name, IP2LDictionary *parent);
int IP2LSaveAllDictionaries(char *dir);
int IP2LSaveDictionaries(char *dir, IP2LDictionary *cursor);
void IP2LFreeDictionary(IP2LDictionary *cursor);
