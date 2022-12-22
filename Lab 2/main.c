#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define cMIN (-1)
#define cLOAD 0
#define cSHOW 1
#define cUNLOAD 2
#define cEXIT 3
#define cRUN 4

#define oMIN (-1)
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

#define greaterThan 888
#define lessThan 999
#define equalTo 777

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

int lookup(char *s) {
    for (int i = 0; i < oMAX; i++) {
        if (strcmp(optab[i], s) == 0) {
            return i;
        }
    }

    return (-1);
}

int readline() {
    int i;
    int tmp = cMIN;

    printf("SIC Simulator>");

    do {
        fgets(c_line, 30, stdin);
        i = strlen(c_line);
        c_line[i-1] = '\0';
    } while (i <= 1);

    sscanf(c_line, "%s", cmd);

    if (strcmp(cmd, s_command[0]) == 0) {
        tmp = cLOAD;
    } else if (strcmp(cmd, s_command[1]) == 0) {
        tmp = cSHOW;
    } else if (strcmp(cmd, s_command[2]) == 0) {
        tmp = cUNLOAD;
    } else if (strcmp(cmd, s_command[3]) == 0) {
        tmp = cEXIT;
    } else if (strcmp( cmd, s_command[4]) == 0) {
        tmp = cRUN;
    }

    c_line[0] = '\0';
    cmd[0] = '\0';

    return tmp;
}

void rd_header() {
    char tmp[7];
    int i, j, s;

    for (i = 7, j = 0; i < 13; i++, j++) {
        tmp[j] = o_line[i];
    }

    tmp[j] = '\0';
    sscanf(tmp, "%x", &start_add);

    for (i = 13, j = 0; i < 19; i++, j++) {
        tmp[j] = o_line[i];
    }

    tmp[j] = '\0';
    sscanf(tmp, "%x", &prog_len);

    s = prog_len * 2 + 1;
    memory = (char *)malloc(sizeof(char) * s);

    if (memory) {
        for (i = 0; i < s; i++) {
            memory[i] = 'X';
        }

        memory[s-1] = '\0';
        loaded = 1;
        mem_size = s;
    } else {
        printf("Loading Failed! (Memory Allocation Error)\n");
    }
}

int rd_text() {
    char tmp[7];
    int i, j, l, s;

    for (i = 1, j = 0; i < 7; i++, j++) {
        tmp[j] = o_line[i];
    }

    tmp[j] = '\0';
    sscanf(tmp, "%x", &s);

    for (i = 7, j = 0; i < 9; i++, j++) {
        tmp[j] = o_line[i];
    }

    tmp[j] = '\0';
    sscanf(tmp, "%x", &l);

    l = 9 + l * 2;

    for (i = 9, j = (s - start_add) * 2; i < l; i++, j++) {
        memory[j] = o_line[i];
    }
}

void rd_end() {
    char tmp[7];
    int i;
    int j;

    for (i = 1, j = 0; i < 7; i++, j++) {
        tmp[j] = o_line[i];
    }

    tmp[j] = '\0';
    sscanf(tmp, "%x", &first_add);
}

/* System Programming Lab Assignment 2 */
void s_load() {
    int i;

    if (loaded) {
        printf("ERROR: Object Program Already Loaded In The Memory!\n");
    } else {
        printf("     Filename>");
        fgets(fname, 20, stdin);
        i = strlen(fname);
        fname[i - 1] = '\0';
        f = fopen(fname, "r");

        if (f == NULL) {
            printf("ERROR : Failed To Open File!\n");
        } else {
            fgets(o_line, 80, f);
            i = strlen(o_line);
            o_line[i - 1] = '\0';
            char prog_name[7];

            if (o_line[0] == 'H') {
                int j, k;

                for (k = 1, j = 0; k < 7; k++, j++) {
                    prog_name[j] = o_line[k];
                }

                prog_name[j] = '\0';
                rd_header();
            }

            fgets(o_line, 80, f);

            // verify program name & length
            printf("Program Name = [%s], Program Length = [%x]\n", prog_name, prog_len);

            while (o_line[0] != 'E') {
                int l;

                l = strlen(o_line);
                o_line[l - 1] = '\0';

                rd_text();
                fgets(o_line, 80, f);
            }

            rd_end();
            loaded = 1;
            o_line[0] = '\0';
            printf("Load Successful!\n");
        }

        fclose(f);
    }
}

// System Programming Lab Assignment 2
void s_show() {
    int i;
    int j = 0;

    for (curr_add = first_add; curr_add < first_add + prog_len; curr_add += 16) {
        printf("%x      ", curr_add);

        for (i = 0 + j * 32; i < 8 + j * 32 && memory[i] != '\0'; i++) {
            printf("%c", memory[i]);
        }

        printf("      ");

        for (; i < 16 + j * 32 && memory[i] != '\0'; i++) {
            printf("%c", memory[i]);
        }

        printf("      ");

        for (; i < 24 + j * 32 && memory[i] != '\0'; i++) {
            printf("%c", memory[i]);
        }

        printf("      ");

        for (; i < 32 + j * 32 && memory[i] != '\0'; i++) {
            printf("%c", memory[i]);
        }

        printf("\n");
        j++;
    }
}

