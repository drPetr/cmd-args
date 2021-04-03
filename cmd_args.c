#include "cmd_args.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>



/*
cmdOpt_t
*/
typedef struct cmdOpt_s {
    const char      *shortKey;  /* short option key '-h' */
    int             shortKeyLen;
    const char      *longKey;   /* long option key '--help' */
    int             longKeyLen;
    const char      *descr;     /* option description 'descr' */
    int             descrLen;
    unsigned char   useOptVal;  /* use option value */
    
    fnCmdOptHandler handler;    /* function handler */
    
    struct cmdOpt_s *next;      /* next option */
} cmdOpt_t;



#if defined(CMD_ARGS_NODBG) || (!defined(DEBUG))
    #define onlydebug(act)
    #define cmdassert(e)
    #define cmdasserta(e,a,...)
#else
/*
============
__impl_cmdasserta
============
*/
static void __impl_cmdasserta( int e, const char* expr, const char* file, 
        int line, const char *function, const char* fmt, ... ) {
    va_list args;
    if( !e ){
        va_start( args, fmt );
        fprintf( stderr, "assertion failed: %s\n", expr );
        fprintf( stderr, "file: %s\n", file );
        fprintf( stderr, "line: %d\n", line );
        fprintf( stderr, "function: %s\n", function );
        vfprintf( stderr, fmt, args );
        va_end(args);
        exit(1);
    }
}

