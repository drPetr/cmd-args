#ifndef __COMMAND_LINE_ARGUMENTS_H__
#define __COMMAND_LINE_ARGUMENTS_H__

#include <stdint.h>

/*
defines
CMD_ARGS_NODBG
DEBUG
_DEBUG
*/



/* allocator interface */
typedef void*(*fnCmdAlloc)(size_t);
typedef void(*fnCmdFree)(void*);


/* function-handler
the function should return 0 on success
                              key,        value,       userData */
typedef int(*fnCmdOptHandler)(const char*,const char*, void*);



/* cmdArgs_t */
typedef struct {
    int     argc;       /* arg count */
    char    **argv;     /* arg values */
} cmdArgs_t;



void    CmdOptInit( void );
/* Initialization and addition of an automatic
handler for the '-h' and '--help' flags
*/

void    CmdOptFinal( void );
/* Called only after function call CmdOptInit().
The function frees the previously allocated memory.
Removes all previously added flag handlers.
*/

void    CmdOptAdd( const char *sk, const char *lk, const char *descr,
                unsigned char useOptVal, fnCmdOptHandler handler );
/* Add a new option with 'sk' (short key '-h') and lk
(long key '--help') keys. 'descr' is option description.
Using option value 'useOptVal' ('-ltext' or '-l text'
where 'text' is a value). 'handler' is option handler
function.
*/

int     CmdOptProcess( cmdArgs_t *out, int argc, char **argv, void *userData );
/* Process command line arguments. Arguments that
were not recognized as flags are written to 'out'.
'argc' and 'argv' these are command line arguments.
'userData' - passed to the function argument handler.

return values:
0       - when flags are processed correctly;
not 0   - the return value of the handler function.
*/

void    CmdOptProcessFinal( cmdArgs_t *out );
/* Called only after function call CmdOptProcess().
The function frees the previously allocated memory.
*/



#endif //__COMMAND_LINE_ARGUMENTS_H__