#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

int MAXINPUTLENGTH = 2048;
int MAXINPUTARGS = 512;
int bgAllowed = 1;

/*
* Function: expando
* -------------------------
* finds two consecutive dollar signs in input string and replaces them with processIDStr.
*
* Parameters:
* -------------------------
* userInputWithMoneys - the string to be transformed. Contains "$$" upon input.
* processIDStr - the process ID in string form.
*
* Returns:
* -------------------------
* A transformed string with "$$" replaced with the process ID string.
*/
char *expando(const char* userInputWithMoneys, const char* processIDStr) {

    const char* moneys = "$$";

    // Making new string of enough length
    char* newStr;
    newStr = (char*)malloc(512);

    // ints needed to find and replace money signs with PID
    int a, pIDStrLen, lenMonehhs;
    a = 0;
    pIDStrLen = strlen(processIDStr); // get & store the PID length
    lenMonehhs = 2; // length of two money signs

    while (*userInputWithMoneys) {
        // compare the strings and replace when money signs found
        if (strstr(userInputWithMoneys, moneys) == userInputWithMoneys) {
            strcpy(&newStr[a], processIDStr); // copying PID to string
            a += pIDStrLen; // index needs to jump this much since PID has been added to the string
            userInputWithMoneys += lenMonehhs; // increase pointer
        }
        else
            newStr[a++] = *userInputWithMoneys++; // normal copy
    }
    newStr[a] = '\0'; // add null terminator
    return newStr;
}

/*
 * Function: setArrayToNull
 * -------------------------
 * Sets all array positions to NULL up to the terminator position.
 *
 * Parameters:
 * -------------------------
 * inputArray - array of strings to be set to NULL
 * terminator - the last position in the array to be set to NULL.
 *
 * Returns:
 * -------------------------
 * void - Sets all array positions to NULL up to the terminator position.
 */
void setArrayToNull(char **inputArray, int terminator) {
    for (int i = 0; i < terminator; i++) {
        inputArray[i] = NULL;
    }
}

/*
* Function: parseCommandInput
* -------------------------
* Parses out the user's command line input, copies in
*
* Parameters:
* -------------------------
* buffer - the array to hold the user's command line inputs
* fileIn - container for the input file if the user enters one
* fileOut - container for the output file if the user enters one
* pidStr - the process ID converted to a string
* bgProcessFlag - used to track if the user enters an "&", which in turn is used to run the process with WNOHANG.
*
* Returns:
* -------------------------
* User commands are stored in the buffer input and the number of input commands is returned.
*/
int parseCommandInput(char **buffer, char *fileIn, char *fileOut, char *pidStr, int* bgProcessFlag) {

    char *inputContainer[MAXINPUTARGS];
    int commandCount = 0;

    for (int i = 0; i < MAXINPUTARGS; i++) {
        inputContainer[i] = NULL;
    }

    char incoming[MAXINPUTLENGTH];

    fgets(incoming, MAXINPUTLENGTH, stdin);// gets user input from command line

    // Changes the new line char to null terminator to prevent parsing issues.
    int newLine = 1;
    int i = 0;
    while (newLine == 1 || i > MAXINPUTLENGTH - 2) {
        if (incoming[i] == '\n') {
            incoming[i] = '\0';
            newLine = 0;
        }
        i++;
    }
    i = 0;

    // get the first set of chars in the input
    char *nextWordInInput = strtok(incoming, " ");

    // used in parsing below
    const char rightArrow[2] = ">";
    const char leftArrow[2] = "<";
    const char ampersand[2] = "&";
    const char moneys[3] = "$$";

    // check for blank input or comments
    if ((strlen(incoming) < 1) || (strstr(incoming, "#") != 0)) {
        buffer[0] = strdup("");
        return commandCount;
    }

    // parses out the user input
    while (nextWordInInput) {
        commandCount++; // count each command
        if (strstr(nextWordInInput, rightArrow) != 0) {
            if (strcmp(nextWordInInput, rightArrow) == 0) {
                nextWordInInput = strtok(NULL, " "); // next arg should be a file
                strcpy(fileOut, nextWordInInput); // copy input if it's a file to fileout
            }
        } else if (strstr(nextWordInInput, leftArrow) != 0) {
            if (strcmp(nextWordInInput, "<") == 0) {
                nextWordInInput = strtok(NULL, " "); // next arg should be a file
                strcpy(fileIn, nextWordInInput); // copy input to fileIn if it's a file
            }
        } else if (strstr(nextWordInInput, ampersand) != 0) {
            if (strcmp(nextWordInInput, "&") == 0) {
                *bgProcessFlag = 1; // toggle to run background process (run with WNOHANG flag)
            }
        } else {
            buffer[i] = strdup(nextWordInInput); // add input to array
            if (strstr(nextWordInInput, moneys) != 0) {
                char *newString = (char*)malloc(1024);
                newString = expando(nextWordInInput, pidStr); // replace all occurrences of "$$" with the PID string.
                buffer[i] = strdup(newString); // copy to buffer
            }
        }
        nextWordInInput = strtok(NULL, " "); // gets the next command
        i++;
    }
    return commandCount;
}

