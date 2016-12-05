















#include "AAC.h"

SOCKET socket_cli;
SOCKADDR_IN addrser;  //Initialize Win Socket
RTP_HEADER  * rtp_hdr = NULL ;
int pocket_number  = 0;   //包号
int frame_number = 0;     //帧号
int total_sent = 0;       //已经发送的总共数据
int  bytes = 0;           //一次发送的数据
unsigned char  adts_headerbuf[7] ;
ADTS_HEADER  *adts_header = NULL  ;
int timestamp = 0;
unsigned int Framelen = 0;

uint8_t sendbuf[MAXDATASIZE];
uint8_t playload[MAXDATASIZE];

ADTS_HEADER adts ; 

ADTS_HEADER  *ReadAdtsHeader(unsigned char * adts_headerbuf)
{
	//ADTS_HEADER adts ; 

	static int frame_number = 0;
	adts.aac_frame_length = 0;
	adts.adts_buffer_fullness = 0;
	adts.channel_configuration = 0;
	adts.copyright_identification_bit = 0;
	adts.copyright_identification_start = 0;
	adts.home = 0;
	adts.id = 0;
	adts.layer = 0;
	adts.no_raw_data_blocks_in_frame = 0;
	adts.original = 0;
	adts.private_bit = 0;
	adts.profile = 0;
	adts.protection_absent = 0;
	adts.sf_index = 0;
	adts.syncword = 0;

	if ((adts_headerbuf[0] == 0xFF)&&((adts_headerbuf[1] & 0xF0) == 0xF0))    //syncword 12个1
	{
		adts.id = ((unsigned int) adts_headerbuf[1] & 0x08) >> 3;
		printf("adts:id  %d\n",adts.id);
		adts.layer = ((unsigned int) adts_headerbuf[1] & 0x06) >> 1;
		printf( "adts:layer  %d\n",adts.layer);
		adts.protection_absent = (unsigned int) adts_headerbuf[1] & 0x01;
		printf( "adts:protection_absent  %d\n",adts.protection_absent);
		adts.profile = ((unsigned int) adts_headerbuf[2] & 0xc0) >> 6;
		printf( "adts:profile  %d\n",adts.profile);
		adts.sf_index = ((unsigned int) adts_headerbuf[2] & 0x3c) >> 2;
		printf( "adts:sf_index  %d\n",adts.sf_index);
		adts.private_bit = ((unsigned int) adts_headerbuf[2] & 0x02) >> 1;
		printf( "adts:pritvate_bit  %d\n",adts.private_bit);
		adts.channel_configuration = ((((unsigned int) adts_headerbuf[2] & 0x01) << 2) | (((unsigned int) adts_headerbuf[3] & 0xc0) >> 6));
		printf( "adts:channel_configuration  %d\n",adts.channel_configuration);
		adts.original = ((unsigned int) adts_headerbuf[3] & 0x20) >> 5;
		printf( "adts:original  %d\n",adts.original);
		adts.home = ((unsigned int) adts_headerbuf[3] & 0x10) >> 4;
		printf( "adts:home  %d\n",adts.home);
		adts.copyright_identification_bit = ((unsigned int) adts_headerbuf[3] & 0x08) >> 3;
		printf( "adts:copyright_identification_bit  %d\n",adts.copyright_identification_bit);
		adts.copyright_identification_start = (unsigned int) adts_headerbuf[3] & 0x04 >> 2;
		printf( "adts:copyright_identification_start  %d\n",adts.copyright_identification_start);
		adts.aac_frame_length = (((((unsigned int) adts_headerbuf[3]) & 0x03) << 11) | (((unsigned int) adts_headerbuf[4] & 0xFF) << 3)| ((unsigned int) adts_headerbuf[5] & 0xE0) >> 5) ;
		printf( "adts:aac_frame_length  %d\n",adts.aac_frame_length);
		adts.adts_buffer_fullness = (((unsigned int) adts_headerbuf[5] & 0x1f) << 6 | ((unsigned int) adts_headerbuf[6] & 0xfc) >> 2);
		printf( "adts:adts_buffer_fullness  %d\n",adts.adts_buffer_fullness);
		adts.no_raw_data_blocks_in_frame = ((unsigned int) adts_headerbuf[6] & 0x03);
		printf( "adts:no_raw_data_blocks_in_frame  %d\n",adts.no_raw_data_blocks_in_frame);
	}
	else 
	{
		//printf("读取adts 头失败 : 第 %d帧 \n",frame_number);
		printf("文件传输结束 或 传输错误");
		exit(0);
	}
	frame_number ++ ;
	return &adts;
}



