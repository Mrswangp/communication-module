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
	int recycle_recv(hvg_serial_t* p, char* ptr, int i, char* buf);
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
		p->len = 0;
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
		// open port
		if (RS232_OpenComport(p->port, p->baud, p->mode)) {
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
		if (p == nullptr) {
			return -1;
		}
		char* ptr = p->buf;
		int n = RS232_PollComport(p->port, (unsigned char*)ptr + p->len, BUF_SIZE - p->len);
		if (n == 0) {
			return 0;
		}
		p->len = p->len + n;
		//p->buf[p->len] = '\0';
		spdlog::debug("receive_buf:{:s}", p->buf);
		for (int i = 0; i < n; i++) {
			if (p->buf[i] == '\r' && p->buf[i + 1] == '\n') {
				memcpy(buf, p->buf, i + 2);
				memmove(p->buf, p->buf + i + 2, p->len - i - 2);
				p->len = p->len - i - 2;
				return i + 2;
			}
			else if (i == n - 2) {
				return 0;
			}
		}
		return 0;
	}
	int recycle_recv(hvg_serial_t* p, char* ptr, int i, char* buf)
	{
		int n = RS232_PollComport(p->port, (unsigned char*)ptr + p->len, BUF_SIZE - p->len);
		p->len = p->len + n;
		//p->buf[p->len] = '\0';
		spdlog::debug("receive_buf:{:s}", p->buf);
		spdlog::debug("length:{:d};available space:{:d}", p->len, BUF_SIZE - p->len);
		while (i < p->len) {
			if (p->buf[i] == '\r' && p->buf[i + 1] == '\n') {
				memcpy(buf, p->buf, i + 2);
				//strcat(buf, "\0");
				memmove(p->buf, p->buf + i + 2, p->len - i - 2);
				p->len = p->len - i - 2;
				//	p->buf[p->len] = '\0';
				return i + 2;
			}
			i++;
			spdlog::debug("i:{:d}", i);
		}
		return 0;
	}
	int get_line(hvg_serial_t* p, char* buf, int len)
	{
		if (p == nullptr) {
			return -1;
		}
		char* ptr = p->buf;
		int i = 0;
		spdlog::debug("p->port:{:d}", p->port);
		while (1) {
			int n = recycle_recv(p, ptr, i, buf);
			if (n > 0) {
				return n;
			}
		}
		return 0;
	}
	int get_line(hvg_serial_t* p, char* buf, int len, double timeout)
	{
		if (p == nullptr) {
			return -1;
		}
		char* ptr = p->buf;
		int i = 0;
		spdlog::debug("p->port:{:d}", p->port);
		clock_t start;
		clock_t finish;
		double run_time = 0;
		start = clock();
		while (run_time < timeout) {
			finish = clock();
			run_time = ((double)(finish - start) / CLOCKS_PER_SEC) * 1000.0;
			spdlog::debug("run_time:{:f}", run_time);
			int n = recycle_recv(p, ptr, i, buf);
			if (n > 0) {
				return n;
			}
		}
		return 0;
	}
}
#endif // _HVG_SERIAL_IMPLEMENTED_
#endif // HVG_SERIAL_IMPLEMENTATION
