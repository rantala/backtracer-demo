backtracer: main.c backtrace.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $+ $(LDFLAGS) -pthread -lunwind-ptrace -lunwind-generic -o $@
