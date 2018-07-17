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

    fclose(jpg_data->fd);

    return 0;
}
