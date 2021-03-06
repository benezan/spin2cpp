/*
 * Spin to C/C++ translator
 * Copyright 2011-2020 Total Spectrum Software Inc.
 * 
 * +--------------------------------------------------------------------
 * ¦  TERMS OF USE: MIT License
 * +--------------------------------------------------------------------
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * +--------------------------------------------------------------------
 */
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "common.h"
#include "preprocess.h"
#include "version.h"

/* declarations common to all front ends */

Module *current;
Module *allparse;
Module *globalModule;
SymbolTable *currentTypes;

int gl_p2;
int gl_errors;
int gl_output;
int gl_outputflags;
int gl_nospin;
int gl_gas_dat;
int gl_normalizeIdents;
int gl_debug;
int gl_expand_constants;
int gl_optimize_flags;
int gl_dat_offset;
int gl_exit_status = 0;
int gl_printprogress = 0;
int gl_infer_ctypes = 0;
int gl_listing = 0;
int gl_fixedreal = 0;
unsigned int gl_hub_base = 0x400;
int gl_no_coginit = 0;
int gl_lmm_kind = LMM_KIND_ORIG;
int gl_relocatable = 0;

// bytes and words are unsigned by default, but longs are signed by default
// this is confusing, and someday we should make all of these explicit
// but it's a legacy from Spin
AST *ast_type_word, *ast_type_long, *ast_type_byte;
AST *ast_type_signed_word, *ast_type_signed_byte;
AST *ast_type_unsigned_long;
AST *ast_type_float, *ast_type_string;
AST *ast_type_ptr_long;
AST *ast_type_ptr_word;
AST *ast_type_ptr_byte;
AST *ast_type_ptr_void;
AST *ast_type_generic;
AST *ast_type_const_generic;
AST *ast_type_void;
AST *ast_type_bitfield;
AST *ast_type_long64, *ast_type_unsigned_long64;
AST *ast_type_generic_funcptr;
AST *ast_type_sendptr;

const char *gl_progname = "spin2cpp";
char *gl_header1 = NULL;
char *gl_header2 = NULL;

typedef struct alias {
    const char *name;
    const char *alias;
} Aliases;

Aliases spinalias[] = {
    { "call", "_call" },
    { "clkfreq", "__clkfreq_var" },
    { "clkmode", "__clkmode_var" },
    { "clkset", "_clkset" },
    { "strsize", "__builtin_strlen" },
#ifdef NEVER    
    { "lockclr", "__builtin_lockclr" },
    { "lockset", "__builtin_lockset" },
    { "locknew", "__builtin_locknew" },
    { "lockret", "__builtin_lockret" },
#endif

    { "_dirl", "__builtin_propeller_dirl" },
    { "_dirh", "__builtin_propeller_dirh" },
    { "_dirnot", "__builtin_propeller_dirnot" },
    { "_dir", "__builtin_propeller_dir" },

    { "_pinw", "__builtin_propeller_drv" },
    { "_pinl", "__builtin_propeller_drvl" },    
    { "_pinh", "__builtin_propeller_drvh" },
    { "_pinnot", "__builtin_propeller_drvnot" },
    { "_pinr", "__builtin_propeller_pinr" },

    { "reboot", "_reboot" },

    { "send", "__sendptr" },
    
    { "_waitx", "__builtin_propeller_waitx" },
    
    /* obsolete aliases */
    { "dirl_", "__builtin_propeller_dirl" },
    { "dirh_", "__builtin_propeller_dirh" },
    { "dirnot_", "__builtin_propeller_dirnot" },
    { "dir_", "__builtin_propeller_dir" },
    { "drvl_", "__builtin_propeller_drvl" },
    { "drvh_", "__builtin_propeller_drvh" },
    { "drvnot_", "__builtin_propeller_drvnot" },
    { "drv_", "__builtin_propeller_drv" },
    { "waitx_", "__builtin_propeller_waitx" },

    { NULL, NULL },
};
Aliases spin2alias[] = {
    { "cnt", "_getcnt" },

    { "locktry", "_locktry" },
    { "lockrel", "lockclr" },
    
    { "pinw", "_pinwrite" },
    { "pinl", "__builtin_propeller_drvl" },    
    { "pinh", "__builtin_propeller_drvh" },
    { "pint", "__builtin_propeller_drvnot" },
    { "pinr", "__builtin_propeller_pinr" },
    { "pinf", "__builtin_propeller_fltl" },
    { "pinstart", "_pinstart" },
    { "pinsetup", "_pinsetup" },
    { "pinclear", "_pinclear" },
    
//    { "pinrnd", "__builtin_propeller_drvrnd" },
    { "pinwrite", "_pinwrite" },
    { "pinlow", "__builtin_propeller_drvl" },    
    { "pinhigh", "__builtin_propeller_drvh" },
    { "pintoggle", "__builtin_propeller_drvnot" },
    { "pinread", "__builtin_propeller_pinr" },
    { "pinfloat", "__builtin_propeller_fltl" },
    { "pinmode", "_pinmode" },
    
    { "getct", "_getcnt" },

    { "wrpin", "_wrpin" },
    { "wxpin", "_wxpin" },
    { "wypin", "_wypin" },
    
    { "akpin", "__builtin_propeller_akpin" },
    { "rdpin", "__builtin_propeller_rdpin" },
    { "rqpin", "__builtin_propeller_rqpin" },

    { "rotxy", "_rotxy" },
    { "polxy", "_polxy" },
    { "xypol", "_xypol" },

    { "cogatn", "__builtin_propeller_cogatn" },
    { "muldiv64", "_muldiv64" },
    { "waitx", "__builtin_propeller_waitx" },
    { "waitms", "_waitms" },
    { "waitus", "_waitus" },
    { "waitct", "waitcnt" },
    
    /* for C usage */
    { "_pinf", "__builtin_propeller_fltl" },
    { "_pinrnd", "__builtin_propeller_drvrnd" },

    /* obsolete aliases */
    { "dirrnd_", "__builtin_propeller_dirrnd" },
    { "drvrnd_", "__builtin_propeller_drvrnd" },
    { "outl_", "__builtin_propeller_outl" },
    { "outh_", "__builtin_propeller_outh" },
    { "outrnd_", "__builtin_propeller_outrnd" },
    { "outnot_", "__builtin_propeller_outnot" },
    { "out_", "__builtin_propeller_out" },

    { "fltl_", "__builtin_propeller_fltl" },
    { "flth_", "__builtin_propeller_flth" },
    { "fltrnd_", "__builtin_propeller_fltrnd" },
    { "fltnot_", "__builtin_propeller_fltnot" },
    { "flt_", "__builtin_propeller_flt" },

    { "wrpin_", "__builtin_propeller_wrpin" },
    { "wxpin_", "__builtin_propeller_wxpin" },
    { "wypin_", "__builtin_propeller_wypin" },
    { NULL, NULL },
};
Aliases basicalias[] = {
    /* ugh, not sure if we want to keep supporting the clk* variables,
     * but changing them to functions is difficult
     */
    { "clkfreq", "__clkfreq_var" },
    { "clkmode", "__clkmode_var" },
    /* the rest of these are OK, I think */
    { "clkset", "_clkset" },
    { "err", "_geterror" },
    { "getcnt",  "_getcnt" },
    { "len", "__builtin_strlen" },
    { "pausems", "_waitms" },
    { "pauseus", "_waitus" },
    { "pinlo", "__builtin_propeller_drvl" },
    { "pinhi", "__builtin_propeller_drvh" },
    { "pinset", "__builtin_propeller_drv" },
    { "pintoggle", "__builtin_propeller_drvnot" },
    { "rnd", "_basic_rnd" },
    { "val", "__builtin_atof" },
    { "val%", "__builtin_atoi" },

    /* extra string functions */
    { "instr", "_instr" },
    { "instrrev", "_instrrev" },
    
    /* math functions */
    { "cos", "__builtin_cosf" },
    { "exp", "__builtin_expf" },
    { "log", "__builtin_logf" },
    { "pow", "__builtin_powf" },
    { "sin", "__builtin_sinf" },
    { "tan", "__builtin_tanf" },
    
    { NULL, NULL },
};
Aliases calias[] = {
    /* these are obsolete but we'll support them for now */
    { "_clkfreq", "__clkfreq_var" },
    { "_clkmode", "__clkmode_var" },

    /* new propeller2.h standard */
    { "_cnt",  "_getcnt" },
    { "_clockfreq", "__builtin_clkfreq" },
    { "_clockmode", "__builtin_clkmode" },
    { "_dirl", "__builtin_propeller_dirl" },
    { "_dirh", "__builtin_propeller_dirh" },
    { "_dirnot", "__builtin_propeller_dirnot" },
    { "_dirw", "__builtin_propeller_dir" },
    { "_pinl", "__builtin_propeller_drvl" },
    { "_pinh", "__builtin_propeller_drvh" },
    { "_pinnot", "__builtin_propeller_drvnot" },
    { "_pinw", "__builtin_propeller_drv" },
    { "_waitx", "__builtin_propeller_waitx" },
    { "_pinr", "__builtin_propeller_pinr" },

    { "_wrpin", "__builtin_propeller_wrpin" },
    { "_wxpin", "__builtin_propeller_wxpin" },
    { "_wypin", "__builtin_propeller_wypin" },

    { "_rdpin", "__builtin_propeller_rdpin" },
    
    { NULL, NULL },
};

