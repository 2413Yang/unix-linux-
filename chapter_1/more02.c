#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/stat.h>

typedef struct termios Termios;

#define PAGELEN 24
#define LINELEN 512
void do_more(FILE *);
int see_more(FILE *);
Termios set_raw_mode(FILE *);
void restore_terminal(FILE *, Termios);
void getfilestat(const char *, struct stat *);

int main(int ac, char *av[]){
    FILE * fp;
    if (ac == 1)
        do_more(stdin);
    else
        while(--ac)
        if ((fp = fopen(*++av,"r")) != NULL){
            do_more(fp);
            close(fp);
        }else
            exit(1);

    return 0;
}

void do_more(FILE * fp){
    char line[LINELEN];
    int num_of_lines = 0;
    int see_more(FILE *), reply;
    FILE * fp_tty;
    fp_tty = fopen("/dev/tty", "r");
    if (fp_tty == NULL)
        exit(1);
    Termios tty = set_raw_mode(fp_tty);
    while( fgets(line, LINELEN, fp) ){
        if (num_of_lines == PAGELEN){
            reply = see_more(fp_tty);
            if (reply == 0)
                break;
            num_of_lines -= reply;
        }
        if ( fputs(line,stdout) == EOF){
            printf("EOF\n");
            restore_terminal(fp_tty,tty);
            exit(1);
        }
        num_of_lines++;
    }
    restore_terminal(fp_tty,tty);
}

int see_more(FILE * cmd){
    int c;
    printf("\033[7m more? \033[m");
    while( (c = getc(cmd)) != EOF ){
        if (c == 'q')
            return 0;
        if (c == ' ')
            return PAGELEN;
        if (c == '\n')
            return 1;
    }
    return 0;
}

Termios set_raw_mode(FILE * fp){
    struct termios tty;
    // 获取当前终端属性
    if (tcgetattr(fileno(fp),&tty) != 0) {
        perror("tcgetattr");
        exit(1);
    }
    //保存原始终端属性
    Termios original_tty = tty;
    //设置为原始模式
    tty.c_lflag &= ~(ICANON | ECHO); //关闭规范模式和回显
    tty.c_cc[VMIN] = 1;              //最小字符数
    tty.c_cc[VTIME] = 0;             // 超时为 0
    // 应用新的终端属性
    if (tcsetattr(fileno(fp), TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        exit(1);
    }
    return original_tty;
}

void restore_terminal(FILE *fp, Termios original_tty){
    tcsetattr(fileno(fp), TCSANOW, &original_tty);
}

void getfilestat(const char *file, struct stat *fileStat){
    if (stat(file, fileStat) < 0){
        perror("stat");
        exit(1);
    }
}