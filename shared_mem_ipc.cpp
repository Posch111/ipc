#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <vector>

typedef enum {
    Ready = 1, // Start at 1 because shared memory will initialize to 0
    Complete = 2,
    Done = 3
} MessageState;

typedef struct {
    int i;
    float f;
    char msg[50];
    MessageState state;
} Message;

// List of messages that will be sent to the child process
std::vector<Message> msgs {
    {
        1,
        1.111,
        "This is the first message",
        MessageState::Ready
    },
    {
        2,
        2.2222,
        "This is the second message",
        MessageState::Ready
    },
    {
        3,
        3.3333,
        "This is the third message",
        MessageState::Ready
    },
    {
        4,
        4.4444,
        "This is the fourth message",
        MessageState::Ready
    },
    {
        5,
        5.5555,
        "This is the fifth message",
        MessageState::Ready
    },
    {
        6,
        6.6666,
        "This is the sixth message",
        MessageState::Ready
    },
    {
        7,
        7.7777,
        "This is the seventh message",
        MessageState::Ready
    },
    {
        8,
        8.8888,
        "This is the eighth message",
        MessageState::Ready
    },
    {
        9,
        9.9999,
        "This is the ninth message",
        MessageState::Ready
    },
    {
        10,
        10.1010,
        "This is the tenth message",
        MessageState::Ready
    },
    {
        11,
        11.1111,
        "This is the 11th message",
        MessageState::Done
    }
};

std::string stateToString(MessageState state) {
    switch(state) {
        case MessageState::Complete: return "Complete";
        case MessageState::Done:     return "Done";
        case MessageState::Ready:    return "Ready";
        default:                     return "None";
    }
}

int main() {
    const char* shm_name = "/ipc";
    const int shm_size = sizeof(Message);

    std::cout << "Sending data to client..." << std::endl;

    // Create shared memory object
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0777);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // Set the size of the shared memory object
    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("ftruncate");
        return 1;
    }

    // Map the shared memory object into the process address space
    char* shared_mem = (char*)mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Send messages over shared memory
    pid_t pid = fork();
    if (pid == 0) { // Child process (client)
        Message *shared = (Message*)shared_mem;

        while(1) {
            // Wait for message to be ready
            while(shared->state != MessageState::Ready && shared->state != MessageState::Done);
            
            shared->msg[49] = 0; // Ensure string is null terminated
            std::cout << "P1: " << "int: " << shared->i << " float: " << shared->f << " string: " << shared->msg
                    << " enum: " << stateToString(shared->state) << std::endl;;
            fflush(stdout);

            if(shared->state == MessageState::Done) {
                break;
            }
            shared->state = MessageState::Complete;
        }

        std::cout << "P1: " << "All messages received" << std::endl;
    } else if (pid > 0) { // Parent process (server)
        // Initialize shared mem with first message
        memcpy(shared_mem, &msgs[0], sizeof(Message));
        while(1) {
            Message *shared = (Message*)shared_mem;
            if(shared->state == MessageState::Complete) {
                break;
            }
        }
        
        // Wait for reader to mark messages complete, send the next message, until exhausted of messages.
        for(int i = 1; i < msgs.size(); i++) {
            while(1) {
                Message *shared = (Message*)shared_mem;
                if(shared->state == MessageState::Complete) {
                    break;
                }
            }
            memcpy(shared_mem, &msgs[i], sizeof(Message));
            std::cout << "P0: Sent message " << msgs[i].i << std::endl;
        }
        std::cout << "P0: All messages Sent" << std::endl;

    } else {
        perror("fork");
        return 1;
    }

    // Clean up
    munmap(shared_mem, shm_size);
    shm_unlink(shm_name);
    sleep(1);
    return 0;
}
