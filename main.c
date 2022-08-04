#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <threads.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>

#define BLOCK (*(gameState.pc)) 
#define W_PRESSED gameState.keys & 1
#define A_PRESSED gameState.keys & (1 << 1)
#define S_PRESSED gameState.keys & (1 << 2)
#define D_PRESSED gameState.keys & (1 << 3)
enum block_t {L, I, T, S, Z, O, J};

int L_block[4][2] = {
    {0,1},
    {1,1},
    {2,1},
    {2,2}
};


int J_block[4][2] = {
    {0,1},
    {1,1},
    {2,0},
    {2,1}
};


int S_block[4][2] = {
    {0,1},
    {0,2},
    {1,0},
    {1,1}
};


int Z_block[4][2] = {
    {0,0},
    {0,1},
    {1,1},
    {1,2}
};

int I_block[4][2] = {
    {1,0},
    {1,1},
    {1,2},
    {1,3}
};


int T_block[4][2] = {
    {0,1},
    {1,0},
    {1,1},
    {1,2}
};


int O_block[4][2] = {
    {0,0},
    {0,1},
    {1,0},
    {1,1}
};

struct game {
    char board[20][10];
    int (* pc)[4][2]; 
    enum block_t block_type; 
    int x; 
    int y; 
    unsigned int keys; 
    float drop_timer;
    int no_block; 
};

struct game gameState;
struct game prevGameState;
struct termios defaultAttributes;
struct termios inputAttributes; 


void reset_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &defaultAttributes);
}


void getKeys() {
    char ch = 0; 
    gameState.keys = 0;
        
    inputAttributes.c_lflag &= ~ICANON; 
    
    tcsetattr(STDIN_FILENO, TCSANOW, &inputAttributes);
    read(STDIN_FILENO, &ch, 1);
    //printf("%c", ch);
    switch(ch) {
    case 'w': 
        //printf("W");
        gameState.keys |= 1; 
        break; 
    case 'a':
        gameState.keys |= (1 << 1); 
        break; 
    case 's':
        gameState.keys |= (1 << 2); 
        break;
    case 'd':
        gameState.keys |= (1 << 3); 
        break;
    case 'q':
        reset_terminal();
        exit(0);
        break;
    default:
        gameState.keys = 0; 
        break;
    }
    while(read(STDIN_FILENO, &ch, 1));
    //tcsetattr(STDIN_FILENO, TCSANOW, &defaultAttributes);
    inputAttributes.c_lflag |= ICANON; 
    tcsetattr(STDIN_FILENO, TCSANOW, &inputAttributes);
}

void sig_handler(int sig_num) {
    reset_terminal();
    exit(0);
}

void init() {
    signal(SIGINT, sig_handler);
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            gameState.board[i][j] = ' ';
        }
    } 
     
    //init piece board
    gameState.drop_timer = 0; 
    gameState.no_block = 1; 
    
    tcgetattr(STDIN_FILENO, &defaultAttributes);
    tcgetattr(STDIN_FILENO, &inputAttributes);
    
    inputAttributes.c_lflag &= ~ECHO; 
    inputAttributes.c_cc[VTIME] = 1; 
    inputAttributes.c_cc[VMIN] = 0;
    
    tcsetattr(STDIN_FILENO, TCSANOW, &inputAttributes);
    srand(time(NULL)); 
} 

void rotate(int left) {
    if (gameState.block_type == O) return; 
    int temp_block[4][2];
    memcpy(temp_block, BLOCK, sizeof(int) * 8);
    
    if (left) {
        for (int point = 0; point < 4; point++) {
            int x = temp_block[point][0]; 
            temp_block[point][0] = temp_block[point][1]; 
            temp_block[point][1] = x; 
            
            temp_block[point][0] -= 1;
            temp_block[point][0] *= -1;
            temp_block[point][0] += 1; 

            if (gameState.board[gameState.y + temp_block[point][0]][gameState.x + temp_block[point][1]] == 'o' ||
                gameState.y + temp_block[point][0] >= 20 || gameState.x + temp_block[point][1] < 0 || gameState.x + temp_block[point][1] >= 10) return;
        }
    } else {
        for (int point = 0; point < 4; point++) {
            int x = temp_block[point][0]; 
            temp_block[point][0] = temp_block[point][1]; 
            temp_block[point][1] = x; 
            
            temp_block[point][1] -= 1;
            temp_block[point][1] *= -1;
            temp_block[point][1] += 1; 

            if (gameState.board[gameState.y + temp_block[point][0]][gameState.x + temp_block[point][1]] == 'o' ||
                gameState.y + temp_block[point][0] >= 20 || gameState.x + temp_block[point][1] < 0 || gameState.x + temp_block[point][1] >= 10) return;
        }
    }

    memcpy(BLOCK, temp_block, 8 * sizeof(int));
}

