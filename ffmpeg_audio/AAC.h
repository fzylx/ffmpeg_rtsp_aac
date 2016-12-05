





#ifndef _AAC_H_
#define _AAC_H_
#include "config.h"

#define  USE_PORT 7000
#define  USE_IP "127.0.0.1"
#define  MAXDATASIZE 1500
#define  AAC 96                   //负载类型
#define  POCKT_MAX_LEN 255

typedef struct
{
	unsigned char v;               //!< Version, 2 bits, MUST be 0x2
	unsigned char p;			   //!< Padding bit, Padding MUST NOT be used
	unsigned char x;			   //!< Extension, MUST be zero
	unsigned char cc;       	   //!< CSRC count, normally 0 in the absence of RTP mixers 		
	unsigned char m;			   //!< Marker bit
	unsigned char pt;			   //!< 7 bits, Payload Type, dynamically established
	unsigned int seq;			   //!< RTP sequence number, incremented by one for each sent packet 
	unsigned int timestamp;	       //!< timestamp, 27 MHz for H.264
	unsigned int ssrc;			   //!< Synchronization Source, chosen randomly
	unsigned char * payload;      //!< the payload including payload headers
	unsigned int paylen;		   //!< length of payload in bytes
} RTPpacket_t;




typedef struct 
{
	/*  0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|V=2|P|X|  CC   |M|     PT      |       sequence number         |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                           timestamp                           |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|           synchronization source (SSRC) identifier            |
	+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
	|            contributing source (CSRC) identifiers             |
	|                             ....                              |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */
	//intel 的cpu 是intel为小端字节序（低端存到底地址） 而网络流为大端字节序（高端存到低地址）
	/*intel 的cpu ： 高端->csrc_len:4 -> extension:1-> padding:1 -> version:2 ->低端
	 在内存中存储 ：
	 低->4001（内存地址）version:2
	     4002（内存地址）padding:1
		 4003（内存地址）extension:1
	 高->4004（内存地址）csrc_len:4

     网络传输解析 ： 高端->version:2->padding:1->extension:1->csrc_len:4->低端  (为正确的文档描述格式)

	 存入接收内存 ：
	 低->4001（内存地址）version:2
	     4002（内存地址）padding:1
	     4003（内存地址）extension:1
	 高->4004（内存地址）csrc_len:4
	 本地内存解析 ：高端->csrc_len:4 -> extension:1-> padding:1 -> version:2 ->低端 ，
	 即：
	 unsigned char csrc_len:4;        // expect 0 
	 unsigned char extension:1;       // expect 1
	 unsigned char padding:1;         // expect 0 
	 unsigned char version:2;         // expect 2 
	*/
	/* byte 0 */
	 unsigned char csrc_len:4;        /* expect 0 */
	 unsigned char extension:1;       /* expect 1, see RTP_OP below */
	 unsigned char padding:1;         /* expect 0 */
	 unsigned char version:2;         /* expect 2 */
	/* byte 1 */
	 unsigned char payloadtype:7;     /* RTP_PAYLOAD_RTSP */
	 unsigned char marker:1;          /* expect 1 */
	/* bytes 2,3 */
	 unsigned short seq_no;            
	/* bytes 4-7 */
	 unsigned int timestamp;        
	/* bytes 8-11 */
	 unsigned int ssrc;              /* stream number is used here. */
} RTP_HEADER;



/*
sampling_frequency_index sampling frequeny [Hz]
0x0                           96000
0x1                           88200
0x2                           64000
0x3                           48000
0x4                           44100
0x5                           32000
0x6                           24000
0x7                           22050
0x8                           16000
0x9                           2000
0xa                           11025
0xb                           8000
0xc                           reserved
0xd                           reserved
0xe                           reserved
0xf                           reserved
*/
typedef struct
{
	unsigned int syncword;  //12 bslbf 同步字The bit string ‘1111 1111 1111’，说明一个ADTS帧的开始
	unsigned int id;        //1 bslbf   MPEG 标示符, 设置为1
	unsigned int layer;     //2 uimsbf Indicates which layer is used. Set to ‘00’
	unsigned int protection_absent;  //1 bslbf  表示是否误码校验
	unsigned int profile;            //2 uimsbf  表示使用哪个级别的AAC，如01 Low Complexity(LC)--- AACLC
	unsigned int sf_index;           //4 uimsbf  表示使用的采样率下标
	unsigned int private_bit;        //1 bslbf 
	unsigned int channel_configuration;  //3 uimsbf  表示声道数
	unsigned int original;               //1 bslbf 
	unsigned int home;                   //1 bslbf 
	/*下面的为改变的参数即每一帧都不同*/
	unsigned int copyright_identification_bit;   //1 bslbf 
	unsigned int copyright_identification_start; //1 bslbf
	unsigned int aac_frame_length;               // 13 bslbf  一个ADTS帧的长度包括ADTS头和raw data block
	unsigned int adts_buffer_fullness;           //11 bslbf     0x7FF 说明是码率可变的码流

	/*no_raw_data_blocks_in_frame 表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧.
	所以说number_of_raw_data_blocks_in_frame == 0 
	表示说ADTS帧中有一个AAC数据块并不是说没有。(一个AAC原始帧包含一段时间内1024个采样及相关数据)
    */
	unsigned int no_raw_data_blocks_in_frame;    //2 uimsfb
} ADTS_HEADER;


#endif