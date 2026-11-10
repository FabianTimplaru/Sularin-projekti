#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Salli ilman jäyttöjärjestelmää (bare metal) toimiva tila

#define NO_SYS                      1
//IPv4 ja perusprotokollat päälle
#define LWIP_IPV4                   1
#define LWIP_TCP                    1
#define LWIP_UDP                    1
#define LWIP_DNS                    1
#define LWIP_ICMP                   1
#define LWIP_DHCP                   1
#define LWIP_ETHERNET               1
#define LWIP_ARP                    1

// Estetään tarpeettomia ominaisuuksia säästämään muistia
#define LWIP_IPV6                   0
#define LWIP_SOCKET                 0
#define LWIP_COMPAT_SOCKETS         0
#define LWIP_NETCONN                0
#define LWIP_IGMP                   0

// Muistin ja puskureiden asetukset (Picon standardit)
#define MEM_LIBC_MALLOC             0
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4000
#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_ARP_QUEUE          10
#define PBUF_POOL_SIZE              24

#define TCP_MSS                     1460
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
#define LWIP_NETIF_TX_SINGLE_PBUF   1
#define DHCP_DOES_ARP_CHECK         0
#define LWIP_DHCP_DOES_ACD_CHECK    0
#endif
