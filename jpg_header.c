#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my_jpeg.h"
#include "jpg_header.h"
#include "jpg_data.h"

/*******************************************************************************
��������: ���ļ��л�ȡ�����ֽ�
�������: jpg_data_s jpg_data : jpg���ݽṹ��ָ�� 
                   char *buff : ����Buff
                   int len : ��ȡ����
�������: ��
����ֵ��:�ɹ�:0 ʧ��:1
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
u32 get_byte(jpg_data_p jpg_data, u8 *buff, u32 len, u32 flag)
{
    int  ret = 0;
    char buff_tmp[len];
    int  i = 0;

    if (NULL == buff)
    {
        ERRDEBUG("buff  is empty\n");
        exit(1);
    }
    ret = fread(buff_tmp, len, 1, jpg_data->fd);
    if (ret == 1)
    {
        if (DATA_TYPE == flag) 
        {
            //fread�����������ݣ���λ�ڸߵ�ַ��������������ͣ�����Ҫת������Ȼ��������ƴ�С�˵�����
            for (i = 0; i < len ; i++) 
            {
                buff[i] = buff_tmp[len - i - 1];
            }
        }
        else
        {
            //��������������ͣ������Ƕ����ɳ��ȣ�����Ҫת��
            for (i = 0; i < len ; i++) 
            {
                buff[i] = buff_tmp[i];
            }
        }
        return RET_OK;
    }
    else
    {
        ERRDEBUG("fread err: \n");
        return RET_ERR;
    }
}

/*******************************************************************************
��������: ��ȡ�ļ�ǰ�����ֽ�
�������: jpg_data_s jpg_data : jpg���ݽṹ��ָ�� 
�������: ��
����ֵ��:��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void get_soi( jpg_data_p  jpg_data)
{
    u8 field_first = 0;
    u8 field_next = 0;

    get_byte(jpg_data, &field_first, sizeof(u8),DATA_TYPE);
    get_byte(jpg_data, &field_next, sizeof(u8), DATA_TYPE);
    if (field_first != MARKER_FLAG || field_next != MARKER_SOI)
    {
        ERRDEBUG("jpg file SOI flag err\n");
        jpg_data->status = HEADER_ERR;
        return;
    }
    jpg_data->status = HEADER_FIND_SOI;

    return;
}

/*******************************************************************************
��������: ��ȡ��һ���ֶε�������
�������: jpg_data_s jpg_data : jpg���ݽṹ��ָ�� 
�������: ��
����ֵ��:��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void get_next(jpg_data_p  jpg_data)
{
    u8  field_first = 0;
    u32 fill_num = 0;

    while (1)
    {    
        get_byte(jpg_data, &field_first, sizeof(u8), DATA_TYPE);

        while (MARKER_FLAG != field_first)
        {
            get_byte(jpg_data, &field_first, sizeof(u8), DATA_TYPE);
        }

        do {
            get_byte(jpg_data, &field_first, sizeof(u8), DATA_TYPE);
        }while (MARKER_FLAG == field_first);
        if (field_first != 0)
        {
            break;
        }
    }
    if (fill_num != 0)
    {
        WARDEBUG("fill num = %d\n",  fill_num);
    }
    jpg_data->status = field_first;

    return;
}

/*******************************************************************************
��������: ��ȡAPP0�ֶε���Ϣ
�������: jpg_data_s jpg_data : jpg���ݽṹ��ָ�� 
                   u8 *buff : �������ݵ�buff
                   u16 inter_len : ����Ȥ�ĳ���(buff�е����ݳ���)
                   u16 all_len : �ֶ��ܳ���
�������: ��
����ֵ��:��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void get_app0(jpg_data_p jpg_data, u8 *buff, u16 inter_len, u16 all_len)
{
    int thumbnail_len = 0;
    if (inter_len >= APP0_DATA_LEN && buff[0] == 0x4A && buff[1] == 0x46&&\
        buff[2] == 0x49 && buff[3] == 0x46 && buff[4] == 0x00)
    {
        jpg_data->app0_E_info.saw_jfif_marker = TRUE;
        jpg_data->app0_E_info.jfif_major_version = buff[5];
        jpg_data->app0_E_info.jfif_minor_version = buff[6];
        jpg_data->app0_E_info.density_unit = buff[7];
        jpg_data->app0_E_info.x_density = (buff[8] << 8) + buff[9];
        jpg_data->app0_E_info.y_density = (buff[10] << 8) + buff[11];
        if (jpg_data->app0_E_info.jfif_major_version != 1 && jpg_data->app0_E_info.jfif_major_version != 2)
        {
            WARDEBUG("Warning: unknown JFIF revision number  %d.%02d\n",\
                     jpg_data->app0_E_info.jfif_major_version,jpg_data->app0_E_info.jfif_minor_version);
        }
        //�������ͼ���ز�Ϊһ
        if (buff[12] | buff[13])
        {
            thumbnail_len = buff[12] * buff[13] * 3;
            all_len -= APP0_DATA_LEN;
            if (all_len != thumbnail_len)
            {
                WARDEBUG("Warning: thumbnail image size does not match data length\n");
                exit(1);
            }
        }
    }
    else if (inter_len >= 6 && buff[0] == 0x4A && buff[1] == 0x46 && buff[2] == 0x58 && buff[3] == 0x58 && buff[4] == 0x00)
    {
        WARDEBUG("Found JFIF -- JFXX  externsion APP0 marker , i don't actually do anything with these\n");
    }
    else
    {
        WARDEBUG("Unknown APP0 marker (not JFIF)\n");
    }

    return;
}

/*******************************************************************************
��������: �������õ��ֽ�
�������: jpg_data_s jpg_data : jpg���ݽṹ��ָ�� 
                   u16 inter_len : ���õ����ݳ���
�������: ��
����ֵ��:��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void skip_byte(jpg_data_p jpg_data, int len)
{
    u8   c;
    u32  i = 0;

    for (i = 0; i < len ; i++)
    {
        get_byte(jpg_data, &c, sizeof(u8), DATA_TYPE);
    }

    return;
}

/*******************************************************************************
��������: �����ݲ�֧�ֵ��ֶ�
�������: jpg_data_s jpg_data : jpg���ݽṹ��ָ�� 
�������: ��
����ֵ��:��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void skip_marker(jpg_data_p jpg_data)
{
    u16 len = 0;
    u8   c;
    u32 i = 0;

    get_byte(jpg_data, (u8 *)(&len), sizeof(u16), DATA_TYPE);

    len -= 2;
    if (len > 0)
    {
        for (i = 0; i < len; i++)
        {
            get_byte(jpg_data, &c, sizeof(u8), NO_DATA_TYPE);
        }
    }
    WARDEBUG("Miscellaneous marker  0x%02X\n", jpg_data->status);

    return;
}

/*******************************************************************************
��������: ��ȡAPP14�ֶε���Ϣ
�������: jpg_data_s jpg_data : jpg���ݽṹ��ָ�� 
                   u8 *buff : �������ݵ�buff
                   u16 inter_len : ����Ȥ�ĳ���(buff�е����ݳ���) 
�������: ��
����ֵ��:��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void get_app14(jpg_data_p jpg_data, u8 *buff, u16 inter_len)
{
    if (inter_len >= APP14_DATA_LEN && buff[0] == 0x41 && buff[1] == 0x64 &&\
        buff[2] == 0x6F && buff[3] == 0x62 && buff[4] == 0x65)
    {
        //found Adobe APP14 marker
        jpg_data->app0_E_info.saw_jfif_marker = TRUE;
        jpg_data->app0_E_info.adobe_transform = buff[11];
    }
    else
    {
        WARDEBUG("Unknown APP14 marker (not Adobe)\n");
    }

    return;
}

/*******************************************************************************
��������: ��ȡAPP0��14�ֶ��е�����
�������: jpg_data_s jpg_data : jpg���ݽṹ��ָ�� 
�������: ��
����ֵ��:��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void get_app0_or_14(jpg_data_p  jpg_data)
{
    s16 len = 0;
    u16 read_len = 0;
    u8  buff[APP0_DATA_LEN] = {0};

    get_byte(jpg_data, (u8 *)(&len), sizeof(u16), DATA_TYPE);
    //��ȥlen�����2���ֽ�
    len -= 2;
    //ֻ��ȡǰ14���ֽ�
    if (len > APPN_DATA_LEN)
    {
        read_len = APPN_DATA_LEN;
    }
    else if (len > 0) 
    {
        read_len = len;
    }
    else
    {
        read_len = 0;
    }

    get_byte(jpg_data, buff, read_len, NO_DATA_TYPE);

    if (jpg_data->status == MARKER_APP0) 
    {
        get_app0(jpg_data, buff, read_len, len);
    }
    else
    {
        get_app14(jpg_data, buff, read_len);
    }
    len -= read_len;
    //������ܻ�������ͼ����,������Щ���������õ�
    if (len > 0)
    {
        //�������õ�����
        skip_byte(jpg_data, len);
    }

    return;
}

/*******************************************************************************
��������: ��ȡ������
�������: jpg_data_s jpg_data : jpg���ݽṹ��ָ�� 
�������: ��
����ֵ��:��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void get_dqt(jpg_data_p  jpg_data)
{
    u16 len = 0;
    u8    c, prec, tmp_8;
    u16 tmp_16;
    u32  i, count;
    dqt_info_p dqt_info = NULL;;
    const u32 *natural_order;

    get_byte(jpg_data, (u8 *)(&len), sizeof(u16), DATA_TYPE);
    len -= 2;

    while (len > 0)
    {
        get_byte(jpg_data, &c, sizeof(u8), DATA_TYPE);
        prec = c >> 4;      //QT���� 0 = 1�ֽ�  ���� = 2�ֽ�
        c &= 0x0F;             //QT��
        len -= 1;

        //IDΪ0,�����������;IDΪ�������ɫ��

        //���QT����4,�򱨴��˳�
        if (c >= NUM_QUANT_TBLS)
        {
            ERRDEBUG("Bogus DQT index \n");
            exit(1);
        }

        if (jpg_data->dqt_info[c] == NULL) 
        {
            jpg_data->dqt_info[c] = (dqt_info_p)malloc(sizeof(struct dqt_info_s));
            if (jpg_data->dqt_info[c] == NULL)
            {
                ERRDEBUG("malloc failed\n");
                exit(1);
            }
        }
        dqt_info = jpg_data->dqt_info[c];
        dqt_info->prec = prec;
        dqt_info->dqt_id = c ;

        if (prec)
        {
            if (len < DCTSIZE2 * 2)
            {
                for (i = 0; i < DCTSIZE2; i++)
                {
                    dqt_info->quantbal[i] = 1;
                }
                count = len >> 1;
            }
            else
            {
                count = DCTSIZE2;
            }
        }
        else
        {
            if (len < DCTSIZE2)
            {
                for (i = 0; i < DCTSIZE2; i++)
                {
                    dqt_info->quantbal[i] = 1;
                }
                count = len;
            }
            else
            {
                count = DCTSIZE2;
            }
        }

        switch (count)
        {
        case (2 * 2):
            natural_order = jpeg_natural_order2;
            break;
        case (3 * 3):
            natural_order = jpeg_natural_order3;
            break;
        case (4 * 4):
            natural_order = jpeg_natural_order4;
            break;
        case (5 * 5):
            natural_order = jpeg_natural_order5;
            break;
        case (6 * 6):
            natural_order = jpeg_natural_order6;
            break;
        case (7 * 7):
            natural_order = jpeg_natural_order7;
            break;
        default:
            natural_order = jpeg_natural_order;
            break;
        }

		DEBUG("count = %d\n", count);

        for (i = 0; i < count; i++)
        {
            if (prec)
            {
                get_byte(jpg_data, (u8 *)(&tmp_16), sizeof(u16), DATA_TYPE);
                dqt_info->quantbal[natural_order[i]] = tmp_16;
            }
            else
            {
                get_byte(jpg_data, &tmp_8, sizeof(u8), DATA_TYPE);
				dqt_info->table_dqt[i] = tmp_8;
                dqt_info->quantbal[natural_order[i]] = tmp_8;
            }
        }

        len -= count;
        if (prec)
        {
            len -= count;
        }
        dqt_info->found_flag = TRUE;
    }
    if (len != 0)
    {
        ERRDEBUG("Bogus marker length\n");
        exit(1);
    }


    return;
}

/*******************************************************************************
��������: ��ȡ֡��ʼ������Ϣ
�������: jpg_data_s  *jpg_data :jpg���ݽṹ��ָ�� �Ķ���ָ��
�������: ��
����ֵ��:�ɹ�:RET_OK ʧ��:RET_ERR
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void get_sof(jpg_data_p  jpg_data, u8 is_baseline, u8 is_prog, u8 is_arith)
{
    u16 len = 0;
    u8   c;
    u32 i, j;
    struct components_info_s  *components_info;

    jpg_data->jpg_mode.is_baseline = is_baseline;
    jpg_data->jpg_mode.progressive_mode = is_prog;
    jpg_data->jpg_mode.arith_code = is_arith;

    get_byte(jpg_data, (u8 *)(&len), sizeof(u16), DATA_TYPE);
    len -= 2;

    get_byte(jpg_data, (u8 *)(&(jpg_data->data_precision)),sizeof(u8),  DATA_TYPE);
    get_byte(jpg_data, (u8 *)(&(jpg_data->height)), sizeof(u16),  DATA_TYPE);
    get_byte(jpg_data, (u8 *)(&(jpg_data->width)), sizeof(u16),  DATA_TYPE);
    get_byte(jpg_data, (u8 *)(&(jpg_data->num_components)), sizeof(u8),  DATA_TYPE);
    len -= 6;

    if (jpg_data->sof_info.saw_sof_marker == TRUE)
    {
        ERRDEBUG("Invalid JPEG file structure: two SOF markers\n");
        exit(1);
    }
    if (jpg_data->height <= 0 || jpg_data->width <= 0 || jpg_data->num_components <= 0)
    {
        ERRDEBUG("Image is empty\n");
        exit(1);
    }
    if (len != jpg_data->num_components * 3)
    {
        ERRDEBUG("Bogus marker length\n");
        exit(1);
    }
    if (jpg_data->sof_info.components_info == NULL)
    {
        jpg_data->sof_info.components_info = (struct components_info_s  *)malloc(sizeof(struct components_info_s) * jpg_data->num_components);
        if (jpg_data->sof_info.components_info == NULL)
        {
            ERRDEBUG("malloc failed\n");
            exit(1);
        }
    }
    for (j = 0; j < jpg_data->num_components; j++)
    {
        get_byte(jpg_data, &c, sizeof(u8), DATA_TYPE);

        for (i = 0,  components_info = jpg_data->sof_info.components_info; i < j; i++, components_info++)
        {
            /*��ʱ�����������ID����ͬ��,��Ȼ��������ǲ�����Э���,����ȷʵ�ᷢ��*/
            if (c == components_info->component_id)
            {
                components_info = jpg_data->sof_info.components_info;
                c = components_info->component_id;
                components_info++;
                for (i = 1; i < j; i++, components_info++)
                {
                    if (components_info->component_id > c)
                    {
                        c = components_info->component_id;
                    }
                }
                c++;
                break;
            }
        }
        components_info->component_id = c;
        components_info->component_index = j;
        get_byte(jpg_data, &c, sizeof(u8), DATA_TYPE);
        components_info->h_samp_factor = (c >> 4) & 0xF;
        components_info->v_samp_factor = c  & 0xF;
        get_byte(jpg_data, &(components_info->quant_tbl_no), sizeof(u8), DATA_TYPE);
    }
    jpg_data->sof_info.saw_sof_marker = TRUE;

    return;
}

