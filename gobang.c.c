/* ============================================================
 * 五子棋（控制台双人对战版）
 * 适合在 Dev C++ 中直接编译运行（无需安装任何图形库）
 *
 * 棋盘全部使用半角 ASCII 字符绘制，每个交叉点严格占 2 列，
 * 与上方数字列号宽度完全一致，任何控制台字体/代码页下都不错位：
 *
 *       1 2 3 4 5 6 7 8 9101112131415
 *    1  + + + + + + + + + + + + + + +
 *    2  + + + + + + + + + + + + + + +
 *   ...
 *
 * 操作方法：
 *   方向键 ...... 移动光标（白底黑字高亮）
 *   回车/空格 ... 在光标处落子（黑棋 @ 先手）
 *   U 键 ........ 悔棋
 *   R 键 ........ 重新开始
 *   ESC 键 ...... 退出游戏
 *
 * 黑棋(@)先手，横、竖、任意一条斜线先连成五子者获胜
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>

#define SIZE   15    /* 棋盘路数：15 x 15 */
#define EMPTY  0
#define BLACK  1
#define WHITE  2

int board[SIZE][SIZE];                      /* 棋盘：0空 1黑 2白 */
int curX, curY;                             /* 光标所在的列、行（0到14） */
int current;                                /* 当前轮到谁落子 */
int histX[SIZE * SIZE], histY[SIZE * SIZE]; /* 落子历史，用于悔棋 */
int moves;                                  /* 已落子总数 */
int gameOver;                               /* 游戏是否结束 */
int winner;                                 /* 获胜方：0表示平局或未分胜负 */

HANDLE hOut;                                /* 控制台输出句柄 */

/* 把光标移动到控制台的 (x, y) 坐标，用于无闪烁刷新画面 */
void gotoXY(int x, int y)
{
    COORD pos;
    pos.X = (SHORT)x;
    pos.Y = (SHORT)y;
    SetConsoleCursorPosition(hOut, pos);
}

/* 隐藏控制台闪烁的下划线光标 */
void hideCursor(void)
{
    CONSOLE_CURSOR_INFO cci;
    cci.dwSize = 1;
    cci.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cci);
}

/* 初始化 / 重置整局游戏 */
void initGame(void)
{
    int i, j;
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            board[i][j] = EMPTY;

    curX = SIZE / 2;
    curY = SIZE / 2;
    current = BLACK;
    moves = 0;
    gameOver = 0;
    winner = 0;
}

/* 判断在 (row, col) 落下 who 颜色的棋子后是否五连获胜 */
int checkWin(int row, int col, int who)
{
    /* 四个方向：横向、纵向、主斜线、副斜线 */
    int dirs[4][2] = { {0, 1}, {1, 0}, {1, 1}, {1, -1} };
    int d;

    for (d = 0; d < 4; d++) {
        int count = 1;
        int dr = dirs[d][0];
        int dc = dirs[d][1];
        int r = row + dr;
        int c = col + dc;

        /* 顺着方向数同色棋子 */
        while (r >= 0 && r < SIZE && c >= 0 && c < SIZE && board[r][c] == who) {
            count++;
            r += dr;
            c += dc;
        }
        /* 反方向再数 */
        r = row - dr;
        c = col - dc;
        while (r >= 0 && r < SIZE && c >= 0 && c < SIZE && board[r][c] == who) {
            count++;
            r -= dr;
            c -= dc;
        }
        if (count >= 5)
            return 1;
    }
    return 0;
}

/* 在当前光标处落子 */
void placeStone(void)
{
    if (gameOver)
        return;
    if (board[curY][curX] != EMPTY)
        return;                       /* 该位置已经有棋子 */

    board[curY][curX] = current;
    histX[moves] = curX;
    histY[moves] = curY;
    moves++;

    if (checkWin(curY, curX, current)) {
        gameOver = 1;
        winner = current;
    } else if (moves >= SIZE * SIZE) {
        gameOver = 1;                 /* 棋盘下满，平局 */
        winner = 0;
    } else {
        current = (current == BLACK) ? WHITE : BLACK;
    }
}

/* 悔一步棋 */
void undo(void)
{
    if (moves <= 0)
        return;

    moves--;
    board[histY[moves]][histX[moves]] = EMPTY;
    curX = histX[moves];
    curY = histY[moves];

    /* 如果是在分出胜负/平局后悔棋，让对局继续进行 */
    gameOver = 0;
    winner = 0;
    current = (current == BLACK) ? WHITE : BLACK;
}

