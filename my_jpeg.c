#include <stdio.h>
#include <stdlib.h>

#include "my_jpeg.h"
#include "jpg_header.h"

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
        printf("\t��ʽ: [JFIF]\n");
        printf("\t�汾�� %d.%d\n", jpg_data->app0_E_info.jfif_major_version, jpg_data->app0_E_info.jfif_minor_version);
        printf("\t�ܶȵ�λ : %d\n", jpg_data->app0_E_info.density_unit);
        printf("\tX�ܶ� %d\n", jpg_data->app0_E_info.x_density);
        printf("\tY�ܶ� %d\n", jpg_data->app0_E_info.y_density);
    }
    else if (jpg_data->app0_E_info.saw_adobe_marker == TRUE)
    {
        printf("\t��ʽ : [Adobe]\n");
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
                printf("\t���ֶ��������� : %d  ID = %d\n", num, jpg_data->dqt_info[i]->dqt_id);
                if (jpg_data->dqt_info[i]->prec == 0)
                {
                    printf("\t��������1�ֽ�\n");
                }
                else
                {
                    printf("\t��������2�ֽ�\n");
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
                printf("����������  :   %d      ��Ϣ��ӡ���\n", num);
                num++;
            }
        }
    }
    if (num == 0)
    {
        ERRDEBUG("δ���ֶ���������\n");
        return;
    }
    printf("---------------------------------------------------------------\n");
    //����֡��ʼ��SOF
    if (jpg_data->sof_info.saw_sof_marker == TRUE)
    {
        printf("\t����֡��ʼ��SOF0\n");
        printf("\t������������ :  %d\n", jpg_data->data_precision);
        printf("\tͼ��߶� : %d\n", jpg_data->height);
        printf("\tͼ���� : %d\n", jpg_data->width);
        printf("\t��ɫ������ : %d\n", jpg_data->num_components);
        for (i = 0 ,components_info =jpg_data->sof_info.components_info; i < jpg_data->num_components; i++, components_info++)
        {
            printf("\t��ɫID : %d  ˮƽ�������� : %d ��ֱ�������� : %d ������ID : %d\n", components_info->component_id,\
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
                printf("\t���ֻ�����AC�� :%d  ############\n", i);

                for (a = 0, c = 0; a < 16; a++)
                {
                    printf("\t������AC��bit%02d  ��%03d��Ԫ����  :", a+1,  jpg_data->dht_ac_info[i]->bits[a]);
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
                printf("\t ���� %d ��Ԫ��\n", c);
            }
        }
    }
    for (i = 0; i < NUM_HUFF_TABLE; i++)
    {
        if (jpg_data->dht_dc_info[i] != NULL)
        {
            if (jpg_data->dht_dc_info[i]->flag = TRUE)
            {
                printf("\t���ֻ�����DC�� :%d  ##########\n", i);

                for (a = 0, c = 0; a < 16; a++)
                {
                    printf("\t������AC��bit%02d  ��%03d��Ԫ����  :", a+1,  jpg_data->dht_dc_info[i]->bits[a]);
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
                printf("\t ���� %d ��Ԫ��\n", c);
            }
        }
    }
    printf("---------------------------------------------------------------\n");
    if (jpg_data->restart_interval != 0)
    {
        printf("\t���ָ�λ�����\n");
        printf("\t�μ��Ϊ %d\n", jpg_data->restart_interval);
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
                printf("\t��ɫ����ID : %d    ", jpg_data->sos_info.cur_components_info[i]->component_id);
                printf("\tֱ������ʹ�õĻ���������Ҫ : %d    ", jpg_data->sos_info.cur_components_info[i]->dc_tbl_no);
                printf("\t��������ʹ�õĻ���������Ҫ : %d    \n", jpg_data->sos_info.cur_components_info[i]->ac_tbl_no);
            }
        }
    }






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
    if(RET_OK != jpg_decompress_init(&jpg_data, argv[1]))
    {
        ERRDEBUG("jpg decompress init failed\n");
        return 1;
    }
    //��ȡjpg�ļ�ͷ
    if(RET_OK != jpg_get_header(jpg_data))
    {
        ERRDEBUG("jpg get header failed\n");
        return 1;
    }

    jpg_debug(jpg_data,  argv[1]);

    fclose(jpg_data->fd);

    return 0;
}
