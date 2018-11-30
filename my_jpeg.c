#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "my_jpeg.h"
#include "jpg_header.h"


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
	jpg_data->reg->jpeg_cfg &= ~JPEG_RESET_ALL;

	jpg_data->reg->jpeg_cfg |= 1 << 1;
	jpg_data->reg->jpeg_cfg |= 1 << 2;
	jpg_data->reg->jpeg_cfg |= 1 << 3;
	
	sleep(1);

	jpg_data->reg->jpeg_cfg &= ~(1 << 1);
	jpg_data->reg->jpeg_cfg &= ~(1 << 2);
	jpg_data->reg->jpeg_cfg &= ~(1 << 3);

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
void jpg_open_file(jpg_data_p jpg_data, char *file_name)
{
	struct stat statbuf;

	//��ȡ�ļ���С
	stat(file_name, &statbuf);
	jpg_data->file_size = statbuf.st_size;
	
    jpg_data->fd = fopen(file_name, "r");
    if (jpg_data->fd == NULL)
    {
        ERRDEBUG("fopen failed\n");
        return;
    }
    
	return;
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
	void  *temp = 0;
	u32   cp_size = 0;
	int i = 0;
	
	temp = malloc(JPEG_IMAGE_MMAP_LEN);
	if(!temp)
	{
		ERRDEBUG("malloc failed\n");
		return ;
	}
	//memcpy�����ڴ�8�ֽڶ��룬�����Ϊʲô
	cp_size = (jpg_data->file_size %8)?(jpg_data->file_size + (8 - jpg_data->file_size % 8)):jpg_data->file_size;
	//��ȡ�ļ�����
	fseek(jpg_data->fd, 0, SEEK_SET);
	fread(temp, jpg_data->file_size, 1, jpg_data->fd);
	//�������ڴ濽��
	memcpy(jpg_data->jpg_input_addr , temp, cp_size);
	
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

	tmp = (jpg_data->file_size % 128)?((jpg_data->file_size /128 + 1) * 128):jpg_data->file_size;
	tmp -= 16;

	printf("tmp = %d\n", tmp);

	//��ȡ��Ҫ���ĳ���
	jpg_data->width_block = (jpg_data->width % 16)?(jpg_data->width /16 + 1):(jpg_data->width /16);
	jpg_data->height_block = (jpg_data->height % 16)?(jpg_data->height /16 + 1):(jpg_data->height /16);
	jpg_data->read_len = jpg_data->width_block * jpg_data->height_block * 256 * 4;
	
	//reset FPGA
	//jpg_hw_res(jpg_data);
	//���ÿ�߼Ĵ���
	jpg_data->reg->jpeg_high_width = ((jpg_data->height << 16) | jpg_data->width);
	//����FPGA����ַ
	jpg_data->reg->jpeg_rd_addr_low = JPEG_RESERV_MEM;
	jpg_data->reg->jpeg_rd_addr_high = JPEG_RESERV_MEM + tmp;
	//����FPGAд��ַ
	jpg_data->reg->jpeg_wr_addr_low = JPEG_RESERV_MEM + JPEG_IMAGE_MMAP_LEN;
	jpg_data->reg->jpeg_wr_addr_high = JPEG_RESERV_MEM + JPEG_IMAGE_MMAP_LEN + jpg_data->read_len;
	//����Ϊ����ģʽ
	//jpg_data->reg->jpeg_cfg |= JPEG_NORMAL_MODE;
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
	int done = 0;

	while(!done)
		done = jpg_data->reg->jpeg_status & JPEG_DECODE_DONE;
		
	DEBUG("jpg decode done\n");

	jpg_data->reg->jpeg_cfg |= (1 << 3);
	sleep(1);
	jpg_data->reg->jpeg_cfg &= ~(1 << 3);

	return;
}

