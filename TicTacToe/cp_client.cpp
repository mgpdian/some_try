/*
 * @Date: 2022-03-25 07:39:15
 * @LastEditors: mgpdian
 * @LastEditTime: 2022-03-26 20:41:28
 * @FilePath: /data/quanzhu/cp_client.cpp
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // 大量针对系统调用的封装
#include <arpa/inet.h>
#include <sys/socket.h>
#include <assert.h>


#define BUF_SIZE 100
#define width 3

void error_handling(std::string message) {
    std::cout << message << std::endl;
    exit(1);
}

// 展示棋盘
void show_chess_board(char chess_board[][3]) {
    for (int i = 0; i < width; i++) {
        std::cout << "=====================" <<std::endl;
        for (int j = 0; j < width; j++) {
            std::cout << chess_board[i][j] << "\t";
        }
        std::cout << std::endl;
    }
    std::cout << "=====================" <<std::endl;
}

int main(int argc, char* argv[])
{
    int sock;
    char message[BUF_SIZE];
    char chess_board[width][width];
    memset(chess_board, '?', sizeof(chess_board));

    // if (argc != 3) {
    //     printf("Usage : %s <IP> <port>\n", argv[0]);
    //     exit(1);
    // }
    ssize_t str_len, recv_len, recv_cnt;
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    assert(sock != -1);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, '\0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi("9190"));
    serv_addr.sin_addr.s_addr = inet_addr("192.168.200.130");

    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("connect error");
    } else {
        printf("Connected......\n");
    }
    // 接受欢迎
    std::cout << "welcome to paly" << std::endl;
    // 展示棋盘
    show_chess_board(chess_board);
    
    // 接收消息，进行对战
    //char choice[5];
    int x = -1, y = -1;
    while (true)
    {
        recv_len = read(sock, message, BUF_SIZE -1);

        if(recv_len < 1)
        {
            break;
        }
        if(strcmp(message, "now it's your turn.") == 0)
        {
            memset(message, 0, BUF_SIZE);
            while(true)
            {
                char choice[5];
                printf("input the x and y of your choice(etc. 0 2) : ");
                scanf("%d %d", &x,&y);
                if (x < 0 || x > 2 || y < 0 || y > 2 || chess_board[x][y] != '?') {
                    printf("wrong choice, try again.\n");
                    sleep(1);
                    continue;
                }
                choice[0] = x + '0';
                choice[2] = y + '0';
                write(sock, choice, 4);
                break;
            }

        }
        else if(strcmp(message, "you win") == 0)
        {
            printf("你赢了!!!");
            break;
        }
        else if(strcmp(message, "you less" ) == 0)
        {
            printf("你输了");
            break;
        }
        else if(recv_len == 9){
            memcpy(chess_board, message, 9);
            show_chess_board(chess_board);
            printf("wait another player...\n");
            printf("+++++++++++++++++++++\n");
            memset(message, 0, BUF_SIZE);
        }
        else{
            printf("%s\n", message);
            memset(message, 0, BUF_SIZE);
        }
        //sleep(1);

    }   
    close(sock);
    return 0;
    
    

}