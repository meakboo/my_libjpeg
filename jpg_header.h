#ifndef __JPG_HEADER_H_
#define __JPG_HEADER_H_

//字段信息
#define MARKER_FLAG                     0xFF
#define MARKER_FILL                       0xFF
#define MARKER_SOI                         0xD8
#define MARKER_SOF0                      0xC0
#define MARKER_SOF1                      0xC1
#define MARKER_SOF2                      0xC2
#define MARKER_DHT                        0xC4
#define MARKER_DQT                        0xDB
#define MARKER_DRI                          0xDD
#define MARKER_SOS                         0xDA
#define MARKER_RST1                       0xD1
#define MARKER_APP0                      0xE0
#define MARKER_APP1                      0xE1
#define MARKER_APP2                      0xE2
#define MARKER_APP3                      0xE3
#define MARKER_APP4                      0xE4
#define MARKER_COM                       0xFE
#define MARKER_EOI                         0xD9

//获取字段步骤
#define HEADER_ERR                        0x00
#define HEADER_START                   0x01
#define HEADER_FIND_SOI             0x02


extern u32 jpg_get_header( jpg_data_p  jpg_data);

#endif //__JPG_HEADER_H_
