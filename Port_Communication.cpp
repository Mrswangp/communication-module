#include <iostream>
#include <thread>
#include<string.h>
#include<stdio.h>
#define HVG_SERIAL_IMPLEMENTATION
#include"hvg_serial.hxx"
#include"spdlog/spdlog.h"
int main()
{  //init 
	mod::hvg::control::hvg_serial_t* p = (mod::hvg::control::hvg_serial_t*)malloc(sizeof(mod::hvg::control::hvg_serial_t));
	int port = 2;
	int bdrate = 19200;
	char mode[] = { '8','N','2',0 };
	bool ret = mod::hvg::control::init(p, port, bdrate, mode);
	if (ret == false) {
		spdlog::error("init failed!\n");
		return -1;
	}
	//spdlog::set_level(spdlog::level::debug); // Set global log level to debug
	spdlog::debug("init successfully!");
	//open port
	ret = mod::hvg::control::open(p);
	if (ret == false) {
		spdlog::error("open port failed\n");
		return -1;
	}
	//spdlog::debug("open port successfully!");
	double timeout;
	spdlog::info("please input receive time limit seconds");
	scanf("%lf", &timeout);
	while (getchar() != '\n');
	while (1) {
		char send_buff[100] = {};
		printf("before send_buff length is %d\n", strlen(send_buff));
		spdlog::info("please input your command that you want to send:");
		printf("# ");
		gets_s(send_buff, 100);
		printf("then send_buff length is %d\n", strlen(send_buff));
		int res = mod::hvg::control::send(p, send_buff, strlen(send_buff));
		printf("after send_buff length is %d\n", strlen(send_buff));
		spdlog::debug("actually send_byte:{:d}", res);
		spdlog::debug("actually send buff:{:s}", send_buff);
		spdlog::debug("recv...");
		while (1)
		{
			char recv_buff[1024] = {};
			printf("before complete command is%s\n", recv_buff);
			mod::hvg::control::get_line(p, recv_buff, 1024, timeout);
			//mod::hvg::control::get_line(p, recv_buff, 1024);
			printf("complete command is%s\n", recv_buff);
			if (strlen(recv_buff) != 0) {
				break;
			}
			spdlog::debug("p->buf is :{:s}", p->buf);
		}
	}
	ret = mod::hvg::control::close(p);
	if (ret == false) {
		spdlog::error("close port failed");
		return -1;
	}
	spdlog::info("close port successfully\n");
	free(p);
	return 0;
}