/*
 * Function: getExit
 * -------------------------
 * gets the current exit status and prints it.
 *
 * Parameters:
 * -------------------------
 * wstatus - the variable holding the current exit status
 *
 * Returns:
 * -------------------------
 * uses the WIFEXITED or WIFSIGNALED macros to determine if exit status is from signal or exit, and prints using the
 * appropriate macro to interpret the status.
 */
void getExit(int wstatus) {
    if (WIFEXITED(wstatus)) { // for exit status: WIFEXITED = true for stat exit.
        printf("exit value %d\n", WEXITSTATUS(wstatus)); // WEXITSTATUS macro interprets wstatus
        fflush(stdout);
    } else if WIFSIGNALED(wstatus){ // WIFSIGNALED = true for signal exit
        printf("terminated by signal %d\n", WTERMSIG(wstatus)); // WTERMSIG interprets wstatus
        fflush((stdout));
    }
}

/*
 * Function: changeDir
 * -------------------------
 * executes the "cd" function when the user enters cd.
 *
 * Parameters:
 * -------------------------
 * newDirectory: the destination directory the user wants to change to.
 *
 * Returns:
 * -------------------------
 * void
 */
void changeDir(char *newDirectory) {

    if (newDirectory) {
        if (chdir(newDirectory) == -1) {
            printf("Could not find that folder.\n");
            fflush(stdout);
        }
    } else {
        chdir(getenv("HOME")); // use getenv to find the value of the home path.
    }
}

/*
 * Function: openOrCloseFile
 * -------------------------
 * Opens or closes a file, depending on the value of the input parameters.
 *
 * Parameters:
 * -------------------------
 * readOrWrite: 0  = read only, 1 = write, 2 = used for close file option.
 * fd: file descriptor value to be closed.
 * fileName: the file to be opened.
 * openOrClose: 1 = open, 0 = close.
 *
 * Returns:
 * -------------------------
 * returnValue: the value returned by a file open or file close operation.
 */
int openOrCloseFile(int openOrClose,char *fileName, int fd, int readOrWrite) {
    int returnValue = 0;

    if (openOrClose == 1 && readOrWrite == 0) {
        returnValue = open(fileName, O_RDONLY);
    } else if (openOrClose == 1 && readOrWrite == 1) {
        returnValue = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    } else if (openOrClose == 0 && readOrWrite == 2) {
        returnValue = close(fd);
    }
    return returnValue;
}

/*
 * Function: handlePiping
 * -------------------------
 * Handles the piping operations required by the "<" and ">" operators.
 *
 * Parameters:
 * -------------------------
 * in: the input file to be handled.
 * out: the output file to be handled.
 *
 * Returns:
 * -------------------------
 * void
 */
