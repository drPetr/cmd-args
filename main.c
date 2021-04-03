#include <stdio.h>
#include "cmd_args.h"

int HelloMessage( const char *key, const char *val, void *userData ) {
    printf( "Hello '%s'\n", val );
    return 0;
}

int main( int argc, char **argv ) {
    cmdArgs_t args;
    
    /* Инициализация библиотеки */
    /* Обработчик флагов '-h' и '--help' создётся вызовом этой функции */
    CmdOptInit();
    
    /* Добавление своего обработчика флага */
    CmdOptAdd( "-H", "--hello", "Print `Hello <value>` message.", 1, HelloMessage );
   
   /* Обработать все аргументы командной строки */
    if( 0 != CmdOptProcess( &args, argc, argv, NULL ) ) {
        /* если какой-либо обработчик вернул отличное от нуля значение, то будет
        выполнен этот фрагмент кода. Например -h и --help после обработки продолжат
        выполнение этого кода */
        return 0;
    }
    
    /* Далее обработчик флагов не нужен. Можно освобождать память. */
    CmdOptFinal();
    
    /* Все аргументы которые не были флагами после обработки будут записаны в 
    переменную args */
    int i;
    for( i = 0; i < args.argc; i++ ) {
        printf( "%s\n", args.argv[i] );
    }
    
    /* Освобождене памяти */
    CmdOptProcessFinal( &args );
    return 0;
}