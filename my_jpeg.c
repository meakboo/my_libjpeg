#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>

#include "my_jpeg.h"
#include "jpg_header.h"
#include "jpg_gpio.h"

u32 wait_done = 1;
u32 bug_size = 0;
void  *temp = 0;
void *temp_1 = 0;
/*******************************************************************************
��������: jpgӲ����ʼ��
�������: jpg_data_p  jpg_data : jpg�ṹ��
�������: ��
����ֵ��:��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void jpg_hw_res(jpg_data_p jpg_data)
{
	//jpg_data->reg->jpeg_cfg &= ~JPEG_RESET_ALL;
	jpg_data->reg->jpeg_cfg |= JPEG_CLR_INPUT_FIFO;
	jpg_data->reg->jpeg_cfg |= JPEG_CLR_OUTPUT_FIFO;
	jpg_data->reg->jpeg_cfg |= JPEG_CLR_DONE;	
	usleep(5000);
	//jpg_data->reg->jpeg_cfg |= JPEG_RESET_ALL;
	jpg_data->reg->jpeg_cfg &= ~JPEG_CLR_INPUT_FIFO;
	jpg_data->reg->jpeg_cfg &= ~JPEG_CLR_OUTPUT_FIFO;
	jpg_data->reg->jpeg_cfg &= ~JPEG_CLR_DONE;
	usleep(5000);
	return;
}

/*******************************************************************************
��������: ��ʼ��jpg������Դ
�������: jpg_data_s  *jpg_data :jpg���ݽṹ��ָ�� �Ķ���ָ��
        char *file_name �� ͼƬ�ļ�����
�������: ��
����ֵ��:�ɹ�:0 ʧ��:1
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
jpg_data_p jpg_decompress_init( void)
{
	void *temp = NULL;
	jpg_data_p jpg_data = NULL;
    
    jpg_data = (jpg_data_p)malloc(sizeof(jpg_data_s));
    if (!jpg_data)
    {
        ERRDEBUG("malloc failed\n");
        return NULL;
    }
    //�����ڴ棺ֱ�Ӵ�FPGA out�ӿڶ�ȡ����
	jpg_data->data_buff = malloc(JPEG_READ_MAX_LEN);
	if(!(jpg_data->data_buff))
	{
        ERRDEBUG("malloc failed\n");
        return NULL;		
	}
	//�����ڴ棺����ֱ�ӿ����õ�RGBͼ������
	jpg_data->rgb_buff = malloc(JPEG_READ_MAX_LEN);
	if(!(jpg_data->rgb_buff))
	{
        ERRDEBUG("malloc failed\n");
        return NULL;		
	}
#ifdef JPEG_SOC
	jpg_data->mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(jpg_data->mem_fd < 0)
	{
		ERRDEBUG("open /dev/mem failed\n");
		return NULL;
	}
	temp = (void *)mmap(NULL, JPEG_SOC_LEN,  PROT_READ | PROT_WRITE ,\
						MAP_SHARED,  jpg_data->mem_fd, JPEG_SOC_ADDR) ;
	if(temp == NULL )
	{
		ERRDEBUG("mmap failed\n");
		return NULL;
	}
	
	//ȷ�����������ڴ��ַ
	jpg_data->dht1_addr = temp + DHT1_OFFSET;
	jpg_data->dht2_addr = temp + DHT2_OFFSET;
	jpg_data->dht3_addr = temp + DHT3_OFFSET;
	jpg_data->dht4_addr = temp + DHT4_OFFSET;
	jpg_data->dqt_addr = temp + DQT_OFFSET;
	jpg_data->reg = temp + JPEG_SOC_OFFSET;
	jpg_data->huffmantable_addr = temp + HUFFMAN_TABLE_OFFSET;

	printf("%p  %p  %p\n", temp, jpg_data->dht1_addr , jpg_data->reg );

	jpg_data->jpg_input_addr = mmap(NULL, JPEG_IMAGE_MMAP_LEN,  PROT_READ | PROT_WRITE ,\
						MAP_SHARED,  jpg_data->mem_fd, JPEG_RESERV_MEM);
	if((jpg_data->jpg_input_addr) == NULL)
	{
		ERRDEBUG("mmap failed\n");
		return NULL;
	}
	jpg_data->jpg_output_addr = mmap(NULL, JPEG_IMAGE_MMAP_LEN,  PROT_READ | PROT_WRITE ,\
						MAP_SHARED,  jpg_data->mem_fd, JPEG_RESERV_MEM + JPEG_IMAGE_MMAP_LEN);
	if((jpg_data->jpg_output_addr) == NULL)
	{
		ERRDEBUG("mmap failed\n");
		return NULL;
	}		
	jpg_hw_res(jpg_data);
#endif
    jpg_data->status = HEADER_START;

    return jpg_data;
}

/*******************************************************************************
��������: ��jpg�ļ�
�������: jpg_data_p  jpg_data : jpg�ṹ��
		char *file_name : �ļ�����
�������: ��
����ֵ��:��
-----------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
u32 jpg_open_file(jpg_data_p jpg_data, char *file_name)
{
	struct stat statbuf;
	u8 *name_p = NULL;

	name_p = strstr(file_name, ".jpg");
	if(!name_p)
	{
		ERRDEBUG("file err\n");
		return RET_ERR;
	}
	//��ȡ�ļ���С
	stat(file_name, &statbuf);
	if(statbuf.st_size == 0)
	{
		ERRDEBUG("file size = 0\n");
		return RET_ERR;
	}
	jpg_data->file_size = statbuf.st_size;
	
    jpg_data->fd = fopen(file_name, "r");
    if (jpg_data->fd == NULL)
    {
        ERRDEBUG("fopen failed\n");
        return RET_ERR;
    }
	memset(jpg_data->jpg_name, 0, sizeof(jpg_data->jpg_name));
	memcpy(jpg_data->jpg_name, file_name, strlen(file_name));
    
	return RET_OK;
}

/*******************************************************************************
��������: ���Դ�ӡ����
�������: jpg_data_p  jpg_data : jpg�ṹ��
�������: ��
����ֵ��:��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void jpg_debug(jpg_data_p  jpg_data,  char *file_name)
{
    int i = 0, j = 0;
    int a = 0, b = 0, c= 0;
    int num = 0;
    struct components_info_s  *components_info = NULL;;

    printf("\n \n \n \n");

    printf("\tFile Path : %s\n", file_name);
    printf("---------------------------------------------------------------\n");
    /* ����APP0 */
    if (jpg_data->app0_E_info.saw_jfif_marker == TRUE)
    {
        printf("\tsaw: [JFIF]\n");
        printf("\tVer %d.%d\n", jpg_data->app0_E_info.jfif_major_version, jpg_data->app0_E_info.jfif_minor_version);
        printf("\tdensity : %d\n", jpg_data->app0_E_info.density_unit);
        printf("\tXdensity %d\n", jpg_data->app0_E_info.x_density);
        printf("\tYdensity %d\n", jpg_data->app0_E_info.y_density);
    }
    else if (jpg_data->app0_E_info.saw_adobe_marker == TRUE)
    {
        printf("\tsaw : [Adobe]\n");
    }
    else
    {
        ERRDEBUG("Not found app0 or app14\n");
        return;
    }
    printf("---------------------------------------------------------------\n");
    /* �������������� */
    for (i = 0;i < NUM_QUANT_TBLS; i++)
    {
        if (jpg_data->dqt_info[i] != NULL)
        {
            if (jpg_data->dqt_info[i]->found_flag == TRUE)
            {
                printf("\tfound DQT : %d  ID = %d\n", num, jpg_data->dqt_info[i]->dqt_id);
                if (jpg_data->dqt_info[i]->prec == 0)
                {
                    printf("\tDQT 1 byte\n");
                }
                else
                {
                    printf("\tDQT 2 byte\\n");
                }
                printf("\t");
                for (j = 1; j <= DCTSIZE2; j++)
                {
                    printf("%2d     ", jpg_data->dqt_info[i]->quantbal[j - 1]);
                    if (j % 8 == 0)
                    {
                        printf("\n\t");
                    }
                }
                printf("DQT  :   %d      printf done\n", num);
                num++;
            }
        }
    }
    if (num == 0)
    {
        ERRDEBUG("not found DQT table\n");
        return;
    }
    printf("---------------------------------------------------------------\n");
    //����֡��ʼ��SOF
    if (jpg_data->sof_info.saw_sof_marker == TRUE)
    {
        printf("\tfound SOF0\n");
        printf("\tdata precision :  %d\n", jpg_data->data_precision);
        printf("\theight : %d\n", jpg_data->height);
        printf("\twidth : %d\n", jpg_data->width);
        printf("\tcomponents num : %d\n", jpg_data->num_components);
        for (i = 0 ,components_info =jpg_data->sof_info.components_info; i < jpg_data->num_components; i++, components_info++)
        {
            printf("\tcomponent ID : %d  h_samp_factor : %d v_samp_factor : %d quant_tbl_no ID : %d\n", components_info->component_id,\
                          components_info->h_samp_factor, components_info->v_samp_factor, components_info->quant_tbl_no);
        }
    }
    printf("---------------------------------------------------------------\n");
    //������������
    for (i = 0; i < NUM_HUFF_TABLE; i++)
    {
        if (jpg_data->dht_ac_info[i] != NULL)
        {
            if (jpg_data->dht_ac_info[i]->flag = TRUE)
            {
                printf("\tfound DHT AC table :%d  ############\n", i);

                for (a = 0, c = 0; a < 16; a++)
                {
                    printf("\tDHT (AC) bit%02d  have %03d  value :", a+1,  jpg_data->dht_ac_info[i]->bits[a]);
                    for (b = 1; b <= jpg_data->dht_ac_info[i]->bits[a]; b++)
                    {
                        printf("%02X   ", jpg_data->dht_ac_info[i]->huffval[c]);
                        c++;
                        if (b   > 0  &&   b  % 10 == 0)
                        {
                            printf("\n\t\t\t\t\t ");
                        }
                    }
                    printf("\n");
                }
                printf("\t all %d  value\n", c);
            }
        }
    }
    for (i = 0; i < NUM_HUFF_TABLE; i++)
    {
        if (jpg_data->dht_dc_info[i] != NULL)
        {
            if (jpg_data->dht_dc_info[i]->flag = TRUE)
            {
                printf("\tfound DC table :%d  ##########\n", i);

                for (a = 0, c = 0; a < 16; a++)
                {
                    printf("\tDHT (DC) bit%02d  have %03d value  :", a+1,  jpg_data->dht_dc_info[i]->bits[a]);
                    for (b = 1; b <= jpg_data->dht_dc_info[i]->bits[a]; b++)
                    {
                        printf("%02X   ", jpg_data->dht_dc_info[i]->huffval[c]);
                        c++;
                        if (b > 0 &&   b % 10 == 0)
                        {
                            printf("\n\t\t\t\t\t ");
                        }
                    }
                    printf("\n");
                }
                printf("\t all %d  value\n", c);
            }
        }
    }
    printf("---------------------------------------------------------------\n");
    if (jpg_data->restart_interval != 0)
    {
        //printf("\t���ָ�λ�����\n");
        //printf("\t�μ��Ϊ %d\n", jpg_data->restart_interval);
        printf("---------------------------------------------------------------\n");
    }

    //������ʼɨ���SOS
    if (jpg_data->comps_in_scan  != 0)
    {
        printf("\t���ֿ�ʼɨ���\n");
        printf("\t�ö��������Ϊ : %d\n", jpg_data->comps_in_scan );

        for (i = 0; i <  MAX_COMPS_IN_SCAN; i++)
        {
            if (jpg_data->sos_info.cur_components_info[i] != NULL)
            {
                //printf("\t��ɫ����ID : %d    ", jpg_data->sos_info.cur_components_info[i]->component_id);
                //printf("\tֱ������ʹ�õĻ���������Ҫ : %d    ", jpg_data->sos_info.cur_components_info[i]->dc_tbl_no);
                //printf("\t��������ʹ�õĻ���������Ҫ : %d    \n", jpg_data->sos_info.cur_components_info[i]->ac_tbl_no);
            }
        }
    }

    return;
}

