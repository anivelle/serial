#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUFFER_SIZE 2048

typedef enum state {
    SEL_STATE,
    READING,
    WRITING,
} state_t;

struct readexit {
    int should_exit;
    int reason;
};

void *exit_check(void *param);
static struct readexit exit_read = {.should_exit = 0, .reason = -1};
static int fd;

int main(int argc, char *argv[]) {

    pthread_t tid = 0;
    pthread_attr_t attr;

    fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        printf("Could not open port\n");
        return 1;
    }
    fcntl(fd, F_SETFL, 0);
    // For the thread (does not currently work)
    pthread_attr_init(&attr);
    state_t curstate = SEL_STATE;
    int do_quit = 1;
    int char_read;
    char *writebuf = malloc(sizeof(char));

    // Proper program loop
    while (do_quit) {
        char next_state[2];
        char readbuf[BUFFER_SIZE];
        switch (curstate) {
        case SEL_STATE:
            printf("\nDo you want to read or write (r/w/q)? ");
            fflush(stdout);
            read(STDIN_FILENO, &next_state, 2);
            switch (next_state[0]) {
            case 'r':
                curstate = READING;
                break;
            case 'w':
                curstate = WRITING;
                break;
            case 'q':
                do_quit = 0;
                break;
            default:
                printf("Please enter a valid option\n");
                break;
            }
            break;
        case READING:
            if (!tid)
                pthread_create(&tid, &attr, exit_check, NULL);
            if (read(fd, readbuf, BUFFER_SIZE) > 0)
                printf("%s", readbuf);
            else {
                exit_read.should_exit = 1;
            }
            if (exit_read.should_exit) {
                switch (exit_read.reason) {
                case 0:
                    pthread_join(tid, NULL);
                    exit_read.should_exit = 0;
                    exit_read.reason = -1;
                    curstate = SEL_STATE;
                    tid = 0;
                    break;
                case 1:
                    exit_read.should_exit = 0;
                    exit_read.reason = -1;
                    curstate = WRITING;
                    break;
                default:
                    break;
                }
            }
            break;
        case WRITING:

            getline(&writebuf, (size_t *)&char_read, stdin);

            // Sanitize input (remove '\n')
            writebuf[char_read - 2] = 0;
            char_read--;
            int written, left = char_read - 1;
            
            // TODO: Test this section
            // Write the data fully (untested, cause I don't have anything to
            // write to yet)
            while((written = write(fd, writebuf, char_read - 1)) < left) {
              left = left - written;
              writebuf = &writebuf[char_read - 1 - left];
            }
            // After sending a command, wait for any responses. The struct
            // should allow for the state to transition back to writing, but again,
            // untested
            exit_read.reason = 1;
            curstate = READING;
            break;
        default:
            break;
        }
    }
    return 0;
}

// Thread to check for enter input when reading (to leave the state)
void *exit_check(void *param) {
    char buf;
    while (read(STDIN_FILENO, &buf, 1) == 1) {
        if (buf == '\n')
            break;
    }
    exit_read.should_exit = 1;
    exit_read.reason = 0;
    pthread_exit(0);
}
