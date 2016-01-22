/**
  ******************************************************************************
  * @file    client.c
  * @author  MCD Application Team
  * @version wangjunwei
  * @date    20150717
  * @brief   A sample TCP client  Test
 */

#include "main.h"
#include <string.h>
#include <stdio.h>
#include "lwip/opt.h"
#include "lwip/tcp_impl.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include "filesystem.h"

#define TCP_PORT      40096
//5s 一次TCP连接	 (250ms*4)*5
#define TCP_CREATTRM_INTERVAL  (250*4)*5
//2s	SENT TCP DATA
#define TCPSent_CREATTRM_INTERVAL (250*4)*2 
//500ms	SENT UDP DATA
#define UDPSent_CREATTRM_INTERVAL (250*2)
/* Private function prototypes -----------------------------------------------*/
void udp_client_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
void tcp_client_err(void *arg, err_t err);
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

void  tcp_client_callback(void);
char Tcplink=1;	//TCP连接 1是代表可以重练 0表示不可以当前已经在连接中了
char Udplink=0;	//UDP连接 0是代表可以重练 1表示不可以当前已经在连接中了
struct tcp_pcb *wpcb; 
uint32_t retry_TCP_connect=0;
uint32_t SystemRunTime=0 ;
uint32_t TCPCreatTrm =0;
struct udp_pcb *upcb;
u8 Udpbuf[10];


void udp_client_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	  	    struct pbuf *ps;
		
	//接收固定长度的广播到数据并转发广播出去 test data len >10

	 if(p->tot_len<10){
	    memcpy((u8*)&Udpbuf[0],(u8*)p->payload,p->tot_len);
		ps = pbuf_alloc(PBUF_TRANSPORT, p->tot_len, PBUF_RAM);
		memcpy((u8*)ps->payload,(u8*)&Udpbuf[0],p->tot_len);
		udp_sendto(upcb, ps,IP_ADDR_BROADCAST, 21223);
		     pbuf_free(ps);
	  				  }

	   pbuf_free(p);
	  
}

