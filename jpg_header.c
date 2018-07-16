#include <stdio.h>
#include <stdlib.h>

#include "my_jpeg.h"
#include "jpg_header.h"

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
        DEBUG("buff  is empty\n");
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
        DEBUG("fread err: \n");
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
        DEBUG("jpg file SOI flag err\n");
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
        DEBUG("fill num = %d\n",  fill_num);
    }
    jpg_data->status = field_first;

    return;
}

void get_app0(jpg_data_p jpg_data, u8 *buff, u16 len)
{
    if (len >= APP0_DATA_LEN && buff[0] == 0x4a && buff[1] == 0x46&&\
        buff[2] == 0x49 && buff[3] == 0x46 && buff[4] == 0x00)
    {
        printf("hello world\n");
    }
    else
    {
        printf("fuck you\n");
        printf("%x  %x  %x  %x\n", buff[0], buff[1], buff[2], buff[3]);
    }
    exit(1);

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
    if (len > APP0_DATA_LEN)
    {
        read_len = APP0_DATA_LEN;
    }
    else
    {
        read_len = len;
    }

    get_byte(jpg_data, buff, read_len, NO_DATA_TYPE);

    if (jpg_data->status == MARKER_APP0) 
    {
        get_app0(jpg_data, buff, read_len);
    }
    else
    {
        //get_app14(jpg_data, read_len);
    }
    len -= read_len;

    if (len > 0)
    {
        //���õ�������Ҫ����
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
        }
    }


    return RET_OK;
}

