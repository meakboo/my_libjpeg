#include <stdio.h>
#include <stdlib.h>

#include "my_jpeg.h"
#include "jpg_header.h"

/*******************************************************************************
功能描述: 初始化jpg解码资源
输入参数: jpg_data_s  *jpg_data :jpg数据结构体指针 的二级指针
                  char *file_name ： 图片文件名称
输出参数: 无
返回值域:成功:0 失败:1
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
*******************************************************************************/
u32 jpg_decompress_init( jpg_data_p  *jpg_data, char *file_name)
{
    *jpg_data = (jpg_data_p)malloc(sizeof(jpg_data_s));
    if (NULL ==  (*jpg_data))
    {
        ERRDEBUG("malloc failed\n");
        return RET_ERR;
    }

    (*jpg_data)->fd = fopen(file_name, "r");
    if ((*jpg_data)->fd == NULL)
    {
        ERRDEBUG("fopen failed\n");
        return RET_ERR;
    }

    (*jpg_data)->status = HEADER_START;

    return RET_OK;
}

/*******************************************************************************
功能描述: 调试打印函数
输入参数: jpg_data_p  jpg_data : jpg结构体
输出参数: 无
返回值域:无
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
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
    /* 解析APP0 */
    if (jpg_data->app0_E_info.saw_jfif_marker == TRUE)
    {
        printf("\t格式: [JFIF]\n");
        printf("\t版本号 %d.%d\n", jpg_data->app0_E_info.jfif_major_version, jpg_data->app0_E_info.jfif_minor_version);
        printf("\t密度单位 : %d\n", jpg_data->app0_E_info.density_unit);
        printf("\tX密度 %d\n", jpg_data->app0_E_info.x_density);
        printf("\tY密度 %d\n", jpg_data->app0_E_info.y_density);
    }
    else if (jpg_data->app0_E_info.saw_adobe_marker == TRUE)
    {
        printf("\t格式 : [Adobe]\n");
    }
    else
    {
        ERRDEBUG("Not found app0 or app14\n");
        return;
    }
    printf("---------------------------------------------------------------\n");
    /* 解析定义量化表 */
    for (i = 0;i < NUM_QUANT_TBLS; i++)
    {
        if (jpg_data->dqt_info[i] != NULL)
        {
            if (jpg_data->dqt_info[i]->found_flag == TRUE)
            {
                printf("\t发现定义量化表 : %d  ID = %d\n", num, jpg_data->dqt_info[i]->dqt_id);
                if (jpg_data->dqt_info[i]->prec == 0)
                {
                    printf("\t量化精度1字节\n");
                }
                else
                {
                    printf("\t量化精度2字节\n");
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
                printf("定义量化表  :   %d      信息打印完毕\n", num);
                num++;
            }
        }
    }
    if (num == 0)
    {
        ERRDEBUG("未发现定义量化表\n");
        return;
    }
    printf("---------------------------------------------------------------\n");
    //解析帧开始段SOF
    if (jpg_data->sof_info.saw_sof_marker == TRUE)
    {
        printf("\t发现帧开始段SOF0\n");
        printf("\t数据样本精度 :  %d\n", jpg_data->data_precision);
        printf("\t图像高度 : %d\n", jpg_data->height);
        printf("\t图像宽度 : %d\n", jpg_data->width);
        printf("\t颜色分量数 : %d\n", jpg_data->num_components);
        for (i = 0 ,components_info =jpg_data->sof_info.components_info; i < jpg_data->num_components; i++, components_info++)
        {
            printf("\t颜色ID : %d  水平采样因子 : %d 垂直采样因子 : %d 量化表ID : %d\n", components_info->component_id,\
                          components_info->h_samp_factor, components_info->v_samp_factor, components_info->quant_tbl_no);
        }
    }
    printf("---------------------------------------------------------------\n");
    //解析霍夫曼表
    for (i = 0; i < NUM_HUFF_TABLE; i++)
    {
        if (jpg_data->dht_ac_info[i] != NULL)
        {
            if (jpg_data->dht_ac_info[i]->flag = TRUE)
            {
                printf("\t发现霍夫曼AC表 :%d  ############\n", i);

                for (a = 0, c = 0; a < 16; a++)
                {
                    printf("\t霍夫曼AC表bit%02d  有%03d个元素数  :", a+1,  jpg_data->dht_ac_info[i]->bits[a]);
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
                printf("\t 共有 %d 个元素\n", c);
            }
        }
    }
    for (i = 0; i < NUM_HUFF_TABLE; i++)
    {
        if (jpg_data->dht_dc_info[i] != NULL)
        {
            if (jpg_data->dht_dc_info[i]->flag = TRUE)
            {
                printf("\t发现霍夫曼DC表 :%d  ##########\n", i);

                for (a = 0, c = 0; a < 16; a++)
                {
                    printf("\t霍夫曼AC表bit%02d  有%03d个元素数  :", a+1,  jpg_data->dht_dc_info[i]->bits[a]);
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
                printf("\t 共有 %d 个元素\n", c);
            }
        }
    }
    printf("---------------------------------------------------------------\n");
    if (jpg_data->restart_interval != 0)
    {
        printf("\t发现复位间隔段\n");
        printf("\t段间隔为 %d\n", jpg_data->restart_interval);
        printf("---------------------------------------------------------------\n");
    }

    //解析开始扫描段SOS
    if (jpg_data->comps_in_scan  != 0)
    {
        printf("\t发现开始扫描段\n");
        printf("\t该段组件个数为 : %d\n", jpg_data->comps_in_scan );

        for (i = 0; i <  MAX_COMPS_IN_SCAN; i++)
        {
            if (jpg_data->sos_info.cur_components_info[i] != NULL)
            {
                printf("\t颜色分量ID : %d    ", jpg_data->sos_info.cur_components_info[i]->component_id);
                printf("\t直流分量使用的霍夫曼表需要 : %d    ", jpg_data->sos_info.cur_components_info[i]->dc_tbl_no);
                printf("\t交流分量使用的霍夫曼表需要 : %d    \n", jpg_data->sos_info.cur_components_info[i]->ac_tbl_no);
            }
        }
    }






    return;
}

/*******************************************************************************
功能描述: 测试主函数
输入参数: int argc : 程序启动参数个数
                  char **argv : 程序启动参数
输出参数: 无
返回值域:成功:0 失败:1
--------------------------------------------------------------------------------
修改作者: 任栋
修改日期:
修改说明:新作
*******************************************************************************/
int main(int argc, char**argv)
{
    jpg_data_p jpg_data;

    //判断输入参数
    if (2 != argc)
    {
        ERRDEBUG("usage :  ./a.out  test.jpg\n");
        return 1;
    }
    //初始化需要的资源
    if(RET_OK != jpg_decompress_init(&jpg_data, argv[1]))
    {
        ERRDEBUG("jpg decompress init failed\n");
        return 1;
    }
    //获取jpg文件头
    if(RET_OK != jpg_get_header(jpg_data))
    {
        ERRDEBUG("jpg get header failed\n");
        return 1;
    }

    jpg_debug(jpg_data,  argv[1]);

    fclose(jpg_data->fd);

    return 0;
}
