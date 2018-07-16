#ifndef __MY_JPEG_H_
#define __MY_JPEG_H_

#define DEBUG_ON

#ifndef DEBUG_ON
#define DEBUG(...)
#else
#define DEBUG(fmt, args...) fprintf(stderr, "\033[1;36m  ERR(%s:%d):\t\033[0m" fmt, __func__, __LINE__, ## args)
#endif

#define RET_OK               0
#define RET_ERR             1

typedef unsigned   int       u32;
typedef            int       s32;
typedef unsigned   short     u16;
typedef            short     s16;
typedef unsigned    char     u8;
typedef             char     s8;


struct app0_info_s{
    u8  saw_jfif_marker;
    u8  jfif_major_version;
    u8  jfif_minor_version;
    u8  density_unit;
    u16 x_density;
    u16 y_density;
};

typedef struct jpg_data_t
{
    FILE   *fd;
    u32    status;
    struct app0_info_s app0_info;

    u32    width;
    u32    height;


}jpg_data_s;

typedef  jpg_data_s * jpg_data_p;





#endif //__MY_JPEG_H_