int Read_Naked_Data(uint8_t* aac_data, unsigned int aac_frame_length)
{
	memcpy(playload, aac_data + 7, aac_frame_length - 7);
	return 1;
}

int POCKT_MAX_LEN1(unsigned int aac_frame_length)
{
	sendbuf[12] = aac_frame_length - 7;
	memcpy(sendbuf + 13 ,playload,aac_frame_length -7);
	return 1;
}

int POCKT_MAX_LEN2(unsigned int aac_frame_length)
{
	sendbuf[12] = POCKT_MAX_LEN ; 
	sendbuf[13] = (aac_frame_length -7) % POCKT_MAX_LEN;
	memcpy(sendbuf +14 ,playload,aac_frame_length -7);
	return 1;
}



// 初始化rtp
int init_rtp_sender(){

	// 加载socket 库
	int Error;
	WORD Version=MAKEWORD(2,2);
	WSADATA WsaData;
	Error=WSAStartup(Version,&WsaData);	      //Start up WSA	  
	if(Error!=0)
		return 0;
	else
	{
		if(LOBYTE(WsaData.wVersion)!=2||HIBYTE(WsaData.wHighVersion)!=2)
		{
			WSACleanup();
			return 0;
		}

	}

	//创建socket套接字
	socket_cli= socket(AF_INET ,SOCK_DGRAM/*UDP协议的是流式*/,0);
	addrser.sin_addr.S_un.S_addr =inet_addr(USE_IP); 
	addrser.sin_family = AF_INET;
	addrser.sin_port = htons(USE_PORT);       //网络字节序

	return 1;
}



// 采用rtp打包的方式发送aac
int send_aac_over_rtp(uint8_t* aac_data, int size){

	memset(adts_headerbuf,0,7);
	memcpy(adts_headerbuf, aac_data, 7);
	adts_header = ReadAdtsHeader(adts_headerbuf);
	Framelen = adts_header->aac_frame_length;
	Read_Naked_Data(aac_data, Framelen);
	memset(sendbuf, 0 ,MAXDATASIZE);
	rtp_hdr =(RTP_HEADER*)&sendbuf[0];            
	rtp_hdr->payloadtype     = AAC;							    //Payload type
	rtp_hdr->version         = 2;                               //Payload version
	rtp_hdr->marker          = 0;							    //Marker sign
	printf("frame_number = %d\n",frame_number);
	rtp_hdr->ssrc            = htonl(frame_number++);	        //帧数
	printf("rtp_hdr->ssrc  = %d\n",rtp_hdr->ssrc );
	timestamp = timestamp + 6000;
	printf("timestamp = %d\n",timestamp);
	rtp_hdr->timestamp = htonl(timestamp);                      //事件戳
	printf("rtp_hdr->timestamp = %d\n",rtp_hdr->timestamp);
	printf("pocket_number = %d\n",pocket_number);
	rtp_hdr->seq_no = htons(pocket_number ++);                  //包号
	printf("rtp_hdr->seq_no = %d\n",rtp_hdr->seq_no);
	if (Framelen - 7 <= POCKT_MAX_LEN)
	{
		bytes = Framelen + 13 -7;
		POCKT_MAX_LEN1(Framelen);
		sendto(socket_cli, (const char*)sendbuf,bytes,0,(SOCKADDR *)&addrser,sizeof(SOCKADDR));  
		printf("%d\n",sendbuf[12]);
	}
	else if (Framelen - 7 > POCKT_MAX_LEN)
	{
		bytes = Framelen + 14 -7;
        POCKT_MAX_LEN2(Framelen);
		sendto(socket_cli, (const char*)sendbuf,bytes,0,(SOCKADDR *)&addrser,sizeof(SOCKADDR));
	}
	return 1;
}



// 关闭发送端口
int close_rtp(){
	closesocket(socket_cli); //关闭套接字
	WSACleanup();
	return getchar();
}