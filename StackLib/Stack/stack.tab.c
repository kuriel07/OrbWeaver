/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "stack.y" /* yacc.c:339  */

int errors = 0;
int yyerror ( char *s );
/*************************************************************************
Compiler for the Stack language
***************************************************************************/
/*=========================================================================
C Libraries, Symbol Table, Code Generator & other C code
=========================================================================*/
#include <stdio.h> /* For I/O */
#include <stdlib.h> /* For malloc here and in symbol table */
#include <string.h> /* For strcmp in symbol table */
#include <stdarg.h>
#include "token.h"
#include "sem_proto.h"
#include "il_streamer.h"
#include "asm_streamer.h"
#include "lex_proto.h"
#include "scr_generator.h"

#define YYDEBUG 1 /* For Debugging */
#define alloca malloc


#line 91 "stack.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif


/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    VARIABLE = 257,
    CONSTANT = 258,
    N_CONSTANT = 259,
    ARRAY = 260,
    NUMERIC = 261,
    N_NUMERIC = 262,
    P_VOID = 290,
    P_AUTO = 291,
    P_BYTE = 292,
    P_HWORD = 293,
    P_WORD = 294,
    P_DWORD = 295,
    P_FLOAT = 296,
    P_DOUBLE = 297,
    P_STRING = 298,
    P_PTR = 299,
    P_VAR = 300,
    P_NEW = 301,
    P_HEX = 302,
    P_SIZEOF = 310,
    P_STDCALL = 314,
    P_FASTCALL = 315,
    P_TRY = 345,
    P_CATCH = 346,
    P_IN = 347,
    P_FOREACH = 348,
    P_GOTO = 349,
    P_IF = 350,
    P_ELSE = 351,
    P_SWITCH = 352,
    P_CASE = 353,
    P_DEFAULT = 354,
    P_BREAK = 355,
    P_FOR = 356,
    P_WHILE = 357,
    P_DO = 358,
    P_CONTINUE = 359,
    P_FUNCTION = 360,
    P_RETURN = 361,
    P_ALIAS = 362,
    P_EVENT = 363,
    P_PROTO = 364,
    P_EXTERN = 365,
    P_SYSCALL = 366,
    P_CLASS = 380,
    P_INTERFACE = 381,
    P_PACKAGE = 382,
    P_IMPORT = 383,
    P_PUBLIC = 385,
    P_PRIVATE = 386,
    P_PROTECTED = 387,
    P_LIB = 388,
    L_SB = 398,
    R_SB = 399,
    L_CL = 400,
    R_CL = 401,
    EOS = 402,
    EOC = 403,
    L_BR = 406,
    R_BR = 407,
    T_EQ = 410,
    T_GT = 411,
    T_LT = 412,
    T_GTEQ = 413,
    T_LTEQ = 414,
    T_NE = 415,
    T_AND = 416,
    T_OR = 417,
    S_EQ = 450,
    S_ADD = 451,
    S_SUB = 452,
    S_ADDEQ = 453,
    S_SUBEQ = 454,
    S_ADDADD = 455,
    S_SUBSUB = 456,
    S_MUL = 457,
    S_DIV = 458,
    S_MULEQ = 459,
    S_DIVEQ = 460,
    S_MOD = 461,
    S_MODEQ = 462,
    S_RNEXT = 470,
    S_PDOT = 471,
    S_AND = 472,
    S_OR = 473,
    S_XOR = 474,
    S_NOT = 475,
    S_ANDEQ = 476,
    S_OREQ = 477,
    S_XOREQ = 478,
    S_SHL = 480,
    S_SHR = 481,
    S_SHLEQ = 482,
    S_SHREQ = 483
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
#line 26 "stack.y" /* yacc.c:355  */
union semrec
{
#line 27 "stack.y" /* yacc.c:355  */

	uchar string[4096];
	struct {
		uint16 length;
		uchar value[4096];
	} bytes;
	struct {
		uint16 type;
		uchar string[4096];
	} object;
	uint32 state;
	uint16 api_id;
	uchar cast_string[10];
	yytoken token;

#line 241 "stack.tab.c" /* yacc.c:355  */
};
#line 26 "stack.y" /* yacc.c:355  */
typedef union semrec YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);



/* Copy the second part of user declarations.  */

#line 258 "stack.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  13
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2061

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  100
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  189
/* YYNRULES -- Number of rules.  */
#define YYNRULES  416
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  638

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   484

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,    99,     2,    97,     2,    98,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,     2,     2,     2,     2,     2,     2,     2,
      22,     2,     2,     2,    23,    24,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      47,    48,    49,    50,     2,    51,    52,    53,    54,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    55,    56,
      57,    58,    59,    60,     2,     2,    61,    62,     2,     2,
      63,    64,    65,    66,    67,    68,    69,    70,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,     2,     2,     2,     2,     2,     2,     2,
      84,    85,    86,    87,    88,    89,    90,    91,    92,     2,
      93,    94,    95,    96,     2
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   187,   187,   188,   189,   191,   192,   193,   195,   196,
     197,   198,   200,   201,   202,   202,   203,   204,   205,   206,
     207,   208,   209,   211,   212,   213,   215,   216,   216,   217,
     217,   218,   218,   219,   220,   222,   223,   225,   226,   228,
     229,   230,   235,   236,   236,   237,   239,   241,   243,   245,
     247,   248,   249,   251,   254,   254,   254,   255,   256,   258,
     260,   260,   261,   263,   265,   266,   266,   266,   269,   271,
     273,   272,   276,   277,   277,   277,   278,   278,   280,   278,
     284,   284,   284,   286,   286,   289,   290,   289,   295,   297,
     299,   297,   304,   304,   306,   306,   310,   311,   315,   316,
     317,   315,   319,   320,   322,   323,   325,   322,   327,   328,
     329,   330,   331,   332,   333,   335,   336,   337,   339,   339,
     340,   340,   341,   341,   342,   342,   343,   343,   344,   344,
     345,   345,   346,   346,   347,   347,   348,   348,   349,   349,
     350,   351,   352,   353,   368,   369,   370,   371,   372,   377,
     379,   379,   379,   380,   380,   381,   382,   383,   384,   386,
     386,   388,   389,   391,   392,   393,   394,   394,   396,   396,
     396,   397,   399,   401,   401,   401,   403,   405,   406,   407,
     408,   409,   410,   411,   412,   413,   414,   429,   430,   431,
     432,   432,   433,   433,   434,   434,   435,   435,   436,   436,
     437,   437,   438,   438,   439,   439,   440,   440,   441,   441,
     442,   442,   443,   443,   444,   444,   445,   445,   446,   446,
     447,   447,   448,   451,   452,   455,   455,   457,   458,   459,
     460,   461,   462,   463,   464,   465,   466,   467,   468,   469,
     470,   471,   472,   473,   474,   475,   476,   477,   478,   481,
     482,   485,   485,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,   502,   503,
     504,   505,   506,   507,   508,   510,   511,   512,   513,   514,
     515,   516,   518,   519,   520,   521,   523,   524,   525,   526,
     528,   529,   531,   531,   533,   534,   536,   536,   537,   539,
     540,   541,   542,   544,   545,   546,   547,   548,   549,   550,
     552,   553,   554,   555,   557,   558,   559,   561,   561,   563,
     563,   565,   566,   568,   569,   571,   572,   572,   573,   575,
     575,   576,   576,   578,   579,   579,   580,   582,   583,   583,
     584,   586,   587,   587,   589,   590,   591,   593,   594,   593,
     600,   602,   602,   605,   606,   606,   607,   608,   610,   610,
     611,   611,   612,   614,   616,   619,   619,   621,   622,   624,
     625,   625,   626,   628,   629,   629,   630,   632,   633,   633,
     633,   634,   640,   641,   643,   643,   643,   645,   646,   647,
     649,   650,   650,   651,   652,   652,   653,   655,   655,   657,
     658,   659,   661,   662,   664,   665,   664,   669,   670,   672,
     673,   674,   676,   677,   677,   678,   680
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "VARIABLE", "CONSTANT", "N_CONSTANT",
  "ARRAY", "NUMERIC", "N_NUMERIC", "P_VOID", "P_AUTO", "P_BYTE", "P_HWORD",
  "P_WORD", "P_DWORD", "P_FLOAT", "P_DOUBLE", "P_STRING", "P_PTR", "P_VAR",
  "P_NEW", "P_HEX", "P_SIZEOF", "P_STDCALL", "P_FASTCALL", "P_TRY",
  "P_CATCH", "P_IN", "P_FOREACH", "P_GOTO", "P_IF", "P_ELSE", "P_SWITCH",
  "P_CASE", "P_DEFAULT", "P_BREAK", "P_FOR", "P_WHILE", "P_DO",
  "P_CONTINUE", "P_FUNCTION", "P_RETURN", "P_ALIAS", "P_EVENT", "P_PROTO",
  "P_EXTERN", "P_SYSCALL", "P_CLASS", "P_INTERFACE", "P_PACKAGE",
  "P_IMPORT", "P_PUBLIC", "P_PRIVATE", "P_PROTECTED", "P_LIB", "L_SB",
  "R_SB", "L_CL", "R_CL", "EOS", "EOC", "L_BR", "R_BR", "T_EQ", "T_GT",
  "T_LT", "T_GTEQ", "T_LTEQ", "T_NE", "T_AND", "T_OR", "S_EQ", "S_ADD",
  "S_SUB", "S_ADDEQ", "S_SUBEQ", "S_ADDADD", "S_SUBSUB", "S_MUL", "S_DIV",
  "S_MULEQ", "S_DIVEQ", "S_MOD", "S_MODEQ", "S_RNEXT", "S_PDOT", "S_AND",
  "S_OR", "S_XOR", "S_NOT", "S_ANDEQ", "S_OREQ", "S_XOREQ", "S_SHL",
  "S_SHR", "S_SHLEQ", "S_SHREQ", "','", "'.'", "'*'", "$accept", "program",
  "stmts", "multi_block_stmt", "single_block_stmt", "$@1", "block_stmt",
  "global_decl", "$@2", "$@3", "$@4", "interface_stmts", "class_stmts",
  "case_block_stmt", "inline_var_decl", "$@5", "case_expr_param",
  "expr_params", "body_func_params", "stmt_end", "lazy_stmts",
  "local_decl", "global_import_path", "$@6", "$@7", "var_init_decl",
  "var_decl", "$@8", "label_decl", "matched_if_stmt", "$@9", "$@10",
  "else_stmt", "$@11", "open_if_stmt", "$@12", "$@13", "$@14", "$@15",
  "$@16", "while_stmt", "$@17", "$@18", "$@19", "catch_stmt", "$@20",
  "$@21", "try_catch_stmt", "$@22", "$@23", "for_stmt", "$@24", "$@25",
  "for_init_stmt", "for_expr_stmt", "$@26", "$@27", "$@28",
  "foreach_init_stmt", "foreach_expr_stmt", "$@29", "$@30", "$@31", "stmt",
  "lazy_stmt", "assignment", "$@32", "$@33", "$@34", "$@35", "$@36",
  "$@37", "$@38", "$@39", "$@40", "$@41", "$@42", "step_variable",
  "instance_val", "$@54", "$@55", "$@56", "$@57", "rhs_val", "lhs_val",
  "$@58", "$@59", "$@60", "lhs_method_call", "$@61", "$@62", "lhs_expr",
  "lhs_scope_expr", "$@74", "$@75", "$@76", "$@77", "$@78", "$@79", "$@80",
  "$@81", "$@82", "$@83", "$@84", "$@85", "$@86", "$@87", "$@88", "$@89",
  "lazy_scope", "lazy_scope_expr", "$@90", "lazy_expr", "rhs_lazy_scope",
  "rhs_lazy_scope_expr", "$@91", "rhs_lazy_expr", "var_type_primitive",
  "var_type_param", "var_type", "scope_val", "scope_expr", "$@92",
  "cast_val", "cast_expr", "$@93", "lazy_val_decl_new", "lazy_val_decl",
  "lazy_val_function", "lazy_val", "lazy_func_call", "$@94", "array_decl",
  "$@95", "constant_val", "numeric_val", "constant_seq", "$@96",
  "args_expr", "$@97", "$@98", "obj_params", "$@99", "arr_params", "$@100",
  "obj_param", "$@101", "arr_param", "lambda_func_decl", "$@102", "$@103",
  "instance_decl", "case_stmt", "$@104", "case_single_stmt", "$@105",
  "exprs", "$@106", "$@107", "expr", "exprval", "sys_func_decl", "$@108",
  "sys_extended_decl", "sys_param_list", "$@109", "lambda_param_list",
  "$@110", "call_param_list", "$@111", "$@112", "lhs_method_param_list",
  "$@115", "$@116", "accessor_func", "interface_param_list", "$@117",
  "$@118", "interface_func_decl", "$@119", "call_type", "interface_stmt",
  "class_stmt", "$@120", "$@121", "body_func_extended_decl",
  "body_func_param_list", "$@122", "error_stmt", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   484,   257,   258,   259,   260,   261,   262,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   310,   314,   315,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   366,   380,   381,   382,
     383,   385,   386,   387,   388,   398,   399,   400,   401,   402,
     403,   406,   407,   410,   411,   412,   413,   414,   415,   416,
     417,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   470,   471,   472,   473,   474,   475,
     476,   477,   478,   480,   481,   482,   483,    44,    46,    42
};
# endif

#define YYPACT_NINF -578

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-578)))