/* 打印棋盘上的一个交叉点：固定输出“字符+空格”两个半角列 */
void drawCell(int row, int col)
{
    int isCur = (row == curY && col == curX);
    char ch;

    if (board[row][col] == BLACK)
        ch = '@';                     /* 黑棋 */
    else if (board[row][col] == WHITE)
        ch = 'O';                     /* 白棋 */
    else
        ch = '+';                     /* 空交叉点 */

    if (isCur) {
        /* 光标格：白底黑字反色高亮，醒目且不改变占位宽度 */
        SetConsoleTextAttribute(hOut,
            BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE
            | BACKGROUND_INTENSITY);
    } else if (board[row][col] == BLACK) {
        SetConsoleTextAttribute(hOut,
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
            | FOREGROUND_INTENSITY);  /* 黑棋：亮白色 */
    } else if (board[row][col] == WHITE) {
        SetConsoleTextAttribute(hOut,
            FOREGROUND_GREEN | FOREGROUND_BLUE
            | FOREGROUND_INTENSITY);  /* 白棋：亮青色 */
    } else {
        SetConsoleTextAttribute(hOut,
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); /* 空点：灰色 */
    }

    printf("%c ", ch);                /* 每格严格 2 个半角字符 */

    SetConsoleTextAttribute(hOut,
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

/* 打印棋盘下方的状态信息 */
void drawStatus(void)
{
    gotoXY(0, SIZE + 2);
    if (gameOver) {
        if (winner == BLACK)
            printf("  黑棋获胜！ 按 R 重新开始，按 ESC 退出。          ");
        else if (winner == WHITE)
            printf("  白棋获胜！ 按 R 重新开始，按 ESC 退出。          ");
        else
            printf("  双方平局！ 按 R 重新开始，按 ESC 退出。          ");
    } else {
        printf("  当前轮到：%s   已落子：%d 步                     ",
               current == BLACK ? "黑棋(@)" : "白棋(O)", moves);
    }

    gotoXY(0, SIZE + 3);
    printf("  方向键移动  回车/空格落子  U 悔棋  R 重新开始  ESC 退出   ");
}

/* 重画整个游戏画面 */
void drawBoard(void)
{
    int row, col;

    gotoXY(0, 0);
    SetConsoleTextAttribute(hOut,
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    printf("                          五 子 棋\n");

    /* 列号行：前缀 4 个空格与左侧行号对齐；
       1~9 输出“数字+空格”，10~15 输出两位数，
       每个列号严格占 2 列，数字首位正对下方的交叉点 */
    printf("    ");
    for (col = 0; col < SIZE; col++) {
        if (col < 9)
            printf("%d ", col + 1);
        else
            printf("%d", col + 1);
    }
    printf("\n");

    /* 每一行：行号(2列) + 两个空格 + 15 个交叉点(各2列) */
    for (row = 0; row < SIZE; row++) {
        printf("%2d  ", row + 1);
        for (col = 0; col < SIZE; col++)
            drawCell(row, col);
        printf("\n");
    }

    drawStatus();
}

#ifdef TEST_ALIGN
/* 编译对齐自检版本时使用：画一盘含棋子的棋盘后直接退出 */
int main(void)
{
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    initGame();
    board[3][3] = BLACK;
    board[4][4] = WHITE;
    board[10][9] = BLACK;
    curX = 5;
    curY = 5;
    drawBoard();
    return 0;
}
#else

int main(void)
{
    int running = 1;
    int key;

    /* 强制控制台按 GBK 代码页输出，保证中文提示正常显示 */
    SetConsoleOutputCP(936);

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    hideCursor();
    initGame();

    system("cls");
    drawBoard();

    while (running) {
        key = getch();               /* 读取按键，不用回车，输入不回显 */

        if (key == 0 || key == 0xE0) {
            /* 方向键等扩展按键会先返回 0 或 0xE0，再返回扫描码 */
            key = getch();
            switch (key) {
            case 72:                 /* 上 */
                if (curY > 0) curY--;
                break;
            case 80:                 /* 下 */
                if (curY < SIZE - 1) curY++;
                break;
            case 75:                 /* 左 */
                if (curX > 0) curX--;
                break;
            case 77:                 /* 右 */
                if (curX < SIZE - 1) curX++;
                break;
            default:
                break;
            }
        } else {
            switch (key) {
            case 13:                 /* 回车 */
            case ' ':                /* 空格 */
                placeStone();
                break;
            case 'u':
            case 'U':
                undo();
                break;
            case 'r':
            case 'R':
                initGame();
                system("cls");       /* 重开时彻底清屏一次 */
                break;
            case 27:                 /* ESC */
                running = 0;
                break;
            default:
                break;
            }
        }

        if (running)
            drawBoard();
    }

    /* 退出前恢复默认颜色并清屏 */
    SetConsoleTextAttribute(hOut,
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    system("cls");
    printf("感谢使用五子棋，再见！\n");
    return 0;
}

#endif
