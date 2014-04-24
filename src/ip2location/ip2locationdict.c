#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "ip2locationdict.h"

static IP2LDictionary *dict_root = NULL;

#define DICT_FILE_NAME_COUNTRIES "0_countries"
#define DICT_FILE_NAME_REGIONS "1_regions"
#define DICT_FILE_NAME_CITIES "2_cities"
#define DICT_FILE_EXT ".txt"

IP2LDictionary *IP2LCreateDictionaryElement(char *name, unsigned int level) {
  IP2LDictionary *dict = malloc(sizeof(IP2LDictionary));
  memset(dict, 0, sizeof(IP2LDictionary));
  dict->name = strdup(name);
  dict->level = level;
  return dict;
}
IP2LDictionary *IP2LFindOrAddDictionaryElement(char *name, IP2LDictionary *parent) {
  if (name[0] == '-') return NULL;
  IP2LDictionary *cursor, *found = NULL;
  size_t nlen = strlen(name);
  unsigned int level;
  int res = 1;
  if (parent != NULL) {
    cursor = parent->child;
    level = parent->level + 1;
  } else {
    cursor = dict_root;
    level = 0;
  }
  while (cursor != NULL && (res = strncmp(cursor->name, name, nlen)) < 0) {
    found = cursor;
    cursor = cursor->next;
  }
  if (res != 0) {
    if (found == NULL) {
      found = IP2LCreateDictionaryElement(name, level);
      found->next = cursor;
      if (parent != NULL) {
        parent->child = found;
      } else {
        dict_root = found;
      }
      return found;
    } else {
      cursor = IP2LCreateDictionaryElement(name, level);
      cursor->next = found->next;
      found->next = cursor;
    }
  }
  return cursor;
}

int IP2LSaveDictionary(IP2LDictionary *cursor, char *name) {
  int count = 0;
  size_t nlen = 0;
  static char eol[1] = { '\n' };
  FILE *file = fopen(name, "w");
  if (file == NULL)
    return -1;
  while (cursor != NULL) {
    nlen = strlen(cursor->name);
    if (fwrite(cursor->name, sizeof(char), nlen, file) == nlen &&
        fwrite(eol, sizeof(char), 1, file) == 1) {
      count += 1;
      cursor = cursor->next;
    } else {
      fclose(file);
      return -2;
    }
  }
  fclose(file);
  return count;
}

char *IP2LCreateName(char *dir, char *name, char *ext) {
  char *filename = malloc(strlen(dir) + 1 + strlen(name) + (ext != NULL ? strlen(ext) : 0) + 1);
  strcpy(filename, dir);
  strcat(filename, "/");
  strcat(filename, name);
  if (ext != NULL) strcat(filename, ext);
  return filename;
}

int IP2LSaveAllDictionaries(char *dir) {
  return IP2LSaveDictionaries(dir, dict_root);
}
int IP2LSaveDictionaries(char *dir, IP2LDictionary *cursor) {
  char *name;
  int ret;
  int count = 0;
  if (cursor == NULL) return count;
  switch(cursor->level) {
    case 0: name = DICT_FILE_NAME_COUNTRIES; break;
    case 1: name = DICT_FILE_NAME_REGIONS; break;
    case 2: name = DICT_FILE_NAME_CITIES; break;
    default:
      return -3;
  }
  name = IP2LCreateName(dir, name, DICT_FILE_EXT);
  ret = IP2LSaveDictionary(cursor, name);
  free(name);
  if (ret < 0) return ret;
  count += ret;
  if (cursor->level < 2) {
    do {
      name = IP2LCreateName(dir, cursor->name, NULL);
      if (mkdir(name, 0777) && errno != EEXIST) {
        free(name);
        return -1;
      }
      ret = IP2LSaveDictionaries(name, cursor->child);
      free(name);
      if (ret < 0) return ret;
      count += ret;
    } while ((cursor = cursor->next));
  }
  return count;
}

void IP2LFreeDictionary(IP2LDictionary *cursor) {
  IP2LDictionary *next;
  if (cursor == NULL) cursor = dict_root;
  while (cursor != NULL) {
    free(cursor->name);
    if (cursor->child != NULL) {
      IP2LFreeDictionary(cursor->child);
    }
    next = cursor->next;
    free(cursor);
    cursor = next;
  }
  dict_root = NULL;
}
