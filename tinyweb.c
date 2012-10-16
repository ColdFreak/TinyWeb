#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "hacking.h"
#include "hacking-network.h"

#define PORT 50000   // 用户连接端口 
#define WEBROOT "./webroot" //浏览器的默认目录 

void handle_connection(int, struct sockaddr_in *); //处理請求的函数 
int get_file_size(int); // 获得打开文件的大小 

int main(void) {
   int sockfd, new_sockfd, yes=1;
   struct sockaddr_in host_addr, client_addr;   // 
   socklen_t sin_size;

   printf("接受来自端口 %d 的請求\n", PORT);

   if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
      fatal("无法生成套接字");

   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
      fatal("設定套接字为SO_REUSEADDR ");

   host_addr.sin_family = AF_INET;      
   host_addr.sin_port = htons(PORT);   
   host_addr.sin_addr.s_addr = INADDR_ANY; // 自动設定自身ＩＰ 
   memset(&(host_addr.sin_zero), '\0', 8); // 结构体的最后８字节清０
   if (bind(sockfd, (struct sockaddr *)&host_addr, sizeof(struct sockaddr)) == -1)
      fatal("Bind失败");

   if (listen(sockfd, 20) == -1)
      fatal("Listen失败");

   while(1) {   
      sin_size = sizeof(struct sockaddr_in);
      new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
      if(new_sockfd == -1)
         fatal("新连接失败");

      handle_connection(new_sockfd, &client_addr);
   }
   return 0;
}

void handle_connection(int sockfd, struct sockaddr_in *client_addr_ptr) {
   unsigned char *ptr, request[500], resource[500];
   int fd, length;
   
   length = recv_line(sockfd, request);
   
   printf("来自%s：%d　的請求  \"%s\"\n", inet_ntoa(client_addr_ptr->sin_addr), ntohs(client_addr_ptr->sin_port), request);

   ptr = strstr(request, " HTTP/"); // 注意第一个字符是空格 
   if(ptr == NULL) { // 找不到HTTP就说明不是一个合法的請求 
      printf(" 非HTTP！\n");
   } else {
      *ptr = 0; //找到了HTTP，" HTTP/“　就结束使命了，在第一个空格处加'\0' 
      ptr = NULL; // 接下来找"GET "或是" HEAD "，若没有找到，ptr设为NULL就有用了 
      if(strncmp(request, "GET ", 4) == 0)  // GET請求
         ptr = request+4; // 請求会是类似于 GET /image.jpg HTTP/1.1"这种形式，就可以理解为什么加４了
      if(strncmp(request, "HEAD ", 5) == 0) // 同理 
         ptr = request+5; // 同理

      if(ptr == NULL) { // 找不到"GET " 或是"HEAD "的任何一个
         printf("\t未知請求！\n");
      } else { // 找到"GET "或"HEAD "中任何一个，就开始处理后续的 比如"GET /image.jpg" 的"/image.jpg"部分
         if (ptr[strlen(ptr) - 1] == '/')  // 若只有一个"/"，则默认返回index.html 
            strcat(ptr, "index.html");     
         strcpy(resource, WEBROOT);     // 开始构建一个完整的路径 
         strcat(resource, ptr);         
         fd = open(resource, O_RDONLY, 0); //  试着打开这个文件
         printf("\t打开\'%s\' \t", resource);
         if(fd == -1) { // 没有找到文件 
            printf(" 404 Not Found\n");
            send_string(sockfd, "HTTP/1.0 404 NOT FOUND\r\n");
            send_string(sockfd, "Server: Tiny webserver\r\n\r\n");
            send_string(sockfd, "<html><head><title>404 Not Found</title></head>");
            send_string(sockfd, "<body><h1>URL not found</h1></body></html>\r\n");
         } else {      // 找到文件则发送， 
            printf(" 200 OK\n");
            send_string(sockfd, "HTTP/1.0 200 OK\r\n");
            send_string(sockfd, "Server: Tiny webserver\r\n\r\n");
            if(ptr == request + 4) { // 上面ptr 已经被設定好，现在看ptr如果是GET 命令的话 
               if( (length = get_file_size(fd)) == -1)
                  fatal("取得文件大小失败。");
               if( (ptr = (unsigned char *) malloc(length)) == NULL)
                  fatal("内存的分配失败。");
               read(fd, ptr, length); // 将文件读入内存
               send(sockfd, ptr, length, 0);  // 发送 
               free(ptr); // 释放存储文件的内存 
            }
            close(fd); // 关闭文件 
         } 
      } 
   } 
   shutdown(sockfd, SHUT_RDWR); 
}

int get_file_size(int fd) {
   struct stat stat_struct;

   if(fstat(fd, &stat_struct) == -1)
      return -1;
   return (int) stat_struct.st_size;
}