//L, I, T, S, Z, O, J
void initBlock() {
    gameState.block_type = rand()%7;
    printf("Block type: %d\n", gameState.block_type);
    switch (gameState.block_type) {
        case L: 
        gameState.pc = &L_block;
        break; 

        case I:
        gameState.pc = &I_block;
        break; 

        case T:
        gameState.pc = &T_block;
        break;
        
        case S: 
        gameState.pc = &S_block;
        break;

        case Z: 
        gameState.pc = &Z_block;
        break; 

        case O: 
        gameState.pc = &O_block;
        break;

        case J:
        gameState.pc = &J_block;
        break; 
    }
    gameState.x = 3;
    gameState.y = 2;
    gameState.no_block = 0; 
    gameState.drop_timer = 1;
}

void drawBoard() {
    for (int i = 0; i < 4; i++) {
        gameState.board[gameState.y + BLOCK[i][0]][gameState.x + BLOCK[i][1]] = 'o';
    }
    for (int i = 0; i < 20; i++) {
        printf("x");
        for (int j = 0; j < 10; j++) {
            printf("%c", gameState.board[i][j]);
        }
        printf("x\n");
    }
    for (int j = 0; j < 12; j++) printf("x");
    printf("\n");
    if (!gameState.no_block) {
    for (int i = 0; i < 4; i++) {
        gameState.board[gameState.y + BLOCK[i][0]][gameState.x + BLOCK[i][1]] = ' ';
    }
    }
}

void rowClear() {
    for (int i = 19; i >= 0; i--) {
        char boolean = 1; 
        for (int j = 0; j < 10; j++) {
            if (gameState.board[i][j] == ' ') {
                boolean = 0; 
                break;
            }  
        }
        if (!boolean) continue;
        //char tempBoard[20][10];
        memcpy(gameState.board[1], gameState.board, 10 * i * sizeof(char));
        memset(gameState.board[0], ' ', 10 * sizeof(char));
    }
}

void drop() {
    for (int i = 0; i < 4; i++) {
        if (gameState.board[gameState.y + BLOCK[i][0] + 1][gameState.x + BLOCK[i][1]] == 'o' ||
            gameState.y+BLOCK[i][0] + 1 >= 20) {
            printf("collide\n");
            gameState.no_block = 1; 
            return;
        } 
    }
    if (gameState.drop_timer > 0) {
        gameState.drop_timer--;
    } else {
        gameState.y += 1; 
        gameState.drop_timer = 1; 
    }
}

void horiControl(int left) {
    if (left) {
        for (int i = 0; i < 4; i++) {
            if (gameState.board[gameState.y + BLOCK[i][0]][gameState.x + BLOCK[i][1] - 1] == 'o' ||
                gameState.x+BLOCK[i][1] - 1 < 0) {
                //printf("collide\n");
                //sleep(1);
                return;
            } 
        }
        gameState.x -= 1;
    } else {
        for (int i = 0; i < 4; i++) {
            if (gameState.board[gameState.y + BLOCK[i][0]][gameState.x + BLOCK[i][1] + 1] == 'o' ||
                gameState.x+BLOCK[i][1] + 1 >= 10) {
                //printf("collide\n");
                //sleep(1);
                return;
            } 
        }
        gameState.x += 1;
    }
}


void control() {
    //printf("%d", gameState.keys);
    if (W_PRESSED || S_PRESSED) {
        rotate(W_PRESSED);
    }

    if (A_PRESSED || D_PRESSED) {
        horiControl(A_PRESSED);
        //printf("Pressed");
    }
}

int main() {
    init();
    //thrd_sleep(&(struct timespec){.tv_sec = 1.5}, NULL);
    //int game_loop = 100;
    while (1) {
        
        prevGameState = gameState; 
        if (gameState.no_block) initBlock();
        getKeys();
        control();
        drop();
        rowClear();
        system("clear");
        drawBoard();
        //printf("Droptimer: %f\n", gameState.drop_timer);
        //sleep(1);
        thrd_sleep(&(struct timespec){.tv_sec = .85}, NULL);
    }
    reset_terminal();

    printf("\n");
    return 0; 
}