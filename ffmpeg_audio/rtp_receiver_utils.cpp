






#include "AAC.h"

char recvbuf[MAXDATASIZE];  //����ͷ��������� 1500
SOCKET  socket1;
SOCKADDR_IN client;//����һ����ַ�ṹ��
int len_client = sizeof(client);
int	receive_bytes = 0;
unsigned char playload[MAXDATASIZE];                                  //ȫ�ֱ���������   

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

	if ((adts_headerbuf[0] == 0xFF)&&((adts_headerbuf[1] & 0xF0) == 0xF0))    //syncword 12��1
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
	int total_bytes = 0;                 //��ǰ������������
	static int total_recved = 0;         //һ�����������
	int fwrite_number = 0;               //�����ļ������ݳ���
	unsigned int  framelen = 0;          //�����ݳ���

	memcpy(recvbuf,bufIn, len);          //����rtp��            
	printf("������+ rtpͷ��   = %d\n",len);

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
	printf("�汾�� : %d\n",rtp_hdr->version);
	p->v  = rtp_hdr->version;
	p->p  = rtp_hdr->padding;
	p->x  = rtp_hdr->extension;
	p->cc = rtp_hdr->csrc_len;
	printf("��־λ : %d\n",rtp_hdr->marker);
	p->m = rtp_hdr->marker;
	printf("��������:%d\n",rtp_hdr->payloadtype);
	p->pt = rtp_hdr->payloadtype;

	printf("before����   : %d \n",rtp_hdr->seq_no);
	 rtp_hdr->seq_no = ntohs( rtp_hdr->seq_no);	
	printf("����   : %d \n",rtp_hdr->seq_no);	
	p->seq = rtp_hdr->seq_no;

	printf("beforeʱ��� : %d\n",rtp_hdr->timestamp);
    rtp_hdr->timestamp = ntohl(rtp_hdr->timestamp);
	printf("ʱ��� : %d\n",rtp_hdr->timestamp);
	p->timestamp = rtp_hdr->timestamp;

	printf("before֡��   : %d\n",rtp_hdr->ssrc);
	rtp_hdr->ssrc = ntohl(rtp_hdr->ssrc);
	printf("֡��   : %d\n",rtp_hdr->ssrc);
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
		// д��aacͷ ��aac��ʵ��
	}
	else if (recvbuf[12] == 255)
	{ 
		framelen = 255 + recvbuf[13] +7;
		WriteAdtsHeader(framelen);
		Read_Naked_Data2(bufIn ,len - 14);
		// fwrite_number = fwrite(adts_headerbuf,1,7,poutfile);
		// fwrite_number = fwrite(playload,1,len - 14,poutfile);
		// д��aacͷ ��aac��ʵ��
	}
    return (unsigned char*)1;
}


// ��ʼ���������������˿ڽ���
int init_receiver(){

	// ��ʼ��socket
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

	socket1 = socket(AF_INET/*Trtp_header_t/IPЭ��ֻ��������Э��*/,SOCK_DGRAM/*UDPЭ�������ʽ*/,0/*�Զ�ѡ��Э��*/);
	socket1=socket(AF_INET,SOCK_DGRAM,0);

	client.sin_family = AF_INET;
	client.sin_addr.s_addr = inet_addr(USE_IP);
	client.sin_port = htons(USE_PORT);

	// �󶨶˿�
	if(bind(socket1,(struct sockaddr*)&client,sizeof(client))== -1)
	{
		printf("Bind to local machine error.\n");
		WSACleanup();
		return getchar();
	}

	return 1;
}


// ��ʼ����
int start_recive(){
	while((receive_bytes = recvfrom(socket1,recvbuf,MAXDATASIZE,0,(struct sockaddr *)&client,&len_client)) >0) {
		rtp_unpackage(recvbuf,receive_bytes);
	}
	return 1;
}


// �رն˿�
int close_receiver(){
	closesocket(socket1);
	WSACleanup(); //�ͷſ���Դ
	return getchar();
}