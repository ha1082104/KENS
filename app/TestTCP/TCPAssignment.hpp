/*
 * E_TCPAssignment.hpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: 근홍
 */

#ifndef E_TCPASSIGNMENT_HPP_
#define E_TCPASSIGNMENT_HPP_


#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_RoutingInfo.hpp>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <list>
#include <E/Networking/E_Packet.hpp>

#include <E/E_TimerModule.hpp>

namespace E
{
	enum TCP_STATE
	{
		CLOSED,
		LISTEN,
		SYN_RCVD,
		SYN_SENT,
		ESTABLISHED,
		CLOSE_WAIT,
		FIN_WAIT_1,
		FIN_WAIT_2,
		CLOSING,
		TIME_WAIT,
		LAST_ACK
	};

	struct wakeup_arguments
	{
		UUID syscallUUID;
		struct sockaddr *addr;
		socklen_t *addrlen;
	};

	struct timer_arguments
	{
		int pid;
		int sockfd;
	};

	struct tcp_context
	{
		TCP_STATE tcp_state = CLOSED;
		int pid;
		int sockfd;
		unsigned int src_addr;
		unsigned int dst_addr;
		unsigned short src_port;
		unsigned short dst_port;
		bool is_bound = false;
	   int backlog;
		int seq_num;
		int to_be_accepted = 0;
		std::list< struct tcp_context > pending_conn_list;
		std::list< struct tcp_context > estab_conn_list;
		struct wakeup_arguments wake_args;
	};

	struct tcp_header
	{
		unsigned short src_port;
		unsigned short dst_port;
		int seq_num;
		int ack_num;
		uint8_t hdr_len;
		uint8_t flags;
		unsigned short recv_window;
		unsigned short checksum;
		unsigned short urg_ptr;
	};

class TCPAssignment : public HostModule, public NetworkModule, public SystemCallInterface, private NetworkLog, private TimerModule
{
private:
	std::list < struct tcp_context > tcp_context_list;
	int random_seq_num = 0;
	int random_port = 10000;

private:
	virtual void timerCallback(void* payload) final;

	/* System Call */
	void syscall_socket (UUID, int, int, int);
	void syscall_close (UUID, int, int);
	void syscall_connect (UUID, int, int, struct sockaddr *, socklen_t addrlen);
	void syscall_listen (UUID, int, int, int);
	void syscall_accept (UUID, int, int, struct sockaddr *, socklen_t *);
	void syscall_bind (UUID, int, int, struct sockaddr *, socklen_t);
	void syscall_getsockname (UUID, int, int, struct sockaddr *, socklen_t *);
	void syscall_getpeername (UUID, int, int, struct sockaddr *, socklen_t *);
	std::list< struct tcp_context >::iterator find_tcp_context (int, int);
	unsigned short calculate_checksum (unsigned int, unsigned int, struct tcp_header);
	std::list< struct tcp_context >::iterator get_context_addr_port (unsigned int, unsigned int, unsigned short, unsigned short);
	std::list< struct tcp_context >::iterator find_pending_context (int, std::list< struct tcp_context > *);
	void remove_tcp_context (int, int);

public:
	TCPAssignment(Host* host);
	virtual void initialize();
	virtual void finalize();
	virtual ~TCPAssignment();
protected:
	virtual void systemCallback(UUID syscallUUID, int pid, const SystemCallParameter& param) final;
	virtual void packetArrived(std::string fromModule, Packet* packet) final;
};

class TCPAssignmentProvider
{
private:
	TCPAssignmentProvider() {}
	~TCPAssignmentProvider() {}
public:
	static HostModule* allocate(Host* host) { return new TCPAssignment(host); }
};

}


#endif /* E_TCPASSIGNMENT_HPP_ */