// System Programming Lab Assignment 2
void s_unload() {
    if (loaded) {
        fclose(f);
        fname[0] = '\0';
        o_line[0] = '\0';
        free(memory);
        prog_len = 0;
        start_add = 0;
        first_add = 0;
        curr_add = 0;
        mem_size = 0;
        loaded = 0;
        op = 0;
        indexed = 0;
        operand = 0;
    }
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
    s[1] = memory[curr_add + 1];
    s[2] = '\0';
    op = lookup(s);
    t[0] = memory[curr_add + 2];
    t[1] = memory[curr_add + 3];
    t[2] = memory[curr_add + 4];
    t[3] = memory[curr_add + 5];
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
    j = 0;
    s[j++] = memory[i++];
    s[j++] = memory[i++];
    s[2] = '\0';
    sscanf(s, "%X", &tmp);
    printf("LDCH: r = [%X], x = [%d], tmp = [%c]\n", r, x, tmp);

    return tmp;
}

void put_byte(int k, int r, int x) {
    char s[3];
    int i, j;

    if (x) {
        r += reg_X;
    }

    sprintf(s, "%02X", k);
    i = (r - start_add) * 2;
    j = 0;
    memory[i++] = s[j++];;
    memory[i++] = s[j++];;
}

void put_value(int k, int r, int x) {
    char s[7];
    int i, j;

    printf("<put_value> Operation (%X, %X, %X) Starts!\n", k, r, x);

    if (x) {
        r += reg_X;
    }

    sprintf(s, "%06X", k);
    i = (r - start_add) * 2;

    for (j = 0; j < 6; j++) {
        memory[i++] = s[j];
    }

    printf("<put_value> Operation (%X, %X, %X) Finishes!\n", k, r, x);
}

void show_reg() {
    printf("Register A  = [%06X];\n", reg_A);
    printf("Register X  = [%06X];\n", reg_X);
    printf("Register L  = [%06X];\n", reg_L);
    printf("Register SW = [%06X];\n", reg_SW);
    printf("Register PC = [%06X];\n", reg_PC);
}

// System Programming Lab Assignment 2
void s_run() {
    init_run();

    while (running) {
        get_op();
        // printf("%X", operand);
        // printf("%d\n", op);

        switch (op) {
            case oMIN:
                break;
            case oADD:
                reg_A += get_value(operand, indexed);
                break;
            case oAND:
                reg_A &= get_value(operand, indexed);
                break;
            case oCOMP:
                if (reg_A > get_value(operand, indexed)) {
                    reg_SW = greaterThan;
                } else if (reg_A < get_value(operand, indexed)) {
                    reg_SW = lessThan;
                } else {
                    reg_SW = equalTo;
                }
                break;
            case oDIV:
                reg_A /= get_value(operand, indexed);
                break;
            case oJ:
                reg_PC = operand;
                curr_add = (reg_PC - start_add) * 2;
                break;
            case oJEQ:
                if(reg_SW == equalTo) {
                    reg_PC = operand;
                    curr_add = (reg_PC - start_add) * 2;
                }
                break;
            case oJGT:
                if(reg_SW == greaterThan) {
                    reg_PC = operand;
                    curr_add = (reg_PC - start_add) * 2;
                }
                break;
            case oJLT:
                if(reg_SW == lessThan) {
                    reg_PC = operand;
                    curr_add = (reg_PC - start_add) * 2;
                }
                break;
            case oJSUB:
                reg_L = reg_PC;
                reg_PC = operand;
                curr_add = (reg_PC - start_add) * 2;
                break;
            case oLDA:
                reg_A = get_value(operand, indexed);
                break;
            case oLDCH:
                reg_A = (reg_A & 16776960) | get_byte(operand, indexed);
                break;
            case oLDL:
                reg_L = get_value(operand, indexed);
                break;
            case oLDX:
                reg_X = get_value(operand, indexed);
            case oMUL:
                reg_A *= get_value(operand, indexed);
                break;
            case oOR:
                reg_A |= get_value(operand, indexed);
                break;
            case oRD:
                reg_A = reg_A;
                break;
            case oRSUB:
                if(reg_L == 0) {
                    running = 0;
                } else {
                    reg_PC = reg_L;
                    curr_add = (reg_PC - start_add) * 2;
                }
                break;
            case oSTA:
                put_value(reg_A, operand, indexed);
                break;
            case oSTCH:
                put_byte(reg_A, operand, indexed);
                break;
            case oSTL:
                put_value(reg_L, operand, indexed);
                break;
            case oSTSW:
                put_value(reg_SW, operand, indexed);
                break;
            case oSTX:
                put_value(reg_X, operand, indexed);
                break;
            case oSUB:
                reg_A -= get_value(operand, indexed);
                break;
            case oTD:
                reg_SW = greaterThan;
                break;
            case oTIX:
                reg_X += 1;
                if (reg_X > get_value(operand, indexed)) {
                    reg_SW = greaterThan;
                } else if (reg_X < get_value(operand, indexed)) {
                    reg_SW = lessThan;
                } else {
                    reg_SW = equalTo;
                }
                break;
            case oWD:
                reg_A = reg_A;
                break;
            default:
                break;
        }
    }

    show_reg();
}

int main() {
    int comm;

    comm = readline();

    while (comm != cEXIT) {
        switch (comm) {
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