void handlePiping(char *in, char *out) {

    int inFileDescriptor = 0;
    int dup2FileDescriptor = 0;
    int outFileDescriptor = 0;

    // if string length for the input file is > 0, then there's one to be used.
    if (strlen(in) > 0) {

        inFileDescriptor = openOrCloseFile(1, in, 0, 0);

        if (inFileDescriptor == -1) {
            printf("Unable to open %s\n", in);
            exit(1);
        }

        dup2FileDescriptor = dup2(inFileDescriptor, 0);

        if (dup2FileDescriptor == -1) {
            printf("dup2 failed!!!!\n");
            exit(2);
        }

        int closeVal = openOrCloseFile(0, in, inFileDescriptor, 2);

        if (closeVal == -1) {
            printf("File close failed!");
            fflush(stdout);
            exit(1);
        }
    }

    // if string length for the output file is > 0, then there's one to be used.
    if (strlen(out) > 0) {

        outFileDescriptor = openOrCloseFile(1, out, 0, 1);
        // check if open was successful
        if (outFileDescriptor == -1) {
            printf("Unable to open %s\n", out);
            fflush(stdout);
            exit(1);
        }

        dup2FileDescriptor = dup2(outFileDescriptor, 1);
        // check if dup2 succeeded
        if (dup2FileDescriptor == -1) {
            printf("dup2 failed!!!!\n");
            fflush(stdout);
            exit(2);
        }

        int closeValue = openOrCloseFile(0, out, outFileDescriptor, 2);
        // check if close was successful
        if (closeValue == -1) {
            printf("File close failed!");
            fflush(stdout);
            exit(1);
        }
    }
}

/*
 * Function: runCommandNotCovered
 * -------------------------
 * Runs commands the assignment doesn't require us to cover. Runs everything except "cd", "exit", and "status".
 *
 * Parameters:
 * -------------------------
 * stat: the exit status
 * sigactionStruct: the custom sigaction handler used to define handling SIGINT.
 * fileIn: the input the user entered
 * fileOut: the output file the user entered
 * buffer: the input buffer containing the command line arguments
 * foregroundOnly: flag used to control running with WNOHANG
 *
 * Returns:
 * -------------------------
 * returnValue: the value returned by a file open or file close operation.
 */
void runCommandNotCovered(char *fileIn, char *fileOut, char **buffer, int* stat, struct sigaction sigactionStruct, int* foregroundOnly) {

    // Some of this code comes from the exploration on processes

    pid_t spawnPid = -5;
    // from lectures;
    spawnPid = fork();

    // failed fork branch
    if (spawnPid == -1) {
        printf("--Fork failed!!!---\n");
        fflush(stdout);
        printf("Spawn PID = %d", spawnPid);
        fflush(stdout);
        exit(1);
    }

        // child process executes here
    else if (spawnPid == 0) {

        // sets up sig int for child process
        sigactionStruct.sa_handler = SIG_DFL;
        sigaction(SIGINT, &sigactionStruct, NULL);

        handlePiping(fileIn, fileOut);

        if (execvp(buffer[0], buffer)) {
            // message taken from the Sample Program Execution
            printf("%s: no such file or directory\n", buffer[0]);
            fflush(stdout);
            exit(2);
        }
    }

        // Parent process happens here
    else {
        // waitPID is used with the WNOHANG parameter to allow the parent to not wait for spawnPID to finish.
        if (*foregroundOnly && bgAllowed) {
            pid_t actualPid = waitpid(spawnPid, stat, WNOHANG);
            printf("Background running process ID: %d\n", spawnPid);
            fflush(stdout);
        }
            // Run in foreground mode
        else {
            pid_t actualPid = waitpid(spawnPid, stat, 0);
        }

        // checks for terminated processes
        while ((spawnPid = waitpid(-1, stat, WNOHANG)) > 0) {
            printf("child process %d ended\n", spawnPid); //
            getExit(*stat);
            fflush(stdout);
        }
    }
}

