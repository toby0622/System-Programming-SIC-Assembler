#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define cMIN -1
#define cLOAD 0
#define cSHOW 1
#define cUNLOAD 2
#define cEXIT 3
#define cRUN 4
#define cMAX 5

#define oMIN -1
#define oADD 0
#define oAND 1
#define oCOMP 2
#define oDIV 3
#define oJ 4
#define oJEQ 5
#define oJGT 6
#define oJLT 7
#define oJSUB 8
#define oLDA 9
#define oLDCH 10
#define oLDL 11
#define oLDX 12
#define oMUL 13
#define oOR 14
#define oRD 15
#define oRSUB 16
#define oSTA 17
#define oSTCH 18
#define oSTL 19
#define oSTSW 20
#define oSTX 21
#define oSUB 22
#define oTD 23
#define oTIX 24
#define oWD 25
#define oMAX 26

#define specialByte 1024000

FILE *f;
char fname[20];
char tname[20];
char c_line[30];
char o_line[80];
char cmd[8];
int prog_len = 0;
int start_add = 0;
int first_add = 0;
int curr_add = 0;
char* memory;
int mem_size = 0;
int loaded = 0;
int op = 0;
int indexed = 0;
int operand = 0;
int running = 0;
char filename[100] = {};

int reg_A, reg_X, reg_L, reg_PC, reg_SW;

const char s_command[5][7] = {"load", "show", "unload", "exit", "run"};
const int MAXADD = 0x7FFF;
const int IKEY = 0x8000;

const char optab[26][3] = {"18", "40", "28", "24", "3C", "30", "34",
                           "38", "48", "00", "50", "08", "04", "20",
                           "44", "D8", "4C", "0C", "54", "14", "E8",
                           "10", "1C", "E0", "2C", "DC"};

void memory_set() {
    int s = specialByte;

    memory = (char *)malloc(sizeof(char) * s);

    if (memory) {
        for (int i = 0; i < s; i++) {
            memory[i] = 'X';
        }

        memory[s - 1] = '\0';
    } else {
        puts("Error Message: Memory Set Error");
    }
}

int lookup(char *s) {
    for (int i = 0; i < oMAX; i++) {
        if (strcmp(optab[i], s) == 0) {
            return i;
        }
    }

    return (-1);
}

int readline() { // read the input command
    int i = 0;
    int tmp = cMIN;

    printf("SIC Simulator> ");

    do {
        fgets(c_line, 30, stdin);
        i = strlen(c_line);
        c_line[i - 1] = '\0';
    } while (i <= 1);

    sscanf(c_line, "%s", cmd);

    if (strcmp(cmd, s_command[0]) == 0) { // modify the input file name
        tmp = cLOAD;
        strcpy(filename, (c_line + 5));
        f = fopen(filename, "r");

        if (!f) {
            printf("No Such Filename %s\n", filename);
            return (-100);
        }
    } else if (strcmp(cmd, s_command[1]) == 0) {
        tmp = cSHOW;
    } else if (strcmp(cmd, s_command[2]) == 0) {
        strcpy(filename, (c_line + 7));
        f = fopen(filename, "r");
        tmp = cUNLOAD;
    } else if (strcmp(cmd, s_command[3]) == 0) {
        tmp = cEXIT;
    } else if (strcmp(cmd, s_command[4]) == 0) {
        tmp = cRUN;
    }

    c_line[0] = '\0';
    cmd[0] = '\0';
    return tmp;
}

void rd_header() { // read the header
    char tmp[7];
    int i, j, s;

    memory_set();
    fgets(o_line, 80, f);

    for (i = 7, j = 0; i < 13; i++, j++) {
        tmp[j] = o_line[i];
    }

    tmp[j] = '\0';
    sscanf(tmp, "%x", &start_add);

    for (i = 13, j = 0; i < 19; i++, j++) {
        tmp[j] = o_line[i];
    }

    tmp[j] = '\0';
    sscanf( tmp, "%x", &prog_len );
    printf("Start Address: %x, Program Length: %x\n", start_add, prog_len);
    loaded = 1;
    mem_size = s;
}

int rd_text() { // write the text record into the memory
    char tmp[7];
    int i, j, l, s;

    fgets(o_line, 80, f);

    if (o_line[0] == 'E') {
        return 0;
    }

    for (i = 1, j = 0; i < 7; i++, j++) {
        tmp[j] = o_line[i]; // obtain the starting address
    }

    tmp[j] = '\0';
    sscanf(tmp, "%x", &s);

    for (i = 7, j = 0; i < 9; i++, j++) {
        tmp[j] = o_line[i]; // obtain the length
    }

    tmp[j] = '\0';
    sscanf( tmp, "%x", &l );

    l = 9 + l * 2;

    int len = strlen(o_line);

    for (i = 9, j = (s - start_add) * 2 + start_add; i < len; i++, j++) { // write into the memory
        if(o_line[i] != '\n') {
            memory[j] = o_line[i];
        }
    }

    return 1;
}

