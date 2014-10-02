# List of the required lwIP files.
LWIP = 	${CHIBIOS}/os/ext/lwip/src

LWBINDSRC = \
        $(CHIBIOS)/os/various/lwip_bindings/arch/sys_arch.c

LWCORESRC = \
        ${LWIP}/core/dhcp.c \
        ${LWIP}/core/dns.c \
        ${LWIP}/core/init.c \
        ${LWIP}/core/mem.c \
        ${LWIP}/core/memp.c \
        ${LWIP}/core/netif.c \
        ${LWIP}/core/pbuf.c \
        ${LWIP}/core/raw.c \
        ${LWIP}/core/stats.c \
        ${LWIP}/core/sys.c \
        ${LWIP}/core/tcp.c \
        ${LWIP}/core/tcp_in.c \
        ${LWIP}/core/tcp_out.c \
        ${LWIP}/core/udp.c

LWIPV4SRC = \
        ${LWIP}/core/ipv4/autoip.c \
        ${LWIP}/core/ipv4/icmp.c \
        ${LWIP}/core/ipv4/igmp.c \
        ${LWIP}/core/ipv4/inet.c \
        ${LWIP}/core/ipv4/inet_chksum.c \
        ${LWIP}/core/ipv4/ip.c \
        ${LWIP}/core/ipv4/ip_addr.c \
        ${LWIP}/core/ipv4/ip_frag.c \
        ${LWIP}/core/def.c \
        ${LWIP}/core/timers.c

LWIPPPSRC = \
		${LWIP}/netif/ppp/auth.c \
		${LWIP}/netif/ppp/chap.c \
		${LWIP}/netif/ppp/ipcp.c \
		${LWIP}/netif/ppp/fsm.c \
		${LWIP}/netif/ppp/lcp.c \
		${LWIP}/netif/ppp/magic.c \
		${LWIP}/netif/ppp/md5.c \
		${LWIP}/netif/ppp/pap.c \
		${LWIP}/netif/ppp/ppp.c \
		${LWIP}/netif/ppp/randm.c

LWAPISRC = \
        ${LWIP}/api/api_lib.c \
        ${LWIP}/api/api_msg.c \
        ${LWIP}/api/err.c \
        ${LWIP}/api/netbuf.c \
        ${LWIP}/api/netdb.c \
        ${LWIP}/api/netifapi.c \
        ${LWIP}/api/sockets.c \
        ${LWIP}/api/tcpip.c

LWSRC = $(LWBINDSRC) $(LWCORESRC) $(LWIPV4SRC) $(LWAPISRC) $(LWIPPPSRC)

LWINC = \
        $(CHIBIOS)/os/various/lwip_bindings \
        ${LWIP}/include \
        ${LWIP}/include/ipv4 \
        ${LWIP}/netif
