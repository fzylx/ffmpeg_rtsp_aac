






#include "AAC.h"

char recvbuf[MAXDATASIZE];  //加上头最大传输数据 1500
SOCKET  socket1;
SOCKADDR_IN client;//分配一个地址结构体
int len_client = sizeof(client);
int	receive_bytes = 0;
unsigned char playload[MAXDATASIZE];                                  //全局变量裸数据   

unsigned char  adts_headerbuf[7] ;

int Read_Naked_Data1(char *bufIn,unsigned int aac_frame_length)
{
	memcpy(playload,bufIn + 13,aac_frame_length);
	return 1;
}

int Read_Naked_Data2(char *bufIn,unsigned int aac_frame_length)
{
	memcpy(playload,bufIn + 14,aac_frame_length);
	return 1;
}

int WriteAdtsHeader(unsigned int  framelen)
{
	ADTS_HEADER adts ; 

	adts_headerbuf[0] = 0xFF;
	adts_headerbuf[1] = 0xF9;
	adts_headerbuf[2] = 0x40;
	adts_headerbuf[2] |= 0x10;
	adts_headerbuf[2] |= 0x00;
	adts_headerbuf[2] |= 0x00;
	adts_headerbuf[2] |= 0x00;
	adts_headerbuf[3] = 0x80;
	adts_headerbuf[3] |= 0x00;
	adts_headerbuf[3] |= 0x00;
	adts_headerbuf[3] |= 0x00;
	adts_headerbuf[3] |= 0x00;
	adts_headerbuf[3] |= (framelen & 0x1800) >> 11;
	adts_headerbuf[4] = (framelen & 0x1FF8) >> 3;
	adts_headerbuf[5] = (framelen & 0x0007) << 5;
	adts_headerbuf[5] |= 0x1F;
	adts_headerbuf[6] = 0xFC;
	adts_headerbuf[6] |= 0x00;

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
	return 1;
}



unsigned char * rtp_unpackage(char *bufIn,int len)
{
	RTPpacket_t *p = NULL; 
	RTP_HEADER * rtp_hdr = NULL;
	int total_bytes = 0;                 //当前包传出的数据
	static int total_recved = 0;         //一共传输的数据
	int fwrite_number = 0;               //存入文件的数据长度
	unsigned int  framelen = 0;          //罗数据长度

	memcpy(recvbuf,bufIn, len);          //复制rtp包            
	printf("包长度+ rtp头：   = %d\n",len);

//////////////////////////////////////////////////////////////////////////
//begin rtp_payload and rtp_header

	p = (RTPpacket_t*)&recvbuf[0];
	if ((p = (RTPpacket_t*)malloc (sizeof (RTPpacket_t)))== NULL)
	{
		printf ("RTPpacket_t MMEMORY ERROR\n");
	}
	if ((p->payload = (uint8_t*)malloc (MAXDATASIZE))== NULL)
	{
		printf ("RTPpacket_t payload MMEMORY ERROR\n");
	}

	if ((rtp_hdr = (RTP_HEADER*)malloc(sizeof(RTP_HEADER))) == NULL)
	{
		printf("RTP_HEADER MEMORY ERROR\n");
	}
    
	rtp_hdr =(RTP_HEADER*)&recvbuf[0]; 
	printf("版本号 : %d\n",rtp_hdr->version);
	p->v  = rtp_hdr->version;
	p->p  = rtp_hdr->padding;
	p->x  = rtp_hdr->extension;
	p->cc = rtp_hdr->csrc_len;
	printf("标志位 : %d\n",rtp_hdr->marker);
	p->m = rtp_hdr->marker;
	printf("负载类型:%d\n",rtp_hdr->payloadtype);
	p->pt = rtp_hdr->payloadtype;

	printf("before包号   : %d \n",rtp_hdr->seq_no);
	 rtp_hdr->seq_no = ntohs( rtp_hdr->seq_no);	
	printf("包号   : %d \n",rtp_hdr->seq_no);	
	p->seq = rtp_hdr->seq_no;

	printf("before时间戳 : %d\n",rtp_hdr->timestamp);
    rtp_hdr->timestamp = ntohl(rtp_hdr->timestamp);
	printf("时间戳 : %d\n",rtp_hdr->timestamp);
	p->timestamp = rtp_hdr->timestamp;

	printf("before帧号   : %d\n",rtp_hdr->ssrc);
	rtp_hdr->ssrc = ntohl(rtp_hdr->ssrc);
	printf("帧号   : %d\n",rtp_hdr->ssrc);
	p->ssrc = rtp_hdr->ssrc;
	
//end rtp_payload and rtp_header
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
	memset(adts_headerbuf,0,7);
	if (recvbuf[12] < 255)
	{
		framelen  = recvbuf[12] + 7;
		WriteAdtsHeader(framelen) ;
		Read_Naked_Data1(bufIn ,len - 13);
		// fwrite_number = fwrite(adts_headerbuf,1,7,poutfile);
		// fwrite_number = fwrite(playload,1,len - 13,poutfile);
		// 写出aac头 和aac的实体
	}
	else if (recvbuf[12] == 255)
	{ 
		framelen = 255 + recvbuf[13] +7;
		WriteAdtsHeader(framelen);
		Read_Naked_Data2(bufIn ,len - 14);
		// fwrite_number = fwrite(adts_headerbuf,1,7,poutfile);
		// fwrite_number = fwrite(playload,1,len - 14,poutfile);
		// 写出aac头 和aac的实体
	}
    return (unsigned char*)1;
}


// 初始化接收器，开启端口接收
int init_receiver(){

	// 初始化socket
	int Error;
	WORD Version = MAKEWORD(2, 2);
	WSADATA WsaData;
	Error = WSAStartup(Version,&WsaData);	      //Start up WSA	  
	if(Error!=0){
		return 0;
	}
	else
	{
		if(LOBYTE(WsaData.wVersion)!=2||HIBYTE(WsaData.wHighVersion)!=2)
		{
			WSACleanup();
			return 0;
		}
	}

	socket1 = socket(AF_INET/*Trtp_header_t/IP协议只能是这种协议*/,SOCK_DGRAM/*UDP协议的是流式*/,0/*自动选择协议*/);
	socket1=socket(AF_INET,SOCK_DGRAM,0);

	client.sin_family = AF_INET;
	client.sin_addr.s_addr = inet_addr(USE_IP);
	client.sin_port = htons(USE_PORT);

	// 绑定端口
	if(bind(socket1,(struct sockaddr*)&client,sizeof(client))== -1)
	{
		printf("Bind to local machine error.\n");
		WSACleanup();
		return getchar();
	}

	return 1;
}


// 开始接收
int start_recive(){
	while((receive_bytes = recvfrom(socket1,recvbuf,MAXDATASIZE,0,(struct sockaddr *)&client,&len_client)) >0) {
		rtp_unpackage(recvbuf,receive_bytes);
	}
	return 1;
}


// 关闭端口
int close_receiver(){
	closesocket(socket1);
	WSACleanup(); //释放库资源
	return getchar();
}