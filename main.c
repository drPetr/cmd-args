#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_args.h"




char *salloc( size_t size ) {
    return (char*)malloc(size);
}

void sfree( char *s ) {
    free( s );
}

char *slorem( size_t len ) {
    const char *lorem =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
        "sed do eiusmod tempor incididunt ut labore et dolore magna "
        "aliqua. Ut enim ad minim veniam, quis nostrud exercitation "
        "ullamco laboris nisi ut aliquip ex ea commodo consequat. "
        "Duis aute irure dolor in reprehenderit in voluptate velit "
        "esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
        "occaecat cupidatat non proident, sunt in culpa qui officia "
        "deserunt mollit anim id est laborum.\n";
    char *str, *s;
    const char *p;
    
    str = s = salloc( len + 1 );
    while( len ) {
        p = lorem;
        while( *p && len ) {
            *s++ = *p++;
            len--;
        }
    }
    *s = 0;
    
    return str;
}


int Hello( const char *key, const char *val, void *userData ) {
    printf( "Hello world '%s' '%s'\n", key, val );
    return 0;
}

int Lorem( const char *key, const char *val, void *userData ) {
    char *s = slorem( 446 );
    printf( s );
    sfree( s );
    return 1;
}

int my( const char *key, const char *val, void *userData ) {
    printf( "MY function '%s'\n", val );
    return 0;
}

int my2( const char *key, const char *val, void *userData ) {
    if( 0 == strcmp( key, "-1" ) ) {
        printf( "ONE\n" );
        return 0;
    } else if( 0 == strcmp( key, "-2" ) ) {
        printf( "TWO\n" );
        return 0;
    } else if( 0 == strcmp( key, "-3" ) ) {
        printf( "THREE\n" );
        return 0;
    } else if( 0 == strcmp( key, "-4" ) ) {
        printf( "FOUR\n" );
        return 0;
    } else if( 0 == strcmp( key, "-5" ) ) {
        printf( "FIVE\n" );
        return 0;
    } else if( 0 == strcmp( key, "-6" ) ) {
        printf( "SIX '%s'\n", val );
        return 0;
    }
    
    return -1;
}

int main( int argc, char **argv ) {
    cmdArgs_t args;
    
    CmdOptInit();
    CmdOptAdd( "-H", "--hello",
        "Print 'Hello world' message and exit. "
        "This description message showed once. ", 1, Hello );
    CmdOptAdd( "-m", "--my",
        "This is SPARTAAAAAAAA.", 1, my );
    CmdOptAdd( "-1", NULL, "ONE.", 0, my2 );
    CmdOptAdd( "-2", NULL, "TWO.", 0, my2 );
    CmdOptAdd( "-3", NULL, "THREE.", 0, my2 );
    CmdOptAdd( "-4", NULL, "FOUR.", 0, my2 );
    CmdOptAdd( "-5", NULL, "FIVE.", 0, my2 );
    CmdOptAdd( "-6", NULL, "SIX.", 1, my2 );
    CmdOptAdd( "-L", "--lorem",
        "Print 'Lorem' message and exit. "
        "This description message showed once. Example message: "
        "'Lorem ipsum dolor sit amet, consectetur adipiscing elit'",
        0, Lorem );
        
    if( CmdOptProcess( &args, argc, argv, NULL ) ) {
        return 0;
    }
    CmdOptFinal();
    
    int i;
    printf( "input:\n" );
    for( i = 0; i < args.argc; i++ ) {
        printf( "%s\n", args.argv[i] );
    }
    
    CmdOptProcessFinal( &args );
    
    return 0;
}