//
// create aliases appropriate to the language
//
static void
addAliases(SymbolTable *tab, Aliases *A)
{
    while (A && A->name) {
        AddSymbol(tab, A->name, SYM_WEAK_ALIAS, (void *)A->alias, NULL);
        A++;
    }
}

static void
initSymbols(Module *P, int language)
{
    Aliases *A;
    if (IsBasicLang(language)) {
        A = basicalias;
    } else if (IsCLang(language)) {
        A = calias;
    } else {
        A = spinalias;
    }
    addAliases(&P->objsyms, A);
    if (gl_p2 && (IsBasicLang(language)||IsSpinLang(language))) {
        addAliases(&P->objsyms, spin2alias);
    } else if (language == LANG_SPIN_SPIN2) {
        addAliases(&P->objsyms, spin2alias);
    }
}

/*
 * allocate a new parser state
 */ 
Module *
NewModule(const char *fullname, int language)
{
    Module *P;
    char *s;
    char *root;

    P = (Module *)calloc(1, sizeof(*P));
    if (!P) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    P->mainLanguage = P->curLanguage = language;
    P->longOnly = 1;
    /* set up the base file name */
    P->fullname = fullname;
    P->basename = strdup(fullname);
    s = strrchr(P->basename, '.');
    if (s) *s = 0;
    root = strrchr(P->basename, '/');
#if defined(WIN32) || defined(WIN64)
    if (!root) {
      root = strrchr(P->basename, '\\');
    }
#endif
    if (!root)
      root = P->basename;
    else
      P->basename = root+1;
    /* set up the class name */
    P->classname = (char *)calloc(1, strlen(P->basename)+1);
    strcpy(P->classname, P->basename);

    /* link the global symbols */
    if (globalModule) {
        P->objsyms.next = &globalModule->objsyms;
    } else if (IsBasicLang(language)) {
        P->objsyms.next = &basicReservedWords;
    } else if (IsCLang(language)) {
        P->objsyms.next = &cReservedWords;
    } else {
        P->objsyms.next = &spinReservedWords;
    }
    initSymbols(P, language);
    P->body = NULL;
    return P;
}

/*
 * declare constant symbols
 */
