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

    fclose(jpg_data->fd);

    return 0;
}
