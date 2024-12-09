# ipc

to build:
g++ shared_mem_ipc.cpp

to run (may need to run as root):
./a.out

This exmaple uses shared memory mapping between two processes to send messages from one process to another. In this case there is a parent process that forks a child process off of it, although you could do the same thing with independent processes. The parent process (P0) sends messages to the child proccess (P1) by writing data to the shared memory space, and setting a shared state variable to the value 1 to indicate that the message has been set. The child process then reads the message, prints the contents to stdout and sets the message's state to 2 to indicate that it has been read and is ready for the next message. When the parent has sent all messages, it sets the last message's indicator to 3 to indicate that it is the final message.