static Symbol *
EnterConstant(const char *name, AST *expr)
{
    Symbol *sym;

    // check to see if the symbol already has a definition
    sym = FindSymbol(&current->objsyms, name);
    if (sym) {
        if (sym->kind == SYM_CONSTANT || sym->kind == SYM_FLOAT_CONSTANT) {
            int32_t origval;
            int32_t newval;

            origval = EvalConstExpr(sym->val);
            newval = EvalConstExpr(expr);
            if (origval != newval) {
                ERROR(expr, "Redefining %s with a different value", name);
                return NULL;
            }
            return NULL; /* did not create new symbol */
        }
    }
    if (IsFloatConst(expr)) {
        sym = AddSymbol(&current->objsyms, name, SYM_FLOAT_CONSTANT, (void *)expr, NULL);
    } else {
        sym = AddSymbol(&current->objsyms, name, SYM_CONSTANT, (void *)expr, NULL);
    }
    return sym;
}

void
DeclareConstants(AST **conlist_ptr)
{
    AST *conlist;
    AST *upper, *ast, *id;
    AST *next;
    AST *completed_declarations = NULL;
    int default_val;
    int default_val_ok = 0;
    int n;
    
    conlist = *conlist_ptr;

    // first do all the simple assignments
    // this is necessary because Spin will sometimes allow out-of-order
    // assignments

    do {
        n = 0; // no assignments yet
        default_val = 0;
        default_val_ok = 1;
        upper = conlist;
        while (upper) {
            next = upper->right;
            if (upper->kind == AST_LISTHOLDER) {
                ast = upper->left;
                if (ast->kind == AST_COMMENTEDNODE)
                    ast = ast->left;

                switch (ast->kind) {
                case AST_ASSIGN:
                    if (IsConstExpr(ast->right)) {
                        if (!IsIdentifier(ast->left)) {
                            ERROR(ast, "Internal error, bad constant declaration");
                            return;
                        }                            
                        EnterConstant(GetIdentifierName(ast->left), ast->right);
                        n++;
                        // now pull the assignment out so we don't see it again
                        RemoveFromList(conlist_ptr, upper);
                        upper->right = NULL;
                        completed_declarations = AddToList(completed_declarations, upper);
                        conlist = *conlist_ptr;
                    } else {
                        AST *typ;
                        typ = ExprType(ast->right);
                        if (typ && (IsStringType(typ) || IsPointerType(typ))) {
                            if (!IsIdentifier(ast->left)) {
                                ERROR(ast, "Internal error, bad constant declaration");
                                return;
                            }
                            typ = NewAST(AST_MODIFIER_CONST, typ, NULL);
                            // pull it out and put it in the DAT section
                            DeclareOneGlobalVar(current, ast, typ, 1);
                            RemoveFromList(conlist_ptr, upper);
                            upper->right = NULL;
                            conlist = *conlist_ptr;
                        }
                    }
                    break;
                case AST_ENUMSET:
                    if (IsConstExpr(ast->left)) {
                        default_val = EvalConstExpr(ast->left);
                        default_val_ok = 1;
                        RemoveFromList(conlist_ptr, upper);
                        upper->right = NULL;
                        completed_declarations = AddToList(completed_declarations, upper);
                        conlist = *conlist_ptr;
                    } else {
                        default_val_ok = 0;
                    }
                    break;
                case AST_ENUMSKIP:
                    if (default_val_ok) {
                        id = ast->left;
                        if (id->kind != AST_IDENTIFIER) {
                            ERROR(ast, "Internal error, expected identifier in constant list");
                        } else {
                            EnterConstant(id->d.string, AstInteger(default_val));
                            default_val += EvalConstExpr(ast->right);
                        }
                        n++;
                        // now pull the assignment out so we don't see it again
                        RemoveFromList(conlist_ptr, upper);
                        upper->right = NULL;
                        completed_declarations = AddToList(completed_declarations, upper);
                        conlist = *conlist_ptr;
                    }
                    break;
                case AST_IDENTIFIER:
                    if (default_val_ok) {
                        EnterConstant(ast->d.string, AstInteger(default_val));
                        default_val++;
                        n++;
                        // now pull the assignment out so we don't see it again
                        RemoveFromList(conlist_ptr, upper);
                        upper->right = NULL;
                        completed_declarations = AddToList(completed_declarations, upper);
                        conlist = *conlist_ptr;
                    }
                    break;
                default:
                    /* do nothing */
                    break;
                }
            }
            upper = next;
        }
    } while (n > 0);

    default_val = 0;
    default_val_ok = 1;
    for (upper = conlist; upper; upper = upper->right) {
        if (upper->kind == AST_LISTHOLDER) {
            ast = upper->left;
            if (ast->kind == AST_COMMENTEDNODE)
                ast = ast->left;
            switch (ast->kind) {
            case AST_ENUMSET:
                default_val = EvalConstExpr(ast->left);
                break;
            case AST_IDENTIFIER:
                EnterConstant(ast->d.string, AstInteger(default_val));
                default_val++;
                break;
            case AST_ENUMSKIP:
                id = ast->left;
                if (id->kind != AST_IDENTIFIER) {
                    ERROR(ast, "Internal error, expected identifier in constant list");
                } else {
                    EnterConstant(id->d.string, AstInteger(default_val));
                    default_val += EvalConstExpr(ast->right);
                }
                break;
            case AST_ASSIGN:
                EnterConstant(ast->left->d.string, ast->right);
                default_val = EvalConstExpr(ast->right) + 1;
                break;
            case AST_COMMENT:
                /* just skip it for now */
                break;
            default:
                ERROR(ast, "Internal error: bad AST value %d", ast->kind);
                break;
            }
        } else {
            ERROR(upper, "Expected list in constant, found %d instead", upper->kind);
        }
    }
    completed_declarations = AddToList(completed_declarations, conlist);
    *conlist_ptr = completed_declarations;
}

#if 0
/*
 * transform AST_SRCCOMMENTs into comments with the line data of the 
 * next non-comment AST
 */
static void
TransformSrcCommentsBlock(AST *ast, AST *upper)
{
    while (ast) {
        if (ast->kind == AST_SRCCOMMENT) {
            ast->kind = AST_COMMENT;
            ast->d.string = "+++";
        }
        TransformSrcCommentsBlock(ast->left, ast);
        upper = ast;
        ast = ast->right;
    }
}
#endif