#define YYTABLE_NINF -416

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     279,  -578,    -4,    43,    45,  -578,    38,   279,  -578,    47,
    -578,     6,    16,  -578,  -578,    10,     4,    33,    63,    55,
    -578,    62,  -578,  -578,   143,  -578,    48,  -578,    99,  -578,
     112,   133,   136,   131,   188,    30,  -578,   146,  -578,   150,
     153,    47,  -578,  -578,  -578,  -578,  -578,  2037,    16,   119,
     173,   166,   170,  -578,  -578,   175,   559,  1913,  -578,  -578,
      99,   233,  -578,  -578,   237,  -578,  -578,  -578,  -578,  -578,
    -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,   240,  -578,
    -578,  -578,   182,   183,    -9,   189,   198,   165,  -578,   259,
     260,  1232,  -578,  -578,   237,  -578,  -578,  1883,  -578,  -578,
    -578,  -578,   267,   274,   222,   224,  -578,  -578,   287,  -578,
     230,  -578,  -578,  -578,  -578,  -578,   232,    36,  -578,   201,
    -578,  1603,   239,  1507,    36,  -578,  -578,  -578,   202,  -578,
    -578,   236,   234,  -578,  -578,  -578,  -578,  -578,  -578,  -578,
     243,   206,  -578,    19,  -578,  1949,  -578,  1021,  -578,  1818,
    -578,  -578,     2,  -578,  -578,  -578,  -578,  -578,  -578,  -578,
    -578,  -578,   245,   351,  -578,  -578,  -578,  -578,  -578,  -578,
     248,  -578,    36,  1550,   249,  -578,   250,   250,   250,    36,
    -578,   253,   250,  1461,  -578,   256,  -578,  1844,  1844,  1021,
    -578,   231,  1399,    20,    29,    29,   244,  1626,     3,     2,
     113,     2,   103,   316,  -578,  -578,  -578,  1626,    36,   317,
     318,  1733,  -578,  -578,  -578,  -578,  -578,  -578,   319,   320,
    -578,  -578,  -578,  -578,  -578,  -578,  -578,    36,    36,    36,
      36,    36,    36,    36,    36,    36,    36,    36,    36,    36,
      36,    36,    36,  1841,  -578,  -578,  -578,  -578,  -578,   275,
      25,   269,    36,   270,    80,   917,  -578,    26,  -578,    36,
    -578,  -578,  -578,   281,   293,   330,  -578,  -578,   314,   349,
     256,    35,   559,     2,  -578,  -578,   298,  -578,   315,  -578,
     334,  1399,  -578,   341,   342,    50,   403,   404,   406,  -578,
     471,  -578,   346,  -578,  -578,   350,  -578,  1922,  -578,  1949,
    1021,   352,   345,    36,  -578,   418,   357,   358,  -578,   108,
    1755,   417,   422,  -578,   176,  -578,  -578,    36,  1633,  1633,
    1633,  1633,  1633,   368,   371,  1633,  1633,  1633,  1633,  1633,
    1021,  1021,  1021,  1021,  1021,  1021,  1087,  1087,  1130,  1130,
    1130,   405,   405,   405,   103,   103,  1603,  -578,   379,   374,
     343,  -578,   955,   376,  -578,   380,   381,  -578,   415,  -578,
    -578,   419,   382,   144,  -578,   595,   690,  1504,  -578,  1294,
     202,   386,   690,   250,   385,   387,   388,  -578,   364,  -578,
    -578,  -578,   370,  -578,  -578,  -578,    39,  -578,  -578,  -578,
    -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,
    -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,  1021,
    -578,    36,  -578,  -578,  -578,  -578,   355,  -578,  -578,  -578,
    1692,  1692,  1692,  1692,  1692,  1021,  1603,  1633,  -578,  -578,
    1055,  -578,  1055,  1055,  1055,  1055,  -578,  -578,  1055,  1055,
    1055,  1055,  1055,  -578,   383,   351,   391,  -578,   392,  -578,
     785,  -578,  -578,  -578,  -578,  -578,  -578,  -578,   446,  -578,
     461,    80,   433,  -578,  1356,  -578,  -578,  -578,   435,   398,
    -578,    36,   231,    20,    36,    36,    36,    36,    36,    36,
      36,    36,    36,    36,    36,    36,    36,    36,    36,    36,
      36,   989,    36,    36,  -578,   127,   127,  -578,  -578,  -578,
      29,   292,  -578,  -578,  1633,  1633,  1633,  1633,  1633,  1633,
    1633,  1633,  1633,  1633,    36,  1633,  1633,  1633,  1633,  1633,
      36,    36,   351,  -578,  -578,    80,  1714,  -578,  1841,    36,
      36,  -578,  -578,  -578,  -578,  -578,  -578,  -578,    36,  -578,
     349,   880,  1021,  -578,  -578,  1021,  1021,  1021,  1021,  1021,
    1021,  1021,  1021,  1021,  1021,  1021,  1021,  1021,  1021,  1021,
    1021,  1021,  -578,   114,   439,   440,   445,  1055,  1055,  1055,
    1055,  1055,  1055,  1121,  1121,  1162,  1162,  1130,   640,   640,
     640,   292,   292,   447,   448,  -578,    36,  -578,   349,   451,
    -578,  -578,   595,   478,   454,   456,  -578,  -578,   420,  -578,
    -578,  -578,  -578,  -578,  -578,   458,  -578,  -578,  -578,  -578,
    -578,  -578,  -578,   432,   432,  -578,  -578,  1626,    36,   515,
    -578,  -578,   690,   690,   459,  -578,   462,  -578,  -578,  -578,
    -578,   690,    36,  -578,   460,  -578,   432,  -578
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,   416,     0,     0,     0,    27,     0,     0,    34,     0,
      29,     0,     0,     1,     3,     0,     0,     0,     0,    54,
      58,     0,   365,    33,     0,    31,     0,    28,   369,   389,
       0,     0,     0,     0,     0,     0,   408,     0,    55,   370,
       0,     0,   387,   388,    30,   404,    37,     0,     0,     0,
     367,     0,     0,   400,   401,     0,     0,     0,   403,    56,
     369,     0,   366,   407,   412,   405,    32,   289,   287,   275,
     276,   277,   278,   279,   280,   288,   281,   286,     0,    35,
     371,   368,   413,     0,   409,     0,     0,     0,    48,     0,
       0,     0,   397,   402,   412,   410,   411,   300,   321,   322,
     323,   324,     0,     0,     0,     0,    89,    94,     0,    65,
       0,   110,    92,    80,    83,   111,     0,   112,   331,    14,
      22,   292,     0,     0,     0,    25,    24,   406,    53,    19,
     109,     0,    62,    45,    20,     9,    10,    16,    11,    17,
       0,    52,   116,   162,   247,   117,   227,   115,   295,   298,
     310,   303,   149,   161,   304,   301,   313,   302,   305,   248,
     311,    18,     0,   390,   414,    63,   317,   142,   143,    59,
       0,   319,     0,     0,     0,   114,     0,     0,     0,     0,
     351,     0,     0,     0,   347,   300,   329,     0,     0,   364,
     113,   337,     0,   333,   187,   187,     0,     0,   300,   308,
     300,   309,   246,     0,    13,    60,    49,    50,     0,     0,
       0,     0,   118,   120,   122,   124,   126,   128,     0,     0,
     130,   132,   134,   136,   138,   228,   229,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   306,   307,    21,   284,   285,   282,
     391,     0,   377,     0,   325,     0,    90,     0,    95,     0,
      66,    74,    77,     0,     0,    96,    93,    81,     0,   373,
     300,   300,   292,   315,   314,   346,     0,   345,   338,   316,
       0,     0,   342,     0,   334,   171,     0,     0,     0,   249,
       0,   250,     0,   223,   224,     0,   294,   300,   290,     0,
     291,     0,    42,     0,    51,     0,   155,   156,   186,   183,
       0,     0,     0,   182,     0,   184,   185,     0,     0,     0,
       0,     0,     0,   164,   163,     0,     0,     0,     0,     0,
     240,   243,   242,   245,   244,   241,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   292,   297,     0,   394,
       0,   398,   378,     0,   299,   326,     0,   312,     0,   103,
     102,     0,     0,   362,   363,     0,     0,     0,    46,     0,
      97,     0,     0,     0,   374,     0,     0,   332,     0,    15,
       6,   330,     0,   140,   141,   171,   222,   188,   189,   210,
     216,   214,   220,   218,   212,   190,   192,   194,   196,   198,
     200,   202,   204,   206,   208,   160,   226,   293,    43,    61,
     157,     0,   150,   153,   147,   148,     0,   145,   146,   165,
       0,     0,     0,     0,     0,   119,   292,     0,   273,   253,
     121,   274,   123,   125,   127,   129,   168,   166,   131,   133,
     135,   137,   139,   283,     0,   390,     0,   318,     0,   320,
       0,    91,   104,    47,   358,   360,    65,    68,    69,    75,
       0,     0,     0,   357,     0,    41,    98,    82,     0,     0,
     348,     0,   337,   333,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   382,   377,   176,   177,   178,   179,   180,   181,
     187,   272,   254,   255,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     382,   377,   390,   392,   379,   325,   292,    88,     0,     0,
       0,    70,    67,    78,   354,   356,   352,    40,     0,    84,
     373,     0,   343,   339,   335,   211,   217,   215,   221,   219,
     213,   191,   193,   195,   197,   199,   201,   203,   205,   207,
     209,    44,   158,   383,     0,     0,     0,   266,   269,   268,
     271,   270,   267,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,     0,     0,   395,   377,   327,   373,     0,
     359,   361,     0,    72,     0,     0,   375,   349,     0,   151,
     154,   252,   169,   167,   380,     0,   105,    71,    73,    79,
     355,    99,   385,   172,   172,    86,   106,    50,   382,     0,
     152,   170,     0,     0,     0,   386,     0,    87,   107,   100,
     173,     0,   382,   101,     0,   174,   172,   175
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -578,   517,   246,   159,  -168,  -578,   -91,  -578,  -578,  -578,
    -578,   472,   493,  -578,   265,  -578,  -578,  -156,  -578,  -578,
    -206,  -578,   499,  -578,  -578,   284,  -578,  -578,  -578,  -351,
    -578,  -578,  -578,  -578,   -45,  -578,  -578,  -578,  -578,  -578,
    -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,
    -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,
    -578,  -578,  -578,  -578,  -578,  -182,  -578,  -578,  -578,  -578,
    -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,
    -578,  -578,  -578,   -53,  -187,  -578,  -578,  -578,  -577,  -578,
    -578,  -259,   365,  -578,  -578,  -578,  -578,  -578,  -578,  -578,
    -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,  -578,
    -578,  -578,   -99,    79,  -578,  -578,    76,  -161,  -578,   520,
    -578,  -578,  -578,  -578,  -578,  -578,  -578,  -119,  -185,  -578,
    -578,  -578,  -578,  -578,  -200,  -194,    65,  -578,  -578,  -578,
    -578,   124,  -578,   129,  -578,  -184,  -578,  -578,  -171,  -578,
    -578,  -578,  -578,  -578,   140,  -578,  -477,  -578,  -578,  -578,
     -93,   564,  -578,  -578,   546,  -578,  -505,  -578,  -466,  -578,
    -578,  -473,  -578,  -578,  -578,  -415,  -578,  -578,  -578,  -578,
    -578,  -578,  -578,  -578,  -578,  -578,   513,  -578,    42
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     6,   280,   125,   126,   192,   281,     7,    12,    17,
      37,    55,    33,   464,   128,   490,   180,   260,    65,   129,
     130,   131,    21,    26,    48,   132,   133,   303,   134,   135,
     176,   365,   532,   592,   136,   177,   366,   178,   367,   593,
     137,   182,   372,   183,   451,   588,   622,   138,   173,   358,
     139,   181,   174,   371,   266,   538,   617,   631,   361,   258,
     528,   616,   623,   140,   141,   142,   317,   318,   319,   320,
     321,   322,   325,   326,   327,   328,   329,   313,   143,   492,
     613,   493,   194,   144,   145,   521,   520,   614,   620,   632,
     636,   314,   291,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   474,   479,   476,   475,   478,   477,   295,
     146,   195,   147,   292,   429,   500,   430,    77,   250,   196,
     301,   148,   197,   149,   150,   243,   151,   152,   153,   275,
     154,   252,   155,   254,   156,   157,   356,   448,   158,   193,
     191,   283,   382,   276,   378,   277,   376,   278,   159,   269,
     541,   160,   161,   264,   465,   594,   362,   529,   530,   363,
     364,    16,    28,    62,    40,    49,   375,   469,   353,   446,
     586,   564,   598,   618,    34,   251,   350,   444,    86,   163,
      56,    57,    35,    52,    84,    91,    83,    87,   162
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     127,   304,   249,   199,   201,   256,   274,   290,   290,   284,
     299,   315,   289,   293,   458,   298,   460,   316,   189,    19,
     279,   261,   262,   282,   190,   202,   267,   565,   349,   359,
     523,     1,   285,    89,    90,   596,     9,   621,    13,   185,
      98,    99,     8,   100,   101,   102,    10,   583,    11,     8,
      15,   416,   590,   591,   355,   584,   103,   104,   105,   637,
      18,   595,  -140,    23,   166,  -140,    36,    25,   199,   201,
      29,    22,   273,   255,   208,    30,   116,    36,   244,   245,
     189,    31,    32,   605,    98,    99,   263,  -393,   -38,    58,
      24,   118,   268,   186,   211,  -342,   166,   121,   300,    58,
    -140,   386,    39,   209,   210,   286,   287,   585,   225,   305,
     315,   226,   187,   188,   -57,    20,   316,  -341,   288,   225,
     604,    27,   226,   218,   219,   124,   167,   168,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,     1,   625,    38,   431,   431,   431,
     431,   431,    41,   352,   431,   431,   431,   431,   431,   634,
     189,   495,   496,   497,   498,   499,   227,   228,   229,   230,
     231,   232,  -141,    42,   166,  -141,    43,   227,   228,   229,
     230,   231,   232,    29,   414,   415,   233,   234,    30,    44,
     347,    45,   235,   236,    31,    32,   237,   457,   457,   457,
     238,   239,   240,    47,   409,   422,   423,   241,   242,   424,
    -141,  -384,  -372,   454,   455,    50,    60,   468,   425,    61,
     315,   315,   315,   315,   315,    63,   316,   316,   316,   316,
     316,    64,   419,    66,   271,    98,    99,    81,   100,   101,
      82,   607,   198,    85,  -415,    88,   100,   101,   420,   421,
      92,   103,   104,   105,   422,   423,   431,    93,   424,   103,
     104,   534,    94,    95,    96,   428,   428,   428,   428,   428,
     169,   116,   428,   428,   428,   428,   428,   170,   463,    -2,
       1,   467,   527,   171,   249,   172,   118,   274,   186,   284,
     175,   179,   272,   184,   118,   204,   186,   502,  -329,   203,
     503,   279,   206,   207,   246,   205,   296,   187,   188,   253,
     257,   259,   491,   290,   265,   187,   188,   166,   289,   302,
     306,   307,   323,   324,     2,   355,     3,     4,  -341,     5,
     348,   351,   354,   431,   431,   431,   431,   431,   431,   431,
     431,   431,   431,   368,   431,   431,   431,   431,   431,   102,
     369,   373,   374,   273,   377,   504,   505,   506,   507,   508,
     509,   249,    69,    70,    71,    72,    73,    74,   247,   248,
      76,  -340,   542,   463,   428,   545,   546,   547,   548,   549,
     550,   551,   552,   553,   554,   555,   556,   557,   558,   559,
     560,   561,   379,   563,   352,   432,   433,   434,   435,   381,
    -336,   438,   439,   440,   441,   442,   383,   384,   405,   385,
     225,   624,   406,   226,   407,   577,   408,   494,   412,   413,
     417,   563,   352,   225,   457,   418,   226,   420,   421,   436,
     189,   189,   437,   422,   423,   443,  -396,   424,   447,   189,
     445,   450,  -328,   449,   453,   466,   452,  -376,   471,   470,
     597,   428,   428,   428,   428,   428,   428,   428,   428,   428,
     428,   472,   428,   428,   428,   428,   428,   473,   227,   228,
     229,   230,   231,   232,   410,   589,   387,   531,   411,   388,
     522,   227,   228,   229,   230,   231,   232,   352,   524,   525,
     233,   234,   533,   535,   539,   540,   235,   236,   241,   242,
     237,   599,   600,   501,   238,   239,   240,   601,   608,   602,
     603,   241,   242,   606,   610,   611,   619,   612,   626,   563,
     615,   629,   635,   630,    14,   459,   211,   380,    46,    79,
     370,   627,   628,   563,   389,   390,   391,   392,   393,   394,
     633,   360,   212,   395,   396,   213,   214,    59,   609,   397,
     398,   215,   216,   399,   217,   218,   219,   400,   401,   402,
     294,   220,   221,   222,   403,   404,   223,   224,    67,    68,
      69,    70,    71,    72,    73,    74,    78,    75,    76,   566,
     567,   568,   569,   570,   571,   572,   573,   574,   575,   576,
     587,   578,   579,   580,   581,   582,     1,   544,    97,    98,
      99,   543,   100,   101,   537,    51,    80,   164,     0,     0,
       0,     0,     0,     0,   102,   103,   104,   105,     0,     0,
     -12,   -12,     0,   107,   108,   456,   -12,   110,   -12,   -12,
     111,   112,   113,   114,   115,   116,   117,     0,     0,     0,
     -12,     0,     0,     0,     0,   502,   -12,   -12,   503,     0,
     118,   -12,   119,   -12,   120,   -12,   121,   -12,   -12,   -12,
     -12,   -12,   -12,   -12,   -12,   -12,     0,   -12,   -12,     0,
       0,   122,   123,   -12,   -12,     0,     0,   -12,     0,     0,
       0,   -12,   -12,   -12,   124,     0,     0,     0,   -12,   -12,
       0,     1,   -12,    97,    98,    99,     0,   100,   101,     0,
       0,     0,     0,   504,   505,   506,   507,   508,   509,   102,
     103,   104,   105,     0,     0,   106,    -8,     0,   107,   108,
     109,    -8,   110,    -8,    -8,   111,   112,   113,   114,   115,
     116,   117,     0,   518,   519,    -8,     0,     0,     0,     0,
       0,    -8,    -8,     0,     0,   118,    -8,   119,    -8,   120,
      -8,   121,    -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,
      -8,     0,    -8,    -8,     0,     0,   122,   123,    -8,    -8,
       0,     0,    -8,     0,     0,     0,    -8,    -8,    -8,   124,
       0,     0,     0,    -8,    -8,     0,     1,    -8,    97,    98,
      99,     0,   100,   101,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   102,   103,   104,   105,     0,     0,
     -12,   -12,     0,   107,   108,   -12,   -12,   110,   -12,   -12,
     111,   112,   113,   114,   115,   116,   117,     0,     0,     0,
     -12,     0,     0,     0,     0,     0,   -12,   -12,     0,     0,
     118,   -12,   119,   -12,   120,   -12,   526,   -12,   -12,   -12,
     -12,   -12,   -12,   -12,   -12,   -12,     0,   -12,   -12,     0,
       0,   122,   123,   -12,   -12,     0,     0,   -12,     0,     0,
       0,   -12,   -12,   -12,   124,     0,     0,     0,   -12,   -12,
       0,     1,   -12,    97,    98,    99,     0,   100,   101,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   102,
     103,   104,   105,     0,     0,   106,     0,     0,   107,   108,
     109,     0,   110,     0,     0,   111,   112,   113,   114,   115,
     116,   117,   225,     0,     0,   226,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   118,    -8,   119,    -8,   120,
      -8,   121,    -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,
      -8,     0,    -8,    -8,     0,     0,   122,   123,    -8,    -8,
     225,     0,    -8,   226,     0,     0,    -8,    -8,    -8,   124,
       0,     0,     0,    -8,    -8,     0,     0,    -8,     0,   357,
     227,   228,   229,   230,   231,   232,     0,     0,     0,   233,
     234,     0,     0,     0,   225,   235,   236,   226,     0,   237,
       0,     0,     0,   238,   239,   240,     0,     0,     0,     0,
     241,   242,     0,     0,     0,     0,     0,  -381,   227,   228,
     229,   230,   231,   232,     0,     0,   225,   233,   234,   226,
       0,     0,     0,   235,   236,     0,     0,   237,     0,     0,
       0,   238,   239,   240,     0,   562,     0,     0,   241,   242,
       0,     0,   227,   228,   229,   230,   231,   232,     0,     0,
     502,   233,   234,   503,     0,     0,     0,   235,   236,     0,
       0,   237,     0,     0,     0,   238,   239,   240,     0,     0,
       0,     0,   241,   242,   227,   228,   229,   230,   231,   232,
       0,     0,   225,   233,   234,   226,     0,     0,     0,   235,
     236,     0,     0,   237,     0,     0,     0,   238,   239,   240,
       0,     0,     0,     0,   241,   242,     0,     0,   504,   505,
     506,   507,   508,   509,     0,     0,   502,   510,   511,   503,
       0,     0,     0,   512,   513,   225,     0,   514,   226,     0,
       0,   515,   516,   517,     0,     0,     0,     0,   518,   519,
     227,   228,   229,   230,   231,   232,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   235,   236,   502,     0,   237,
     503,     0,     0,   238,   239,   240,     0,     0,     0,     0,
     241,   242,     0,     0,   504,   505,   506,   507,   508,   509,
       0,     0,     0,   227,   228,   229,   230,   231,   232,   512,
     513,     0,     0,   514,     0,     0,     0,   515,   516,   517,
       0,     0,     0,     0,   518,   519,   238,   239,   240,     0,
       0,     0,     0,   241,   242,   504,   505,   506,   507,   508,
     509,     0,     0,     1,     0,    97,    98,    99,     0,   100,
     101,     0,     0,     0,     0,     0,     0,     0,   515,   516,
     517,   102,   103,   104,   105,   518,   519,   106,     0,     0,
     107,   108,   109,     0,   110,     0,     0,   111,   112,   113,
     114,   115,   116,   117,     0,     0,     0,    -8,     0,     0,
       0,     0,     0,    -8,    -8,     0,     0,   118,     0,   119,
      -8,   120,     0,   121,     0,     1,     0,    97,    98,    99,
       0,   100,   101,     0,     0,     0,     0,     0,   122,   123,
       0,     0,     0,   102,   103,   104,   105,     0,     0,   106,
       0,   124,   107,   108,   109,     0,   110,   461,   462,   111,
     112,   113,   114,   115,   116,   117,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   118,
       0,   119,    -8,   120,     0,   121,     0,     1,     0,    97,
      98,    99,     0,   100,   101,     0,     0,     0,     0,     0,
     122,   123,     0,     0,     0,   102,   103,   104,   105,     0,
       0,   106,     0,   124,   107,   108,   109,     0,   110,   461,
     462,   111,   112,   113,   114,   115,   116,   117,     0,     0,
       1,     0,    97,    98,    99,     0,   100,   101,     0,     0,
       0,   118,     0,   119,   536,   120,     0,   121,   102,   103,
     104,   105,     0,     0,   106,     0,     0,   107,   108,   109,
       0,   110,   122,   123,   111,   112,   113,   114,   115,   116,
     117,     0,     0,     0,     0,   124,     0,     0,     0,     0,
       0,     0,     0,     0,   118,     0,   119,    -5,   120,     0,
     121,     0,     1,     0,    97,    98,    99,     0,   100,   101,
       0,     0,     0,     0,     0,   122,   123,     0,     0,     0,
     102,   103,   104,   105,     0,     0,   106,     0,   124,   107,
     108,   109,     0,   110,     0,     0,   111,   112,   113,   114,
     115,   116,   117,     0,     0,     1,     0,    97,    98,    99,
     200,   100,   101,     0,   100,   101,   118,     0,   119,     0,
     120,     0,   121,   102,   103,   104,   105,   103,   104,     0,
       0,     0,   107,   108,   456,   -12,   110,   122,   123,   111,
     112,   113,   114,   115,   116,   117,     0,     0,     0,     0,
     124,     1,     0,    97,    98,    99,     0,   100,   101,   118,
       0,   119,   118,   120,   186,   121,     0,     0,     0,   102,
     103,   104,   105,     0,     0,     0,   -12,     0,   107,   108,
     122,   123,   110,   187,   188,   111,   112,   113,   114,   115,
     116,   117,     0,   124,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   118,  -159,   119,     0,   120,
       0,   121,    67,    68,    69,    70,    71,    72,    73,    74,
       0,    75,    76,     0,     0,     0,   122,   123,     0,   297,
      98,    99,     0,   100,   101,     0,   185,    98,    99,   124,
     100,   101,     0,     0,     0,     0,   103,   104,   105,     0,
       0,     0,     0,   103,   104,   105,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  -159,   116,     0,     0,     0,
       0,     0,     0,   116,     0,     0,     0,     0,     0,  -159,
    -159,   118,     0,   186,     0,     0,     0,   121,   118,     0,
     186,     0,  -159,   308,   426,   309,    98,    99,     0,   100,
     101,     0,   122,   123,     0,     0,     0,     0,     0,   187,
     188,     0,     0,     0,     0,   124,     0,   -85,     0,     0,
       0,     0,   427,    67,    68,    69,    70,    71,    72,    73,
      74,     0,    75,    76,   308,     0,   309,    98,    99,     0,
     100,   101,     0,     0,     0,     0,     0,     0,  -144,     0,
       0,     0,     0,   310,  -144,     0,   308,     0,   309,    98,
      99,     0,   100,   101,  -144,  -144,     0,     0,   311,   312,
    -144,  -144,     0,     0,  -144,     0,   -85,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  -144,
    -159,  -159,     0,     0,   310,     0,     0,     0,     0,     0,
       0,     0,     0,  -159,     0,  -144,  -144,     0,     0,   311,
     312,  -144,  -144,     0,     0,  -144,   310,  -144,     0,     0,
       0,  -296,  -296,  -296,     0,  -296,  -296,  -144,  -144,     0,
       0,   311,   312,  -144,  -144,     0,     0,  -144,  -296,  -296,
    -296,     0,     0,     0,   185,    98,    99,   270,   100,   101,
       0,   100,   101,     0,     0,     0,     0,     0,     0,     0,
       0,   103,   104,   105,   103,   104,     0,     0,     0,     0,
       0,     0,     0,  -296,     0,  -296,     0,     0,     0,  -296,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  -296,  -296,   118,     0,   186,   118,
       0,   186,   346,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     1,     0,     0,   187,   188,     0,
     187,   188,  -399,  -399,  -399,  -399,  -399,  -399,  -399,  -399,
       0,  -399,  -399,     0,     0,     0,    53,    54,  -171,     0,
       0,     0,  -171,   165,   166,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  -171,     0,     0,  -171,  -171,   167,
     168,     0,     0,  -171,  -171,     0,  -171,  -171,  -171,     0,
       0,   -36,     0,  -171,  -171,  -171,     0,  -171,  -171,  -171,
    -171,  -171,     0,   166,  -171,     0,     0,     0,     0,     0,
       0,     0,     0,  -171,     0,     0,  -171,  -171,   167,   168,
       0,     0,  -171,  -171,   211,  -171,  -171,  -171,     0,     0,
       0,     0,  -171,  -171,  -171,     0,     0,  -171,  -171,  -171,
     212,     0,     0,   213,   214,     0,     0,     0,     0,   215,
     216,     0,   217,   218,   219,     0,     0,     0,     1,   220,
     221,   222,     0,     0,   223,   224,  -399,  -399,  -399,  -399,
    -399,  -399,  -399,  -399,     0,  -399,  -399,     0,     0,     0,
      53,    54
};

