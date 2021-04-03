#ifndef __COMMAND_LINE_ARGUMENTS_H__
#define __COMMAND_LINE_ARGUMENTS_H__

#include <stdint.h>



/*
defines
CMD_ARGS_NODBG
DEBUG
_DEBUG
*/



/*
allocator interface
*/
typedef void*(*fnCmdAlloc)(size_t);
typedef void(*fnCmdFree)(void*);


/* function-handler
the function should return 0 on success
*/
/*                            key,        value,       userData */
typedef int(*fnCmdOptHandler)(const char*,const char*, void*);



/*
cmdArgs_t
*/
typedef struct {
    int             argc;       /* arg count */
    char            **argv;     /* arg values */
} cmdArgs_t;




void    CmdOptInit( void );

void    CmdOptFinal( void );

void    CmdOptAdd( const char *sk, const char *lk, const char *descr,
                unsigned char useOptVal, fnCmdOptHandler handler );

int     CmdOptProcess( cmdArgs_t *out, int argc, char **argv, void *userData );

void    CmdOptProcessFinal( cmdArgs_t *out );



#endif //__COMMAND_LINE_ARGUMENTS_H__