AST *
NewObject(AST *identifier, AST *string)
{
    AST *ast;
    const char *filename;

    if (string) {
        filename = string->d.string;
    } else {
        filename = NULL;
    }

    ast = NewAST(AST_OBJECT, identifier, NULL);
    if (filename) {
        ast->d.ptr = ParseFile(filename);
    }
    return ast;
}

AST *
NewAbstractObject(AST *identifier, AST *string)
{
    return NewObject( NewAST(AST_OBJDECL, identifier, 0), string );
}

void
ERRORHEADER(const char *fileName, int lineno, const char *msg)
{
    if (fileName && lineno)
        fprintf(stderr, "%s:%d: %s: ", fileName, lineno, msg);
    else
        fprintf(stderr, "%s: ", msg);

}

void
ERROR(AST *instr, const char *msg, ...)
{
    va_list args;
    LineInfo *info = GetLineInfo(instr);

    if (info)
        ERRORHEADER(info->fileName, info->lineno, "error");
    else
        ERRORHEADER(NULL, 0, "error");

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
    gl_errors++;
}

void
SYNTAX_ERROR(const char *msg, ...)
{
    va_list args;

    if (current)
        ERRORHEADER(current->Lptr->fileName, current->Lptr->lineCounter, "error");
    else
        ERRORHEADER(NULL, 0, "error");

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
    gl_errors++;
}