/*
============
__impl_cmdassert
============
*/
static void __impl_cmdassert( int e, const char* expr, const char* file, 
        int line, const char *function ) {
    if( !e ){
        fprintf( stderr, "assertion failed: %s\n", expr );
        fprintf( stderr, "file: %s\n", file );
        fprintf( stderr, "line: %d\n", line );
        fprintf( stderr, "function: %s\n", function );
        exit(1);
    }
}

    #define onlydebug(act)    act
    
    #define cmdassert(e)                                \
        __impl_cmdassert( (int)(!!(e)), #e, __FILE__,   \
        __LINE__, __FUNCTION__ )
        
    #define cmdasserta(e,a,...)                         \
        __impl_cmdasserta( (int)(!!(e)), #e, __FILE__,  \
        __LINE__, __FUNCTION__, a, ##__VA_ARGS__ )
#endif

#define cmderror(err,...)                                 \
        fprintf( stderr, err, ##__VA_ARGS__ ); exit(1);


/* allocator */
fnCmdAlloc  cmdAllocatorAlloc = malloc;
fnCmdFree   cmdAllocatorFree = free;

cmdOpt_t *optionList = NULL;
cmdOpt_t *optionListLast = NULL;



/*
============
__impl_cmdverifya
============
*/
static void __impl_cmdverifya( int e, const char* expr, const char* file, 
        int line, const char *function, const char* fmt, ... ) {
    va_list args;
    if( !e ){
        va_start( args, fmt );
        fprintf( stderr, "verification failed: %s\n", expr );
        fprintf( stderr, "file: %s\n", file );
        fprintf( stderr, "line: %d\n", line );
        fprintf( stderr, "function: %s\n", function );
        vfprintf( stderr, fmt, args );
        va_end(args);
        exit(1);
    }
}

/*
============
__impl_cmdverify
============
*/
static void __impl_cmdverify( int e, const char* expr, const char* file, 
        int line, const char *function ) {
    if( !e ){
        fprintf( stderr, "verification failed: %s\n", expr );
        fprintf( stderr, "file: %s\n", file );
        fprintf( stderr, "line: %d\n", line );
        fprintf( stderr, "function: %s\n", function );
        exit(1);
    }
}

#define cmdverify(e)                                \
    __impl_cmdverify( (int)(!!(e)), #e, __FILE__,   \
    __LINE__, __FUNCTION__ )
    
#define cmdverifya(e,a,...)                         \
    __impl_cmdverifya( (int)(!!(e)), #e, __FILE__,  \
    __LINE__, __FUNCTION__, a, ##__VA_ARGS__ )


/*
============
IsKeyChar
============
*/
static int IsKeyChar( int c ) {
    return (c >= 'a' && c <= 'z') || 
        (c >= 'A' && c <= 'Z') || 
        (c >= '0' && c <= '9');
}

/*
============
ShortKeyIsValid
============
*/
static int ShortKeyIsValid( const char *key ) {
    char c;
    
    /* check for NULL string */
    if( !key ) {
        return 0;
    }
    
    /* check first character (must be '-') */
    c = *key++;
    if( !(c && c == '-') ) {
        return 0;
    }
    
    /* check next character (must be Symbol or Digit) */
    c = *key++;
    if( !IsKeyChar(c) ) {
        return 0;
    }
    
    /* check last character (must be '\0') */
    c = *key++;
    return c == 0;
}

/*
============
LongKeyIsValid
============
*/
static int LongKeyIsValid( const char *key ) {
    char c;
    int i;
    
    /* check for NULL string */
    if( !key ) {
        return 0;
    }
    
    /* check two characters (must be "--") */
    for( i = 0; i < 2; i++ ) {
        c = *key++;
        if( !(c && c == '-') ) {
            return 0;
        }
    }
    
    /* check next character (must be Symbol or Digit) */
    c = *key++;
    if( IsKeyChar(c) ) {
        do {
            c = *key++;
        } while( IsKeyChar(c) || c == '_' || c == '-' );
    } else {
        return 0;
    }
    
    /* check last character (must be '\0') */
    return c == 0;
}  

/*
============
CmdFindOpt
============
*/
static cmdOpt_t *CmdFindOpt( cmdOpt_t *list, const char *sk, const char *lk ) {
    cmdassert( list );
    cmdassert( sk || lk );
    
    /* find an option by key */
    while( list ) {
        if( sk && list->shortKey && strcmp( list->shortKey, sk ) == 0 ) {
            return list;
        }
        if( lk && list->longKey && strcmp( list->longKey, lk ) == 0 ) {
            return list;
        }
        list = list->next;
    }
    
    /* option not found */
    return NULL;
}

/*
============
CmdAlloc
============
*/
static void *CmdAlloc( size_t size ) {
    return cmdAllocatorAlloc( size );
}

/*
============
CmdFree
============
*/
static void CmdFree( void *memptr ) {
    cmdAllocatorFree( memptr );
}

/*
============
CmdOptCreate
============
*/
cmdOpt_t *CmdOptCreate( const char *sk, const char *lk, const char *descr,
        unsigned char useOptVal, fnCmdOptHandler handler ) {
    cmdOpt_t *opt;
    
    cmdasserta( sk || lk, 
            "error: at least one key must be specified" );
    cmdasserta( handler,
            "error: handler must be specified" );
    /* check short key */
    if( sk ) {
        cmdverifya( ShortKeyIsValid(sk), 
                "error: invalid short key '%s'", sk );
    }
    /* check long key */
    if( lk ) {
        cmdverifya( LongKeyIsValid(lk),
                "error: invalid long key '%s'", lk );
    }
    
    opt = CmdAlloc( sizeof(cmdOpt_t) );
    opt->shortKey = sk;
    opt->shortKeyLen = sk ? (int)strlen( sk ) : 0;
    opt->longKey = lk;
    opt->longKeyLen = lk ? (int)strlen( lk ) : 0;
    opt->descr = descr;
    opt->useOptVal = !!useOptVal;
    opt->handler = handler;
    opt->next = NULL;
    
    return opt;
}

/*
============
CmdOptListFree
============
*/
static void CmdOptListFree( cmdOpt_t *list ) {
    cmdOpt_t *tmp = list;
    
    cmdassert( list );
    
    while( list ) {
        tmp = list;
        list = list->next;
        CmdFree( tmp );
    }
}

/*
============
CmdOptAppend
============
*/
static void CmdOptAppend( cmdOpt_t **list, cmdOpt_t *append ) {
    cmdOpt_t *a, *tmp;
    
    cmdassert( list );
    cmdassert( append );
    
    if( !*list ) {
        *list = append;
        return;
    }
    
    while( append ) {
        a = append;
        append = append->next;
        
        /* check for identical options */
        cmdverifya( !CmdFindOpt( *list, a->shortKey, a->longKey ),
                "error: option with key '%s' or '%s' already exists", 
                a->shortKey, a->longKey );
        
        /* append option */
        tmp = (cmdOpt_t*)*list;
        while( tmp->next != NULL ) {
            tmp = tmp->next;
        }
        a->next = NULL;
        tmp->next = a;
    }
}

/*
============
CmdArgsProcess
============
*/
static int CmdArgsProcessOptions( cmdArgs_t *out, const cmdArgs_t *in,
        const cmdOpt_t *options, void *userData ) {

    struct optList_s {
        unsigned char   isLong;
        cmdOpt_t        *opt;
        const char      *key;
        const char      *val;
    };
    
    struct optList_s *flags;
    struct optList_s *flag;
    
    int flagsCount = 0, len, ret, i;
    int argc = in->argc;
    char **argv = in->argv;
    const char *key;
    cmdOpt_t *opt;
    
    /* check the number of arguments */
    if( argc <= 1 ) {
        return 0;
    }
    
    /* find all flags */
    flags = CmdAlloc( sizeof(struct optList_s) * argc );
    for( i = 1; i < argc; i++ ) {
        char *arg = argv[i];
        if( arg[0] == '-' ) {
            /* it is flag */
            flag = &flags[ flagsCount++ ];
            flag->isLong = 0 == strncmp( arg, "--", 2 );
            flag->opt = NULL;
            flag->key = NULL;
            flag->val = NULL;
        } else {
            out->argv[ out->argc++ ] = arg;
            continue;
        }
        
        /* find options for flag */
        opt = (cmdOpt_t*)options;
        while( opt ) {
            if( flag->isLong ) {
                key = opt->longKey;
                len = opt->longKeyLen;
            } else {
                key = opt->shortKey;
                len = opt->shortKeyLen;
            }
            
            /* compare flag with option */
            if( key && (0 == strncmp( key, arg, len )) ) {
                flag->opt = opt;
                flag->key = key;
                if( opt->useOptVal ) {
                    /* set falg value */
                    if( arg[ len ] != 0 ) {
                        flag->val = &arg[ len ];
                    } else {
                        if( i + 1 < argc ) {
                            flag->val = argv[ i + 1 ];
                            i++;
                        } else {
                            flag->val = "";
                        }
                    }
                } else {
                    if( arg[ len ] != 0 ) {
                        /* go to next option */
                        flag->opt = NULL;
                        flag->key = NULL;
                        opt = opt->next;
                        continue;
                    }
                }               
                break;
            }
            
            /* go to next option */
            opt = opt->next;
        }
        
        /* if flag not found print error */
        if( flag->opt == NULL ) {
            CmdFree( flags );
            cmderror( "error: unknown flag '%s'.\n", arg );
            /* unreachable code */
            return -1;
        }
    }
    
    /* process flags */
    for( i = 0; i < flagsCount; i++ ) {
        flag = &flags[i];
        ret = flag->opt->handler( flag->key, flag->val, userData );
        if( ret != 0 ) {
            break;
        }
    }
    
    CmdFree( flags );
    return ret;
}

/*
============
StrBetweenSpaces

for '-h' and '--help' options
============
*/
static size_t StrBetweenSpaces( const char *s, size_t max ) {
    const char *f, *e;
    
    cmdverify(s);
    
    if( !max || *s == 0 ) {
        return 0;
    }
    
    f = s;
    
    /* skip spaces */
    while( max && (*f == ' ' || *f == '\t' ) ) {
        f++;
        max--;
    }
    
    /* scan first word */
    while( max && *f && !(*f == ' ' || *f == '\t' || 
            *f == '\n' || *f == '\r') ) {
        max--;
        f++;
    }
    e = f;    
    
    /* scan next words */
    while( max && *f && !(*f == '\n' || *f == '\r') ) {
        while( max && *f && !(*f == ' ' || *f == '\t' || 
                *f == '\n' || *f == '\r') ) {
            f++;
            max--;
        }
        if( max == 0 ) {
            break;
        }
        while( max && (*f == ' ' || *f == '\t' ) ) {
            f++;
            max--;
        }
        e = f;
    }
    
    /* skip new line character */
    if( max >= 1 && (*f == '\n' || *f == '\r') ) {
        if( max > 1 ) {
            if( f[0] == '\n' && f[1] == '\r' ) {
                f += 2;
            } else if( f[0] == '\r' && f[1] == '\n' ) {
                f += 2;
            } else {
                f++;
            }
        } else {
            f++;
        }
        e = f;
    }
    
    return (size_t)(e - s);
}

/*
============
CmdOptPrintHelp
============
*/
static void CmdOptPrintHelp( FILE *out, cmdOpt_t *list, int colOptSiz,
        int colDescrSiz ) {
    cmdOpt_t *o = list;
    const char *p;
    size_t len;
    
    while( o ) {
        len = 0;
        /* print short key */
        if( o->shortKey ) {
            len += fprintf( out, "%s", o->shortKey );
            if( o->longKey ) {
                len += fprintf( out, ", " );
            }
        }
        
        /* print long key */
        if( o->longKey ) {
            len += fprintf( out, "%s", o->longKey );
        }
        
        /* print description */
        if( p ) {
            p = o->descr;
            if( len >= colOptSiz ) {
                fprintf( out, "\n%-*s", (int)(colOptSiz), "" );
            } else {
                fprintf( out, "%-*s", (int)(colOptSiz-len), "" );
            }
            
            len = StrBetweenSpaces( p, colDescrSiz );
            fprintf( out, "%.*s\n", (int)len, p );
            p += len;
            
            while( len ) {
                len = StrBetweenSpaces( p, colDescrSiz );
                fprintf( out, "%-*s", (int)(colOptSiz), "" );
                fprintf( out, "%.*s\n", (int)len, p );
                p += len;
            }
        } else {
            fprintf( out, "\n" );
        }
        
        o = o->next;
    }
}

/*
============
OptHelpHandler

handler for '-h' and '--help' options
============
*/
static int OptHelpHandler( const char *key, const char *val, void *userData ) {
    (void)key;
    (void)val;
    (void)userData;
    CmdOptPrintHelp( stdout, optionList, 20, 59 );
    return 1;
}

/*
============
CmdOptInit
============
*/
void CmdOptInit( void ) {
    cmdasserta( optionList == NULL, 
            "error: CmdOptInit() function must be called only once." );
    
    optionListLast = optionList = CmdOptCreate( "-h", "--help", 
            "Print this message and exit.", 0, OptHelpHandler );
}

/*
============
CmdOptFinal
============
*/
void CmdOptFinal( void ) {
    cmdasserta( optionList, 
            "error: the CmdOptFinal() function should be called "
            "only if the CmdOptInit() function was called." );
    CmdOptListFree( optionList );
    optionListLast = optionList = NULL;
}

/*
============
CmdOptAdd
============
*/
void CmdOptAdd( const char *sk, const char *lk, const char *descr,
        unsigned char useOptVal, fnCmdOptHandler handler ) {
    cmdOpt_t *opt;
    
    if( optionList == NULL ) {
        optionListLast = optionList = 
                CmdOptCreate( sk, lk, descr, useOptVal, handler );
    } else {
        opt = CmdOptCreate( sk, lk, descr, useOptVal, handler );
        CmdOptAppend( &optionListLast, opt );
        optionListLast = opt;
    }
}

/*
============
CmdOptProcess
============
*/
int CmdOptProcess( cmdArgs_t *out, int argc, char **argv, void *userData ) {
    cmdasserta( out, "error: pointer can't be NULL." );
    cmdasserta( argc > 0, "error: argc must be greater than zero." );
    cmdasserta( argv, "error: argv can't be NULL." );
    
    cmdArgs_t in;
    in.argc = argc;
    in.argv = argv;
    
    out->argc = 1;
    out->argv = CmdAlloc( sizeof(const char*) * argc );
    out->argv[0] = argv[0];
    
    return CmdArgsProcessOptions( out, &in, optionList, userData );
}

/*
============
CmdOptProcessFinal
============
*/
void CmdOptProcessFinal( cmdArgs_t *out ) {
    cmdasserta( out, "error: pointer can't be NULL." );
    
    CmdFree( out->argv );
    out->argc = 0;
    out->argv = NULL;
}