/*******************************************************************************
��������: ��ȡDAC�������ڱ�
�������: jpg_data_s  *jpg_data :jpg���ݽṹ��ָ�� �Ķ���ָ��
�������: ��
����ֵ��:�ɹ�:RET_OK ʧ��:RET_ERR
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void get_dac(jpg_data_p  jpg_data)
{
    //��ʱ��֧�ֻ�ȡDAC����
    skip_marker(jpg_data);

    return;
}

/*******************************************************************************
��������: ��ȡ���¿�ʼ���
�������: jpg_data_s  *jpg_data :jpg���ݽṹ��ָ�� �Ķ���ָ��
�������: ��
����ֵ��:�ɹ�:RET_OK ʧ��:RET_ERR
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void get_dri(jpg_data_p  jpg_data)
{
    u16 len = 0;
    get_byte(jpg_data, (u8 *)(&len), sizeof(u16), DATA_TYPE);

    if (len != 4)
    {
        ERRDEBUG("Bogus marker length\n");
        exit(1);
    }
    get_byte(jpg_data, (u8 *)(&(jpg_data->restart_interval)) , sizeof(u16), DATA_TYPE);

    return;
}

/*******************************************************************************
��������: ��ȡ��ɨ����Ϣ
�������: jpg_data_s  *jpg_data :jpg���ݽṹ��ָ�� �Ķ���ָ��
�������: ��
����ֵ��:�ɹ�:RET_OK ʧ��:RET_ERR
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void get_sos(jpg_data_p  jpg_data)
{
    u16 len;
    u8    n, c , i, j;
    struct components_info_s  *components_info;

    if (jpg_data->sof_info.saw_sof_marker != TRUE)
    {
        ERRDEBUG("Invalid JPEG file structure SOS before SOF");
        exit(1);
    }
    get_byte(jpg_data, (u8 *)(&len), sizeof(u16), DATA_TYPE);
    get_byte(jpg_data,&n, sizeof(u8), DATA_TYPE);

    if (len != (n * 2 + 6) || n > MAX_COMPS_IN_SCAN || (n == 0 && jpg_data->jpg_mode.progressive_mode != TRUE))
    {
        ERRDEBUG("Bogus marker length\n");
        exit(1);
    }

    jpg_data->comps_in_scan = n;

    for (i = 0; i < n; i++)
    {
        get_byte(jpg_data, &c, sizeof(u8), DATA_TYPE);
        for (j = 0; j < i; j++)
        {
            if (c == jpg_data->sos_info.cur_components_info[j]->component_id)
            {
                c =  jpg_data->sos_info.cur_components_info[0]->component_id;
                for (j = 1; j < i; j++)
                {
                    components_info = jpg_data->sos_info.cur_components_info[j];
                    if (components_info->component_id > c)
                    {
                        c = components_info->component_id;
                    }
                }
                c++;
                break;
            }
        }

        for (j = 0, components_info = jpg_data->sof_info.components_info; j < jpg_data->num_components; j++, components_info++)
        {
            if (c == components_info->component_id)
            {
                goto id_found;
            }
        }

        ERRDEBUG("Invalid component ID %d in SOS\n",c );

id_found:
        jpg_data->sos_info.cur_components_info[i] = components_info;
       get_byte(jpg_data, &c, sizeof(u8), DATA_TYPE);
       components_info->dc_tbl_no = (c >> 4) & 0x0F;
       components_info->ac_tbl_no = c & 0x0F;
    }

    get_byte(jpg_data, &c, sizeof(u8), DATA_TYPE);
    jpg_data->ss = c;
    get_byte(jpg_data, &c, sizeof(u8), DATA_TYPE);
    jpg_data->se = c;
    get_byte(jpg_data, &c, sizeof(u8), DATA_TYPE);
    jpg_data->ah = (c >> 4) & 0x0F;
    jpg_data->al  = c & 0x0F;

    return;
}

/*******************************************************************************
��������: ��ȡ��������
�������: jpg_data_s  *jpg_data :jpg���ݽṹ��ָ�� �Ķ���ָ��
�������: ��
����ֵ��:�ɹ�:RET_OK ʧ��:RET_ERR
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void get_dht(jpg_data_p  jpg_data)
{
    u16 len = 0;
    u16 count = 0;
    u8    index = 0;
    u8    bits[16];
    u8    huffval[256];
	u16 huffmandata = 0;
	u16 shiftdata = 0x8000;
    u32 i;
	u8 tmp = 0;
    dht_info_p dht_info = NULL;

    get_byte(jpg_data, (u8 *)(&len), sizeof(u16), DATA_TYPE);
    len -= 2;

    while (len > 16)
    {
        get_byte(jpg_data, &index, sizeof(u8), DATA_TYPE);
        count = 0;

        memset(bits, 0, sizeof(bits));
        memset(huffval, 0, sizeof(huffval));

		if (index & HUFF_AC_TABLE)
        {
            //ΪAC��
            index -= HUFF_AC_TABLE;
            if (jpg_data->dht_ac_info[index] == NULL)
            {
                jpg_data->dht_ac_info[index] = (struct dht_info_s * )malloc(sizeof(struct dht_info_s));
                if (jpg_data->dht_ac_info[index] == NULL)
                {
                    ERRDEBUG("malloc failed\n");
                    exit(1);
                }
            }
            dht_info = jpg_data->dht_ac_info[index] ;
        }
        else
        {
            //ΪDC��
            if (jpg_data->dht_dc_info[index] == NULL)
            {
                jpg_data->dht_dc_info[index] = (struct dht_info_s * )malloc(sizeof(struct dht_info_s));
                if (jpg_data->dht_dc_info[index] == NULL)
                {
                    ERRDEBUG("malloc failed\n");
                    exit(1);
                }
            }
            dht_info = jpg_data->dht_dc_info[index] ;
        }
     
        for (i = 0; i < 16; i++)
        {
            get_byte(jpg_data, &(bits[i]), sizeof(u8), DATA_TYPE);
            tmp = bits[i];
			dht_info->table_ht[i] = huffmandata;
			dht_info->table_hn[i] = count;
			
            count += bits[i];
			while(!(tmp == 0))
			{
				huffmandata += shiftdata;
				tmp--;
			}
			shiftdata = shiftdata >> 1;
        }
        len -= 17;
        //count����С��256
        if (count > 256 || count > len)
        {
            ERRDEBUG("Bogus Huffman table definition");
            exit(1);
        }
        
        for (i = 0; i < count; i++)
        {
            get_byte(jpg_data, &(huffval[i]), sizeof(u8), DATA_TYPE);
        }
        len -= count;
        //�����������Ҫֻ����0~3
        if ((index & 0x0F) < 0 || (index & 0x0F) > NUM_HUFF_TABLE)
        {
            ERRDEBUG("Bogus DHT index");
            exit(1);
        }

        
        dht_info->flag = TRUE;
        memcpy(dht_info->bits, bits, sizeof(bits));
        memcpy(dht_info->huffval, huffval, sizeof(huffval));

    }
    if (len != 0)
    {
        ERRDEBUG("Bogus marker length\n");
        exit(1);
    }

    return;
}

/*******************************************************************************
��������: ��ȡjpg�ļ�ͷ
�������: jpg_data_s  *jpg_data :jpg���ݽṹ��ָ�� �Ķ���ָ��
�������: ��
����ֵ��:�ɹ�:RET_OK ʧ��:RET_ERR
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
u32 jpg_get_header( jpg_data_p  jpg_data)
{
    u32 ret = 0;

    while (1)
    {
        if (jpg_data->status == HEADER_START) 
        {
            get_soi(jpg_data);
            if (jpg_data->status == HEADER_ERR)
            {
                return RET_ERR;
            }
        }
        else
        {
            get_next(jpg_data);
        }

        switch (jpg_data->status)
        {
        case MARKER_APP0:
        case MARKER_APP14:
            get_app0_or_14(jpg_data);
            break;
        case MARKER_APP1:
        case MARKER_APP2:
        case MARKER_APP3:
        case MARKER_APP4:
        case MARKER_APP5:
        case MARKER_APP6:
        case MARKER_APP7:
        case MARKER_APP8:
        case MARKER_APP9:
        case MARKER_APP10:
        case MARKER_APP11:
        case MARKER_APP12:
        case MARKER_APP13:
        case MARKER_APP15:
             skip_marker(jpg_data);
            break;
        case MARKER_DQT:
            get_dqt(jpg_data);
            break;
        case MARKER_SOF0:
            get_sof(jpg_data, TRUE,FALSE,FALSE);
            break;
        case MARKER_SOF1:
            get_sof(jpg_data,FALSE, FALSE, FALSE);
            break;
        case MARKER_SOF2:
            get_sof(jpg_data,FALSE,TRUE,FALSE);
            break;
        case MARKER_SOF9:
            get_sof(jpg_data,FALSE, FALSE,TRUE);
            break;
        case MARKER_SOF10:
            get_sof(jpg_data,FALSE,TRUE,TRUE);
            break;
        case MARKER_DAC:
            get_dac(jpg_data);
            break;
        case MARKER_DHT:
            get_dht(jpg_data);
            break;
        case MARKER_DRI:
            get_dri(jpg_data);
            break;
        case MARKER_COM:
            break;
        case MARKER_EOI:
            break;
        case HEADER_FIND_SOI:
            break;
        case MARKER_SOS:
            get_sos(jpg_data);
            return RET_OK;
            break;
        default:
            ERRDEBUG("The marker can not be supported .   0x%x\n", jpg_data->status);
            exit(1);
            break;
        }
    }

    return RET_OK;
}
