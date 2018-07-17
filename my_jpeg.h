#ifndef __MY_JPEG_H_
#define __MY_JPEG_H_

#define DEBUG_ON

#ifndef DEBUG_ON
#define EERDEBUG(...)
#define DEBUG(...)
#define WARDEBUG(...)
#else
#define ERRDEBUG(fmt, args...) fprintf(stderr, "\033[1;31m  ERROR(%s:%d):\t\033[0m" fmt, __func__, __LINE__, ## args)
#define DEBUG(fmt, args...) fprintf(stderr, "\033[1;35m  DEBUG(%s:%d):\t\033[0m" fmt, __func__, __LINE__, ## args)
#define WARDEBUG(fmt, args...) fprintf(stderr, "\033[1;33m  WARNING(%s:%d):\t\033[0m" fmt, __func__, __LINE__, ## args)
#endif

#define RET_OK               0
#define RET_ERR             1

#define TRUE                    0xAA
#define FALSE                  0xBB

typedef unsigned   int       u32;
typedef            int       s32;
typedef unsigned   short     u16;
typedef            short     s16;
typedef unsigned    char     u8;
typedef             char     s8;

//app0和14的信息结构体
struct app0_E_info_s{
    u8  saw_jfif_marker;                            //是否发现JFIF
    u8  jfif_major_version;                       //主版本号
    u8  jfif_minor_version;                      //次版本号
    u8  density_unit;                                  //密度单位
    u16 x_density;                                       //水平方向的密度
    u16 y_density;                                       //垂直方向的密度
    u8  saw_adobe_marker;                     //是否发现Adobe
    u8  adobe_transform;                         //adobe的颜色转换代码
};

//DQT序号为0~3
#define NUM_QUANT_TBLS             4
//每个DQT表为64个元素,128个字节
#define DCTSIZE2                                 64
//定义量化表的信息结构体
struct dqt_info_s{
    u16 quantbal[DCTSIZE2];
    u32 flag;
};
typedef struct dqt_info_s * dqt_info_p;

//SOF 组件信息结构体
struct components_info_s
{
    u8 component_id;
    u8 h_samp_factor;
    u8 v_samp_factor;
    u8 quant_tbl_no;
};
//SOF信息结构体
struct sof_info_s
{
    u8 saw_sof_marker;
    struct components_info_s  *components_info;
};

typedef struct jpg_data_t
{
    FILE   *fd;
    u32    status;
    struct app0_E_info_s app0_E_info;
    struct sof_info_s sof_info;
    dqt_info_p dqt_info[NUM_QUANT_TBLS];

    u8      data_precision;                          //样本精度
    u8      num_components;                   //组件个数
    u16    width;                                           //图像的宽
    u16    height;                                         //图像的高
    u16    restart_interval;                       //重新开始间隔


}jpg_data_s;

typedef  jpg_data_s * jpg_data_p;





#endif //__MY_JPEG_H_
