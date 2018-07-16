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
u32 get_byte(jpg_data_p jpg_data, u8 *buff, u32 len)
{
    int ret = 0;

    if (NULL == buff)
    {
        DEBUG("buff  is empty\n");
        exit(1);
    }

    ret = fread(buff, len, 1, jpg_data->fd);
    if (ret == 1)
    {
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
void get_soI( jpg_data_p  jpg_data)
{
    u8 field_first = 0;
    u8 field_next = 0;

    get_byte(jpg_data, &field_first, sizeof(u8));
    get_byte(jpg_data, &field_next, sizeof(u8));
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
    u8    field_first = 0;
    u32 fill_num = 0;

    while (1)
    {    
        get_byte(jpg_data, &field_first, sizeof(u8));

        while (MARKER_FLAG != field_first)
        {
            get_byte(jpg_data, &field_first, sizeof(u8));
        }

        do {
            get_byte(jpg_data, &field_first, sizeof(u8));
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

/*******************************************************************************
��������: ��ȡAPP0�ֶ��е�����
�������: jpg_data_s jpg_data : jpg���ݽṹ��ָ�� 
�������: ��
����ֵ��:��
--------------------------------------------------------------------------------
�޸�����: �ζ�
�޸�����:
�޸�˵��:����
*******************************************************************************/
void get_app0(jpg_data_p  jpg_data)
{
    u16 len = 0;

    get_byte(jpg_data, len, sizeof(u16));


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
            get_soI(jpg_data);
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
            get_app0(jpg_data);
            break;
        }
    }


    return RET_OK;
}

