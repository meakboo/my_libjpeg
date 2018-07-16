#include <stdio.h>
#include <stdlib.h>

#include "my_jpeg.h"
#include "jpg_header.h"

/*******************************************************************************
功能描述: 从文件中获取若干字节
输入参数: jpg_data_s jpg_data : jpg数据结构体指针 
                   char *buff : 数据Buff
                   int len : 获取长度
输出参数: 无
返回值域:成功:0 失败:1
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
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
            //fread读上来的数据，低位在高地址，如果是数据类型，则需要转换，不然会出现类似大小端的问题
            for (i = 0; i < len ; i++) 
            {
                buff[i] = buff_tmp[len - i - 1];
            }
        }
        else
        {
            //如果不是数据类型，仅仅是读若干长度，则不需要转换
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
功能描述: 获取文件前两个字节
输入参数: jpg_data_s jpg_data : jpg数据结构体指针 
输出参数: 无
返回值域:无
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
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
功能描述: 获取下一个字段的描述符
输入参数: jpg_data_s jpg_data : jpg数据结构体指针 
输出参数: 无
返回值域:无
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
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
功能描述: 获取APP0或14字段中的数据
输入参数: jpg_data_s jpg_data : jpg数据结构体指针 
输出参数: 无
返回值域:无
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
*******************************************************************************/
void get_app0_or_14(jpg_data_p  jpg_data)
{
    s16 len = 0;
    u16 read_len = 0;
    u8  buff[APP0_DATA_LEN] = {0};

    get_byte(jpg_data, (u8 *)(&len), sizeof(u16), DATA_TYPE);
    //减去len本身的2个字节
    len -= 2;
    //只获取前14个字节
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
        //无用的数据需要跳过
    }


    return;
}

/*******************************************************************************
功能描述: 获取jpg文件头
输入参数: jpg_data_s  *jpg_data :jpg数据结构体指针 的二级指针
输出参数: 无
返回值域:成功:RET_OK 失败:RET_ERR
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
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