static const yytype_int16 yycheck[] =
{
      91,   207,   163,   122,   123,   173,   191,   194,   195,   193,
     197,   211,   194,   195,   365,   197,   367,   211,   117,     3,
     191,   177,   178,     3,   117,   124,   182,   493,     3,     3,
     445,     1,     3,    42,    43,   540,    40,   614,     0,     3,
       4,     5,     0,     7,     8,    19,     3,   520,     3,     7,
       3,   310,   529,   530,   254,   521,    20,    21,    22,   636,
      54,   538,    59,    59,    61,    62,    24,     4,   187,   188,
      40,    61,   191,   172,    55,    45,    40,    35,    76,    77,
     179,    51,    52,   588,     4,     5,   179,    62,    58,    47,
      57,    55,   183,    57,    55,    60,    61,    61,   197,    57,
      97,   288,     3,    84,    85,    76,    77,   522,     5,   208,
     310,     8,    76,    77,    59,    99,   310,    97,    89,     5,
     586,    59,     8,    84,    85,    89,    76,    77,   227,   228,
     229,   230,   231,   232,   233,   234,   235,   236,   237,   238,
     239,   240,   241,   242,     1,   618,    98,   318,   319,   320,
     321,   322,    40,   252,   325,   326,   327,   328,   329,   632,
     259,   420,   421,   422,   423,   424,    63,    64,    65,    66,
      67,    68,    59,    40,    61,    62,    40,    63,    64,    65,
      66,    67,    68,    40,    76,    77,    72,    73,    45,    58,
     243,     3,    78,    79,    51,    52,    82,   365,   366,   367,
      86,    87,    88,    57,   303,    78,    79,    93,    94,    82,
      97,    97,    62,    69,    70,    62,    97,   373,   317,    46,
     420,   421,   422,   423,   424,    59,   420,   421,   422,   423,
     424,    61,    56,    58,     3,     4,     5,     4,     7,     8,
       3,   592,     3,     3,    62,    62,     7,     8,    72,    73,
      61,    20,    21,    22,    78,    79,   427,    59,    82,    20,
      21,   461,    97,     4,     4,   318,   319,   320,   321,   322,
       3,    40,   325,   326,   327,   328,   329,     3,   369,     0,
       1,   372,   450,    61,   445,    61,    55,   472,    57,   473,
       3,    61,    61,    61,    55,    59,    57,     5,    97,    97,
       8,   472,    59,    97,    59,    71,    62,    76,    77,    61,
      61,    61,   411,   500,    61,    76,    77,    61,   500,     3,
       3,     3,     3,     3,    45,   525,    47,    48,    97,    50,
      55,    62,    62,   504,   505,   506,   507,   508,   509,   510,
     511,   512,   513,    62,   515,   516,   517,   518,   519,    19,
      57,    37,     3,   472,    56,    63,    64,    65,    66,    67,
      68,   522,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    56,   471,   464,   427,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,    58,   492,   493,   319,   320,   321,   322,    58,
      58,   325,   326,   327,   328,   329,     3,     3,    62,     3,
       5,   617,    62,     8,    62,   514,    71,    62,    61,    61,
       3,   520,   521,     5,   592,     3,     8,    72,    73,    61,
     529,   530,    61,    78,    79,    56,    62,    82,    62,   538,
      97,    26,    62,    62,    62,    59,    27,    62,    60,    62,
     541,   504,   505,   506,   507,   508,   509,   510,   511,   512,
     513,    97,   515,   516,   517,   518,   519,    97,    63,    64,
      65,    66,    67,    68,    56,   528,     5,    31,    60,     8,
      97,    63,    64,    65,    66,    67,    68,   586,    97,    97,
      72,    73,    31,    60,    59,    97,    78,    79,    93,    94,
      82,    62,    62,   427,    86,    87,    88,    62,    30,    62,
      62,    93,    94,    62,    60,    59,    84,    97,     3,   618,
      62,    62,    62,    61,     7,   366,    55,   281,    35,    57,
     265,   622,   623,   632,    63,    64,    65,    66,    67,    68,
     631,   257,    71,    72,    73,    74,    75,    48,   593,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
     195,    90,    91,    92,    93,    94,    95,    96,     9,    10,
      11,    12,    13,    14,    15,    16,    56,    18,    19,   500,
     504,   505,   506,   507,   508,   509,   510,   511,   512,   513,
     525,   515,   516,   517,   518,   519,     1,   473,     3,     4,
       5,   472,     7,     8,   464,    41,    60,    94,    -1,    -1,
      -1,    -1,    -1,    -1,    19,    20,    21,    22,    -1,    -1,
      25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,     5,    51,    52,     8,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    -1,
      -1,    76,    77,    78,    79,    -1,    -1,    82,    -1,    -1,
      -1,    86,    87,    88,    89,    -1,    -1,    -1,    93,    94,
      -1,     1,    97,     3,     4,     5,    -1,     7,     8,    -1,
      -1,    -1,    -1,    63,    64,    65,    66,    67,    68,    19,
      20,    21,    22,    -1,    -1,    25,    26,    -1,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    -1,    93,    94,    45,    -1,    -1,    -1,    -1,
      -1,    51,    52,    -1,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    -1,    -1,    76,    77,    78,    79,
      -1,    -1,    82,    -1,    -1,    -1,    86,    87,    88,    89,
      -1,    -1,    -1,    93,    94,    -1,     1,    97,     3,     4,
       5,    -1,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    19,    20,    21,    22,    -1,    -1,
      25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    -1,    72,    73,    -1,
      -1,    76,    77,    78,    79,    -1,    -1,    82,    -1,    -1,
      -1,    86,    87,    88,    89,    -1,    -1,    -1,    93,    94,
      -1,     1,    97,     3,     4,     5,    -1,     7,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,
      20,    21,    22,    -1,    -1,    25,    -1,    -1,    28,    29,
      30,    -1,    32,    -1,    -1,    35,    36,    37,    38,    39,
      40,    41,     5,    -1,    -1,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    -1,    72,    73,    -1,    -1,    76,    77,    78,    79,
       5,    -1,    82,     8,    -1,    -1,    86,    87,    88,    89,
      -1,    -1,    -1,    93,    94,    -1,    -1,    97,    -1,    62,
      63,    64,    65,    66,    67,    68,    -1,    -1,    -1,    72,
      73,    -1,    -1,    -1,     5,    78,    79,     8,    -1,    82,
      -1,    -1,    -1,    86,    87,    88,    -1,    -1,    -1,    -1,
      93,    94,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,
      65,    66,    67,    68,    -1,    -1,     5,    72,    73,     8,
      -1,    -1,    -1,    78,    79,    -1,    -1,    82,    -1,    -1,
      -1,    86,    87,    88,    -1,    56,    -1,    -1,    93,    94,
      -1,    -1,    63,    64,    65,    66,    67,    68,    -1,    -1,
       5,    72,    73,     8,    -1,    -1,    -1,    78,    79,    -1,
      -1,    82,    -1,    -1,    -1,    86,    87,    88,    -1,    -1,
      -1,    -1,    93,    94,    63,    64,    65,    66,    67,    68,
      -1,    -1,     5,    72,    73,     8,    -1,    -1,    -1,    78,
      79,    -1,    -1,    82,    -1,    -1,    -1,    86,    87,    88,
      -1,    -1,    -1,    -1,    93,    94,    -1,    -1,    63,    64,
      65,    66,    67,    68,    -1,    -1,     5,    72,    73,     8,
      -1,    -1,    -1,    78,    79,     5,    -1,    82,     8,    -1,
      -1,    86,    87,    88,    -1,    -1,    -1,    -1,    93,    94,
      63,    64,    65,    66,    67,    68,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    79,     5,    -1,    82,
       8,    -1,    -1,    86,    87,    88,    -1,    -1,    -1,    -1,
      93,    94,    -1,    -1,    63,    64,    65,    66,    67,    68,
      -1,    -1,    -1,    63,    64,    65,    66,    67,    68,    78,
      79,    -1,    -1,    82,    -1,    -1,    -1,    86,    87,    88,
      -1,    -1,    -1,    -1,    93,    94,    86,    87,    88,    -1,
      -1,    -1,    -1,    93,    94,    63,    64,    65,    66,    67,
      68,    -1,    -1,     1,    -1,     3,     4,     5,    -1,     7,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,
      88,    19,    20,    21,    22,    93,    94,    25,    -1,    -1,
      28,    29,    30,    -1,    32,    -1,    -1,    35,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    45,    -1,    -1,
      -1,    -1,    -1,    51,    52,    -1,    -1,    55,    -1,    57,
      58,    59,    -1,    61,    -1,     1,    -1,     3,     4,     5,
      -1,     7,     8,    -1,    -1,    -1,    -1,    -1,    76,    77,
      -1,    -1,    -1,    19,    20,    21,    22,    -1,    -1,    25,
      -1,    89,    28,    29,    30,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,
      -1,    57,    58,    59,    -1,    61,    -1,     1,    -1,     3,
       4,     5,    -1,     7,     8,    -1,    -1,    -1,    -1,    -1,
      76,    77,    -1,    -1,    -1,    19,    20,    21,    22,    -1,
      -1,    25,    -1,    89,    28,    29,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    -1,    -1,
       1,    -1,     3,     4,     5,    -1,     7,     8,    -1,    -1,
      -1,    55,    -1,    57,    58,    59,    -1,    61,    19,    20,
      21,    22,    -1,    -1,    25,    -1,    -1,    28,    29,    30,
      -1,    32,    76,    77,    35,    36,    37,    38,    39,    40,
      41,    -1,    -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    55,    -1,    57,    58,    59,    -1,
      61,    -1,     1,    -1,     3,     4,     5,    -1,     7,     8,
      -1,    -1,    -1,    -1,    -1,    76,    77,    -1,    -1,    -1,
      19,    20,    21,    22,    -1,    -1,    25,    -1,    89,    28,
      29,    30,    -1,    32,    -1,    -1,    35,    36,    37,    38,
      39,    40,    41,    -1,    -1,     1,    -1,     3,     4,     5,
       3,     7,     8,    -1,     7,     8,    55,    -1,    57,    -1,
      59,    -1,    61,    19,    20,    21,    22,    20,    21,    -1,
      -1,    -1,    28,    29,    30,    31,    32,    76,    77,    35,
      36,    37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,
      89,     1,    -1,     3,     4,     5,    -1,     7,     8,    55,
      -1,    57,    55,    59,    57,    61,    -1,    -1,    -1,    19,
      20,    21,    22,    -1,    -1,    -1,    26,    -1,    28,    29,
      76,    77,    32,    76,    77,    35,    36,    37,    38,    39,
      40,    41,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    55,     3,    57,    -1,    59,
      -1,    61,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    18,    19,    -1,    -1,    -1,    76,    77,    -1,     3,
       4,     5,    -1,     7,     8,    -1,     3,     4,     5,    89,
       7,     8,    -1,    -1,    -1,    -1,    20,    21,    22,    -1,
      -1,    -1,    -1,    20,    21,    22,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    62,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    76,
      77,    55,    -1,    57,    -1,    -1,    -1,    61,    55,    -1,
      57,    -1,    89,     1,    61,     3,     4,     5,    -1,     7,
       8,    -1,    76,    77,    -1,    -1,    -1,    -1,    -1,    76,
      77,    -1,    -1,    -1,    -1,    89,    -1,     3,    -1,    -1,
      -1,    -1,    89,     9,    10,    11,    12,    13,    14,    15,
      16,    -1,    18,    19,     1,    -1,     3,     4,     5,    -1,
       7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    61,    62,    -1,     1,    -1,     3,     4,
       5,    -1,     7,     8,    72,    73,    -1,    -1,    76,    77,
      78,    79,    -1,    -1,    82,    -1,    62,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      76,    77,    -1,    -1,    61,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    -1,    72,    73,    -1,    -1,    76,
      77,    78,    79,    -1,    -1,    82,    61,    62,    -1,    -1,
      -1,     3,     4,     5,    -1,     7,     8,    72,    73,    -1,
      -1,    76,    77,    78,    79,    -1,    -1,    82,    20,    21,
      22,    -1,    -1,    -1,     3,     4,     5,     3,     7,     8,
      -1,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    20,    21,    22,    20,    21,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    57,    -1,    -1,    -1,    61,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    76,    77,    55,    -1,    57,    55,
      -1,    57,    61,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     1,    -1,    -1,    76,    77,    -1,
      76,    77,     9,    10,    11,    12,    13,    14,    15,    16,
      -1,    18,    19,    -1,    -1,    -1,    23,    24,    55,    -1,
      -1,    -1,    59,    60,    61,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    -1,    -1,    74,    75,    76,
      77,    -1,    -1,    80,    81,    -1,    83,    84,    85,    -1,
      -1,    58,    -1,    90,    91,    92,    -1,    55,    95,    96,
      97,    59,    -1,    61,    62,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    -1,    -1,    74,    75,    76,    77,
      -1,    -1,    80,    81,    55,    83,    84,    85,    -1,    -1,
      -1,    -1,    90,    91,    92,    -1,    -1,    95,    96,    97,
      71,    -1,    -1,    74,    75,    -1,    -1,    -1,    -1,    80,
      81,    -1,    83,    84,    85,    -1,    -1,    -1,     1,    90,
      91,    92,    -1,    -1,    95,    96,     9,    10,    11,    12,
      13,    14,    15,    16,    -1,    18,    19,    -1,    -1,    -1,
      23,    24
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     1,    45,    47,    48,    50,   101,   107,   288,    40,
       3,     3,   108,     0,   101,     3,   261,   109,    54,     3,
      99,   122,    61,    59,    57,     4,   123,    59,   262,    40,
      45,    51,    52,   112,   274,   282,   288,   110,    98,     3,
     264,    40,    40,    40,    58,     3,   112,    57,   124,   265,
      62,   261,   283,    23,    24,   111,   280,   281,   288,   122,
      97,    46,   263,    59,    61,   118,    58,     9,    10,    11,
      12,    13,    14,    15,    16,    18,    19,   217,   219,   111,
     264,     4,     3,   286,   284,     3,   278,   287,    62,    42,
      43,   285,    61,    59,    97,     4,     4,     3,     4,     5,
       7,     8,    19,    20,    21,    22,    25,    28,    29,    30,
      32,    35,    36,    37,    38,    39,    40,    41,    55,    57,
      59,    61,    76,    77,    89,   103,   104,   106,   114,   119,
     120,   121,   125,   126,   128,   129,   134,   140,   147,   150,
     163,   164,   165,   178,   183,   184,   210,   212,   221,   223,
     224,   226,   227,   228,   230,   232,   234,   235,   238,   248,
     251,   252,   288,   279,   286,    60,    61,    76,    77,     3,
       3,    61,    61,   148,   152,     3,   130,   135,   137,    61,
     116,   151,   141,   143,    61,     3,    57,    76,    77,   212,
     260,   240,   105,   239,   182,   211,   219,   222,     3,   227,
       3,   227,   212,    97,    59,    71,    59,    97,    55,    84,
      85,    55,    71,    74,    75,    80,    81,    83,    84,    85,
      90,    91,    92,    95,    96,     5,     8,    63,    64,    65,
      66,    67,    68,    72,    73,    78,    79,    82,    86,    87,
      88,    93,    94,   225,    76,    77,    59,    17,    18,   217,
     218,   275,   231,    61,   233,   212,   104,    61,   159,    61,
     117,   117,   117,   260,   253,    61,   154,   117,   106,   249,
       3,     3,    61,   227,   228,   229,   243,   245,   247,   248,
     102,   106,     3,   241,   245,     3,    76,    77,    89,   165,
     184,   192,   213,   165,   192,   209,    62,     3,   165,   184,
     212,   220,     3,   127,   120,   212,     3,     3,     1,     3,
      61,    76,    77,   177,   191,   234,   235,   166,   167,   168,
     169,   170,   171,     3,     3,   172,   173,   174,   175,   176,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   212,
     212,   212,   212,   212,   212,   212,    61,   183,    55,     3,
     276,    62,   212,   268,    62,   234,   236,    62,   149,     3,
     125,   158,   256,   259,   260,   131,   136,   138,    62,    57,
     114,   153,   142,    37,     3,   266,   246,    56,   244,    58,
     102,    58,   242,     3,     3,     3,   184,     5,     8,    63,
      64,    65,    66,    67,    68,    72,    73,    78,    79,    82,
      86,    87,    88,    93,    94,    62,    62,    62,    71,   212,
      56,    60,    61,    61,    76,    77,   191,     3,     3,    56,
      72,    73,    78,    79,    82,   212,    61,    89,   183,   214,
     216,   248,   216,   216,   216,   216,    61,    61,   216,   216,
     216,   216,   216,    56,   277,    97,   269,    62,   237,    62,
      26,   144,    27,    62,    69,    70,    30,   104,   129,   103,
     129,    33,    34,   106,   113,   254,    59,   106,   117,   267,
      62,    60,    97,    97,   203,   206,   205,   208,   207,   204,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     115,   212,   179,   181,    62,   191,   191,   191,   191,   191,
     215,   216,     5,     8,    63,    64,    65,    66,    67,    68,
      72,    73,    78,    79,    82,    86,    87,    88,    93,    94,
     186,   185,    97,   275,    97,    97,    61,   104,   160,   257,
     258,    31,   132,    31,   234,    60,    58,   254,   155,    59,
      97,   250,   212,   243,   241,   212,   212,   212,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   212,
     212,   212,    56,   212,   271,   268,   213,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   212,   216,   216,
     216,   216,   216,   271,   268,   275,   270,   236,   145,   183,
     256,   256,   133,   139,   255,   256,   266,   106,   272,    62,
      62,    62,    62,    62,   268,   266,    62,   129,    30,   134,
      60,    59,    97,   180,   187,    62,   161,   156,   273,    84,
     188,   188,   146,   162,   120,   271,     3,   106,   106,    62,
      61,   157,   189,   106,   271,    62,   190,   188
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   100,   101,   101,   101,   102,   102,   102,   103,   103,
     103,   103,   104,   104,   105,   104,   104,   104,   104,   104,
     104,   104,   104,   106,   106,   106,   107,   108,   107,   109,
     107,   110,   107,   107,   107,   111,   111,   112,   112,   113,
     113,   113,   114,   115,   114,   114,   116,   117,   118,   119,
     120,   120,   120,   121,   123,   124,   122,   122,   122,   125,
     127,   126,   126,   128,   129,   130,   131,   129,   129,   132,
     133,   132,   134,   135,   136,   134,   137,   138,   139,   134,
     141,   142,   140,   143,   140,   145,   146,   144,   144,   148,
     149,   147,   151,   150,   152,   150,   153,   153,   155,   156,
     157,   154,   158,   158,   160,   161,   162,   159,   163,   163,
     163,   163,   163,   163,   163,   164,   164,   164,   166,   165,
     167,   165,   168,   165,   169,   165,   170,   165,   171,   165,
     172,   165,   173,   165,   174,   165,   175,   165,   176,   165,
     165,   165,   165,   165,   177,   177,   177,   177,   177,   178,
     179,   180,   178,   181,   178,   178,   178,   178,   178,   182,
     178,   183,   183,   184,   184,   184,   185,   184,   186,   187,
     184,   184,   188,   189,   190,   188,   191,   191,   191,   191,
     191,   191,   191,   191,   191,   191,   191,   192,   192,   192,
     193,   192,   194,   192,   195,   192,   196,   192,   197,   192,
     198,   192,   199,   192,   200,   192,   201,   192,   202,   192,
     203,   192,   204,   192,   205,   192,   206,   192,   207,   192,
     208,   192,   192,   209,   209,   211,   210,   212,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   213,
     213,   215,   214,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   216,   216,   216,   216,   216,
     216,   216,   216,   216,   216,   217,   217,   217,   217,   217,
     217,   217,   218,   218,   218,   218,   219,   219,   219,   219,
     220,   220,   222,   221,   223,   223,   225,   224,   224,   226,
     226,   226,   226,   227,   227,   227,   227,   227,   227,   227,
     228,   228,   228,   228,   229,   229,   229,   231,   230,   233,
     232,   234,   234,   235,   235,   236,   237,   236,   236,   239,
     238,   240,   238,   241,   242,   241,   241,   243,   244,   243,
     243,   245,   246,   245,   247,   247,   247,   249,   250,   248,
     251,   253,   252,   254,   255,   254,   254,   254,   257,   256,
     258,   256,   256,   259,   260,   262,   261,   263,   263,   264,
     265,   264,   264,   266,   267,   266,   266,   268,   269,   270,
     268,   268,   271,   271,   272,   273,   271,   274,   274,   274,
     275,   276,   275,   275,   277,   275,   275,   279,   278,   280,
     280,   280,   281,   281,   283,   284,   282,   282,   282,   285,
     285,   285,   286,   287,   286,   286,   288
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     0,     2,     1,     0,     1,
       1,     1,     0,     2,     0,     4,     1,     1,     1,     1,
       1,     2,     1,     0,     1,     1,     0,     0,     4,     0,
       6,     0,     8,     4,     1,     2,     1,     2,     1,     0,
       2,     1,     3,     0,     6,     1,     3,     3,     3,     2,
       0,     3,     1,     1,     0,     0,     5,     1,     1,     2,
       0,     4,     1,     2,     0,     0,     0,     6,     1,     0,
       0,     3,     0,     0,     0,     5,     0,     0,     0,     8,
       0,     0,     5,     0,     6,     0,     0,     7,     2,     0,
       0,     5,     0,     3,     0,     3,     0,     1,     0,     0,
       0,    11,     1,     1,     0,     0,     0,     9,     0,     1,
       1,     1,     1,     2,     2,     1,     1,     1,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       2,     2,     2,     2,     0,     2,     2,     2,     2,     1,
       0,     0,     9,     0,     7,     3,     3,     4,     6,     0,
       4,     1,     1,     3,     3,     4,     0,     7,     0,     0,
       9,     1,     0,     0,     0,     8,     3,     3,     3,     3,
       3,     3,     1,     1,     1,     1,     1,     0,     2,     2,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     4,     2,     1,     1,     0,     4,     1,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     1,     1,     1,
       1,     0,     4,     1,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     4,     3,     1,     0,     3,     1,     4,
       1,     1,     1,     1,     1,     1,     2,     2,     2,     2,
       1,     1,     4,     1,     1,     1,     1,     0,     5,     0,
       5,     1,     1,     1,     1,     0,     0,     4,     1,     0,
       4,     0,     4,     0,     0,     4,     1,     0,     0,     4,
       1,     0,     0,     4,     0,     1,     1,     0,     0,     7,
       1,     0,     6,     0,     0,     4,     2,     1,     0,     4,
       0,     4,     1,     1,     1,     0,     6,     0,     2,     0,
       0,     4,     1,     0,     0,     4,     1,     0,     0,     0,
       5,     1,     0,     1,     0,     0,     5,     2,     2,     1,
       0,     0,     4,     1,     0,     5,     2,     0,     5,     0,
       1,     1,     4,     1,     0,     0,     7,     4,     1,     0,
       2,     2,     0,     0,     4,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 13:
#line 201 "stack.y" /* yacc.c:1646  */
    { sp_flush_scope_now(); }
#line 2161 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 202 "stack.y" /* yacc.c:1646  */
    { sp_create_new_scope(NULL); sp_push_varctx_scope(); }
#line 2167 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 202 "stack.y" /* yacc.c:1646  */
    { sp_pop_varctx_scope(); sp_flush_scope_now(); sp_destroy_scope(); }
#line 2173 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 209 "stack.y" /* yacc.c:1646  */
    { sp_flush_scope_now(); }
#line 2179 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 216 "stack.y" /* yacc.c:1646  */
    { }
#line 2185 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 216 "stack.y" /* yacc.c:1646  */
    { }
#line 2191 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 217 "stack.y" /* yacc.c:1646  */
    { sp_start_class_body((yyvsp[0].string)); }
#line 2197 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 217 "stack.y" /* yacc.c:1646  */
    { sp_end_class_body(); }
#line 2203 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 218 "stack.y" /* yacc.c:1646  */
    { sp_start_interface_body((yyvsp[-2].string), (yyvsp[0].bytes).value); }
#line 2209 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 218 "stack.y" /* yacc.c:1646  */
    { sp_end_interface_body(); }
#line 2215 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 235 "stack.y" /* yacc.c:1646  */
    { sp_create_variable((yyvsp[0].string)); }
#line 2221 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 236 "stack.y" /* yacc.c:1646  */
    { sp_create_new_scope(NULL); }
#line 2227 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 236 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); symrec * rec = sp_create_variable((yyvsp[-3].string)); sp_flush_scope(); sp_destroy_scope(); sp_store_stack(rec->name); sp_clear_operation_stack(); sp_lhs_clear(); }
#line 2233 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 237 "stack.y" /* yacc.c:1646  */
    { }
#line 2239 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 241 "stack.y" /* yacc.c:1646  */
    { sp_clear_operation_stack(); sp_lhs_clear(); }
#line 2245 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 245 "stack.y" /* yacc.c:1646  */
    { sp_flush_scope_now(); sp_clear_operation_stack(); sp_lhs_clear(); }
#line 2251 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 251 "stack.y" /* yacc.c:1646  */
    { sp_clear_operation_stack(); sp_lhs_clear(); }
#line 2257 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 254 "stack.y" /* yacc.c:1646  */
    { sp_push_construct((yyvsp[0].string)); }
#line 2263 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 254 "stack.y" /* yacc.c:1646  */
    { sp_push_construct(_RECAST(uchar *, "\\")); }
#line 2269 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 255 "stack.y" /* yacc.c:1646  */
    { sp_push_construct((yyvsp[0].string)); }
#line 2275 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 258 "stack.y" /* yacc.c:1646  */
    { strcpy(_RECAST(char *, (yyval.string)), _RECAST(const char *, (yyvsp[0].string))); /*sp_push_symrec(sp_create_variable($2));*/ }
#line 2281 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 260 "stack.y" /* yacc.c:1646  */
    { sp_create_new_scope(NULL); }
#line 2287 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 260 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); symrec * rec = sp_create_variable((yyvsp[-3].string)); sp_flush_scope_now(); sp_destroy_scope(); sp_store_stack(rec->name); sp_clear_operation_stack(); sp_lhs_clear(); }
#line 2293 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 261 "stack.y" /* yacc.c:1646  */
    { sp_create_variable((yyvsp[0].string)); }
#line 2299 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 263 "stack.y" /* yacc.c:1646  */
    { sp_new_label((yyvsp[-1].string)); }
#line 2305 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 266 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_create_label(_RECAST(uchar *, "__b"))); sp_push_symrec(sp_create_label(_RECAST(uchar *, "__i"))); }
#line 2311 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 266 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); }
#line 2317 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 268 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); }
#line 2323 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 273 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); symrec * rel = sp_create_label(_RECAST(uchar *, "__l")); sp_push_symrec(rel); sp_jump_to(INS_JMP, rel->name); sp_new_label(rec->name); }
#line 2329 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 277 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_create_label(_RECAST(uchar *, "__b"))); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__i"))); }
#line 2335 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 277 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); }
#line 2341 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 277 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); }
#line 2347 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 278 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_create_label(_RECAST(uchar *,"__b"))); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__i"))); }
#line 2353 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 278 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); }
#line 2359 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 280 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); symrec * rel = sp_create_label(_RECAST(uchar *,"__l")); sp_push_symrec(rel); sp_jump_to(INS_JMP, rel->name); sp_new_label(rec->name); }
#line 2365 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 282 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); }
#line 2371 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 284 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_create_label(_RECAST(uchar *,"__h"))); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__w"))); sp_start_loop(); }
#line 2377 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 284 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_new_label(rec->name); }
#line 2383 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 285 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_end_loop(); sp_new_label(rec->name); }
#line 2389 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 286 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_start_loop()); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__w"))); }
#line 2395 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 287 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_pop_symrec(); sp_flush_scope_now(); sp_new_label(rec->name); }
#line 2401 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 289 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_start_lambda_decl(0)); }
#line 2407 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 290 "stack.y" /* yacc.c:1646  */
    { symrec * func; symrec * wfunc = sp_pop_symrec(); sp_execute_lambda_call(wfunc); 
  func = sp_start_lambda_body(sp_pop_symrec()); if(func == NULL) YYABORT;
		sp_push_symrec(wfunc); sp_push_symrec(func); sp_flush_scope_now(); }
#line 2415 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 294 "stack.y" /* yacc.c:1646  */
    { symrec * func = sp_pop_symrec(); sp_end_lambda_decl(func); sp_push_inode(sp_create_inode(INS_CALL, func)); }
#line 2421 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 297 "stack.y" /* yacc.c:1646  */
    { sp_flush_scope_now(); sp_push_try_catch(); }
#line 2427 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 299 "stack.y" /* yacc.c:1646  */
    {  symrec * rel = sp_create_label(_RECAST(uchar *,"__tc")); sp_push_symrec(rel); sp_jump_to(INS_JMP, rel->name); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__cr"))); sp_set_cblock(); }
#line 2433 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 301 "stack.y" /* yacc.c:1646  */
    { symrec * func = sp_pop_symrec(); symrec * rel = sp_pop_symrec(); sp_flush_scope_now(); sp_new_label(rel->name); 
	sp_pop_try_catch(func->ptotal& 0x7F); }
#line 2440 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 304 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_create_label(_RECAST(uchar *,"__f"))); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__i"))); }
#line 2446 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 305 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); rec = sp_pop_symrec(); sp_new_label(rec->name); }
#line 2452 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 306 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_create_label(_RECAST(uchar *,"__f"))); sp_push_symrec(sp_create_label(_RECAST(uchar *,"__i"))); }
#line 2458 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 307 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); rec = sp_pop_symrec(); sp_new_label(rec->name); 
	sp_flush_scope_now(); }
#line 2465 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 310 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_create_label(_RECAST(uchar *,"__fv"))); /* dummy variable */ }
#line 2471 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 311 "stack.y" /* yacc.c:1646  */
    { sp_clear_operation_stack(); sp_lhs_clear(); }
#line 2477 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 315 "stack.y" /* yacc.c:1646  */
    { sp_flush_scope_now(); sp_clear_current_scope(); sp_start_loop(); }
#line 2483 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 316 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_flush_scope_now(); sp_new_label(rec->name); sp_create_new_scope(NULL); }
#line 2489 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 317 "stack.y" /* yacc.c:1646  */
    { sp_clear_current_scope(); sp_create_new_scope(NULL); }
#line 2495 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 317 "stack.y" /* yacc.c:1646  */
    { sp_merge_scope_left(); sp_end_scope(); sp_end_loop(); sp_clear_current_scope(); }
#line 2501 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 319 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_create_variable((yyvsp[0].string))); }
#line 2507 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 320 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, (yyvsp[0].string))); }
#line 2513 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 322 "stack.y" /* yacc.c:1646  */
    { sp_flush_scope_now(); sp_clear_current_scope(); }
#line 2519 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 323 "stack.y" /* yacc.c:1646  */
    { symrec * rec; sp_pop_symrec(); sp_start_each_loop(sp_pop_symrec()); sp_flush_scope_now(); rec = sp_pop_symrec(); ; 
	sp_jump_to(INS_JFALSE, sp_peek_symrec()->name); sp_push_symrec(rec); sp_create_new_scope(NULL); }
#line 2526 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 325 "stack.y" /* yacc.c:1646  */
    { sp_clear_current_scope(); sp_create_new_scope(NULL); }
#line 2532 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 325 "stack.y" /* yacc.c:1646  */
    { sp_merge_scope_left(); sp_end_scope(); sp_end_each_loop(); }
#line 2538 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 328 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); }
#line 2544 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 329 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_peek_symrec(); sp_jump_to(INS_JMP, rec->name); }
#line 2550 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 330 "stack.y" /* yacc.c:1646  */
    { sp_continue_loop(); }
#line 2556 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 331 "stack.y" /* yacc.c:1646  */
    { sp_exit_function_body(); }
#line 2562 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 332 "stack.y" /* yacc.c:1646  */
    { sp_return_function_body(); }
#line 2568 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 333 "stack.y" /* yacc.c:1646  */
    { sp_jump_to(INS_JMP, (yyvsp[0].string)); }
#line 2574 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 335 "stack.y" /* yacc.c:1646  */
    { sp_push_inode(sp_create_inode(INS_OBJDEL, NULL)); sp_clear_operation_stack(); sp_lhs_clear(); sm_printf("lazy_stmt>lazy_expr\n"); }
#line 2580 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 336 "stack.y" /* yacc.c:1646  */
    { sp_clear_operation_stack(); sp_lhs_clear(); }
#line 2586 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 337 "stack.y" /* yacc.c:1646  */
    { sp_push_inode(sp_create_inode(INS_OBJDEL, NULL)); sp_clear_operation_stack(); sp_lhs_clear(); sm_printf("lazy_stmt>lhs_val\n");  }
#line 2592 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 339 "stack.y" /* yacc.c:1646  */
    { sm_printf("equalizer\n"); sp_lhs_push_api_call((yyvsp[-1].api_id)); sp_set_scope_var(sp_peek_symrec()); }
#line 2598 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 339 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_end_expr(sp_peek_symrec()); sm_printf("assign %s\n", sp_peek_symrec()->name); }
#line 2604 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 340 "stack.y" /* yacc.c:1646  */
    { sp_lhs_push_api_call((yyvsp[-1].api_id)); sp_lhs_get((yyvsp[-1].api_id)); sp_set_scope_var(sp_peek_symrec()); }
#line 2610 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 340 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_ADD, NULL)); sp_end_expr(sp_peek_symrec()); }
#line 2616 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 341 "stack.y" /* yacc.c:1646  */
    { sp_lhs_push_api_call((yyvsp[-1].api_id)); sp_lhs_get((yyvsp[-1].api_id)); sp_set_scope_var(sp_peek_symrec()); }
#line 2622 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 341 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SUB, NULL)); sp_end_expr(sp_peek_symrec()); }
#line 2628 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 342 "stack.y" /* yacc.c:1646  */
    { sp_lhs_push_api_call((yyvsp[-1].api_id)); sp_lhs_get((yyvsp[-1].api_id)); sp_set_scope_var(sp_peek_symrec()); }
#line 2634 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 342 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MUL, NULL)); sp_end_expr(sp_peek_symrec()); }
#line 2640 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 343 "stack.y" /* yacc.c:1646  */
    { sp_lhs_push_api_call((yyvsp[-1].api_id)); sp_lhs_get((yyvsp[-1].api_id)); sp_set_scope_var(sp_peek_symrec()); }
#line 2646 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 343 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_DIV, NULL)); sp_end_expr(sp_peek_symrec()); }
#line 2652 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 344 "stack.y" /* yacc.c:1646  */
    { sp_lhs_push_api_call((yyvsp[-1].api_id)); sp_lhs_get((yyvsp[-1].api_id)); sp_set_scope_var(sp_peek_symrec()); }
#line 2658 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 344 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MOD, NULL)); sp_end_expr(sp_peek_symrec()); }
#line 2664 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 345 "stack.y" /* yacc.c:1646  */
    { sp_lhs_push_api_call((yyvsp[-1].api_id)); sp_lhs_get((yyvsp[-1].api_id)); sp_set_scope_var(sp_peek_symrec()); }
#line 2670 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 345 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_AND, NULL)); sp_end_expr(sp_peek_symrec()); }
#line 2676 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 346 "stack.y" /* yacc.c:1646  */
    { sp_lhs_push_api_call((yyvsp[-1].api_id)); sp_lhs_get((yyvsp[-1].api_id)); sp_set_scope_var(sp_peek_symrec()); }
#line 2682 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 346 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_OR, NULL)); sp_end_expr(sp_peek_symrec()); }
#line 2688 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 347 "stack.y" /* yacc.c:1646  */
    { sp_lhs_push_api_call((yyvsp[-1].api_id)); sp_lhs_get((yyvsp[-1].api_id)); sp_set_scope_var(sp_peek_symrec()); }
#line 2694 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 347 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_XOR, NULL)); sp_end_expr(sp_peek_symrec()); }
#line 2700 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 348 "stack.y" /* yacc.c:1646  */
    { sp_lhs_push_api_call((yyvsp[-1].api_id)); sp_lhs_get((yyvsp[-1].api_id)); sp_set_scope_var(sp_peek_symrec()); }
#line 2706 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 348 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHL, NULL)); sp_end_expr(sp_peek_symrec()); }
#line 2712 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 349 "stack.y" /* yacc.c:1646  */
    { sp_lhs_push_api_call((yyvsp[-1].api_id)); sp_lhs_get((yyvsp[-1].api_id)); sp_set_scope_var(sp_peek_symrec()); }
#line 2718 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 349 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHR, NULL)); sp_end_expr(sp_peek_symrec()); }
#line 2724 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 350 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, (yyvsp[0].string))); sp_set_scope_var(sp_peek_symrec()); sp_load_stack((yyvsp[0].string)); sp_load_constant(_RECAST(uchar *, "1")); sp_operation_stack(INS_ADD); sp_store_stack((yyvsp[0].string)); }
#line 2730 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 351 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, (yyvsp[0].string))); sp_set_scope_var(sp_peek_symrec()); sp_load_stack((yyvsp[0].string)); sp_load_constant(_RECAST(uchar *, "1")); sp_operation_stack(INS_SUB); sp_store_stack((yyvsp[0].string)); }
#line 2736 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 352 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, (yyvsp[-1].string))); sp_set_scope_var(sp_peek_symrec()); sp_lz_constant_after_scope((yyvsp[-1].string), _RECAST(uchar *, "1"), INS_ADD); }
#line 2742 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 353 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, (yyvsp[-1].string))); sp_set_scope_var(sp_peek_symrec()); sp_lz_constant_after_scope((yyvsp[-1].string), _RECAST(uchar *, "1"), INS_SUB); }
#line 2748 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 369 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, (yyvsp[0].string))); sp_set_scope_var(sp_peek_symrec()); sp_load_stack((yyvsp[0].string)); sp_load_constant(_RECAST(uchar *, "1")); sp_operation_stack(INS_ADD); sp_store_stack((yyvsp[0].string)); }
#line 2754 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 370 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, (yyvsp[0].string))); sp_set_scope_var(sp_peek_symrec()); sp_load_stack((yyvsp[0].string)); sp_load_constant(_RECAST(uchar *, "1")); sp_operation_stack(INS_SUB); sp_store_stack((yyvsp[0].string)); }
#line 2760 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 371 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, (yyvsp[-1].string))); sp_set_scope_var(sp_peek_symrec()); sp_lz_constant_after_scope((yyvsp[-1].string), _RECAST(uchar *, "1"), INS_ADD); }
#line 2766 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 372 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(st_sym_select(SYM_TYPE_VAR, (yyvsp[-1].string))); sp_set_scope_var(sp_peek_symrec()); sp_lz_constant_after_scope((yyvsp[-1].string), _RECAST(uchar *, "1"), INS_SUB); }
#line 2772 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 377 "stack.y" /* yacc.c:1646  */
    { sm_printf("lazy_val_decl %s\n", sp_peek_symrec()->name); }
#line 2778 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 379 "stack.y" /* yacc.c:1646  */
    { sm_printf("instance method call\n"); sp_create_method_scope(); sp_push_symrec(sp_start_method_call((yyvsp[-1].string))); sp_create_new_scope(NULL); }
#line 2784 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 379 "stack.y" /* yacc.c:1646  */
    { symrec * meth = sp_pop_symrec();  sp_flush_scope_now(); sp_destroy_scope(); sp_end_method_call(meth); sm_printf("lhs method call\n"); }
#line 2790 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 379 "stack.y" /* yacc.c:1646  */
    { sp_destroy_method_scope(); }
#line 2796 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 380 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_flush_scope_now(); sm_printf("rhs api access %s\n", (yyvsp[-1].string)); sp_push_symrec(sp_start_function_call((yyvsp[-1].string), 1)); sp_create_new_scope(NULL); }
#line 2802 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 380 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_flush_scope_now(); sp_destroy_scope(); sp_push_symrec(sp_end_function_call(rec)); }
#line 2808 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 381 "stack.y" /* yacc.c:1646  */
    { sm_printf("ins var access 2\n"); sp_lz_load_constant((yyvsp[0].string)); (yyval.api_id)=2; sp_push_symrec(sp_lhs_load(2, 1, sp_pop_symrec())); sm_printf("ins var access 2 end\n"); }
#line 2814 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 382 "stack.y" /* yacc.c:1646  */
    { sm_printf("rhs object access\n"); sp_lz_load_constant((yyvsp[0].string)); sp_push_symrec(sp_lhs_load(18, 1, sp_pop_symrec())); }
#line 2820 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 383 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sm_printf("rhs array access 1\n"); sp_push_symrec(sp_lhs_load(17, 1, sp_pop_symrec())); }
#line 2826 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 384 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_pop_symrec(); sm_printf("rhs array access 2\n"); sp_push_symrec(sp_lhs_load(17, 2, sp_pop_symrec())); }
#line 2832 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 386 "stack.y" /* yacc.c:1646  */
    { }
#line 2838 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 386 "stack.y" /* yacc.c:1646  */
    {   }
#line 2844 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 388 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id)=0;  }
#line 2850 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 389 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id)=0; }
#line 2856 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 391 "stack.y" /* yacc.c:1646  */
    { sp_lhs_push_api_call((yyvsp[-2].api_id)); sp_lhs_get((yyvsp[-2].api_id)); sm_printf("lhs object access\n"); sp_lz_load_constant((yyvsp[0].string)); (yyval.api_id)=18; sp_lhs_set(18); }
#line 2862 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 392 "stack.y" /* yacc.c:1646  */
    { sp_lhs_push_api_call(128); sp_lhs_get(128); sm_printf("lhs var access\n"); sp_lz_load_constant((yyvsp[0].string)); (yyval.api_id)=2; sp_lhs_set(2); }
#line 2868 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 393 "stack.y" /* yacc.c:1646  */
    { sp_lhs_push_api_call((yyvsp[-3].api_id)); sp_lhs_get((yyvsp[-3].api_id)); sm_printf("lhs array access\n"); sp_pop_symrec(); (yyval.api_id)=17; sp_lhs_set(17); sm_printf("lhs array access done\n"); }
#line 2874 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 394 "stack.y" /* yacc.c:1646  */
    { sm_printf("lhs api access %s\n", (yyvsp[-1].string)); sp_lhs_push_api_call((yyvsp[-3].api_id)); sp_lhs_get((yyvsp[-3].api_id)); sp_push_symrec(sp_start_function_call((yyvsp[-1].string), 1)); sp_create_new_scope(NULL); }
#line 2880 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 394 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_flush_scope_now(); sp_destroy_scope(); sp_end_function_call(rec); }
#line 2886 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 396 "stack.y" /* yacc.c:1646  */
    { sp_lhs_get((yyvsp[-3].api_id)); sm_printf("lhs method call\n"); sp_create_method_scope(); sp_push_symrec(sp_start_method_call((yyvsp[-1].string))); sp_create_new_scope(NULL); }
#line 2892 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 396 "stack.y" /* yacc.c:1646  */
    { symrec * meth = sp_pop_symrec(); sp_flush_scope_now(); sp_destroy_scope(); sp_end_method_call(meth); sm_printf("lhs method call\n"); }
#line 2898 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 396 "stack.y" /* yacc.c:1646  */
    { sp_destroy_method_scope(); }
#line 2904 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 397 "stack.y" /* yacc.c:1646  */
    { sm_printf("push %s\n", (yyvsp[0].string)); sp_lhs_store(0, sp_push_symrec(st_sym_select(SYM_TYPE_VAR, (yyvsp[0].string)))); (yyval.api_id)=0; sm_printf("push %s done\n", (yyvsp[0].string)); }
#line 2910 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 401 "stack.y" /* yacc.c:1646  */
    { sp_create_method_scope(); sp_enqueue_symrec(sp_push_symrec(sp_start_method_call((yyvsp[-1].string)))); sp_create_new_scope(NULL); }
#line 2916 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 401 "stack.y" /* yacc.c:1646  */
    { symrec * meth = sp_dequeue_symrec(); sp_pop_symrec(); sp_destroy_scope(); sp_end_method_call(meth); sm_printf("lhs method call\n"); }
#line 2922 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 401 "stack.y" /* yacc.c:1646  */
    { sp_destroy_method_scope(); }
#line 2928 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 405 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_varctx(sp_create_varctx(INS_ADD, NULL)); }
#line 2934 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 406 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_varctx(sp_create_varctx(INS_SUB, NULL)); }
#line 2940 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 407 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_varctx(sp_create_varctx(INS_MUL, NULL)); }
#line 2946 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 408 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_varctx(sp_create_varctx(INS_DIV, NULL)); }
#line 2952 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 409 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_varctx(sp_create_varctx(INS_MOD, NULL)); }
#line 2958 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 410 "stack.y" /* yacc.c:1646  */
    { sp_lhs_store(0, sp_get_scope_var()); }
#line 2964 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 411 "stack.y" /* yacc.c:1646  */
    { sm_printf("push %s\n", (yyvsp[0].string)); sp_lhs_store(0, sp_push_symrec(st_sym_select(SYM_TYPE_VAR, (yyvsp[0].string)))); }
#line 2970 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 412 "stack.y" /* yacc.c:1646  */
    { sp_push_constant_s((yyvsp[0].bytes).value, (yyvsp[0].bytes).length); sp_lhs_store(0, sp_push_symrec(st_sym_select(SYM_TYPE_CONST, (yyvsp[0].bytes).value))); }
#line 2976 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 413 "stack.y" /* yacc.c:1646  */
    { sp_push_constant((yyvsp[0].bytes).value); sp_lhs_store(0, sp_push_symrec(st_sym_select(SYM_TYPE_CONST, (yyvsp[0].bytes).value))); sp_lhs_store(INS_SYSCALL1, (symrec *)40); }
#line 2982 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 414 "stack.y" /* yacc.c:1646  */
    { YYABORT; }
#line 2988 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 430 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); sp_lz_load_constant_s((yyvsp[0].bytes).value, (yyvsp[0].bytes).length); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
#line 2994 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 431 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); sp_lz_load_numeric((yyvsp[0].bytes).value); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
#line 3000 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 432 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); }
#line 3006 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 432 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
#line 3012 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 433 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); }
#line 3018 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 433 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SUB, NULL)); }
#line 3024 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 434 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); }
#line 3030 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 434 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MUL, NULL)); }
#line 3036 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 435 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); }
#line 3042 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 435 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_DIV, NULL)); }
#line 3048 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 436 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); }
#line 3054 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 436 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MOD, NULL)); }
#line 3060 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 437 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); }
#line 3066 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 201:
#line 437 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_AND, NULL)); }
#line 3072 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 438 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); }
#line 3078 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 203:
#line 438 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_OR, NULL)); }
#line 3084 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 204:
#line 439 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); }
#line 3090 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 205:
#line 439 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_XOR, NULL)); }
#line 3096 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 206:
#line 440 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); }
#line 3102 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 207:
#line 440 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHL, NULL)); }
#line 3108 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 208:
#line 441 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); }
#line 3114 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 209:
#line 441 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHR, NULL)); }
#line 3120 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 210:
#line 442 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); }
#line 3126 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 211:
#line 442 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CREQ, NULL)); }
#line 3132 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 212:
#line 443 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); }
#line 3138 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 213:
#line 443 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRNE, NULL)); }
#line 3144 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 214:
#line 444 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); }
#line 3150 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 215:
#line 444 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRLT, NULL)); }
#line 3156 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 216:
#line 445 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2);  }
#line 3162 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 217:
#line 445 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRGT, NULL)); }
#line 3168 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 218:
#line 446 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); }
#line 3174 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 219:
#line 446 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRLTEQ, NULL)); }
#line 3180 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 220:
#line 447 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[-1].api_id), 2); }
#line 3186 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 221:
#line 447 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRGTEQ, NULL)); }
#line 3192 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 222:
#line 448 "stack.y" /* yacc.c:1646  */
    { sp_lhs_api_call((yyvsp[0].api_id), 2); sp_push_inode(sp_create_inode(INS_NOT, NULL)); }
#line 3198 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 223:
#line 451 "stack.y" /* yacc.c:1646  */
    {  }
#line 3204 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 224:
#line 452 "stack.y" /* yacc.c:1646  */
    { sp_clear_operation_stack(); sp_lhs_clear();  }
#line 3210 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 225:
#line 455 "stack.y" /* yacc.c:1646  */
    { sp_flush_scope(); sp_push_varctx_scope(); sp_create_new_scope(NULL); }
#line 3216 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 226:
#line 455 "stack.y" /* yacc.c:1646  */
    { sp_flush_scope(); sp_lhs_get(17); sp_clear_operation_stack(); sp_lhs_clear(); sp_destroy_scope(); sp_pop_varctx_scope(); sm_printf("scope assignment end\n"); }
#line 3222 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 228:
#line 458 "stack.y" /* yacc.c:1646  */
    { sp_lz_load_constant_s((yyvsp[0].bytes).value, (yyvsp[0].bytes).length); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
#line 3228 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 229:
#line 459 "stack.y" /* yacc.c:1646  */
    { sp_lz_load_numeric((yyvsp[0].bytes).value); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
#line 3234 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 230:
#line 460 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
#line 3240 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 231:
#line 461 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SUB, NULL)); }
#line 3246 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 232:
#line 462 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MUL, NULL)); }
#line 3252 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 233:
#line 463 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_DIV, NULL)); }
#line 3258 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 234:
#line 464 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MOD, NULL)); }
#line 3264 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 235:
#line 465 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_AND, NULL)); }
#line 3270 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 236:
#line 466 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_OR, NULL)); }
#line 3276 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 237:
#line 467 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_XOR, NULL)); }
#line 3282 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 238:
#line 468 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHL, NULL)); }
#line 3288 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 239:
#line 469 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHR, NULL)); }
#line 3294 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 240:
#line 470 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CREQ, NULL)); }
#line 3300 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 241:
#line 471 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRNE, NULL)); }
#line 3306 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 242:
#line 472 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRLT, NULL)); }
#line 3312 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 243:
#line 473 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRGT, NULL)); }
#line 3318 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 244:
#line 474 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRLTEQ, NULL)); }
#line 3324 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 245:
#line 475 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRGTEQ, NULL)); }
#line 3330 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 246:
#line 476 "stack.y" /* yacc.c:1646  */
    { sp_push_inode(sp_create_inode(INS_NOT, NULL)); }
#line 3336 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 247:
#line 477 "stack.y" /* yacc.c:1646  */
    { sm_printf("lazy_expr:rhs_val\n"); }
#line 3342 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 248:
#line 478 "stack.y" /* yacc.c:1646  */
    {  }
#line 3348 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 249:
#line 481 "stack.y" /* yacc.c:1646  */
    {  }
#line 3354 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 250:
#line 482 "stack.y" /* yacc.c:1646  */
    { }
#line 3360 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 251:
#line 485 "stack.y" /* yacc.c:1646  */
    { sp_flush_scope(); sp_push_varctx_scope(); sp_create_new_scope(NULL); }
#line 3366 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 252:
#line 485 "stack.y" /* yacc.c:1646  */
    { sp_flush_scope(); sp_lhs_get(17); sp_clear_operation_stack(); sp_lhs_clear(); sp_destroy_scope(); sp_pop_varctx_scope(); sm_printf("scope assignment end\n"); }
#line 3372 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 254:
#line 488 "stack.y" /* yacc.c:1646  */
    { sp_lz_load_constant_s((yyvsp[0].bytes).value, (yyvsp[0].bytes).length); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
#line 3378 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 255:
#line 489 "stack.y" /* yacc.c:1646  */
    { sp_lz_load_numeric((yyvsp[0].bytes).value); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
#line 3384 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 256:
#line 490 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_ADD, NULL)); }
#line 3390 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 257:
#line 491 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SUB, NULL)); }
#line 3396 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 258:
#line 492 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MUL, NULL)); }
#line 3402 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 259:
#line 493 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_DIV, NULL)); }
#line 3408 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 260:
#line 494 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_MOD, NULL)); }
#line 3414 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 261:
#line 495 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_AND, NULL)); }
#line 3420 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 262:
#line 496 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_OR, NULL)); }
#line 3426 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 263:
#line 497 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_XOR, NULL)); }
#line 3432 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 264:
#line 498 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHL, NULL)); }
#line 3438 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 265:
#line 499 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_SHR, NULL)); }
#line 3444 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 266:
#line 500 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CREQ, NULL)); }
#line 3450 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 267:
#line 501 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRNE, NULL)); }
#line 3456 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 268:
#line 502 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRLT, NULL)); }
#line 3462 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 269:
#line 503 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRGT, NULL)); }
#line 3468 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 270:
#line 504 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRLTEQ, NULL)); }
#line 3474 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 271:
#line 505 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_inode(sp_create_inode(INS_CRGTEQ, NULL)); }
#line 3480 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 272:
#line 506 "stack.y" /* yacc.c:1646  */
    { sp_push_inode(sp_create_inode(INS_NOT, NULL)); }
#line 3486 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 273:
#line 507 "stack.y" /* yacc.c:1646  */
    { sm_printf("rhs_lazy_expr:rhs_val\n"); }
#line 3492 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 274:
#line 508 "stack.y" /* yacc.c:1646  */
    {  }
#line 3498 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 275:
#line 510 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = VM_INT8; }
#line 3504 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 276:
#line 511 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = VM_INT16; }
#line 3510 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 277:
#line 512 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = VM_INT32; }
#line 3516 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 278:
#line 513 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = VM_INT64; }
#line 3522 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 279:
#line 514 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = VM_FLOAT; }
#line 3528 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 280:
#line 515 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = VM_DOUBLE; }
#line 3534 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 281:
#line 516 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = 0; }
#line 3540 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 282:
#line 518 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = (yyvsp[0].api_id); }
#line 3546 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 283:
#line 519 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = VM_ARRAY | (yyvsp[-2].api_id); }
#line 3552 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 284:
#line 520 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = VM_STRING; }
#line 3558 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 285:
#line 521 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = VM_POINTER; }
#line 3564 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 287:
#line 524 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = VM_AUTO_POINTER; }
#line 3570 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 288:
#line 525 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = VM_POINTER; }
#line 3576 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 289:
#line 526 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = 1; }
#line 3582 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 290:
#line 528 "stack.y" /* yacc.c:1646  */
    { sm_printf("scope val assignment\n"); sp_push_inode(sp_create_inode(INS_OBJPUSH, sp_get_scope_var())); }
#line 3588 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 291:
#line 529 "stack.y" /* yacc.c:1646  */
    { }
#line 3594 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 292:
#line 531 "stack.y" /* yacc.c:1646  */
    { sp_flush_scope(); sp_create_new_scope(NULL); }
#line 3600 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 294:
#line 533 "stack.y" /* yacc.c:1646  */
    { sprintf(_RECAST(char *, (yyval.string)), "%d", (yyvsp[-1].api_id)); }
#line 3606 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 295:
#line 534 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_get_scope_var(); sp_flush_scope(); sp_destroy_scope(); }
#line 3612 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 296:
#line 536 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_start_api_call(30, 2)); }
#line 3618 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 297:
#line 536 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_lz_load_constant_s((yyvsp[-2].string), strlen(_RECAST(const char *,(yyvsp[-2].string)))); sp_end_function_call(sp_pop_symrec()); sp_push_symrec(rec); sp_flush_scope(); }
#line 3624 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 298:
#line 537 "stack.y" /* yacc.c:1646  */
    {  }
#line 3630 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 299:
#line 539 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_lz_create_instance((yyvsp[-2].string))); }
#line 3636 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 300:
#line 540 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_lz_load_variable((yyvsp[0].string))); }
#line 3642 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 301:
#line 541 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_lz_load_array_constant()); }
#line 3648 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 302:
#line 542 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_lz_load_numeric((yyvsp[0].bytes).value)); }
#line 3654 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 303:
#line 544 "stack.y" /* yacc.c:1646  */
    {  }
#line 3660 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 304:
#line 545 "stack.y" /* yacc.c:1646  */
    {  }
#line 3666 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 305:
#line 546 "stack.y" /* yacc.c:1646  */
    { }
#line 3672 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 306:
#line 547 "stack.y" /* yacc.c:1646  */
    { sp_push_inode(sp_create_inode(INS_OBJDUP, NULL)); sp_lz_load_constant(_RECAST(uchar *, "1")); sp_push_inode(sp_create_inode(INS_ADD, NULL)); sp_push_inode(sp_create_inode(INS_OBJPOP, sp_peek_symrec())); }
#line 3678 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 307:
#line 548 "stack.y" /* yacc.c:1646  */
    { sp_push_inode(sp_create_inode(INS_OBJDUP, NULL)); sp_lz_load_constant(_RECAST(uchar *, "1")); sp_push_inode(sp_create_inode(INS_SUB, NULL)); sp_push_inode(sp_create_inode(INS_OBJPOP, sp_peek_symrec())); }
#line 3684 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 308:
#line 549 "stack.y" /* yacc.c:1646  */
    { sp_lz_load_constant(_RECAST(uchar *, "1")); sp_push_inode(sp_create_inode(INS_ADD, NULL)); sp_push_inode(sp_create_inode(INS_OBJDUP, NULL)); sp_push_inode(sp_create_inode(INS_OBJPOP, sp_peek_symrec())); }
#line 3690 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 309:
#line 550 "stack.y" /* yacc.c:1646  */
    { sp_lz_load_constant(_RECAST(uchar *, "1")); sp_push_inode(sp_create_inode(INS_SUB, NULL)); sp_push_inode(sp_create_inode(INS_OBJDUP, NULL)); sp_push_inode(sp_create_inode(INS_OBJPOP, sp_peek_symrec())); }
#line 3696 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 310:
#line 552 "stack.y" /* yacc.c:1646  */
    {  }
#line 3702 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 311:
#line 553 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_lz_load_class((yyvsp[0].string))); }
#line 3708 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 312:
#line 554 "stack.y" /* yacc.c:1646  */
    { sp_push_inode(sp_create_inode(INS_OBJSZ, NULL)); }
#line 3714 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 313:
#line 555 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_lz_load_constant_s((yyvsp[0].bytes).value, (yyvsp[0].bytes).length)); }
#line 3720 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 314:
#line 557 "stack.y" /* yacc.c:1646  */
    {  }
#line 3726 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 315:
#line 558 "stack.y" /* yacc.c:1646  */
    {  }
#line 3732 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 316:
#line 559 "stack.y" /* yacc.c:1646  */
    {  }
#line 3738 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 317:
#line 561 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_push_symrec(sp_start_function_call((yyvsp[-1].string), 0)); sm_printf("start call func: %s\n",rec->name); }
#line 3744 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 318:
#line 561 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_peek_symrec(); sm_printf("end call func: %s\n",rec->name); sp_end_function_call(rec); }
#line 3750 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 319:
#line 563 "stack.y" /* yacc.c:1646  */
    { sp_new_array_constant(); }
#line 3756 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 321:
#line 565 "stack.y" /* yacc.c:1646  */
    { (yyval.bytes)=(yyvsp[0].bytes); }
#line 3762 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 322:
#line 566 "stack.y" /* yacc.c:1646  */
    { (yyval.bytes)=(yyvsp[0].bytes); }
#line 3768 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 323:
#line 568 "stack.y" /* yacc.c:1646  */
    { (yyval.bytes)=(yyvsp[0].bytes); }
#line 3774 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 324:
#line 569 "stack.y" /* yacc.c:1646  */
    { (yyval.bytes)=(yyvsp[0].bytes); }
#line 3780 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 326:
#line 572 "stack.y" /* yacc.c:1646  */
    { sp_push_array_constant((yyvsp[0].bytes).value); }
#line 3786 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 328:
#line 573 "stack.y" /* yacc.c:1646  */
    { sp_push_array_constant((yyvsp[0].bytes).value); }
#line 3792 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 329:
#line 575 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_start_api_call(15, 0)); }
#line 3798 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 330:
#line 575 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_end_function_call(sp_pop_symrec())); }
#line 3804 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 331:
#line 576 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_start_api_call(16, 0)); }
#line 3810 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 332:
#line 576 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_end_function_call(sp_pop_symrec())); }
#line 3816 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 333:
#line 578 "stack.y" /* yacc.c:1646  */
    { }
#line 3822 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 334:
#line 579 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); }
#line 3828 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 336:
#line 580 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); }
#line 3834 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 337:
#line 582 "stack.y" /* yacc.c:1646  */
    { }
#line 3840 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 338:
#line 583 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); }
#line 3846 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 340:
#line 584 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); }
#line 3852 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 342:
#line 587 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_start_api_call(14, 2)); sp_lz_load_constant((yyvsp[0].string)); }
#line 3858 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 343:
#line 587 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_symrec(sp_end_function_call(sp_pop_symrec())); }
#line 3864 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 346:
#line 591 "stack.y" /* yacc.c:1646  */
    { }
#line 3870 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 347:
#line 593 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_start_lambda_decl(0)); }
#line 3876 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 348:
#line 594 "stack.y" /* yacc.c:1646  */
    { symrec * func; sp_push_symrec(sp_execute_lambda_call(sp_pop_symrec())); 
  func = sp_start_lambda_body(sp_pop_symrec()); if(func == NULL) YYABORT;
		sp_push_symrec(func); sp_flush_scope_now(); }
#line 3884 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 349:
#line 598 "stack.y" /* yacc.c:1646  */
    { sp_end_lambda_decl(sp_peek_symrec()); sp_lz_load_lambda(sp_peek_symrec()); }
#line 3890 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 350:
#line 600 "stack.y" /* yacc.c:1646  */
    { strcpy(_RECAST(char *, (yyval.string)), _RECAST(const char *, (yyvsp[0].string))); }
#line 3896 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 351:
#line 602 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_start_case()); sp_push_symrec(sp_create_label(_RECAST(uchar *, "__d"))); }
#line 3902 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 352:
#line 603 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); symrec * srec = sp_pop_symrec(); sp_end_case(srec); sp_new_label(rec->name); }
#line 3908 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 354:
#line 606 "stack.y" /* yacc.c:1646  */
    { sp_push_constant_s((yyvsp[0].bytes).value, (yyvsp[0].bytes).length); }
#line 3914 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 355:
#line 606 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_label_case_s(sp_peek_symrec(), (yyvsp[-2].bytes).value, (yyvsp[-2].bytes).length); sp_push_symrec(rec); }
#line 3920 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 356:
#line 607 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_default_case(sp_peek_symrec()); sp_push_symrec(rec); }
#line 3926 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 358:
#line 610 "stack.y" /* yacc.c:1646  */
    { symrec * srec = sp_pop_symrec(), * brec = sp_pop_symrec(); sp_push_symrec(brec); sp_push_symrec(srec); sp_jump_to(INS_JFALSE, brec->name); }
#line 3932 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 360:
#line 611 "stack.y" /* yacc.c:1646  */
    { symrec * srec = sp_peek_symrec(); sp_jump_to(INS_JTRUE, srec->name); }
#line 3938 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 362:
#line 612 "stack.y" /* yacc.c:1646  */
    { symrec * srec = sp_pop_symrec(); symrec * rec = sp_peek_symrec(); sp_jump_to(INS_JFALSE, rec->name); sp_push_symrec(srec); }
#line 3944 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 363:
#line 614 "stack.y" /* yacc.c:1646  */
    { }
#line 3950 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 364:
#line 616 "stack.y" /* yacc.c:1646  */
    { symrec * srec = sp_pop_symrec(); symrec * rec = sp_peek_symrec(); rec->sa_currec = srec; sp_flush_scope_now(); sm_printf("flush end\n"); }
#line 3956 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 365:
#line 619 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_declare_function((yyvsp[-1].string))); }
#line 3962 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 366:
#line 619 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); if(sp_declare_function_end(rec->name) == NULL) YYABORT; }
#line 3968 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 368:
#line 622 "stack.y" /* yacc.c:1646  */
    { sp_declare_function_extern((yyvsp[0].bytes).value); }
#line 3974 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 370:
#line 625 "stack.y" /* yacc.c:1646  */
    { sp_declare_function_param(0); }
#line 3980 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 372:
#line 626 "stack.y" /* yacc.c:1646  */
    { sp_declare_function_param(0); }
#line 3986 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 374:
#line 629 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_create_lambda_param((yyvsp[0].string))); }
#line 3992 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 375:
#line 629 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_lz_store_variable(rec->name); }
#line 3998 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 376:
#line 630 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_create_lambda_param((yyvsp[0].string)); sp_lz_store_variable((yyvsp[0].string)); }
#line 4004 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 378:
#line 633 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); }
#line 4010 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 379:
#line 633 "stack.y" /* yacc.c:1646  */
    {sm_printf("call_param\n"); sp_flush_scope(); }
#line 4016 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 381:
#line 634 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_function_param(sp_peek_symrec()); }
#line 4022 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 383:
#line 641 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_method_param(sp_peek_symrec()); sp_flush_method_param(); }
#line 4028 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 384:
#line 643 "stack.y" /* yacc.c:1646  */
    { sp_pop_symrec(); sp_push_method_param(sp_peek_symrec()); sp_flush_method_param(); }
#line 4034 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 385:
#line 643 "stack.y" /* yacc.c:1646  */
    { sp_destroy_method_scope(); sp_create_method_scope(); }
#line 4040 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 387:
#line 645 "stack.y" /* yacc.c:1646  */
    { (yyval.state) = TRUE; }
#line 4046 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 388:
#line 646 "stack.y" /* yacc.c:1646  */
    { (yyval.state) = FALSE; }
#line 4052 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 389:
#line 647 "stack.y" /* yacc.c:1646  */
    { (yyval.state) = FALSE; }
#line 4058 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 391:
#line 650 "stack.y" /* yacc.c:1646  */
    { sp_declare_function_param((yyvsp[0].api_id)); }
#line 4064 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 393:
#line 651 "stack.y" /* yacc.c:1646  */
    { sp_declare_function_param((yyvsp[0].api_id)); }
#line 4070 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 394:
#line 652 "stack.y" /* yacc.c:1646  */
    { sp_declare_function_param((yyvsp[-1].api_id)); }
#line 4076 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 396:
#line 653 "stack.y" /* yacc.c:1646  */
    { sp_declare_function_param((yyvsp[-1].api_id)); }
#line 4082 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 397:
#line 655 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_declare_interface_function((yyvsp[-1].string))); }
#line 4088 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 398:
#line 655 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_peek_symrec(); if(sp_declare_function_end(rec->name) == NULL) YYABORT; }
#line 4094 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 399:
#line 657 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = CALL_TYPE_C; }
#line 4100 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 400:
#line 658 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = CALL_TYPE_STD; }
#line 4106 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 401:
#line 659 "stack.y" /* yacc.c:1646  */
    { (yyval.api_id) = CALL_TYPE_FAST; }
#line 4112 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 402:
#line 661 "stack.y" /* yacc.c:1646  */
    { sp_register_method(sp_pop_symrec(), (yyvsp[-3].api_id), (yyvsp[-2].api_id)); }
#line 4118 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 404:
#line 664 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_create_function((yyvsp[0].string))); sp_create_new_scope(NULL); }
#line 4124 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 405:
#line 665 "stack.y" /* yacc.c:1646  */
    { symrec * func = sp_start_function_body(sp_pop_symrec(), (yyvsp[-3].state)); if(func == NULL) YYABORT;
		sp_push_symrec(func); sp_flush_scope_now(); }
#line 4131 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 406:
#line 668 "stack.y" /* yacc.c:1646  */
    { sp_end_function_body(sp_pop_symrec()); sp_flush_scope_now(); }
#line 4137 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 410:
#line 673 "stack.y" /* yacc.c:1646  */
    { sp_install_menu((yyvsp[0].bytes).value, sp_peek_symrec()); }
#line 4143 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 411:
#line 674 "stack.y" /* yacc.c:1646  */
    { sp_install_event((yyvsp[0].bytes).value, sp_peek_symrec()); }
#line 4149 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 413:
#line 677 "stack.y" /* yacc.c:1646  */
    { sp_push_symrec(sp_create_param((yyvsp[0].string))); }
#line 4155 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 414:
#line 677 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_pop_symrec(); sp_lz_store_variable(rec->name); }
#line 4161 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 415:
#line 678 "stack.y" /* yacc.c:1646  */
    { symrec * rec = sp_create_param((yyvsp[0].string)); sp_lz_store_variable((yyvsp[0].string)); }
#line 4167 "stack.tab.c" /* yacc.c:1646  */
    break;

  case 416:
#line 680 "stack.y" /* yacc.c:1646  */
    { YYABORT; }
#line 4173 "stack.tab.c" /* yacc.c:1646  */
    break;


#line 4177 "stack.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 682 "stack.y" /* yacc.c:1906  */


void sp_init_parser(void) {
	//yylval.token = (yytoken *)malloc(sizeof(yytoken));
	errors = 0;
	yyinit();
}

uint32 sp_parse(void) {
	//start parse
	yyparse();
	//cleanup for any resources
	sp_cleanup_parser();			//semantic parser clean up
	lp_clear_globals();				//lexical analyzer clean up
	
	return errors;
}

extern void sp_error_push(uchar * msg);
void sp_error(uchar * msg, ...) {
	uchar buffer[512];
	uchar pbuffer[520];
	va_list argptr;
	va_start(argptr, msg);
	vsprintf(_RECAST(char *, buffer), _RECAST(const char *, msg), argptr);
	sprintf(_RECAST(char *, pbuffer), "[line: %d] %s", p_config->line + 1, buffer);
	sp_error_push(pbuffer);
	//printf("%s\n", buffer);
	//yyerror ( _RECAST(char *, buffer));
	va_end(argptr);              /* Reset variable argument list. */
	//errors ++;
}

/*=========================================================================
MAIN
=========================================================================*/
#ifdef STANDALONE_COMPILER
void main( int argc, char *argv[] )
{ 
	//extern FILE *yyin;
	int i = 1;
	err_record * iterator;
	uint16 strCount = 0;
	uchar gen_asm = 0, gen_il = 0, gen_scr = 0;
	//++argv;			//skip exepath
	uchar opt_level = 0;
	//sp_init_parser();
	if(argc == 1) goto print_usage;
	while(i != (argc - 1)) {
		if(strcmp(argv[i], "-ca") == 0) {
			gen_il = 1;
		} else if(strcmp(argv[i], "-c") == 0) {
			gen_asm = 1;
		} else if(strcmp(argv[i], "-cd") == 0) {
			gen_scr = 1;
		} else if(strcmp(argv[i], "-o1") == 0) {
			opt_level |= IS_OPTIMIZE_L1;
		} else if(strcmp(argv[i], "-o2") == 0) {
			opt_level |= IS_OPTIMIZE_L2 | IS_OPTIMIZE_L1;
		} else {
			print_usage:
			printf("usage: stack [options] filename\n	\
					options:\
					\n-ca\tcompilassembler [source->bin]\
					\n-cd\tscript generator [source->apdu]\
					\n-c\tcompile [source->asm]\
					\n-o1\tpeephole optimization\
					\n-o2\tdeadcode elimination");
			exit(0);
		}
		i++;
	}
	//yyin = fopen( argv[i], "r" );
	if(sp_init(_RECAST(uchar *, argv[i])) != NULL) {
		if(gen_il) is_init(_RECAST(uchar *, argv[i]));
		if(gen_asm) as_init(_RECAST(uchar *, argv[i]));
		if(gen_scr) sc_init(_RECAST(uchar *, argv[i]));
		i = sp_parse();
		if(i == 0) {
			is_link_optimize(opt_level);
			if(gen_il) {	
				is_file_flush();
			}
			if(gen_asm) {
				as_flush();
			}
			if(gen_scr) {
				sc_flush();
				sc_cleanup();
			}
		} else {
			iterator = sp_error_get_enumerator();
			while(iterator != NULL) {
				//lstStream->WriteLine(gcnew String(_RECAST(const char *, iterator->buffer)));
				strCount ++;
				printf("%s\n", iterator->buffer);
				iterator = sp_error_next_record(iterator);
			}
		}
		//fclose(yyin);
	}	
}
#endif

/*=========================================================================
YYERROR
=========================================================================*/
extern void sp_error(uchar *, ...);
int yyerror ( char *s ) /* Called by yyparse on error */
{
	errors++;
	//printf("Error (%i) on file %s line %i: %s\n", errors, p_config->filename, p_config->line + 1, s);
	sp_error(SP_ERR_PARSER, s, yylval.bytes);
	return 0;
}
/**************************** End Grammar File ***************************/
