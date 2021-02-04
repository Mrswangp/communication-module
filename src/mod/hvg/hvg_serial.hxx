#ifndef _HVG_SERIAL_H_
#define _HVG_SERIAL_H_
#include <future>
#include <functional>
#include <iostream>
#include "rs232.h"
#include"spdlog/spdlog.h"
namespace mod::hvg::control {
	constexpr size_t BUF_SIZE = 1024 * 4;
	struct hvg_serial_t {
		int port;
		int baud;
		char mode[4];
		int pos;
		int len;
		char buf[BUF_SIZE];
		enum status_t {
			PENDING,
			SENDING,
			SENT,
			RECIEVING,
			RECIEVED,      // not used
			TIMEOUT
		} status;

		using process_data_f = std::function<void(char* p, int n)>;
		process_data_f process_data;
	};

	bool init(hvg_serial_t* p, int port, int baud, const char* mode);
	void drop(hvg_serial_t* p);
	bool open(hvg_serial_t* p);
	bool close(hvg_serial_t* p);
	int send(hvg_serial_t* p, char* buf, int len);
	int recv(hvg_serial_t* p, char* buf, int len);
	int get_line(hvg_serial_t* p, char* buf, int len, double timeout);
	int get_line(hvg_serial_t* p, char* buf, int len);
}

#endif // _HVG_SERIAL_H_
#ifdef HVG_SERIAL_IMPLEMENTATION
#ifndef _HVG_SERIAL_IMPLEMENTED_
#define _HVG_SERIAL_IMPLEMENTED_
namespace mod::hvg::control {
	bool init(hvg_serial_t* p, int port, int baud, const char* mode)
	{
		if (p == nullptr) {
			return false;
		}
		p->port = port;
		p->baud = baud;
		strncpy(p->mode, mode, 4);
		return true;
	}
	void drop(hvg_serial_t* p)
	{

	}
	bool open(hvg_serial_t* p)
	{
		if (p == nullptr) {
			return false;
		}
		if (RS232_OpenComport(p->port, p->baud, p->mode)) { //Open port
			spdlog::error("can not open server port:{:d}", p->port);
			return false;
		}
		spdlog::info("open the server comport:{:d}", p->port);
		return true;
	}
	bool close(hvg_serial_t* p)
	{
		if (p == nullptr) {
			return false;
		}
		RS232_CloseComport(p->port);
		spdlog::info("server port status is closed");
		return true;
	}
	int send(hvg_serial_t* p, char* buf, int len)
	{
		//printf("send->port=%d\n", p->port);
		spdlog::debug("send->port:{:d}", p->port);
		if (p == nullptr) {
			return -1;
		}
		if (*buf == 0) {
			spdlog::info("No data need send!");
			return 0;
		}
		strcat(buf, "\r\n");
		int ret = RS232_SendBuf(p->port, (unsigned char*)buf, strlen(buf));
		spdlog::debug("send_data_number:{:d}", ret);
		return ret;
	}
	int recv(hvg_serial_t* p, char* buf, int len)
	{
		int RET = 0;
		if (p == nullptr) {
			return -1;
		}
		char* ptr = p->buf;
		memset(p->buf, 0x00, BUF_SIZE);
		while (1) {
			RET = RS232_PollComport(p->port, (unsigned char*)ptr, BUF_SIZE);
			if (RET > 0) {
				spdlog::debug("receive_buf:{:s}", p->buf);
			}
			if (RET < 2) {
				memcpy(buf, p->buf, RET);
				return RET;
			}
			if (p->buf[RET - 1] != '\n' && p->buf[RET - 2] != '\r') {
				memcpy(buf, p->buf, RET);
				return RET;
			}
			for (int i = 0; i < RET; i++) {
				if (p->buf[i] != '\n' && p->buf[i - 1] != '\r') {
					memcpy(buf, p->buf, i);
					return i;
				}
			}
		}
		return 0;
	}
	int get_line(hvg_serial_t* p, char* buf, int len)
	{
		int RET = 0;
		if (p == nullptr) {
			return -1;
		}
		char* ptr = p->buf;
		p->len = 0;
		int i = 0;
		int flag = 0;
		spdlog::debug("p->port:{:d}", p->port);
		while (1) {
			RET = RS232_PollComport(p->port, (unsigned char*)ptr + p->len, BUF_SIZE - p->len);
			if (RET > 0) {
				p->buf[RET] = '\0';
				spdlog::debug("receive_buf:{:s}", p->buf);
				p->len = p->len + RET;
				spdlog::debug("length:{:d};available space:{:d}", p->len, BUF_SIZE - p->len);
				while (i < p->len) {
					if (p->buf[i] == '\n' && p->buf[i - 1] == '\r') {
						memcpy(buf, p->buf, i + 1);
						strcat(buf, "\0");
						memmove(p->buf, p->buf + i, p->len - i);
						p->len = p->len - i;
						i = 0;
						flag = 1;
						break;
					}
					else {
						i++;
						spdlog::debug("i:{:d}", i);
					}
				}
				if (flag == 1)
					break;
			}
		}
		return 0;
	}
	int get_line(hvg_serial_t* p, char* buf, int len, double timeout)
	{
		printf("timeout is %f", timeout);
		int RET = 0;
		if (p == nullptr) {
			return -1;
		}
		char* ptr = p->buf;
		p->len = 0;
		int i = 0;
		int flag = 0;
		spdlog::debug("p->port:{:d}", p->port);
		clock_t start;
		clock_t finish;
		double run_time;
		start = clock();
		while (1) {
			RET = RS232_PollComport(p->port, (unsigned char*)ptr + p->len, BUF_SIZE - p->len);
			finish = clock();
			run_time = (double)(finish - start) / CLOCKS_PER_SEC;
			if (RET > 0) {
				p->buf[RET] = '\0';
				spdlog::debug("receive_buf:{:s}", p->buf);
				p->len = p->len + RET;
				spdlog::debug("length:{:d};available space:{:d}", p->len, BUF_SIZE - p->len);
				while (i < p->len) {
					if (p->buf[i] == '\n' && p->buf[i - 1] == '\r') {
						memcpy(buf, p->buf, i + 1);
						strcat(buf, "\0");
						memmove(p->buf, p->buf + i, p->len - i);
						p->len = p->len - i;
						i = 0;
						flag = 1;
						break;
					}
					else {
						i++;
						spdlog::debug("i:{:d}", i);
					}
				}
				if (flag == 1)
					break;
			}
			if (run_time >= timeout)
				break;
		}
		return 0;
	}
}
#endif // _HVG_SERIAL_IMPLEMENTED_
#endif // HVG_SERIAL_IMPLEMENTATION