void rd_end() { // jump to the next executable object code
    char tmp[7];
    int i, j;

    for (i = 1, j = 0; i < 7; i++, j++) {
        tmp[j] = o_line[i];
    }

    tmp[j] = '\0';
    sscanf(tmp, "%x", &first_add);
}

/* System Programming Lab Assignment 2 */
void s_load() { // load the object code into memory
    if (loaded) {
        puts("The Program Hasn't Been Released Yet!\n");
    } else {
        rd_header(); // obtain the start address and the program length
        while(rd_text()); // write the text record into the memory
        rd_end();
    }
}

char integer_to_character_string(int n) {
    return (n >= 10) ? n - 10 + 'a' : n + '0';
}

void dec_to_hex(int num) {
    char ans[5] = {0};

    while (num) {
        char c[2];
        c[0] = integer_to_character_string(num % 16);
        strcat(ans, c);
        num /= 16;
    }

    strrev(ans);
    printf("%04s ", ans);
}

// System Programming Lab Assignment 2
void s_show() { // display the memory status
    int i, cur;

    if (loaded) {
        for (i = cur = start_add; i < 3.5 * pow(16, 3); ++i) {
            if (i % 8 == 0) {
                putchar(' ');
            }

            if (i % 32 == 0) {
                printf("\n");
                printf("%d ", i);
                dec_to_hex(cur);
                cur += 16;
            }

            putchar(memory[i]);
        }
    }

    putchar('\n');
}

// System Programming Lab Assignment 2
void s_unload() { // release all data structures
    free(memory);
    start_add = 0;
    prog_len = 0;
    start_add = 0;
    first_add = 0;
    curr_add = 0;
    mem_size = 0;
    loaded = 0;
    op = 0;
    indexed = 0;
    operand = 0;
    running = 0;

    memset(filename, '\0', sizeof(filename));
}

void init_run() {
    reg_A = 0;
    reg_X = 0;
    reg_L = 0;
    reg_PC = first_add;
    reg_SW = 0;
    curr_add = (first_add - start_add) * 2;
    running  = 1;
}

void get_op() {
    char s[3];
    char t[5];

    s[0] = memory[curr_add];
    s[1] = memory[curr_add+1];
    s[2] = '\0';
    op = lookup(s);
    t[0] = memory[curr_add+2];
    t[1] = memory[curr_add+3];
    t[2] = memory[curr_add+4];
    t[3] = memory[curr_add+5];
    t[4] = '\0';

    sscanf(t, "%X", &operand);

    if (operand >= IKEY) {
        indexed = 1;
        operand -= IKEY;
    } else {
        indexed = 0;
    }

    curr_add += 6;
    reg_PC += 3;
}

int get_value(int r, int x) {
    int tmp = 0;
    char s[7];
    int i, j;

    if (x) {
        r += reg_X;
    }

    i = (r - start_add) * 2;

    for (j = 0; j < 6; j++) {
        s[j] = memory[i++];
    }

    s[6] = '\0';
    sscanf(s, "%X", &tmp);

    return tmp;
}

int get_byte(int r, int x) {
    int tmp = 0;
    char s[3];
    int i, j;

    if (x) {
        r += reg_X;
    }

    i = (r - start_add) * 2;
    j=0;
    s[j++] = memory[i++];
    s[j++] = memory[i++];
    s[2] = '\0';
    sscanf(s, "%X", &tmp);

    return tmp;
}

void put_byte(int k, int r, int x) {
    int tmp = 0;
    char s[3];
    int i, j;

    if (x) {
        r += reg_X;
    }

    sprintf(s, "%02X", k);
    i = (r - start_add) * 2;
    j=0;
    memory[i++] = s[j++];;
    memory[i++] = s[j++];;
}

void put_value(int k, int r, int x) {
    int tmp = 0;
    char s[7];
    int i, j;

    if (x) {
        r += reg_X;
    }

    sprintf(s, "%06X", k);
    i = (r - start_add) * 2;

    for (j = 0; j < 6; j++) {
        memory[i++] = s[j];
    }
}

void show_reg() {
    printf("Register A  = [%06X];\n", reg_A);
    printf("Register X  = [%06X];\n", reg_X);
    printf("Register L  = [%06X];\n", reg_L);
    printf("Register SW = [%06X];\n", reg_SW);
    printf("Register PC = [%06X];\n", reg_PC);
}

// System Programming Lab Assignment 2
// something wrong with this
void s_run() {
    init_run();
    get_op();
    show_reg();
}

int main() {
    int comm = 0;

    comm = readline();

    while (comm != cEXIT) {
        switch (comm) {
            case -100:
                break;
            case cLOAD: s_load();
                break;
            case cSHOW: s_show();
                break;
            case cUNLOAD: s_unload();
                break;
            case cRUN: s_run();
                break;
            default: printf("Unknown Command!\n");
                break;
        }

        comm = cMIN;
        comm = readline();
    }

    if (loaded) {
        s_unload();
    }
}