void
WARNING(AST *instr, const char *msg, ...)
{
    va_list args;
    LineInfo *info = GetLineInfo(instr);

    if (info)
        ERRORHEADER(info->fileName, info->lineno, "warning");
    else
        ERRORHEADER(NULL, 0, "warning: ");

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void
NOTE(AST *instr, const char *msg, ...)
{
    va_list args;
    LineInfo *info = GetLineInfo(instr);

    if (info)
        ERRORHEADER(info->fileName, info->lineno, "note");
    else
        ERRORHEADER(NULL, 0, "note: ");

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void
ERROR_UNKNOWN_SYMBOL(AST *ast)
{
    const char *name;
    Label *labelref;
    
    if (IsIdentifier(ast)) {
        name = GetVarNameForError(ast);
    } else if (ast->kind == AST_VARARGS || ast->kind == AST_VA_START) {
        name = "__vararg";
    } else {
        name = "";
    }
    ERROR(ast, "Unknown symbol %s", name);
    // add a definition for this symbol so we don't get this error again
    if (curfunc) {
        AddLocalVariable(curfunc, ast, NULL, SYM_LOCALVAR);
    } else {
        labelref = (Label *)calloc(1, sizeof(*labelref));
        AddSymbol(&globalModule->objsyms, name, SYM_LABEL, labelref, NULL);
    }
}

AST *
GenericFunctionPtr(int numresults)
{
    AST *fptr = NULL;
    AST *exprlist = NULL;

    if (numresults == 0) {
        exprlist = ast_type_void;
    } else if (numresults == 1) {
        exprlist = NULL;
    } else {
        while (numresults > 0) {
            exprlist = NewAST(AST_TUPLE_TYPE, NULL, exprlist);
            --numresults;
        }
    }
    fptr = NewAST(AST_FUNCTYPE, exprlist, NULL);
    fptr = NewAST(AST_PTRTYPE, fptr, NULL);
    return fptr;
}

void
Init()
{
    ast_type_long64 = NewAST(AST_INTTYPE, AstInteger(8), NULL);
    ast_type_long = NewAST(AST_INTTYPE, AstInteger(4), NULL);
    ast_type_word = NewAST(AST_UNSIGNEDTYPE, AstInteger(2), NULL);
    ast_type_byte = NewAST(AST_UNSIGNEDTYPE, AstInteger(1), NULL);

    ast_type_unsigned_long64 = NewAST(AST_UNSIGNEDTYPE, AstInteger(8), NULL);
    ast_type_unsigned_long = NewAST(AST_UNSIGNEDTYPE, AstInteger(4), NULL);
    ast_type_signed_word = NewAST(AST_INTTYPE, AstInteger(2), NULL);
    ast_type_signed_byte = NewAST(AST_INTTYPE, AstInteger(1), NULL);

    ast_type_float = NewAST(AST_FLOATTYPE, AstInteger(4), NULL);
    ast_type_generic = NewAST(AST_GENERICTYPE, AstInteger(4), NULL);
    ast_type_const_generic = NewAST(AST_MODIFIER_CONST, ast_type_generic, NULL);
    ast_type_void = NewAST(AST_VOIDTYPE, AstInteger(0), NULL);

    ast_type_ptr_long = NewAST(AST_PTRTYPE, ast_type_long, NULL);
    ast_type_ptr_word = NewAST(AST_PTRTYPE, ast_type_word, NULL);
    ast_type_ptr_byte = NewAST(AST_PTRTYPE, ast_type_byte, NULL);
    ast_type_ptr_void = NewAST(AST_PTRTYPE, ast_type_void, NULL);

    ast_type_bitfield = NewAST(AST_BITFIELD, NULL, NULL);
    
    // string is pointer to const byte
    ast_type_string = NewAST(AST_PTRTYPE, NewAST(AST_MODIFIER_CONST, ast_type_byte, NULL), NULL);

    // a generic function returning a single (unknown) value
    ast_type_generic_funcptr = GenericFunctionPtr(1);

    // a generic function for Spin2 SEND type functionality
    ast_type_sendptr = NewAST(AST_MODIFIER_SEND_ARGS, GenericFunctionPtr(0), NULL);
    
    initSpinLexer(gl_p2);

    /* fill in the global symbol table */
    InitGlobalModule();
}

const char *
FindLastDirectoryChar(const char *fname)
{
    const char *found = NULL;
    if (!fname) return NULL;
    while (*fname) {
        if (*fname == '/'
#ifdef WIN32
            || *fname == '\\'
#endif
            )
        {
            found = fname;
        }
        fname++;
    }
    return found;
}

//
// use the directory portion of "directory" (if any) and then add
// on the basename
//
char *
ReplaceDirectory(const char *basename, const char *directory)
{
  char *ret = (char *)malloc(strlen(basename) + strlen(directory) + 2);
  char *dot;
  if (!ret) {
    fprintf(stderr, "FATAL: out of memory\n");
    exit(2);
  }
  strcpy(ret, directory);
  dot = (char *)FindLastDirectoryChar(ret);
  if (dot) {
      *dot++ = '/';
  } else {
      dot = ret;
  }
  strcpy(dot, basename);
  return ret;
}

char *
ReplaceExtension(const char *basename, const char *extension)
{
  char *ret = (char *)malloc(strlen(basename) + strlen(extension) + 1);
  char *dot;
  if (!ret) {
    fprintf(stderr, "FATAL: out of memory\n");
    exit(2);
  }
  strcpy(ret, basename);
  dot = strrchr(ret, '/');
  if (!dot) dot = ret;
  dot = strrchr(dot, '.');
  if (dot) *dot = 0;
  strcat(ret, extension);
  return ret;
}

//
// append an extension
//
char *
AddExtension(const char *basename, const char *extension)
{
    char *ret = (char *)malloc(strlen(basename) + strlen(extension) + 1);
    if (!ret) {
      fprintf(stderr, "FATAL: out of memory\n");
      exit(2);
    }
    strcpy(ret, basename);
    strcat(ret, extension);
    return ret;
}

//
// add a propeller checksum to a binary file
// may also pad the image out to form a .eeprom
// image, if eepromSize is non-zero
//
int
DoPropellerChecksum(const char *fname, size_t eepromSize)
{
    FILE *f = fopen(fname, "r+b");
    unsigned char checksum = 0;
    int c, r;
    size_t len;
    size_t padbytes;

    if (gl_p2) return 0; // no checksum required
    
    if (!f) {
        fprintf(stderr, "checksum: ");
        perror(fname);
        return -1;
    }
    fseek(f, 0L, SEEK_END);
    len = ftell(f);  // find length of file
    // pad file to multiple of 4, if necessary
    padbytes = ((len + 3) & ~3) - len;
    if (padbytes) {
        while (padbytes > 0) {
            fputc(0, f);
            padbytes--;
            len++;
        }
    }
    // update header fields
    fseek(f, 8L, SEEK_SET); // seek to 16 bit vbase field
    fputc(len & 0xff, f);
    fputc( (len >> 8) & 0xff, f);
    // update dbase = vbase + 2 * sizeof(int)
    fputc( (len+8) & 0xff, f);
    fputc( ((len+8)>>8) & 0xff, f);
    // update initial program counter
    fputc( 0x18, f );
    fputc( 0x00, f );
    // update dcurr = dbase + 2 * sizeof(int)
    fputc( (len+16) & 0xff, f);
    fputc( ((len+16)>>8) & 0xff, f);

    fseek(f, 0L, SEEK_SET);
    for(;;) {
        c = fgetc(f);
        if (c < 0) break;
        checksum += (unsigned char)c;
    }
    fflush(f);
    checksum = 0x14 - checksum;
    r = fseek(f, 5L, SEEK_SET);
    if (r != 0) {
        perror("fseek");
        return -1;
    }
    //printf("writing checksum 0x%x\n", checksum);
    r = fputc(checksum, f);
    if (r < 0) {
        perror("fputc");
        return -1;
    }
    fflush(f);
    if (eepromSize && eepromSize >= len + 8) {
        fseek(f, 0L, SEEK_END);
        fputc(0xff, f); fputc(0xff, f);
        fputc(0xf9, f); fputc(0xff, f);
        fputc(0xff, f); fputc(0xff, f);
        fputc(0xf9, f); fputc(0xff, f);
        len += 8;
        while (len < eepromSize) {
            fputc(0, f);
            len++;
        }
    }
    fclose(f);
    return 0;
}

//
// check the version
//
void
CheckVersion(const char *str)
{
    int majorVersion = 0;
    int minorVersion = 0;
    int revVersion = 0;

    while (isdigit(*str)) {
        majorVersion = 10 * majorVersion + (*str - '0');
        str++;
    }
    if (*str == '.') {
        str++;
        while (isdigit(*str)) {
            minorVersion = 10 * minorVersion + (*str - '0');
            str++;
        }
    }
    if (*str == '.') {
        str++;
        while (isdigit(*str)) {
            revVersion = 10 * revVersion + (*str - '0');
            str++;
        }
    }

    if (majorVersion < VERSION_MAJOR)
        return; // everything OK
    if (majorVersion == VERSION_MAJOR) {
        if (minorVersion < VERSION_MINOR)
            return; // everything OK
        if (minorVersion == VERSION_MINOR) {
            if (revVersion <= VERSION_REV) {
                return;
            }
        }
    }
    fprintf(stderr, "ERROR: required version %d.%d.%d but current version is %s\n", majorVersion, minorVersion, revVersion, VERSIONSTR);
    exit(1);
}

AST *
NewCommentedInstr(AST *instr)
{
    AST *comment = GetComments();
    AST *ast;

    ast = NewAST(AST_INSTRHOLDER, instr, NULL);
    if (comment && (comment->d.string || comment->kind == AST_SRCCOMMENT)) {
        ast = NewAST(AST_COMMENTEDNODE, ast, comment);
    }
    return ast;
}

/* add a list element together with accumulated comments */
AST *
CommentedListHolder(AST *ast)
{
    AST *comment;

    if (!ast)
        return ast;

    comment = GetComments();

    if (comment) {
        ast = NewAST(AST_COMMENTEDNODE, ast, comment);
    }
    ast = NewAST(AST_LISTHOLDER, ast, NULL);
    return ast;
}

/* determine whether a loop needs a yield, and if so, insert one */
AST *
CheckYield(AST *body)
{
    AST *ast = body;

    if (!body)
        return AstYield();
    while (ast) {
        if (ast->left)
            return body;
        ast = ast->right;
    }
    return AddToList(body, AstYield());
}


/* push/pop current type symbols */
void PushCurrentTypes(void)
{
    SymbolTable *tab = calloc(1, sizeof(SymbolTable));

    tab->next = currentTypes;
    currentTypes = tab;
}

void PopCurrentTypes(void)
{
    if (currentTypes) {
        currentTypes = currentTypes->next;
    }
}

/* enter a single alias */
void
EnterLocalAlias(SymbolTable *table, AST *globalName, const char *localName)
{
    const char *newName = GetIdentifierName(globalName);
    AddSymbol(table, localName, SYM_REDEF, (void *)globalName, newName);
}

/* fixup declarations in a list */
/* basically this is to handle a case like
     char buf[100], *p = buf
   where the parser doesn't know that we've renamed the identifiers yet;
   after we add a new mapping for "buf" we need to update the reference
   to "buf" later in the list
   replace any identifiers with user name "oldName" in the list with "newIdent"
*/
static void
ReplaceIdentifiers(AST **parent, const char *oldName, AST *newIdent)
{
    AST *item = *parent;
    if (!item) return;
    if (IsIdentifier(item)) {
        if (!strcmp(oldName, GetUserIdentifierName(item))) {
            *parent = newIdent;
        }
    } else {
        ReplaceIdentifiers(&item->left, oldName, newIdent);
        ReplaceIdentifiers(&item->right, oldName, newIdent);
    }
}

static void
RemapIdentifiers(AST *list, AST *newIdent, const char *oldName)
{
    AST *decl;

    while (list) {
        decl = list->left;
        list = list->right;

        if (!decl) {
            continue;
        }
        ReplaceIdentifiers(&decl, oldName, newIdent);
    }
}

/* check a single declaration for typedefs or renamed variables */
static AST *
MakeOneDeclaration(AST *origdecl, SymbolTable *table, AST *restOfList)
{
    AST *decl = origdecl;
    AST *ident;
    AST *identinit = NULL;
    AST **identptr;
    Symbol *sym;
    const char *name;
    if (!decl) return decl;

    if (decl->kind == AST_DECLARE_ALIAS) {
        ERROR(decl, "internal error, DECLARE_ALIAS not supported yet\n");
        return decl;
    }
    if (decl->kind != AST_DECLARE_VAR) {
        ERROR(decl, "internal error, expected DECLARE_VAR");
        return decl;
    }
    ident = decl->right;
    identptr = &decl->right;
    decl = decl->left;
    if (!decl) {
        return decl;
    }
    if (!ident) {
        return origdecl;
    }
    if (ident->kind == AST_ASSIGN) {
        identinit = ident->right;
        identptr = &ident->left;
        ident = *identptr;
    }
    if (ident->kind == AST_LOCAL_IDENTIFIER) {
        // AST_LOCALIDENTIFIER(left,right) == (alias, user_name)
        ident = ident->right;
    }
    if (ident->kind != AST_IDENTIFIER) {
        ERROR(decl, "internal error: expected identifier in declaration");
        return NULL;
    }
    name = ident->d.string;
    sym = FindSymbol(table, name);
    if (sym) {
        WARNING(ident, "Redefining %s", name);
    }
            
    if (decl->kind == AST_TYPEDEF) {
        AddSymbol(table, name, SYM_TYPEDEF, decl->left, NULL);
        return NULL;
    } else {
        const char *oldname = name;
        const char *newName = NewTemporaryVariable(oldname);
        AST *newIdent = AstIdentifier(newName);

        AddSymbol(table, oldname, SYM_REDEF, (void *)newIdent, newName);
        if (identptr) {
            *identptr = NewAST(AST_LOCAL_IDENTIFIER, newIdent, ident);
            RemapIdentifiers(restOfList, newIdent, name);
            if (identinit) {
                RemapIdentifiers(identinit, newIdent, name);
            }
        } else {
            ERROR(decl, "internal error could not find identifier ptr");
        }
    }        
    return origdecl;
}

/* make declarations into a symbol table; if it's a type then add it to currentTypes */
/* returns the list of declarations that really need working on (typedefs are removed) */
AST *
MakeDeclarations(AST *origdecl, SymbolTable *table)
{
    AST *decl = origdecl;
    AST *item;

    if (!decl) return decl;
    if (decl->kind != AST_STMTLIST) {
        origdecl = decl = NewAST(AST_STMTLIST, decl, NULL);
    }
    while (decl && decl->kind == AST_STMTLIST) {
        item = MakeOneDeclaration(decl->left, table, decl->right);
        if (!item) {
            /* remove this from the list */
            decl->left = NULL;
        }
        decl = decl->right;
    }
    return origdecl;
}

/* add a subclass C to a class P */
void
AddSubClass(Module *P, Module *C)
{
    Module **ptr;
    Module *Q;
    ptr = &P->subclasses;
    while ( 0 != (Q = *ptr)) {
        ptr = &Q->subclasses;
    }
    *ptr = C;
    C->subclasses = 0;
}

/* declare a symbol that's an alias for an expression */
void
DeclareMemberAlias(Module *P, AST *ident, AST *expr)
{
    const char *name = GetIdentifierName(ident);
    const char *userName = GetUserIdentifierName(ident);
    Symbol *sym = FindSymbol(&P->objsyms, name);
    if (sym && sym->kind == SYM_VARIABLE) {
        ERROR(ident, "Redefining %s", userName);
        return;
    }
    AddSymbol(&P->objsyms, name, SYM_ALIAS, expr, NULL);
}

// Declare typed global variables
// if inDat is 1, put them in the DAT section, otherwise make them
// member variables
// "ast" is a list of declarations
void
DeclareTypedGlobalVariables(AST *ast, int inDat)
{
    AST *idlist, *typ;
    AST *ident;

    if (!ast) return;
    if (ast->kind == AST_SEQUENCE) {
        ERROR(ast, "Internal error, unexpected sequence");
        return;
    }
    idlist = ast->right;
    typ = ast->left;
    if (!idlist) {
        return;
    }
    if (typ && typ->kind == AST_EXTERN) {
        return;
    }
    if (IsBasicLang(current->curLanguage)) {
        // BASIC does not require pointer notation for pointers to functions
        AST *subtype = RemoveTypeModifiers(typ);
        if (subtype && subtype->kind == AST_FUNCTYPE) {
            typ = NewAST(AST_PTRTYPE, typ, NULL);
        }
    }
    if (idlist->kind == AST_LISTHOLDER) {
        while (idlist) {
            ident = idlist->left;
            DeclareOneGlobalVar(current, ident, typ, inDat);
            idlist = idlist->right;
        }
    } else {
        DeclareOneGlobalVar(current, idlist, typ, inDat);
    }
    return;
}

// returns true if P is the top level module for this project
int
IsTopLevel(Module *P)
{
    return P == allparse;
}

// fetch the top level module
Module *
GetTopLevelModule(void)
{
    return allparse;
}

//
// check for whether a variable of type T must go on the stack
//
#define LARGE_SIZE_THRESHOLD 12

int TypeGoesOnStack(AST *typ)
{
    if (!typ) return 0;
    typ = RemoveTypeModifiers(typ);
    if (TypeSize(typ) > LARGE_SIZE_THRESHOLD) {
        return 1;
    }
    switch (typ->kind) {
    case AST_ARRAYTYPE:
        return 1;
    case AST_OBJECT:
    {
        Module *P = GetClassPtr(typ);
        if (P && P->longOnly) {
            return 0;
        }
        return 1;
    }
    default:
        return 0;
    }
}

static int
GetExprlistLen(AST *list)
{
    int len = 0;
    AST *sub;
    while (list) {
        sub = list->left;
        list = list->right;
        if (sub->kind == AST_EXPRLIST) {
            len += GetExprlistLen(sub);
        } else if (sub->kind == AST_STRING) {
            len += strlen(sub->d.string);
        } else {
            len++;
        }
    }
    return len;
}

//
// find any previous declaration of name
//
static AST *
FindDeclaration(AST *datlist, const char *name)
{
    AST *ident;
    AST *declare;
    
    while (datlist) {
        if (datlist->kind == AST_COMMENTEDNODE
            && datlist->left
            && datlist->left->kind == AST_DECLARE_VAR)
        {
            declare = datlist->left;
            ident = declare->right;
            if (ident->kind == AST_ASSIGN) {
                ident = ident->left;
            }
            if (ident->kind == AST_LOCAL_IDENTIFIER) {
                ident = ident->left;
            }
            if (ident->kind == AST_IDENTIFIER) {
                if (!strcmp(name, ident->d.string)) {
                    return declare;
                }
            } else {
                ERROR(ident, "Internal error, expected identifier while searching for %s", name);
                return NULL;
            }
        }
        datlist = datlist->right;
    }
    return NULL;
}

void
DeclareOneGlobalVar(Module *P, AST *ident, AST *type, int inDat)
{
    AST *ast;
    AST *declare;
    AST *initializer = NULL;
    Symbol *olddef;
    SymbolTable *table = &P->objsyms;
    const char *name = NULL;
    const char *user_name = "variable";
    int is_typedef = 0;
    
    if (!type) {
        type = InferTypeFromName(ident);
    }

    // this may be a typedef
    if (type->kind == AST_TYPEDEF) {
        type = type->left;
        is_typedef = 1;
    }
    if (type->kind == AST_STATIC) {
        // FIXME: this case probably shouldn't happen any more??
        WARNING(ident, "internal error: did not expect static in code");
        type = type->left;
    }
    if (type->kind == AST_FUNCTYPE && !is_typedef) {
        // dummy declaration...
        return;
    }
    if (ident->kind == AST_ASSIGN) {
        if (is_typedef) {
            ERROR(ident, "typedef cannot have initializer");
        }
        initializer = ident->right;
        ident = ident->left;
    }
    if (ident->kind == AST_ARRAYDECL) {
        type = MakeArrayType(type, ident->right);
        type->d.ptr = ident->d.ptr;
        ident = ident->left;
    }
    if (!IsIdentifier(ident)) {
        ERROR(ident, "Internal error, expected identifier");
        return;
    }
    name = GetIdentifierName(ident);
    user_name = GetUserIdentifierName(ident);
    olddef = FindSymbol(table, name);
    if (is_typedef) {
        if (olddef) {
            ERROR(ident, "Redefining symbol %s", user_name);
        }
        AddSymbol(currentTypes, name, SYM_TYPEDEF, type, NULL);
        return;
    }
    if (olddef) {
        ERROR(ident, "Redefining symbol %s", user_name);
    }
    // if this is an array type with no size, there must be an
    // initializer
    if (type->kind == AST_ARRAYTYPE && !type->right) {
        if (!initializer) {
            ERROR(ident, "global array %s declared with no size and no initializer", user_name);
            type->right = AstInteger(1);
        } else {
            if (initializer->kind == AST_EXPRLIST) {
                type->right = AstInteger(AstListLen(initializer));
            } else if (initializer->kind == AST_STRINGPTR) {
                type->right = AstInteger(GetExprlistLen(initializer->left) + 1);
            } else {
                type->right = AstInteger(1);
            }
        }
    }
    if (!inDat) {
        /* make this a member variable */
        DeclareOneMemberVar(P, ident, type, 0);
        return;
    }
    // look through the globals to see if there's already a definition
    // if there is, and that definition has an initializer, we have a conflict
    declare = FindDeclaration(P->datblock, name);
    if (declare && declare->right) {
        if (declare->right->kind == AST_ASSIGN && initializer) {
            ERROR(initializer, "Variable %s is initialized twice",
                  user_name);
        } else if (initializer) {
            declare->right = AstAssign(DupAST(ident), initializer);
        }
    } else {
        if (initializer) {
            ident = AstAssign(ident, initializer);
        }
        declare = NewAST(AST_DECLARE_VAR, type, ident);
        ast = NewAST(AST_COMMENTEDNODE, declare, NULL);
        P->datblock = AddToList(P->datblock, ast);
    }
    return;
}

static int
DeclareMemberVariablesOfSize(Module *P, int basetypesize, int offset)
{
    AST *upper;
    AST *ast;
    AST *curtype;
    int curtypesize = basetypesize;
    int isUnion = P->isUnion;
    AST *varblocklist;
    int oldoffset = offset;
    unsigned sym_flags = 0;

    if (P->defaultPrivate) {
        sym_flags = SYMF_PRIVATE;
    }
    varblocklist = P->pendingvarblock;
    if (basetypesize == 0) {
        P->finalvarblock = AddToList(P->finalvarblock, varblocklist);
        P->pendingvarblock = NULL;
    }
    for (upper = varblocklist; upper; upper = upper->right) {
        AST *idlist;
        if (upper->kind != AST_LISTHOLDER) {
            ERROR(upper, "Expected list holder\n");
        }
        ast = upper->left;
        if (ast->kind == AST_COMMENTEDNODE)
            ast = ast->left;
        switch (ast->kind) {
        case AST_BYTELIST:
            curtype = ast_type_byte;
            curtypesize = 1;
            idlist = ast->left;
            break;
        case AST_WORDLIST:
            curtype = ast_type_word;
            curtypesize = 2;
            idlist = ast->left;
            break;
        case AST_LONGLIST:
	    curtype = NULL; // was ast_type_generic;
            curtypesize = 4;
            idlist = ast->left;
            break;
        case AST_DECLARE_VAR:
        case AST_DECLARE_VAR_WEAK:
            curtype = ast->left;
            idlist = ast->right;
            curtypesize = CheckedTypeSize(curtype); // make sure module variables are declared
            if (curtype->kind == AST_ASSIGN) {
                if (basetypesize == 4 || basetypesize == 0) {
                    ERROR(ast, "Member variables cannot have initial values");
                    return offset;
                }
            }
            break;
        case AST_COMMENT:
            /* skip */
            continue;
        default:
            ERROR(ast, "bad type  %d in variable list\n", ast->kind);
            return offset;
        }
        if (isUnion) {
            curtypesize = (curtypesize+3) & ~3;
            if (curtypesize > P->varsize) {
                P->varsize = curtypesize;
            }
        }
        if (basetypesize == 0) {
            // round offset up to necessary alignment
            if (!gl_p2) {
                if (curtypesize == 2) {
                    offset = (offset + 1) & ~1;
                } else if (curtypesize >= 4) {
                    offset = (offset + 3) & ~3;
                }
            }
            // declare all the variables
            offset = EnterVars(SYM_VARIABLE, &P->objsyms, curtype, idlist, offset, P->isUnion, sym_flags);
        } else if (basetypesize == curtypesize || (basetypesize == 4 && (curtypesize >= 4 || curtypesize == 0))) {
            offset = EnterVars(SYM_VARIABLE, &P->objsyms, curtype, idlist, offset, P->isUnion, sym_flags);
        }
    }
    if (curtypesize != 4 && offset != oldoffset) {
        P->longOnly = 0;
    }
    return offset;   
}

void
DeclareOneMemberVar(Module *P, AST *ident, AST *type, int is_private)
{
    if (!type) {
        type = InferTypeFromName(ident);
    }
    if (1) {
        AST *iddecl = NewAST(AST_LISTHOLDER, ident, NULL);
        AST *newdecl = NewAST(AST_DECLARE_VAR, type, iddecl);
        P->pendingvarblock = AddToList(P->pendingvarblock, NewAST(AST_LISTHOLDER, newdecl, NULL));
    }
}

void
MaybeDeclareMemberVar(Module *P, AST *identifier, AST *typ, int is_private)
{
    AST *sub;
    const char *name;
    sub = identifier;
    if (sub && sub->kind == AST_ASSIGN) {
        sub = sub->left;
    }
    while (sub && sub->kind == AST_ARRAYDECL) {
        sub = sub->left;
    }
    if (!sub || sub->kind != AST_IDENTIFIER) {
        return;
    }
    name = GetIdentifierName(sub);
    Symbol *sym = FindSymbol(&P->objsyms, name);
    if (sym && sym->kind == SYM_VARIABLE) {
        return;
    }
    if (!typ) {
        typ = InferTypeFromName(identifier);
    }
    if (!AstUses(P->pendingvarblock, identifier)) {
        AST *iddecl = NewAST(AST_LISTHOLDER, identifier, NULL);
        AST *newdecl = NewAST(AST_DECLARE_VAR, typ, iddecl);
        P->pendingvarblock = AddToList(P->pendingvarblock, NewAST(AST_LISTHOLDER, newdecl, NULL));
    }
}

void
DeclareMemberVariables(Module *P)
{
    int offset;

    if (P->isUnion) {
        offset = 0;
    } else {
        offset = P->varsize;
    }
    if (IsSpinLang(P->mainLanguage)) {
        // Spin always declares longs first, then words, then bytes
        // but other languages may have other preferences
        offset = DeclareMemberVariablesOfSize(P, 4, offset); // also declares >= 4
        offset = DeclareMemberVariablesOfSize(P, 2, offset);
        offset = DeclareMemberVariablesOfSize(P, 1, offset);
    } else {
        offset = DeclareMemberVariablesOfSize(P, 0, offset);
    }
    if (!P->isUnion) {
        // round up to next LONG boundary
        offset = (offset + 3) & ~3;
        P->varsize = offset;
    }
    if (P->pendingvarblock) {
        P->finalvarblock = AddToList(P->finalvarblock, P->pendingvarblock);
        P->pendingvarblock = NULL;
    }
}
