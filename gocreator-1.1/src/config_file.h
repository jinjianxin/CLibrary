/*
 * config_file.h
 *
 * Created on: 09 12月 2008  12:56:32
 *     Author: lixujia <lixujia.cn@gmail.com>
 */

#ifndef _CONFIG_FILE_H
#define _CONFIG_FILE_H 1

#include <stdio.h>

#define DIRECTORY_NAME ".go-creator/"
#define CONFIGRATION_FILE_NAME "config.conf"

typedef struct _config_content config_content;

struct _config_content{
    char author[64];
    char email[64];
};

typedef enum _create_type c_type;
enum _create_type{
    TYPE_CLASS,
    TYPE_INTERFACE,
    TYPENUMBER
};

config_content* read_config_file(void);

int set_type(c_type type);

FILE* open_template_file_h(void);
FILE* open_template_file_c(void);

#endif /* _CONFIG_FILE_H */

