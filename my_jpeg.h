#ifndef __MY_JPEG_H_
#define __MY_JPEG_H_

#define DEBUG_ON

#ifndef DEBUG_ON
#define ERRDEBUG(...)
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
#define FALSE                   0xBB

//JPEG hw argrument
#define JPEG_SOC_ADDR			0x400000000
#define JPEG_SOC_LEN			0x4000000
#define JPEG_SOC_OFFSET			0x800
#define JPEG_NAME_MAX_LEN		64
//JPEG CFG
#define JPEG_SOC
//JPEG sf argrument
#define JPEG_RESERV_MEM			0x60000000		//1500M
#define JPEG_IMAGE_MMAP_LEN		0x4000000
//JPEG CFG 
#define JPEG_RESET_ALL			(1 << 0)
#define JPEG_CLR_INPUT_FIFO		(1 << 1)
#define JPEG_CLR_OUTPUT_FIFO	(1 << 2)
#define JPEG_CLR_DONE			(1 << 3)
#define JPEG_NORMAL_MODE		(1 <<21)
#define JPEG_START				(1 << 0)
//JPEG STATUS
#define JPEG_DECODE_DONE		(1 << 8)
//DHT OFFSET
#define DHT1_OFFSET				(0)
#define DHT2_OFFSET				(DHT1_OFFSET + 16)
#define DHT3_OFFSET				(DHT2_OFFSET + 256)
#define DHT4_OFFSET				(DHT3_OFFSET + 16)
//DQT OFFSET
#define DQT_OFFSET				0x400
//HUFFMAN TABLE
#define HUFFMAN_TABLE_OFFSET	0xC00
//READ LEN MAX
#define JPEG_READ_MAX_LEN		0xA00000

#define USE_MY_MEMCPY

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
#define NUM_QUANT_TBLS             				 4
//ÿ��DQT��Ϊ64��Ԫ��,128���ֽ�
#define DCTSIZE2                                 64
//�������������Ϣ�ṹ��
struct dqt_info_s{
    u16 quantbal[DCTSIZE2];
    u8 table_dqt[DCTSIZE2];
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
	u16 table_ht[16];
	u8 table_hn[16];
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

//JPEG FPGA �Ĵ���
struct jpeg_soc_reg{
	u32 jpeg_status;
	u32 jpeg_cfg;
	u32 jpeg_high_width;
	u32 jpeg_rd_addr_low;
	u32 jpeg_rd_addr_high;
	u32 jpeg_wr_addr_low;
	u32 jpeg_wr_addr_high;
	u32 jpeg_run;
};


//JPG������Ϣ�ṹ��
typedef struct jpg_data_t
{
    FILE   *fd;
	struct jpeg_soc_reg *reg;
	void *dht1_addr;
	void *dht2_addr;
	void *dht3_addr;
	void *dht4_addr;
	void *dqt_addr;
	void *huffmantable_addr;
	void *jpg_input_addr;
	void *jpg_output_addr;
	void *data_buff;
	void *rgb_buff;
	void *gpio_mem;
	s32		mem_fd;
    u32    status;
    u32		file_size;
    struct app0_E_info_s app0_E_info;
    struct sof_info_s sof_info;
    struct sos_info_s sos_info;
    struct jpg_mode_s jpg_mode;  
    dqt_info_p dqt_info[NUM_QUANT_TBLS];                           //��������Ϣ,���0~3
	//AC��� ����һ����0x01,�ڶ�����0x03
    dht_info_p dht_ac_info[NUM_HUFF_TABLE];                     //������������,���0~3
    //DC��񣬵�һ����0x00,�ڶ�����0x02 �����ԣ���Ӧ�ľ���DC1,AC1,DC2,AC2
    dht_info_p dht_dc_info[NUM_HUFF_TABLE];                     //������ֱ����,���0~3	
    u8      data_precision;                          //��������
    u8      num_components;                   //�������
    u8      comps_in_scan;                         //ɨ�������������
    u16    width;                                           //ͼ��Ŀ�
    u16    height;                                         //ͼ��ĸ�
    u16		width_block;								//ͼ��������
	u16		height_block;							//ͼ���������
	u32		read_len;								//����ɹ�����Ҫ��ȡ�ĳ���
    u16    restart_interval;                       //���¿�ʼ���
	u8 jpg_name[JPEG_NAME_MAX_LEN];
    //progressive JPEG parameters for scan 
    u8 ss;
    u8 se;
    u8 ah;
    u8 al;
}jpg_data_s;

typedef  jpg_data_s * jpg_data_p;




#endif //__MY_JPEG_H_