/*******************************************************************************
��������: DEBUG��������ӡchar�������ݣ���������ļ�
�������: u8 *data �� ����
		u32 len �����ݸ���
		u8 *name �� Ҫ����ɵ��ļ���
�������: ��
����ֵ��: ��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void jpg_table_printf_c(u8 *data, u32 len, u8 *name)
{
	u32 i = 0;
#if 0
	for(i = 1; i <= len; i++)
	{
		printf("%x  ", *((u8 *)data + i - 1));
		if(i % 5 == 0)
			printf("\n");
	}
	printf("\n");
#endif
	FILE *fd = fopen(name, "a+");
	if(fd == NULL)
	{
		ERRDEBUG("fopen err\n");
		return;
	}
	for(i = 0; i < len; i++)
		fprintf(fd, "%x\n", *((u8 *)data + i));
	fclose(fd);
	
	return;
}

/*******************************************************************************
��������: DEBUG��������ӡshort�������ݣ���������ļ�
�������: u8 *data �� ����
		u32 len �����ݸ���
		u8 *name �� Ҫ����ɵ��ļ���
�������: ��
����ֵ��: ��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void jpg_table_printf_s(u16 *data, u32 len, u8 *name)
{
	u32 i = 0;
#if 0
	for(i = 1; i < len; i++)
	{
		printf("%x  ", *((u16 *)data + i - 1));
		if(i % 5 == 0)
			printf("\n");
	}
	printf("\n");
#endif
	FILE *fd = fopen(name, "a+");
	if(fd == NULL)
	{
		ERRDEBUG("fopen err\n");
		return;
	}
	for(i = 0; i < len; i++)
		fprintf(fd, "%x\n", *((u16 *)data + i));
	fclose(fd);

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
	jpg_table_printf_s(jpg_data->dht_dc_info[0]->table_ht, 16, "ht_t.txt");
	jpg_table_printf_s(jpg_data->dht_ac_info[0]->table_ht, 16, "ht_t.txt");
	jpg_table_printf_s(jpg_data->dht_dc_info[1]->table_ht, 16, "ht_t.txt");
	jpg_table_printf_s(jpg_data->dht_ac_info[1]->table_ht, 16, "ht_t.txt");

	jpg_table_printf_c(jpg_data->dht_dc_info[0]->table_hn, 16, "hn_t.txt");
	jpg_table_printf_c(jpg_data->dht_ac_info[0]->table_hn, 16, "hn_t.txt");
	jpg_table_printf_c(jpg_data->dht_dc_info[1]->table_hn, 16, "hn_t.txt");
	jpg_table_printf_c(jpg_data->dht_ac_info[1]->table_hn, 16, "hn_t.txt");
#endif	
	jpg_debug_hwmem(jpg_data->dht1_addr, 16, 1, "dht1.txt");
	jpg_debug_hwmem(jpg_data->dht2_addr, 256, 1, "dht2.txt");
	jpg_debug_hwmem(jpg_data->dht3_addr, 16, 1, "dht3.txt");
	jpg_debug_hwmem(jpg_data->dht4_addr, 256, 1, "dht4.txt");
	jpg_debug_hwmem(jpg_data->dqt_addr, 128, 1, "dqt.txt");
	jpg_debug_hwmem(jpg_data->huffmantable_addr, 64, 4, "hn_ht.txt");
	
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
	fwrite(jpg_data->rgb_buff, write_len, 1, fd);

	fclose(fd);

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

	//�ȴ�OUT buff������Ҫ���������ݶ����������������д���������������Ҫɾ��
	memcpy(jpg_data->data_buff, jpg_data->jpg_output_addr, jpg_data->read_len);

	FILE *fd = fopen("src_data", "wb+");
	if(!fd)
	{
		printf("src_data fopen\n");
		return;
	}

	fwrite(jpg_data->data_buff, jpg_data->read_len, 1, fd);
	fclose(fd);

	
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
	int i = 0;

	//��ȡͼƬ�����Լ�ͼƬ����
	jpg_get_file(jpg_data);

	//����DHT��DQT��Huffman Table���ݵ�ָ�������ڴ�
	jpg_set_header_data(jpg_data);

	//FPGA�Ĵ�������
	jpg_fpga_cfg(jpg_data);
	//�ȴ��������
#if 1
	jpg_wait_done(jpg_data);
	//��ȡ����ɹ���ͼƬ
	jpg_read_xrgb(jpg_data);
	//���������ͼ��
	jpg_save_image(jpg_data);
#endif	
	DEBUG("decode done\n");

	return;
}

/*******************************************************************************
��������: ����������
�������: int argc : ����������������
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

    //�ж��������
    if (2 != argc)
    {
        ERRDEBUG("usage :  ./a.out  test.jpg\n");
        return 1;
    }

    //��ʼ����Ҫ����Դ
    if((jpg_data= jpg_decompress_init()) == NULL)
    {
        ERRDEBUG("jpg decompress init failed\n");
        return 1;
    }
	//��jpg�ļ�
	jpg_open_file(jpg_data, argv[1]);
    //��ȡjpg�ļ�ͷ
    if(RET_OK != jpg_get_header(jpg_data))
    {
        ERRDEBUG("jpg get header failed\n");
        return 1;
    }
#if 1
    //jpg_debug(jpg_data, argv[1]);
	DEBUG("high = %d width = %d\n", jpg_data->height, jpg_data->width);
#endif
	//��FPGA����
#ifdef JPEG_SOC
	jpg_soc_decode(jpg_data);
	//�ͷ���Դ
	munmap(jpg_data->jpg_output_addr, JPEG_IMAGE_MMAP_LEN);
	munmap(jpg_data->jpg_input_addr, JPEG_IMAGE_MMAP_LEN);
	munmap(jpg_data->reg, JPEG_SOC_LEN);
	close(jpg_data->mem_fd);
#endif
    fclose(jpg_data->fd);
	free(jpg_data);
	
    return 0;
}