void udp_client_callback_app(void)
{
                          
   /* Create a new UDP control block  */
   upcb = udp_new();   
   upcb->so_options |= SOF_BROADCAST;
 
   /* Bind the upcb to any IP address and the UDP_PORT port*/
   udp_bind(upcb, IP_ADDR_ANY, 2);  
   /* Set a receive callback for the upcb */
   udp_recv(upcb, udp_client_callback, NULL);

}
  char cansent=0;
  err_t  tcp_client_reciver(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{

					char *data;
					char UID_STM32[16];
				   	char IDon[7] ={0x67,0x74,0,0,1,0x0d,0x0a};
					char IDoff[7]={0x67,0x74,0,0,0,0x0d,0x0a};
					char IDs[7]  ={0x67,0x74,0,0,2,0x0d,0x0a};

			
 if (err == ERR_OK && p != NULL)
  {

    /* Inform TCP that we have taken the data. */
    tcp_recved(tpcb, p->tot_len);
		 data = p->payload;

						if (strncmp(data, "start", 5) == 0)
					 {
							cansent=1;
						 tcp_write(tpcb, "startsent\r\n", sizeof( "startsent\r\n"), 1);
						 tcp_output(tpcb);
					 }
					 else
					  	if (strncmp(data, "stop", 4) == 0)
					 {
							cansent=0;
						 tcp_write(tpcb, "startsent\r\n", sizeof( "startsent\r\n"), 1);
						 tcp_output(tpcb);
					 }
					 else
		             if (strncmp(data, "getru", 5) == 0)
					 {
						  UID_STM32[0]=0xAA;
						  UID_STM32[4]=retry_TCP_connect;
						  UID_STM32[3]=retry_TCP_connect>>8;
						  UID_STM32[2]=retry_TCP_connect>>16;	  //重练次数
						  UID_STM32[1]=retry_TCP_connect>>24;

						  UID_STM32[8]=SystemRunTime;
						  UID_STM32[7]=SystemRunTime>>8;		  //运行时间
						  UID_STM32[6]=SystemRunTime>>16;
						  UID_STM32[5]=SystemRunTime>>24;
									
						  UID_STM32[9]=0xBB;
						  UID_STM32[10]=0x0d;
						  UID_STM32[11]=0x0a;
					 tcp_write(tpcb, &UID_STM32, 12, 1);
							 tcp_output(tpcb);
					 }
					 else
					 if (strncmp(data, "getid", 5) == 0)
						{
				 		  UID_STM32[0]=0xAB;
						  UID_STM32[1]=0x31;
						  UID_STM32[2]=0x32;
						  UID_STM32[3]=0x33;
						  UID_STM32[4]=0x34;

						  UID_STM32[5]=0x35;
						  UID_STM32[6]=0x36;
						  UID_STM32[7]=0x37;
						  UID_STM32[8]=0x38;
									
						  UID_STM32[9]=0x39;
						  UID_STM32[10]=0x40;
						  UID_STM32[11]=0x41;
						  UID_STM32[12]=0x42;

						  UID_STM32[13]=0xCD;
						  UID_STM32[14]=0x0d;
						  UID_STM32[15]=0x0a;
							 tcp_write(tpcb, &UID_STM32, 16, 1);
							 tcp_output(tpcb);

		                  }
						  else
						if(memcmp(&IDoff,data, 5)==0)
						{
						  	tcp_write(tpcb, &IDoff, 7, 1);
							 tcp_output(tpcb);
						
						}
						else if(memcmp(&IDon,data, 5)==0)
						{
							  tcp_write(tpcb, &IDon, 7, 1);
							 tcp_output(tpcb);
						
						}
						else if(memcmp(&IDs,data, 5)==0)
						{
								tcp_write(tpcb, &IDon, 7, 1);
							 tcp_output(tpcb);
						}

	   				 
	 pbuf_free(p);	
		


   }
if ((err == ERR_OK && p == NULL)||(err<0))
  {
	Tcplink=1;
    tcp_close(wpcb);

  }		 
  
	return ERR_OK;
} 
 void tcp_client_err(void *arg, err_t err)
 {
	  if(err==ERR_RST) //主机复位
	  {
		 //host well be free the PCB dont close PCB

	    Tcplink=1;
	  	return ;  //DO nothing..
	  }

 	  if(err==ERR_ABRT)
	  {
 	 Tcplink=1;
  // tcp_close(wpcb);
   
 	   }
	   else
	   {
	 //   tcp_close(wpcb);
	   	 Tcplink=1;

	   }
	 
 }

void tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{


}
	// static char first=0;
 void  tcp_client_callback(void)
  {	
  
    	struct ip_addr ip_addr;	

			wpcb= (struct tcp_pcb*)0;
			  /* Create a new TCP control block  */
			  wpcb = tcp_new();
			
			  /* Assign to the new pcb a local IP address and a port number */
			 if(tcp_bind(wpcb, IP_ADDR_ANY, TCP_PORT)!=ERR_OK)
			   {
			   		//failed
					 return ;
			   
			   }
		
			  IP4_ADDR(&ip_addr, 192,168,0,163);
			   // IP4_ADDR(&ip_addr, 192,168,1,3);
			/* Connect to the server: send the SYN *///TCP_PORT
		    tcp_connect(wpcb, &ip_addr, 2301, tcp_client_connected);
			//	tcp_poll(wpcb,tcp_client_poll,2);
			tcp_err( wpcb,tcp_client_err);  //register err
			tcp_recv(wpcb,tcp_client_reciver);  //register recv

}

/**
  * @brief  This function is called when the connection with the remote 
  *         server is established
  * @param arg user supplied argument
  * @param tpcb the tcp_pcb which received data
  * @param err error value returned by the tcp_connect 
  * @retval error value
  */

err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
  
  	   Tcplink=0;
 	   retry_TCP_connect++;
	tpcb->so_options |= SOF_KEEPALIVE;
    tpcb->keep_idle = 500;// ms
    tpcb->keep_intvl = 500;// ms
    tpcb->keep_cnt = 2;// report error after 2 KA without response

   tcp_write(tpcb, "connecting....", 14, 1);
	tcp_output(tpcb);
  
  return ERR_OK;
}

 
void TcpTestThread(uint32_t Time)
{
if (Time - TCPCreatTrm >= TCP_CREATTRM_INTERVAL)
  {
    TCPCreatTrm =  Time;
	 
	 if(Tcplink)//if link =1,will be connectting to sever ..
	 {
	
	  FS_Write(1,"TCP_Retry\r\n", sizeof("TCP_Retry\r\n"));
	  tcp_client_callback();
	 
	 }
	 if(Udplink==0)//just actived once 
	 {
	   Udplink=1; 
	   FS_Write(1,"UDP_Retry\r\n", sizeof("UDP_Retry\r\n"));
	   udp_client_callback_app();
	   

	 }
    
  }






}

