#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

struct client_info {
	int sockno;
	char ip[INET_ADDRSTRLEN]; 
};

int clients[100];
int n = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//ngăn chặn việc truy cập đồng thời vào một phần mã hoặc dữ liệu chung từ nhiều thread, giúp tránh xung đột và lỗi trong quá trình thực thi chương trình đa luồng.

// Sends the message to all clients.
void sendToAll(char* msg, int curr)
{
	int i;
	pthread_mutex_lock(&mutex);
	//đảm bảo chỉ có một thread có thể truy cập vào một phần mã hoặc dữ liệu chung tại một thời điểm, giúp tránh xung đột và lỗi.
	
	for (i = 0; i < n; i++) 
	{
		if (clients[i] != curr) //không gửi cho người gửi tin nhắn đến
		{
			if (send(clients[i], msg, strlen(msg), 0) < 0) //gửi tin nhắn
			{
				perror("sending failure...");
				continue;
			}
		}
	}

	pthread_mutex_unlock(&mutex);
}

// Receives a message from a client.
void* recvMsg(void* sock)
{
	struct client_info cl = *((struct client_info*)sock);
	char msg[500];
	int len;
	int i;
	int j;
	
	while ((len = recv(cl.sockno, msg, 500, 0)) > 0) 
	{
		msg[len] = '\0'; //thêm 1 kí tự null vào cuối tin nhắn
		sendToAll(msg, cl.sockno);
		memset(msg, '\0', sizeof(msg)); //xóa bộ đệm tin nhắn
	}
	
	pthread_mutex_lock(&mutex);

	printf("%s disconnected...\n", cl.ip);
	
	for (i = 0; i < n; i++) 
	{
		if (clients[i] == cl.sockno) 
		{
			j = i;
		
			while (j < (n - 1)) 
			{
				clients[j] = clients[j + 1];
				j++;
			}
		}
	}

	n--;
	pthread_mutex_unlock(&mutex);
}

int main(int argc, char* argv[])
{
	struct sockaddr_in my_addr, their_addr; //lưu trữ các địa chỉ mạng trong dạng có thể đọc được
	struct client_info cl;
	socklen_t their_addr_size; //lưu trữ kích thước của một đối tượng struct sockaddr_in
	pthread_t sendt, recvt; //lưu trữ ID của hai thread khác nhau, một thread để gửi tin nhắn và một thread để nhận tin nhắn.
	int my_sock;
	int their_sock;
	int portno;
	int len;
	char msg[500];
	char ip[INET_ADDRSTRLEN]; 
	//INET_ADDRSTRLEN là một hằng số được định nghĩa trong thư viện <netinet/in.h>, 
	//để chỉ định kích thước tối đa của một chuỗi có thể chứa một địa chỉ IPv4 dưới dạng chuỗi.



	if (argc > 2) 
	{
		perror("too many arguments...");
		exit(1);
	}
	
	portno = atoi(argv[1]);
	my_sock = socket(AF_INET, SOCK_STREAM, 0);														// Creates socket.
	if (my_sock < 0)																				// Validate the socket.
	{
		perror("opening channel unsuccessful...");
		exit(1);
	}

	memset(my_addr.sin_zero, '\0', sizeof(my_addr.sin_zero)); 
	//sử dụng để đặt tất cả các byte trong mảng my_addr.sin_zero thành giá trị '\0' (tức là 0),
	//sizeof được sử dụng để chỉ định số byte cần đặt giá trị trong hàm memset.
	my_addr.sin_family = AF_INET;																	// Sets IPv4.
	my_addr.sin_port = htons(portno);																// Sets port number.
	my_addr.sin_addr.s_addr = INADDR_ANY; // chỉ định server có thể chấp nhận kết nối từ bất kỳ địa chỉ IP nào.
	their_addr_size = sizeof(their_addr);

	if (bind(my_sock, (struct sockaddr*) & my_addr, sizeof(my_addr)) != 0)							// Binds the socket with the specific IP + port.
	{
		perror("binding unsuccessful...");
		exit(1);
	}

	if (listen(my_sock, 5) != 0) 
	// Tham số thứ hai 5 là độ dài tối đa mà hàng đợi của các kết nối đang chờ cho my_sock có thể tăng lên.
	{
		perror("listening unsuccessful...");
		exit(1);
	}

	while (1) 
	{
		if ((their_sock = accept(my_sock, (struct sockaddr*) & their_addr, &their_addr_size)) < 0)	// Accepts a connection request - blocking call.
		//accept-một hàm chấp nhận một kết nối đến trên một socket thụ động my_sock. 
		//Hàm này trả về một file descriptor mới (their_sock) đại diện cho socket của kết nối đến.
		{
			perror("accept unsuccessful...");
			exit(1);
		}
	
		pthread_mutex_lock(&mutex); 
		//Hàm pthread_mutex_lock sẽ cố gắng khóa mutex này. Nếu mutex đã bị khóa bởi một thread khác, hàm sẽ bị chặn cho đến khi mutex được mở khóa.
		inet_ntop(AF_INET, (struct sockaddr*) & their_addr, ip, INET_ADDRSTRLEN);
		//chuyển đổi địa chỉ IP của kết nối đến từ dạng nhị phân sang dạng chuỗi và lưu vào mảng ip.
		
		printf("%s connected...\n", ip);

		cl.sockno = their_sock;
		strcpy(cl.ip, ip);
		clients[n] = their_sock;
		n++;
		
		pthread_create(&recvt, NULL, recvMsg, &cl); //tạo một thread mới
		pthread_mutex_unlock(&mutex); //mở khóa đối tượng mutex được tham chiếu bởi mutex
	}

	return 0;
}