/*
 * Function: handleSIGSTP
 * -------------------------
 * Defines how the SIGSTP signal should be handled. This is attached to the custom signal set for SIGTSTP.
 *
 * Parameters:
 * -------------------------
 * ID: signal ID
 *
 * Returns:
 * -------------------------
 * void
 */
void handleSIGTSTP(int ID) {

    // comments on the bgallowed variable:
    // toggling this variable also toggles when processed can run with WNOHANG flag. 1 allows bg processes.
    // see the if statement in the runCommandNotCovered function parent process section with this variable.

    // the message code comes from the reentrant exploration
    if (bgAllowed == 1) {
        char* message = "Entering foreground-only mode (& is now ignored)\n";
        write(1, message, 49);
        fflush(stdout);

        bgAllowed = 0;
    }

        // the message code comes from the reentrant exploration
    else if (bgAllowed == 0){
        char* message = "Exiting foreground-only mode\n";
        write (1, message, 29);
        fflush(stdout);

        bgAllowed = 1;
    }
}

int main() {

    int fgOnly = 0; // controls foreground only mode
    int inputCounter = 0; // counts number of command line args.
    int run = 1; // controls while loop operation.
    int currStatus = 0; // tracks exit status.

    int parentProcessID = getpid();
    // get number of digits in PID
    int numDigits = snprintf(NULL, 0, "%d", parentProcessID);
    char pidString[numDigits + 1];
    // converts int to string using snprintf
    snprintf(pidString, numDigits + 1, "%d", parentProcessID);

    char fileNameIn[1024] = "";
    char fileNameOut[1024] = "";
    char* inputBuffer[1024];
    setArrayToNull(inputBuffer, 1024);

    // Signal stuff ********************************************************************
    // most of this code is from "Exploration: Signal Handling API"
    // Declare a new sigaction struct, make SIGINT ignored, and add SIGINT to the struct.
    struct sigaction sig_int_custom = {0};
    sig_int_custom.sa_handler = SIG_IGN; // ignore SIGINT
    sig_int_custom.sa_flags = 0; // we aren't planning to set any flags for SIGINT
    if (sigfillset(&sig_int_custom.sa_mask) == -1) {
        printf("sigfillset failed for SIGINT");
    }
    sigaction(SIGINT, &sig_int_custom, NULL); // Adds SIGINT to sig_int_custom

    // Declare a new SIGTSTP struct, add new handler to it, and add SIGTSTP signal to it.
    struct sigaction sigtstp_custom = {0};
    sigtstp_custom.sa_handler = handleSIGTSTP;
    int sigTest = sigfillset(&sigtstp_custom.sa_mask);
    if (sigTest == -1) {
        printf("sigfillset failed for SIGTSTP\n");
    }
    sigtstp_custom.sa_flags = 0;
    sigaction(SIGTSTP, &sigtstp_custom, NULL);
    // *********************************************************************************

    while (run == 1) {

        // Print prompt
        printf(": ");
        fflush(stdout);

        // Get inputBuffer
        inputCounter = parseCommandInput(inputBuffer, fileNameIn, fileNameOut, pidString, &fgOnly);

        if (!strcmp(inputBuffer[0], "")) { // if user entered an empty command or a comment
            continue;
        } else if (strcmp(inputBuffer[0], "exit") == 0) {
            run = 0;
        } else if (strcmp(inputBuffer[0], "cd") == 0) {
            changeDir(inputBuffer[1]);
        } else if (strcmp(inputBuffer[0], "status") == 0) {
            getExit(currStatus);
        } else {
            runCommandNotCovered(fileNameIn, fileNameOut, inputBuffer, &currStatus, sig_int_custom, &fgOnly);
        }

        // Reset variables
        setArrayToNull(inputBuffer, inputCounter);
        fgOnly = 0;
        fileNameIn[0] = '\0';
        fileNameOut[0] = '\0';

    }
    return 0;
}