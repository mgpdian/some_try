/*
 * @Date: 2022-03-25 07:39:24
 * @LastEditors: mgpdian
 * @LastEditTime: 2022-03-26 20:43:26
 * @FilePath: /data/quanzhu/cp_server.cpp
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // 大量针对系统调用的封装
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <vector>
#include <queue>
#include <sys/time.h>
#include <sys/select.h>
#include <string>
#include <assert.h>
#define BUF_SIZE 300
#define width 3

static char first = 'x';
static char second = 'o';
// 判断胜负
bool judge(int* cur_idx, char chess_board[][3]) {
    if (cur_idx[0] < 0 || cur_idx[1] < 0)
        return false;
    int x = cur_idx[0];
    int y = cur_idx[1];
    char temp = chess_board[x][y];
    int cnt = 0;
    // 判断这一列有没有三个一样的
    for (int i = 0; i < x; i++) {
        if (chess_board[i][y] == temp)
            cnt++;
    }
    for (int i = x; i < 3; i++) {
        if (chess_board[i][y] == temp)
            cnt++;
    }
    if (cnt == 3)
        return true;
    // 判断这一行有没有三个一样的
    cnt = 0;
    for (int i = 0; i < y; i++) {
        if (chess_board[x][i] == temp)
            cnt++;
    }
    for (int i = y; i < 3; i++) {
        if (chess_board[x][i] == temp)
            cnt++;
    }
    if (cnt == 3)
        return true;
    // 如果在对角线上，判断一下对角线
    if ((x == 0 && y == 0) ||
        (x == 0 && y == 2) ||
        (x == 2 && y == 0) ||
        (x == 2 && y == 2) ||
        (x == 1 && y == 1)) {
        if (chess_board[0][0] == temp &&
            chess_board[1][1] == temp &&
            chess_board[2][2] == temp)
            return true;
        if (chess_board[0][2] == temp &&
            chess_board[1][1] == temp &&
            chess_board[2][0] == temp)
            return true;
    }
    return false;
}

int main()
{

    int listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert(listenfd != -1);
    
    //设置端口复用
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt));

    signal(SIGCHLD, SIG_IGN);

    struct sockaddr_in sockaddr;
    memset(&sockaddr, '\0', sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(atoi("9190"));
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(listenfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    int first_client = -1;
    int second_client = -1;

    struct sockaddr_in client_addr;
    memset(&client_addr, '\0', sizeof(client_addr));
    socklen_t client_len = sizeof(client_addr);

    int clientfd;

    //计数器
    int now_num = 0;
     char name[] = "you are client one\0";
     char names[] = "you are client two\0";
    while (true)
    {
        clientfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len);
        if (clientfd == -1) {
            continue;
        }
        else{
            now_num++;
            if(now_num == 1)
            {
                first_client = clientfd;
                write(first_client, name, strlen(name));
                printf("now the first client\n");
            }
            else
            {
                second_client = clientfd;
                write(second_client, names, strlen(names));
                printf("now the second client\n");
                
            }
        }

        if(now_num == 2)
        {
            printf("now_num = %d\n",now_num);
            now_num = 0;
            pid_t pid = fork();

            if(pid == 0)
            {
                printf("进入子进程");
                //关闭子进程的服务器端
                //close(listenfd);
                // 激活服务端的棋盘，客户端会自己激活本地的棋盘，服务端的棋盘没必要展示
                char chess_board[width][width];
                memset(chess_board, '?', sizeof(chess_board));

                // 与客户端开始交互
                bool flag = true;   // true 和clnt_a互动，反之则是clnt_b
                char str2[] = "now it's your turn.";
                char win[] = "you win";
                char lose[] = "you lose";
                int cur_idx[2];
                char choise[5];
                int read_len = 0;
                char sym;
                while (true)
                {
                   if(flag)
                   {
                        write(first_client, str2, strlen(str2));
                        read_len = read(first_client, choise, 5);
                        sym = first;
                   }
                   else
                   {
                       write(second_client, str2, strlen(str2));
                       read_len = read(second_client, choise, 5);
                       sym = second;
                       
                   }
                    cur_idx[0] = choise[0] - '0';
                    cur_idx[1] = choise[2] - '0';
                    printf("%c %c", choise[0],choise[2]);
                    
                    chess_board[cur_idx[0]][cur_idx[1]] = sym;
                    if (judge(cur_idx, chess_board)) {
                        if (chess_board[cur_idx[0]][cur_idx[1]] == first) {
                            write(first_client, win, strlen(win));
                            write(second_client, lose, strlen(lose));
                            break;
                        } else {
                            write(first_client, win, strlen(win));
                            write(second_client, lose, strlen(lose));
                            sleep(2);
                            break;
                        }
                    }
                    // 刷新客户端的棋盘
                    write(first_client, chess_board, 9);
                    write(second_client, chess_board, 9);
                    flag = !flag;
                    sleep(1);
                }
                close(first_client);
                close(second_client);
                break;
            }
            else{
                //关闭父进程的客户端
                close(first_client);
                close(second_client);

            }
        }
    }

    close(listenfd);
    return 0;
    
}