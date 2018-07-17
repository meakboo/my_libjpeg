#ifndef __JPG_HEADER_H_
#define __JPG_HEADER_H_

//字段信息
#define MARKER_FLAG                     0xFF
#define MARKER_FILL                     0xFF
#define MARKER_SOI                      0xD8
//帧开始
#define MARKER_SOF0                     0xC0                      /* Baseline */
#define MARKER_SOF1                     0xC1                      /* Extended sequential, Huffman */
#define MARKER_SOF2                     0xC2                      /* Progressive, Huffman */
#define MARKER_SOF3                     0xC3                      /* Lossless, Huffman */
//定义霍夫曼表
#define MARKER_DHT                      0xC4                       /*Define Huffman table(s)*/
#define MARKER_SOF5                     0xC5                      /* Differential sequential, Huffman */
#define MARKER_SOF6                     0xC6                      /* Differential progressive, Huffman */
#define MARKER_SOF7                     0xC7                      /* Differential lossless, Huffman */
#define MARKER_JPG                       0xC8                       /* Reserved for JPEG extensions */
#define MARKER_SOF9                     0xC9                      /* Extended sequential, arithmetic */
#define MARKER_SOF10                   0xCA                     /* Progressive, arithmetic */
#define MARKER_SOF11                   0xCB                     /* Lossless, arithmetic */
#define MARKER_DAC                       0xCC                      /*Define arithmetic conditioning table*/
#define MARKER_SOF13                   0xCD                     /* Differential sequential, arithmetic */
#define MARKER_SOF14                   0xCE                      /* Differential progressive, arithmetic */
#define MARKER_SOF15                   0xCF                      /* Differential lossless, arithmetic */


#define MARKER_DQT                      0xDB
#define MARKER_DRI                      0xDD
#define MARKER_SOS                      0xDA
#define MARKER_RST1                     0xD1
//APP0~15字段
#define MARKER_APP0                     0xE0
#define MARKER_APP1                     0xE1
#define MARKER_APP2                     0xE2
#define MARKER_APP3                     0xE3
#define MARKER_APP4                     0xE4
#define MARKER_APP5                     0xE5
#define MARKER_APP6                     0xE6
#define MARKER_APP7                     0xE7
#define MARKER_APP8                     0xE8
#define MARKER_APP9                     0xE9
#define MARKER_APP10                     0xEA
#define MARKER_APP11                    0xEB
#define MARKER_APP12                    0xEC
#define MARKER_APP13                    0xED
#define MARKER_APP14                    0xEE
#define MARKER_APP15                    0xEF
//注释字段
#define MARKER_COM                      0xFE
//结束字段
#define MARKER_EOI                      0xD9

//获取字段步骤
#define HEADER_ERR                      0x00
#define HEADER_START                    0x01
#define HEADER_FIND_SOI                 0x02

//APP0有用字段长度
#define APP0_DATA_LEN                   14
#define APP14_DATA_LEN                 12
#define APPN_DATA_LEN                   14

//获取字节时时候是数据类型
#define DATA_TYPE                       0
#define NO_DATA_TYPE                    1

extern u32 jpg_get_header( jpg_data_p  jpg_data);

#endif //__JPG_HEADER_H_
