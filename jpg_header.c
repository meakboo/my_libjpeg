#include <stdio.h>
#include <stdlib.h>

#include "my_jpeg.h"
#include "jpg_header.h"
#include "jpg_data.h"

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
        ERRDEBUG("buff  is empty\n");
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
        ERRDEBUG("fread err: \n");
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
        ERRDEBUG("jpg file SOI flag err\n");
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
        WARDEBUG("fill num = %d\n",  fill_num);
    }
    jpg_data->status = field_first;

    return;
}

/*******************************************************************************
功能描述: 获取APP0字段的信息
输入参数: jpg_data_s jpg_data : jpg数据结构体指针 
                   u8 *buff : 承载数据的buff
                   u16 inter_len : 有兴趣的长度(buff中的数据长度)
                   u16 all_len : 字段总长度
输出参数: 无
返回值域:无
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
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
        if (jpg_data->app0_E_info.jfif_major_version != 1 || jpg_data->app0_E_info.jfif_minor_version != 2)
        {
            WARDEBUG("Warning: unknown JFIF revision number  %d.%02d\n",\
                     jpg_data->app0_E_info.jfif_major_version,jpg_data->app0_E_info.jfif_minor_version);
        }
        //如果缩略图像素不为一
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
功能描述: 跳过无用的字节
输入参数: jpg_data_s jpg_data : jpg数据结构体指针 
                   u16 inter_len : 无用的数据长度
输出参数: 无
返回值域:无
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
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
功能描述: 跳过暂不支持的字段
输入参数: jpg_data_s jpg_data : jpg数据结构体指针 
输出参数: 无
返回值域:无
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
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
功能描述: 获取APP14字段的信息
输入参数: jpg_data_s jpg_data : jpg数据结构体指针 
                   u8 *buff : 承载数据的buff
                   u16 inter_len : 有兴趣的长度(buff中的数据长度) 
输出参数: 无
返回值域:无
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
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

    if (len > 0)
    {
        //跳过无用的数据
        skip_byte(jpg_data, len);
    }

    return;
}

/*******************************************************************************
功能描述: 获取量化表
输入参数: jpg_data_s jpg_data : jpg数据结构体指针 
输出参数: 无
返回值域:无
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
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
        prec = c >> 4;      //QT精度 0 = 1字节  否则 = 2字节
        c &= 0x0F;             //QT号
        len -= 1;

        //如果QT大于4,则报错退出
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
                dqt_info->quantbal[natural_order[i]] = tmp_8;
            }
        }

        len -= count;
        if (prec)
        {
            len -= count;
        }
#if 0
        for (i = 1; i <= DCTSIZE2; i++)
        {
            printf("%02d     ", dqt_info->quantbal[i - 1]);
            if (i >0   &&  i % 8 == 0)
            {
                printf("\n");
            }
        }
        DEBUG("found dqt once !!\n");
#endif

    }
    if (len != 0)
    {
        ERRDEBUG("Bogus marker length\n");
        exit(1);
    }


    return;
}

/*******************************************************************************
功能描述: 获取帧开始数据信息
输入参数: jpg_data_s  *jpg_data :jpg数据结构体指针 的二级指针
输出参数: 无
返回值域:成功:RET_OK 失败:RET_ERR
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
*******************************************************************************/
void get_sof(jpg_data_p  jpg_data)
{
    u16 len = 0;
    u8   c;
    u32 i, j;
    struct components_info_s  *components_info;

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
            /*有时候可能有两个ID是相同的,虽然这种情况是不符合协议的,但是确实会发生*/
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
        get_byte(jpg_data, &c, sizeof(u8), DATA_TYPE);
        components_info->h_samp_factor = (c >> 4) & 0xF;
        components_info->v_samp_factor = c  & 0xF;
        get_byte(jpg_data, &(components_info->quant_tbl_no), sizeof(u8), DATA_TYPE);
    }
    jpg_data->sof_info.saw_sof_marker = TRUE;

    return;
}

/*******************************************************************************
功能描述: 获取DAC算术调节表
输入参数: jpg_data_s  *jpg_data :jpg数据结构体指针 的二级指针
输出参数: 无
返回值域:成功:RET_OK 失败:RET_ERR
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
*******************************************************************************/
void get_dac(jpg_data_p  jpg_data)
{
    //暂时不支持获取DAC功能
    skip_marker(jpg_data);

    return;
}

/*******************************************************************************
功能描述: 获取重新开始间隔
输入参数: jpg_data_s  *jpg_data :jpg数据结构体指针 的二级指针
输出参数: 无
返回值域:成功:RET_OK 失败:RET_ERR
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
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
功能描述: 获取行扫描信息
输入参数: jpg_data_s  *jpg_data :jpg数据结构体指针 的二级指针
输出参数: 无
返回值域:成功:RET_OK 失败:RET_ERR
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
*******************************************************************************/
void get_sos(jpg_data_p  jpg_data)
{
    u16 len;

    if (jpg_data->sof_info.saw_sof_marker != TRUE)
    {
        ERRDEBUG("Invalid JPEG file structure SOS before SOF");
        exit(1);
    }
    get_byte(jpg_data, (u8 *)(&len), sizeof(u16), DATA_TYPE);


    return;
}

/*******************************************************************************
功能描述: 获取霍夫曼表
输入参数: jpg_data_s  *jpg_data :jpg数据结构体指针 的二级指针
输出参数: 无
返回值域:成功:RET_OK 失败:RET_ERR
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
*******************************************************************************/
void get_dht(jpg_data_p  jpg_data)
{
    u16 len = 0;
    get_byte(jpg_data, (u8 *)(&len), sizeof(u16), DATA_TYPE);

    len -= 2;

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
            get_sof(jpg_data);
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
        case MARKER_EOI:
            DEBUG("hello world\n");
            return RET_OK;
            break;
        case MARKER_SOS:
            get_sos(jpg_data);
            break;
        }
    }


    return RET_OK;
}
