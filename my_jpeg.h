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

//app0��14����Ϣ�ṹ��
struct app0_E_info_s{
    u8  saw_jfif_marker;                            //�Ƿ���JFIF
    u8  jfif_major_version;                       //���汾��
    u8  jfif_minor_version;                      //�ΰ汾��
    u8  density_unit;                                  //�ܶȵ�λ
    u16 x_density;                                       //ˮƽ������ܶ�
    u16 y_density;                                       //��ֱ������ܶ�
    u8  saw_adobe_marker;                     //�Ƿ���Adobe
    u8  adobe_transform;                         //adobe����ɫת������
};


//����������DQT���Ϊ0~3
#define NUM_QUANT_TBLS             4
//ÿ��DQT��Ϊ64��Ԫ��,128���ֽ�
#define DCTSIZE2                                 64
//�������������Ϣ�ṹ��
struct dqt_info_s{
    u16 quantbal[DCTSIZE2];
    u8 prec;
    u8 dqt_id;
    u16 found_flag;
};
typedef struct dqt_info_s * dqt_info_p;



//SOF �Լ�SOS�����Ϣ�ṹ��
struct components_info_s
{
    //SOF��Ϣ
    u8 component_id;
    u8 component_index;
    u8 h_samp_factor;
    u8 v_samp_factor;
    u8 quant_tbl_no;
    //SOS��Ϣ
    u8 dc_tbl_no;
    u8 ac_tbl_no;
};
//SOF��Ϣ�ṹ��
struct sof_info_s
{
    u8 saw_sof_marker;
    struct components_info_s  *components_info;
};

#define MAX_COMPS_IN_SCAN                            4
//SOS��Ϣ�ṹ��
struct sos_info_s
{
    u8 saw_sos_marker;
    struct components_info_s  *cur_components_info[MAX_COMPS_IN_SCAN];
};

//����������Ϣ�ṹ��
struct dht_info_s
{
    u8 bits[16];
    u8 huffval[256];
    u32 flag;
};
typedef struct dht_info_s * dht_info_p;
//�����������
#define     NUM_HUFF_TABLE                4


//����ģʽ
struct jpg_mode_s
{
    u8 is_baseline;
    u8 progressive_mode;
    u8 arith_code;
};

//JPG������Ϣ�ṹ��
typedef struct jpg_data_t
{
    FILE   *fd;
    u32    status;
    struct app0_E_info_s app0_E_info;
    struct sof_info_s sof_info;
    struct sos_info_s sos_info;
    struct jpg_mode_s jpg_mode;  
    dqt_info_p dqt_info[NUM_QUANT_TBLS];                           //��������Ϣ,���0~3
    dht_info_p dht_ac_info[NUM_HUFF_TABLE];                     //������������,���0~3
    dht_info_p dht_dc_info[NUM_HUFF_TABLE];                     //������ֱ����,���0~3

    u8      data_precision;                          //��������
    u8      num_components;                   //�������
    u8      comps_in_scan;                         //ɨ�������������
    u16    width;                                           //ͼ��Ŀ�
    u16    height;                                         //ͼ��ĸ�
    u16    restart_interval;                       //���¿�ʼ���

    //progressive JPEG parameters for scan 
    u8 ss;
    u8 se;
    u8 ah;
    u8 al;




}jpg_data_s;

typedef  jpg_data_s * jpg_data_p;





#endif //__MY_JPEG_H_
