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
功能描述: 获取文件前两个字节
输入参数: jpg_data_s jpg_data : jpg数据结构体指针 
输出参数: 无
返回值域:无
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
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
功能描述: 获取APP0字段中的数据
输入参数: jpg_data_s jpg_data : jpg数据结构体指针 
输出参数: 无
返回值域:无
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
*******************************************************************************/
void get_app0(jpg_data_p  jpg_data)
{
    u16 len = 0;

    get_byte(jpg_data, len, sizeof(u16));


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

