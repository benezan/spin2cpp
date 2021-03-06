#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "symbol.h"
#include "util/util.h"

#if 0
/* do case insensitive comparisons */
#define STRCMP strcasecmp
#else
#define STRCMP strcmp
#endif

/*
 * hash function
 */
unsigned
RawSymbolHash(const char *str)
{
    unsigned hash = 987654321;
    unsigned c;

    while (*str) {
        c = *str++;
        hash = hash * 65537;
        hash = hash ^ c;
    }
    return hash;
}

unsigned
SymbolHash(const char *str)
{
    unsigned hash = RawSymbolHash(str);
    return hash % SYMTABLE_HASH_SIZE;
}

/* find a symbol in the table */

Symbol *
FindSymbol(SymbolTable *table, const char *name)
{
    unsigned hash;
    Symbol *sym;

    hash = SymbolHash(name);
    sym = table->hash[hash];
    while (sym) {
        if (!STRCMP(sym->our_name, name)) {
            return sym;
        }
        sym = sym->next;
    }
    /* symbol was not found, give up */
    /* it's up to our caller to look through a containing context */
    return NULL;
}

/* find a symbol in a table or in any of its links */
/* code to find a symbol */
static Symbol *
doLookupSymbolInTable(SymbolTable *table, const char *name, int level)
{
    Symbol *sym = NULL;
    if (!table) return NULL;
    sym = FindSymbol(table, name);
    if (!sym) {
        if (table->next) {
           sym = doLookupSymbolInTable(table->next, name, level);
	}
    }
    if (sym && IsAlias(sym)) {
        // have to look it up again
        Symbol *alias;
        level++;
        if (level > 32) {
            //ERROR(NULL, "recursive definition for symbol %s", name);
            return NULL;
        }
        alias = doLookupSymbolInTable(table, (const char *)sym->val, level);
        if (alias) {
            return alias;
        } else {
            return sym;
        }
    }
    return sym;
}

/* find a symbol in a table or in any of its links */
/* code to find a symbol */
Symbol *
LookupSymbolInTable(SymbolTable *table, const char *name)
{
    return doLookupSymbolInTable(table, name, 0);
}

/*
 * find a symbol by offset
 * this is slow (we have to search the whole table) and
 * not guaranteed by us to be unique
 * Also note: if "origsym" is non-NULL then it's a pointer to
 * the last symbol we tried to find, and it's used to cycle
 * through RESULT, PARAMETER, LOCALS as Spin needs
 */
Symbol *
FindSymbolByOffsetAndKind(SymbolTable *table, int offset, int kind)
{
    Symbol *sym = NULL;
    int hash;
    for (hash = 0; hash < SYMTABLE_HASH_SIZE; hash++) {
        sym = table->hash[hash];
        while (sym) {
            if (sym->offset == offset && sym->kind == kind)
            {
                return sym;
            }
            sym = sym->next;
        }
    }
    /* could not find it */
    if (kind == SYM_RESULT) {
        /* look for first parameter */
        kind = SYM_PARAMETER;
        sym = FindSymbolByOffsetAndKind(table, 0, SYM_PARAMETER);
    }
    if (!sym && kind == SYM_PARAMETER) {
        sym = FindSymbolByOffsetAndKind(table, 0, SYM_LOCALVAR);
    }
    return sym;
}

/*
 * iterate over all symbols in a table
 */
void
IterateOverSymbols(SymbolTable *table, SymbolFunc func, void *arg)
{
    Symbol *sym;
    int hash;
    int more;
    
    for (hash = 0; hash < SYMTABLE_HASH_SIZE; hash++) {
        sym = table->hash[hash];
        while (sym) {
            more = func(sym, arg);
            if (!more) return;
            sym = sym->next;
        }
    }
}

/*
 * create a new symbol
 */
Symbol *
NewSymbol(void)
{
    Symbol *sym = (Symbol *)calloc(1, sizeof(*sym));
    return sym;
}

/*
 * add a symbol to the table
 * returns NULL if there was a conflict
 */
Symbol *
AddSymbol(SymbolTable *table, const char *name, int type, void *val, const char *user_name)
{
    unsigned hash;
    Symbol *sym;
    
    hash = SymbolHash(name);
    sym = table->hash[hash];
    while (sym) {
        if (!STRCMP(sym->our_name, name)) {
            if (sym->kind == SYM_WEAK_ALIAS) {
                // it's OK to override aliases
                break;
            }
            return NULL;
        }
        sym = sym->next;
    }

    if (!sym) {
        sym = NewSymbol();
        sym->next = table->hash[hash];
        table->hash[hash] = sym;
    } 
    sym->our_name = name;
    sym->user_name = user_name ? user_name : name;
    sym->kind = (Symtype)type;
    sym->val = val;
    sym->module = 0;
    return sym;
}

static int tmpvarnum = 1;
static int tmpvarmax = 99999;

/*
 * set the base and max for temporary variable names
 * returns the old base
 */
int
SetTempVariableBase(int base, int max)
{
    int oldbase = tmpvarnum;

    tmpvarnum = base;
    if (max > 0) {
        tmpvarmax = max;
    }
    return oldbase;
}

/*
 * create a temporary variable name
 */
char *
NewTemporaryVariable(const char *prefix)
{
    char *str;
    char buf[32];
    
    if (!prefix)
        prefix = "_tmp_";

    sprintf(buf, "_%04d", tmpvarnum);
    str = strdupcat(prefix, buf);
    tmpvarnum++;
    if (tmpvarnum > tmpvarmax) {
        fprintf(stderr, "Temporary variable limit of %d exceeded", tmpvarmax);
        abort();
    }
    return str;
}