const u8 Tcpbf[1024]={8};
char num=0,num1=0;
uint32_t TCPSentCreatTrm=0;
 void SendDataToSever(uint32_t Time)
 {			err_t erro;
	
  
 		 if((Tcplink==0)&&(0/*cansent*/)&&((Time - TCPSentCreatTrm >= TCPSent_CREATTRM_INTERVAL)))
		 {
		        TCPSentCreatTrm =  Time;
		   //连接成功啦可以发送数据
				   erro= tcp_write(wpcb, Tcpbf, 1024, 1);
				 if(erro==ERR_OK )
			        {
				
				
				         if(tcp_output(wpcb)==ERR_OK)
						 {
						 
							//成功的发送了一包
						 }
						 else
						 {
						   	Tcplink=1;
   							 tcp_close(wpcb);
						 }
					}
					else
					{
						  num++;
						  if(num>=50)
						  {	   num=0;
									  switch(erro)
									  {
								/*	  case -1:{FS_Write(1,"Out of memory error\r\n", sizeof("Out of memory error\r\n"));break;}
									  case -2:{FS_Write(1,"Buffer error", sizeof("Buffer error"));break;}
									  case -3:{FS_Write(1,"Timeout", sizeof("Timeout"));break;}
									  case -4:{FS_Write(1,"Routing problem", sizeof("Routing problem"));break;}
									  case -5:{FS_Write(1,"Operation in progress", sizeof("Operation in progress"));break;}
									  case -6:{FS_Write(1,"Illegal value", sizeof("Illegal value"));break;}
									  case -7:{FS_Write(1,"Operation would block", sizeof("Operation would block"));break;}
									  case -8:{FS_Write(1,"Address in use", sizeof("Address in use"));break;}
									  case -9:{FS_Write(1,"Already connected", sizeof("Already connected"));break;}
									  case -10:{FS_Write(1,"Connection aborted", sizeof("Connection aborted"));break;}
									  case -11:{FS_Write(1,"Connection reset", sizeof("Connection reset"));break;}
									  case -12:{FS_Write(1,"Connection closed", sizeof("Connection closed"));break;}
									  case -13:{FS_Write(1,"Not connected", sizeof("Not connected"));break;}
									  case -14:{FS_Write(1,"Illegal argument", sizeof("Illegal argument"));break;}
									  case -15:{FS_Write(1,"Low-level netif error", sizeof("Low-level netif error"));break;}
									   */
									  }
									  
									   if(erro!=(-1))
									   {
									   	Tcplink=1;
				  					   tcp_close(wpcb);
									   }

						}
					}

		 }
		 else
		 {
		   //没有建立一个成功的连接发什么？

		 }
 
 
 }

 extern u8  temp[33];
  uint8_t Udpbf[1024]={0x55};
  uint32_t UDPSentCreatTrm=0;
 void UDPSendData(uint32_t Time)
 {
 
   	     struct pbuf *Udps;
		 ip_addr_t dst_ip_addr;

		 IP4_ADDR(&dst_ip_addr, 192,168,0,163);
		if((Udplink)&&(cansent)&&((Time - UDPSentCreatTrm >= UDPSent_CREATTRM_INTERVAL)))
		{ 
			   UDPSentCreatTrm=	Time ;
		Udps = pbuf_alloc(PBUF_TRANSPORT, 32, PBUF_RAM);
				if(Udps!=NULL)
				{
				   
							
				memcpy((u8*)Udps->payload,(u8*)&temp[0],32);
		
				udp_sendto(upcb,Udps,IP_ADDR_BROADCAST/*&dst_ip_addr*//* IP_ADDR*/,8080);
				
				 pbuf_free(Udps);
				}
				else
				{
					   num1++;
				   if(num1==50)
				   {	
				   num1=0;
			//	   UART_Write(UART0,"Oups is NULL\r\n", sizeof("Oups is NULL\r\n"));


				   }
				
				}

	 	}
 
 }




 err_t Tcp_recv(void *arg, struct tcp_pcb *pcb,  struct pbuf *p, err_t err)
{
	 if (err == ERR_OK && p != NULL)
  {
  



	   pbuf_free(p);

  }
  if ((err == ERR_OK && p == NULL)||(err<0))
  {
	 
	 tcp_close(pcb);
	
  }


}

static void conn_err(void *arg, err_t err)
{
 
}
err_t Tcp_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{ 

  pcb->so_options |= SOF_KEEPALIVE;
  pcb->keep_idle = 500;// ms
  pcb->keep_intvl = 500;// ms
  pcb->keep_cnt = 2;// report error after 2 KA without response
  //tcp_arg(pcb, pcb);
  tcp_recv(pcb, Tcp_recv);
  tcp_err(pcb, conn_err);
// tcp_poll(pcb, http_poll, 10);
  return ERR_OK;
}


 void tcp_sever_test(void)
{
  struct tcp_pcb *pcb;
  /*create new pcb*/
  pcb = tcp_new();
  
  if (!pcb)
  {
    return ;
  }
  /* bind PORT traffic to pcb */
  tcp_bind(pcb, IP_ADDR_ANY, 8080);
  /* start listening on port 80 */
  pcb = tcp_listen(pcb);
  /* define callback function for TCP connection setup */
  tcp_accept(pcb, Tcp_accept);
}



/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
