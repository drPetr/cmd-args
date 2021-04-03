all:
	gcc main.c cmd_args.c -oapp -DDEBUG -DCMD_ARGS_NODBG -Wall