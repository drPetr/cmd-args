#include <stdio.h>
#include "cmd_args.h"

int HelloMessage( const char *key, const char *val, void *userData ) {
    printf( "Hello '%s'\n", val );
    return 0;
}

int main( int argc, char **argv ) {
    cmdArgs_t args;
    
    /* ������������� ���������� */
    /* ���������� ������ '-h' � '--help' ������� ������� ���� ������� */
    CmdOptInit();
    
    /* ���������� ������ ����������� ����� */
    CmdOptAdd( "-H", "--hello", "Print `Hello <value>` message.", 1, HelloMessage );
   
   /* ���������� ��� ��������� ��������� ������ */
    if( 0 != CmdOptProcess( &args, argc, argv, NULL ) ) {
        /* ���� �����-���� ���������� ������ �������� �� ���� ��������, �� �����
        �������� ���� �������� ����. �������� -h � --help ����� ��������� ���������
        ���������� ����� ���� */
        return 0;
    }
    
    /* ����� ���������� ������ �� �����. ����� ����������� ������. */
    CmdOptFinal();
    
    /* ��� ��������� ������� �� ���� ������� ����� ��������� ����� �������� � 
    ���������� args */
    int i;
    for( i = 0; i < args.argc; i++ ) {
        printf( "%s\n", args.argv[i] );
    }
    
    /* ����������� ������ */
    CmdOptProcessFinal( &args );
    return 0;
}