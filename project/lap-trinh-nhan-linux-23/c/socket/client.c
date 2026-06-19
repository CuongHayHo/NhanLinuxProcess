#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

// Receives a message from a client.
void* recvMsg(void* sock)
{
	int their_sock = *((int*)sock);
	char msg[500];
	int len;

	while ((len = recv(their_sock, msg, 500, 0)) > 0) 
	{
		msg[len] = '\0';
		fputs(msg, stdout);
		memset(msg, '\0', sizeof(msg));
	}
}

int main(int argc, char* argv[])
{
	struct sockaddr_in their_addr; // Cấu trúc chứa địa chỉ của máy chủ mà client sẽ kết nối đến
	pthread_t sendt, recvt; // Hai luồng: một để gửi tin nhắn và một để nhận tin nhắn
	int my_sock; // Socket của client
	int their_sock; // Socket của máy chủ
	int their_addr_size; // Kích thước của cấu trúc địa chỉ máy chủ
	int portno; // Số cổng mà máy chủ đang lắng nghe
	int len; // Độ dài của tin nhắn
	char msg[500]; // Mảng chứa tin nhắn
	char username[100]; // Mảng chứa tên người dùng
	char res[600]; // Mảng chứa kết quả (có thể là tin nhắn đã được định dạng hoặc một loại phản hồi khác)
	char ip[INET_ADDRSTRLEN]; // Mảng chứa địa chỉ IP
	//chỉ định kích thước tối đa của một chuỗi có thể chứa một địa chỉ IPv4 dưới dạng chuỗi.

	if (argc > 3) 
	{
		printf("too many arguments...");
		exit(1);
	}

	portno = atoi(argv[2]);
	strcpy(username, argv[1]);
	my_sock = socket(AF_INET, SOCK_STREAM, 0);												// Creates client socket. 
	if (my_sock < 0)																		// Validates the socket.
	{
		perror("opening channel unsuccessful...");
		exit(1);
	}

	memset(their_addr.sin_zero, '\0', sizeof(their_addr.sin_zero)); 
	// Đặt tất cả các byte trong phần sin_zero của cấu trúc their_addr thành 0.
	// Điều này thường được sử dụng để đảm bảo rằng không có dữ liệu rác nào trong cấu trúc.
	their_addr.sin_family = AF_INET;														// Sets IPv4.
	their_addr.sin_port = htons(portno);	
	//ip addr show để lấy ip wsl												// Sets port number.
	their_addr.sin_addr.s_addr = inet_addr("192.168.0.198");									// Sets IP address.

	if (connect(my_sock, (struct sockaddr*) & their_addr, sizeof(their_addr)) < 0)			// Connects to server and Validates the connection.
	{
		perror("connection not esatablished...");
		exit(1);
	}

	inet_ntop(AF_INET, (struct sockaddr*) & their_addr, ip, INET_ADDRSTRLEN);
	//chuyển đổi một địa chỉ mạng thành chuỗi 
	printf("connected to %s, start chatting\n", ip);
	pthread_create(&recvt, NULL, recvMsg, &my_sock); //tạo một luồng mới trong chương trình. 
	
	while (fgets(msg, 500, stdin) > 0) 
	{
		strcpy(res, username);
		strcat(res, ": ");
		strcat(res, msg);
	
		len = write(my_sock, res, strlen(res)); //gửi tin nhắn đến máy chủ
		
		if (len < 0) 
		{
			perror("message not sent...");
			exit(1);
		}

		memset(msg, '\0', sizeof(msg)); //xóa nội dung sau khi gửi
		memset(res, '\0', sizeof(res));
	}

	pthread_join(recvt, NULL); //hờ cho đến khi luồng có ID recvt kết thúc
	close(my_sock);
}