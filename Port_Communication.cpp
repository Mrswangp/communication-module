#include <iostream>
#define HVG_SERIAL_IMPLEMENTATION
#include"hvg_serial.hxx"
int main()
{  //init 
	mod::hvg::control::hvg_serial_t* p = (mod::hvg::control::hvg_serial_t*)malloc(sizeof(mod::hvg::control::hvg_serial_t));
	int port = 2;
	int bdrate = 9600;
	char mode[] = { '8','N','2',0 };
	bool ret = mod::hvg::control::init(p, port, bdrate, mode);
	if (ret == false) {
		printf("init failed!\n");
		return -1;
	}
	printf("init successfully!\n");
	//open port
	ret = mod::hvg::control::open(p);
	if (ret == false) {
		printf("open port failed\n");
		return -1;
	}
	printf("open port successfully\n");
	printf("please input your command that you want to send:\n");
	char send_buff[50] = {};
	char recv_buff[1024] = {};
	double timeout;
	scanf("%s", send_buff);
	while (getchar() != '\n');
	int res = mod::hvg::control::send(p, send_buff, 50);
	printf("actually send_byte=%d\n", res);
	printf("recv....\n");
	printf("please input receive time limit seconds:\n");
	scanf("%lf", &timeout);
	mod::hvg::control::get_line(p, recv_buff, 1024, timeout);
	printf("complete command is%s\n", recv_buff);
	//memset(recv_buff, 0x00, 1024);
	//mod::hvg::control::get_line(p, recv_buff, 1024, timeout);
	//printf("complete command is%s\n", recv_buff);
	ret = mod::hvg::control::close(p);
	if (ret == false) {
		printf("close port failed\n");
		return -1;
	}
	printf("close port successfully\n");
	free(p);
	return 0;
}
