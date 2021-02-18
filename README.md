# Tracer

This programm helps you to trace all processes which you permitted to trace.

How to compile? Use command "make" and you will recieve final.out. You can run it, all information will be printed in stdout.

1) example.c you can find information about special actions which you can do for example before traced process call syscall. There is check_pid() in example.c it should be rewrite to be suitable for problem that you want to solve.
2) syscall_handle.c contain information of handling syscall, now there is only open/openat, but soon it will grow. Open_cb also place in syscall_handle.c it should be changed in the future.
3) tracer_final.c contain some function which help to work with ptrace without need to get deep in it.
4) set.c is hastily created container which used only to help tracing processes.

Constants which can be change for convenience:
example.c:  "TOUT" it is timeout for wait_for_syscall()
            "UPDATE_LOOP" is how many loops in main while(1) should passed before new update in set (array of pids and some info about them) for it process will look in /proc during this no process will be handle.         