/*******************************************************************************
��������: ��ȡjpeg�ļ������Լ��ļ���С
�������: jpg_data_p jpg_data : jpeg���ݽṹ��
�������: ��
����ֵ��: ��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void jpg_get_file(jpg_data_p jpg_data)
{
	u32   cp_size = 0;
	int i = 0;

	if((!temp) ||(!temp_1) )
	{
		temp = malloc(JPEG_IMAGE_MMAP_LEN);
		if(!temp)
		{
			ERRDEBUG("malloc failed\n");
			return ;
		}
		temp_1 = malloc(JPEG_IMAGE_MMAP_LEN);
		if(!temp_1)
		{
			ERRDEBUG("malloc failed\n");
			return ;
		}
	}
	
	//memcpy�����ڴ�8�ֽڶ��룬�����Ϊʲô
	cp_size = (jpg_data->file_size %8)?(jpg_data->file_size + (8 - jpg_data->file_size % 8)):jpg_data->file_size;
	bug_size = jpg_data->file_size;
	//��ȡ�ļ�����
	fseek(jpg_data->fd, 0, SEEK_SET);
	memset(temp, 0, 0x100000);
	//memset(temp_1 , 0, 0x200000);
	fread(temp, jpg_data->file_size, 1, jpg_data->fd);

	//memcpy(jpg_data->jpg_input_addr, temp_1, 0x200000);
	
	//�������ڴ濽��
	memcpy(jpg_data->jpg_input_addr , temp, 0x100000);
	//memcpy(temp_1, jpg_data->jpg_input_addr, cp_size);
#if 0
	if(memcmp(temp_1, temp, jpg_data->file_size))
	{
		printf("data err!\n");
		exit(1);
	}
	else
	{
		printf("#########################\n");

	}
#endif
	return;
}

/*******************************************************************************
��������: jpeg��������
�������: jpg_data_p jpg_data : jpeg���ݽṹ��
�������: ��
����ֵ��: ��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void jpg_fpga_cfg(jpg_data_p jpg_data)
{
	u32 tmp = 0;
	void *temp_buff = NULL;
	u32 row = 0;

	row = (jpg_data->file_size % 16)?(jpg_data->file_size /16 + 1):(jpg_data->file_size / 16);
	if(row % 8 != 0)
	{
		row = (row / 8 + 1) * 8 + 8;
	}
	else
	{
		row += 8;
	}
	tmp = row * 16 - 16;
#if 0
	tmp = (jpg_data->file_size % 128)?((jpg_data->file_size /128 + 1) * 128):(jpg_data->file_size);
	tmp += (128 - 16);
#endif
	
	printf("tmp = %d  %x\n", tmp, tmp);
#if 0
	FILE *fd = fopen("file.jpg", "wb+");
	if(!fd)
	{
		printf("fopen err!\n");
		return;
	}
	temp_buff = malloc(0xa00000);
	if(!temp_buff)
	{
		printf("malloc err\n");
		return;
	}
	memcpy(temp_buff, jpg_data->jpg_input_addr, tmp);
	fwrite(temp_buff, jpg_data->file_size, 1, fd);

	free(temp_buff);
	fclose(fd);
#endif
	//��ȡ��Ҫ���ĳ���
	jpg_data->width_block = (jpg_data->width % 16)?(jpg_data->width /16 + 1):(jpg_data->width /16);
	jpg_data->height_block = (jpg_data->height % 16)?(jpg_data->height /16 + 1):(jpg_data->height /16);
	jpg_data->read_len = jpg_data->width_block * jpg_data->height_block * 256 * 4;
	//���ÿ�߼Ĵ���
	jpg_data->reg->jpeg_high_width = ((jpg_data->height << 16) | jpg_data->width);
	//����FPGA����ַ
	jpg_data->reg->jpeg_rd_addr_low = JPEG_RESERV_MEM;
	jpg_data->reg->jpeg_rd_addr_high = JPEG_RESERV_MEM + tmp;
	//����FPGAд��ַ
	jpg_data->reg->jpeg_wr_addr_low = JPEG_RESERV_MEM + JPEG_IMAGE_MMAP_LEN;
	jpg_data->reg->jpeg_wr_addr_high = JPEG_RESERV_MEM + JPEG_IMAGE_MMAP_LEN + jpg_data->read_len;
	//��ʼ����
	jpg_data->reg->jpeg_run |= JPEG_START;
	
	return;
}

/*******************************************************************************
��������: �ȴ��������
�������: jpg_data_p jpg_data : jpeg���ݽṹ��
�������: ��
����ֵ��: ��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void jpg_wait_done(jpg_data_p jpg_data)
{
	u32 done = 0;

	wait_done = 1;
	alarm(3);
	while((!done) && (wait_done))
	{
		done = jpg_data->reg->jpeg_status & JPEG_DECODE_DONE;
	}
	alarm(0);

	if(!wait_done)
		jpg_data->status = DECODE_FAILED;
	else
		jpg_data->status = DECODE_SUCCSE;
		
	DEBUG("jpg decode done\n");

	if(jpg_data->status == DECODE_FAILED)
	{
		WARDEBUG("Reset\n");
		jpg_data->reg->jpeg_cfg &= ~JPEG_RESET_ALL;
		usleep(5000);
		jpg_data->reg->jpeg_cfg |= JPEG_RESET_ALL;
	}

	return;
}

void jpg_debug_hwmem(void *addr, u32 count, u8 size, u8 *name)
{
	void *tmp = NULL;
	FILE *fd = NULL;
	u32 i = 0;
	
	tmp = malloc(0x100000);
	if(!tmp)
	{
		printf("malloc err\n");
		return;
	}
	fd = fopen(name, "a+");
	if(!fd)
	{
		printf("fopen err\n");
		free(tmp);
		return;
	}

	memcpy(tmp, addr, size * count);
	if(size == 1)
	{
		for(i = 0; i < count; i++)
			fprintf(fd, "%x\n", *((u8 *)tmp + i));
	}
	else if(size == 2)
	{
		for(i = 0; i < count; i++)
			fprintf(fd, "%x\n", *((u16 *)tmp + i));		
	}
	else if(size == 4)
	{
		for(i = 0; i < count; i++)
			fprintf(fd, "%x\n", *((u32 *)tmp + i));		
	}	
	
	fclose(fd);
	fd = NULL;
	free(tmp);
	tmp = NULL;
	return;
}


/*******************************************************************************
��������: ��ָ����ַ����jpgͷ����
�������: jpg_data_p jpg_data : jpeg���ݽṹ��
�������: ��
����ֵ��: ��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void jpg_set_header_data(jpg_data_p jpg_data)
{
	u8 i = 0;
	u8 j = 0;
	u32 huffman_table[4 * 16];

	//����DHT1, ��Ӧ����DC��1
	memcpy(jpg_data->dht1_addr, jpg_data->dht_dc_info[0]->huffval, 16);
	DEBUG("printf DHT1\n");
	//jpg_table_printf_c(jpg_data->dht_dc_info[0]->huffval, 16, "dht1.txt");
	//����DHT2����Ӧ����AC��1
	memcpy(jpg_data->dht2_addr, jpg_data->dht_ac_info[0]->huffval, 256);
	DEBUG("printf DHT2\n");
	//jpg_table_printf_c(jpg_data->dht_ac_info[0]->huffval, 256, "dht2.txt");
	//����DHT3����Ӧ����DC��2
	memcpy(jpg_data->dht3_addr, jpg_data->dht_dc_info[1]->huffval, 16);
	DEBUG("printf DHT1\n");
	//jpg_table_printf_c(jpg_data->dht_dc_info[1]->huffval, 16, "dht3.txt");
	//����DHT4����Ӧ����AC��2
	memcpy(jpg_data->dht4_addr, jpg_data->dht_ac_info[1]->huffval, 256);
	DEBUG("printf DHT2\n");
	//jpg_table_printf_c(jpg_data->dht_ac_info[1]->huffval, 256, "dht4.txt");
	//����DQT
	for(i = 0; i < NUM_QUANT_TBLS; i++)
	{
		if(jpg_data->dqt_info[i] != NULL)
		{
			DEBUG("printf DQT%d\n", i);
			memcpy(jpg_data->dqt_addr + (j * 64), jpg_data->dqt_info[i]->table_dqt, 64);
			//jpg_table_printf_c(jpg_data->dqt_info[i]->table_dqt, 64, "dqt.txt");
			j++;
		}
	}
	//����Huffman Table
	memset(huffman_table, 0, sizeof(huffman_table));
	for(j = i = 0; i < 16; i++)
		huffman_table[i + j * 16] = (jpg_data->dht_dc_info[0]->table_hn[i] << 16) | (jpg_data->dht_dc_info[0]->table_ht[i]);
	j++;
	for(i = 0; i < 16; i++)
		huffman_table[i + j * 16] = (jpg_data->dht_ac_info[0]->table_hn[i] << 16) | (jpg_data->dht_ac_info[0]->table_ht[i]);
	j++;
	for(i = 0; i < 16; i++)
		huffman_table[i + j * 16] = (jpg_data->dht_dc_info[1]->table_hn[i] << 16) | (jpg_data->dht_dc_info[1]->table_ht[i]);
	j++;	
	for(i = 0; i < 16; i++)
		huffman_table[i + j * 16] = (jpg_data->dht_ac_info[1]->table_hn[i] << 16) | (jpg_data->dht_ac_info[1]->table_ht[i]);

	memcpy(jpg_data->huffmantable_addr, huffman_table, sizeof(huffman_table));
#if 0
	jpg_debug_hwmem(jpg_data->dht1_addr, 16, 1, "dht1.txt");
	jpg_debug_hwmem(jpg_data->dht2_addr, 256, 1, "dht2.txt");
	jpg_debug_hwmem(jpg_data->dht3_addr, 16, 1, "dht3.txt");
	jpg_debug_hwmem(jpg_data->dht4_addr, 256, 1, "dht4.txt");
	jpg_debug_hwmem(jpg_data->dqt_addr, 128, 1, "dqt.txt");
	jpg_debug_hwmem(jpg_data->huffmantable_addr, 64, 4, "hn_ht.txt");
#endif
	
	return;
}

/*******************************************************************************
��������: ÿ4�ֽڿ���3�ֽ�RGB����
�������: void *dst ��Ŀ��Buff
		void *src ��ԴBuff
		u32 len ����������(���Դbuff����)
�������: ��
����ֵ��: ��
------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void jpg_my_memcpy(void *dst, void *src, u32 len)
{
	u32 cur_dst = 0;
	u32 cur_src = 0;
	void *dst_tmp = dst;
	void *src_tmp = src;
	u32 width = 0;
	u32 i = 0;

	if(len &3 != 0)
	{
		ERRDEBUG("jpg len err , len = %d\n", len);
		return;
	}
	width = len >> 2;
	
	while(cur_dst != (width * 3))
	{
		memcpy(dst_tmp + cur_dst, src_tmp + cur_src, 3);
		cur_dst += 3;
		cur_src += 4;
	}

	return;
}

/*******************************************************************************
��������: �����������ݱ�����ļ�
�������: jpg_data_p jpg_data : jpeg���ݽṹ��
�������: ��
����ֵ��: ��
------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void jpg_save_image(jpg_data_p jpg_data)
{
	FILE *fd = NULL;
	u32 write_len = 0;
	u32 ret = 0;
	u32 image_num = 0;
	u32 temp = 0;
	u8 *name_p = NULL;
	u8 tftp_str[JPEG_NAME_MAX_LEN];
	u8 new_file_name[JPEG_NAME_MAX_LEN];
	u8 jpg_name[JPEG_NAME_MAX_LEN];
	
	fd = fopen("rgb_test", "wb+");
	if(!fd)
	{
		ERRDEBUG("fopen rgb_test failed\n");
		return;
	}
#ifndef USE_MY_MEMCPY
	write_len = jpg_data->height * jpg_data->width * 4;
#else
	write_len = jpg_data->height * jpg_data->width * 3;
#endif

	ret = fwrite(jpg_data->rgb_buff, 1, write_len, fd);
	//printf("ret = %d write_len = %d\n", ret, write_len);
	fclose(fd);
	fd = NULL;
	
	name_p = strstr(jpg_data->jpg_name, ".");
	if(!name_p)
		return;
	
	memset(new_file_name, 0, sizeof(new_file_name));
	memcpy(new_file_name, jpg_data->jpg_name, name_p - jpg_data->jpg_name);

	image_num = atoi(new_file_name);

	memset(tftp_str, 0, 64);
	sprintf(tftp_str, "tftp -l rgb_test -r %d_%s.dat -p 192.168.150.162", image_num, new_file_name);
	system(tftp_str);



	memset(jpg_name, 0, sizeof(jpg_name));
	sprintf(jpg_name, "%d_%s.jpg", image_num, new_file_name);
	fd = fopen(jpg_name, "wb+");
	if(!fd)
	{
		ERRDEBUG("fopen %s err\n", jpg_name);
		return;
	}
	fwrite(temp_1, jpg_data->file_size, 1, fd);
	fclose(fd);
	fd = NULL;

	usleep(5000);
	memset(tftp_str, 0, 64);
	sprintf(tftp_str, "tftp -l %s -r %s -p 192.168.150.162", jpg_name, jpg_name);
	system(tftp_str);

	usleep(5000);
	memset(tftp_str, 0, 64);
	sprintf(tftp_str, "rm %s", jpg_name);
	system(tftp_str);
	
#if 0	
	memset(tftp_str, 0, sizeof(tftp_str));
	sprintf(tftp_str, "tftp -l dht1.txt     -r %d_dht1.txt -p 192.168.150.162", image_num);
	system(tftp_str);

	memset(tftp_str, 0, sizeof(tftp_str));
	sprintf(tftp_str, "tftp -l dht2.txt     -r %d_dht2.txt -p 192.168.150.162", image_num);
	system(tftp_str);

	memset(tftp_str, 0, sizeof(tftp_str));
	sprintf(tftp_str, "tftp -l dht3.txt     -r %d_dht3.txt -p 192.168.150.162", image_num);
	system(tftp_str);

	memset(tftp_str, 0, sizeof(tftp_str));
	sprintf(tftp_str, "tftp -l dht4.txt     -r %d_dht4.txt -p 192.168.150.162", image_num);
	system(tftp_str);

	memset(tftp_str, 0, sizeof(tftp_str));
	sprintf(tftp_str, "tftp -l dqt.txt     -r %d_dqt.txt -p 192.168.150.162", image_num);
	system(tftp_str);

	memset(tftp_str, 0, sizeof(tftp_str));
	sprintf(tftp_str, "tftp -l hn_ht.txt     -r %d_hn_ht.txt -p 192.168.150.162", image_num);
	system(tftp_str);

	memset(tftp_str, 0, sizeof(tftp_str));
	sprintf(tftp_str, "tftp -l src_data_32_block     -r %d_src_data_32_block -p 192.168.150.162", image_num);
	system(tftp_str);

	memset(tftp_str, 0, sizeof(tftp_str));
	sprintf(tftp_str, "tftp -l file.jpg    -r %d_file.jpg -p 192.168.150.162", image_num);
	system(tftp_str);
#endif
	usleep(5000);
	system("rm rgb_test");

	return;
}

void meakboo_memcpy(jpg_data_p jpg_data)
{
	int i = 0,  j = 0, z = 0, x= 0;
	void *buff = NULL;

	buff = malloc(0xa00000);
	if(!buff)
	{
		printf("malloc failed\n");
		return;
	}


	for(i = 0; i < jpg_data->height_block; i++)
	{
		for(j = 0; j < jpg_data->width_block;  j++)
		{
			for(z = 0; z < 16; z++, x++)
			{
				memcpy(buff + x * 64, jpg_data->data_buff + (i * jpg_data->width_block * 16 * 4 * 16)  + j * 64 + (z * jpg_data->width_block * 16 * 4)   , 64);
			}
			
		}
	}
#if 0
	FILE *fd = fopen("src_data_128_block", "wb+");
	if(!fd)
	{
		printf("src_data fopen\n");
		return;
	}
	for(i = 0; i < (jpg_data->read_len / 16); i++)
	{
		fprintf(fd , "%08x", *((u32 *)buff + i * 4));
		fprintf(fd , "%08x", *((u32 *)buff + i * 4 + 1));
		fprintf(fd , "%08x", *((u32 *)buff+ i * 4 + 2));
		fprintf(fd , "%08x", *((u32 *)buff + i * 4 + 3));
		fprintf(fd, "\n");
	}	
	fclose(fd);
#endif
#if 1
	FILE *fd = fopen("src_data_32_block", "wb+");
	if(!fd)
	{
		printf("src_data fopen\n");
		return;
	}
	for(i = 0; i < (jpg_data->read_len / 4); i++)
	{
		fprintf(fd , "%06x\n", *((u32 *)buff + i));
	}	
	fclose(fd);
#endif
	free(buff);
	buff = NULL;
	return;
}

/*******************************************************************************
��������: ��ȡ������rgb����
�������: jpg_data_p jpg_data : jpeg���ݽṹ��
�������: ��
����ֵ��: ��
------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void jpg_read_xrgb(jpg_data_p jpg_data)
{
	u32 cur_src = 0;
	u32 cur_dst = 0;
	s32 ret = 0;
	u32 i = 0;
	struct timeval start, end;

	//�ȴ�OUT buff������Ҫ���������ݶ����������������д���������������Ҫɾ��
	gettimeofday( &start, NULL );
	memcpy(jpg_data->data_buff, jpg_data->jpg_output_addr, jpg_data->read_len);
	//printf("read len = %d\n",  jpg_data->read_len);
	gettimeofday( &end, NULL );
	WARDEBUG("read hw take time : %d ms\n", ( end.tv_sec - start.tv_sec ) *1000+ (end.tv_usec - start.tv_usec) / 1000);
	
	//meakboo_memcpy(jpg_data);
#if 0
	FILE *fd = fopen("src_data", "wb+");
	if(!fd)
	{
		printf("src_data fopen\n");
		return;
	}
	fwrite(jpg_data->data_buff, jpg_data->read_len, 1, fd);
	fclose(fd);
#endif
#if 0
	FILE *fd = fopen("src_data_128", "wb+");
	if(!fd)
	{
		printf("src_data_128 fopen\n");
		return;
	}
	printf("jpg_data->read_len = %d\n", jpg_data->read_len);
	for(i = 0; i < (jpg_data->read_len / 16); i++)
	{
		fprintf(fd , "%08x", *((u32 *)jpg_data->data_buff + i * 4));
		fprintf(fd , "%08x", *((u32 *)jpg_data->data_buff + i * 4 + 1));
		fprintf(fd , "%08x", *((u32 *)jpg_data->data_buff + i * 4 + 2));
		fprintf(fd , "%08x", *((u32 *)jpg_data->data_buff + i * 4 + 3));
		fprintf(fd, "\n");
	}	
	fclose(fd);
#endif
	gettimeofday( &start, NULL );
	//��Ҫȥ���߽�
	for(i = 0; i < jpg_data->height; i++)
	{
#ifndef USE_MY_MEMCPY
		memcpy(jpg_data->rgb_buff + cur_dst, jpg_data->data_buff + cur_src, jpg_data->width *4);
		cur_dst += jpg_data->width << 2;
#else
		jpg_my_memcpy(jpg_data->rgb_buff + cur_dst, jpg_data->data_buff + cur_src, jpg_data->width * 4);
		cur_dst += jpg_data->width * 3;
#endif
		//�൱��������width_block * 16 * 4���ֽ�
		cur_src += jpg_data->width_block * 64;	
	}
	gettimeofday( &end, NULL );
	WARDEBUG("3byte copy take time : %d ms\n", ( end.tv_sec - start.tv_sec ) *1000+ (end.tv_usec - start.tv_usec) / 1000);
	
#if 0
	FILE *test = fopen("txt_test", "a+");
	if(!test)
	{
		printf("fopen failed\n");
		return;
	}
	for(i = 0; i < (jpg_data->width * jpg_data->height); i++)
	{
		//fprintf(test, "%x%x%x\n", *((u8 *)jpg_data->rgb_buff + i * 3 + 2), *((u8 *)jpg_data->rgb_buff + i * 3 + 1), *((u8 *)jpg_data->rgb_buff + i * 3));
		fprintf(test, "%6x\n", *((u32 *)jpg_data->rgb_buff + i));
	}
	fclose(test);
#endif	
	return;
}

void sig_alarm(int num)
{
	//wait_done = 0;

	ERRDEBUG("BUG\n");

	FILE *fd = fopen("bug.jpg", "wb+");
	if(!fd)
	{
		ERRDEBUG("signal func fopen bug_rgb failed\n");
		return;
	}
	fwrite(temp_1, bug_size, 1, fd);
	fclose(fd);

	fd = NULL;

	system("tftp -r bug.jpg -l bug.jpg -p 192.168.150.162");




	
	
	return;
}

/*******************************************************************************
��������: jpeg���뺯��
�������: jpg_data_p jpg_data : jpeg���ݽṹ��
�������: ��
����ֵ��: ��
------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void jpg_soc_decode(jpg_data_p jpg_data)
{
	struct timeval start, end;

	//��ȡͼƬ�����Լ�ͼƬ����
	gettimeofday( &start, NULL );
	jpg_get_file(jpg_data);
	gettimeofday( &end, NULL );
	DEBUG("get file take time : %d ms\n", ( end.tv_sec - start.tv_sec ) *1000+ (end.tv_usec - start.tv_usec) / 1000);
	//����DHT��DQT��Huffman Table���ݵ�ָ�������ڴ�
	gettimeofday( &start, NULL );
	jpg_set_header_data(jpg_data);
	gettimeofday( &end, NULL );
	DEBUG("set header take time : %d ms\n", ( end.tv_sec - start.tv_sec ) *1000+ (end.tv_usec - start.tv_usec) / 1000);
	//FPGA�Ĵ�������
	gettimeofday( &start, NULL );
	jpg_fpga_cfg(jpg_data);
	gettimeofday( &end, NULL );
	DEBUG("fpga cfg take time : %d ms\n", ( end.tv_sec - start.tv_sec ) *1000+ (end.tv_usec - start.tv_usec) / 1000);
	//�ȴ��������
	gettimeofday( &start, NULL );
	jpg_wait_done(jpg_data);
	gettimeofday( &end, NULL );
	DEBUG("decode jpg take time : %d ms\n", ( end.tv_sec - start.tv_sec ) *1000+ (end.tv_usec - start.tv_usec) / 1000);
	//����û�гɹ��Ͳ��ñ���
	if(jpg_data->status != DECODE_SUCCSE)
		return;
	//��ȡ����ɹ���ͼƬ
	gettimeofday( &start, NULL );
	jpg_read_xrgb(jpg_data);
	gettimeofday( &end, NULL );
	DEBUG("read jpg take time : %d ms\n", ( end.tv_sec - start.tv_sec ) *1000+ (end.tv_usec - start.tv_usec) / 1000);
#if 0
	//���������ͼ��
	gettimeofday( &start, NULL );
	jpg_save_image(jpg_data);
	gettimeofday( &end, NULL );
	DEBUG("save jpg take time : %d ms\n", ( end.tv_sec - start.tv_sec ) *1000+ (end.tv_usec - start.tv_usec) / 1000);
#endif	
	DEBUG("decode done\n");

	return;
}

/*******************************************************************************
��������: ����������
�������: int    argc : ����������������
        char **argv : ������������
�������: ��
����ֵ��:�ɹ�:0 ʧ��:1
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
int main(int argc, char**argv)
{
    jpg_data_p jpg_data;
	u32 ret = 0;
	u32 i = 0;
	u8 file_name[64];
	u8 system_str[64];
	struct timeval start, end;
#if 1
    //�ж��������
    if (2 != argc)
    {
        ERRDEBUG("usage :  ./a.out  test.jpg\n");
        return 1;
    }
#endif
	i = atoi(argv[1]);

	signal(SIGALRM, sig_alarm); 

    //��ʼ����Ҫ����Դ
    if((jpg_data= jpg_decompress_init()) == NULL)
    {
        ERRDEBUG("jpg decompress init failed\n");
        return 1;
    }
#if 1
	for(; i < 9000; i++)
	{
		memset(file_name, 0, sizeof(file_name));
		memset(system_str, 0, sizeof(system_str));
		if(i < 1000)
			sprintf(file_name, "00000%d.jpg", i);
		else
			sprintf(file_name, "0000%d.jpg", i);
		
		sprintf(system_str, "tftp -r %s -l %s -g 192.168.150.162", file_name, file_name);
		printf("%s \n", system_str);
		system(system_str);
		usleep(50);
		if(0 != access(file_name, F_OK))
		{
			printf("cann't find %s\n", file_name);
			continue;
		}
		gettimeofday( &start, NULL );
		//��jpg�ļ�
		if(RET_OK != jpg_open_file(jpg_data, file_name))
		{
			ERRDEBUG("open jpeg file failed\n");
			continue;
		}
	    //��ȡjpg�ļ�ͷ
	    if(RET_OK != jpg_get_header(jpg_data))
	    {
	        ERRDEBUG("jpg get header failed\n");
	        continue;
	    }
		gettimeofday( &end, NULL );
		DEBUG("get header take time : %d ms\n", ( end.tv_sec - start.tv_sec ) *1000+ (end.tv_usec - start.tv_usec) / 1000);
	    //jpg_debug(jpg_data, argv[1]);
		//DEBUG("high = %d width = %d\n", jpg_data->height, jpg_data->width);

		jpg_hw_res(jpg_data);
		//��FPGA����
		jpg_soc_decode(jpg_data);

		jpg_data->status = HEADER_START;

		fclose(jpg_data->fd);
		jpg_data->fd = NULL;
		jpg_data->sof_info.saw_sof_marker = FALSE;

		memset(system_str, 0, sizeof(system_str));
		sprintf(system_str, "rm %s", file_name);
		system(system_str);
		
	}
#endif
	//�ͷ���Դ
	munmap(jpg_data->jpg_output_addr, JPEG_IMAGE_MMAP_LEN);
	munmap(jpg_data->jpg_input_addr, JPEG_IMAGE_MMAP_LEN);
	munmap(jpg_data->reg, JPEG_SOC_LEN);
	close(jpg_data->mem_fd);
	
	free(jpg_data);
	
    return 0;
}
