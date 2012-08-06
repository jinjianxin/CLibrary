/* @(#)config_file.c
 *
 * Created on: 09 12‘ÔÂ 2008  12:57:21
 *     Author: lixujia <lixujia.cn@gmail.com>
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "config_file.h"

static char* home   = NULL;
static char* path   = NULL;
static char* file   = NULL;
static char* ch_tem_file = NULL;
static char* cc_tem_file = NULL;
static char* ih_tem_file = NULL;
static char* ic_tem_file = NULL;

static int make_sure_path(void){
    int  findit = 0;
    DIR* h_dir = NULL;
    struct dirent* direntp;
    
    if (NULL == home){
        home = getenv("HOME");
    }

    if(NULL == (h_dir = opendir(home))){
        perror("Open the $HOME directory.");
        exit(1);
    }

    while(NULL != (direntp = readdir(h_dir))){
        if(0 != strcmp(DIRECTORY_NAME,direntp->d_name))
            continue;
        findit = 1;
        break;
    }
    if(findit){
        struct stat info;

        if (-1 == stat(path,&info)){
            perror("stat");
            exit(1);
        }

        if(!S_ISDIR(info.st_mode)){
            fprintf(stderr,"The file %s is in your HOME dir, please remove the file.\n",path);
            exit(1);
        }
        if(!(info.st_mode&S_IRUSR) || !(info.st_mode&S_IWUSR))
        {
            fprintf(stderr,"Permission denied.\n");
            fprintf(stderr,"Please check the permission of the %s\n",path);
            exit(1);
        }
    }
    else{
        mkdir(path,S_IRUSR|S_IWUSR|S_IXUSR);
    }
    closedir(h_dir);

    return !findit;
}

static int init_config_file(void){
    char buf[128];
    int file_len = 0;
    static int have_inited = 0;

    if(have_inited)
        return 0;
    
    home = getenv("HOME");
    
    file_len += strlen(home);
    file_len += strlen(DIRECTORY_NAME);

    path = (char*)malloc((file_len+1)*sizeof(char));
    memset(path,'\0',file_len+1);
    strcat(path,home);
    strcat(path,"/");
    strcat(path,DIRECTORY_NAME);

    file_len += strlen(CONFIGRATION_FILE_NAME)+1;
    file = (char*)malloc(file_len*sizeof(char));
    memset(file,'\0',file_len);

    strcat(file,path);
    strcat(file,CONFIGRATION_FILE_NAME);
    
    make_sure_path();
    
    snprintf(buf,128,"touch %s",file);
    system(buf);

    ch_tem_file = (char*)calloc((strlen(path)+strlen("class_h.template")+1),sizeof(char));
    strcat(ch_tem_file,path);
    strcat(ch_tem_file,"class_h.template");
    
    snprintf(buf,128,"touch %s",ch_tem_file);
    system(buf);
    
    cc_tem_file = (char*)calloc((strlen(path)+strlen("class_c.template")+1),sizeof(char));
    strcat(cc_tem_file,path);
    strcat(cc_tem_file,"class_c.template");
    
    snprintf(buf,128,"touch %s",cc_tem_file);
    system(buf);

    ih_tem_file = (char*)calloc((strlen(path)+strlen("interface_h.template")+1),sizeof(char));
    strcat(ih_tem_file,path);
    strcat(ih_tem_file,"interface_h.template");
    snprintf(buf,128,"touch %s",ih_tem_file);
    system(buf);

    ic_tem_file = (char*)calloc((strlen(path)+strlen("interface_c.template")+1),sizeof(char));
    strcat(ic_tem_file,path);
    strcat(ic_tem_file,"interface_c.template");
    snprintf(buf,128,"touch %s",ic_tem_file);
    system(buf);
    
    have_inited = 1;
    
    return 0;
}

config_content* read_config_file(void){
    int cc_size;
    int create = 0;
    int fd;
    config_content* cc;

    init_config_file();
    
    cc_size = sizeof(config_content);
    
    if(NULL == file)
        return NULL;

    cc = (config_content*)malloc(cc_size);
    memset(cc,0,cc_size);
    
    if(-1 == (fd = open(file,O_RDWR))){
        perror("open");
        exit(1);
    }
    
    lseek(fd,0,SEEK_SET);
    if(cc_size != lseek(fd,0,SEEK_END)){
        char tem;
    wait_choice:
        printf("You have probably not configured your information"
               "yet, do you want to input your information now?[Y/N] ");
        tem = getchar();
        if('n' == tem || 'N' == tem){
            exit(0);
        }
        else if('y' != tem && 'Y' != tem)
            goto wait_choice;

        lseek(fd,0,SEEK_SET);
        
        printf("Input your name(this will be insert into your source code's header): ");
        scanf("%s",cc->author);
        printf("Input your email(this will be insert into your source code's header): ");
        scanf("%s",cc->email);
        create = 1;
    }
    lseek(fd,0,SEEK_SET);

    if(create){
        if(write(fd,cc,cc_size) != cc_size){
            perror("writing config file!");
            ftruncate(fd,0);
            close(fd);
            
            exit(1);
        }
        ftruncate(fd,cc_size);
        lseek(fd,0,SEEK_SET);
    }
    else{
        int readlen = 0;
        if((readlen = read(fd,cc,cc_size)) != cc_size){
            perror("Reading config file!");
            
            close(fd);
            exit(1);
        }
    }
    
    return cc;
}

static FILE* (*_open_template_file_h)(void);
static FILE* (*_open_template_file_c)(void);

static FILE* open_class_template_file_h(void){
    FILE* fp = NULL;
    int fd;
    
    init_config_file();
    if( -1 == (fd = open(ch_tem_file,O_RDONLY|O_WRONLY))){
        perror("open");
        exit(1);
    }

    if(0 == lseek(fd,0,SEEK_END)){
        /* lseek return 0 means that the template file is empty */
        char buf[256];

        close(fd);

        snprintf(buf,256,"cp %s/share/gocreator-1.0/class_h.template %s",PREFIX,path);
        system(buf);
    }
    else
        close(fd);

    if(NULL == (fp = fopen(ch_tem_file,"r"))){
        perror("fopen");
        exit(1);
    }

    return fp;
}

static FILE* open_interface_template_file_h(void){
    FILE* fp = NULL;
    int fd;

    init_config_file();
    if( -1 == (fd = open(ih_tem_file,O_RDONLY|O_WRONLY))){
        perror("open");
        exit(1);
    }

    if(0 == lseek(fd,0,SEEK_END)){
        /* lseek return 0 means that the template file is empty */
        char buf[256];

        close(fd);

        snprintf(buf,256,"cp %s/share/gocreator-1.0/interface_h.template %s",PREFIX,path);
        system(buf);
    }
    else
        close(fd);

    if(NULL == (fp = fopen(ih_tem_file,"r"))){
        perror("fopen");
        exit(1);
    }

    return fp;
}

static FILE* open_interface_template_file_c(void){
    FILE* fp = NULL;
    int fd;

    init_config_file();
    if( -1 == (fd = open(ic_tem_file,O_RDONLY|O_WRONLY))){
        perror("open");
        exit(1);
    }

    if(0 == lseek(fd,0,SEEK_END)){
        /* lseek return 0 means that the template file is empty */
        char buf[256];

        close(fd);

        snprintf(buf,256,"cp %s/share/gocreator-1.0/interface_c.template %s",PREFIX,path);
        system(buf);
    }
    else
        close(fd);

    if(NULL == (fp = fopen(ic_tem_file,"r"))){
        perror("fopen");
        exit(1);
    }

    return fp;
}


static FILE* open_class_template_file_c(void){
    FILE* fp = NULL;
    int fd;
    
    init_config_file();
    if( -1 == (fd = open(cc_tem_file,O_RDONLY|O_WRONLY))){
        perror("open");
        exit(1);
    }

    if(0 == lseek(fd,0,SEEK_END)){
        /* lseek return 0 means that the template file is empty */
        char buf[256];

        close(fd);

        snprintf(buf,256,"cp %s/share/gocreator-1.0/class_c.template %s",PREFIX,path);
        system(buf);
    }
    else
        close(fd);

    if(NULL == (fp = fopen(cc_tem_file,"r"))){
        perror("fopen");
        exit(1);
    }

    return fp;
}

FILE* open_template_file_h(void){
    if(_open_template_file_h)
        return _open_template_file_h();
    return open_class_template_file_h();
}

FILE* open_template_file_c(void){
    if(_open_template_file_c)
        return _open_template_file_c();
    return open_class_template_file_c();
}

int set_type(c_type type){
    switch(type){
    case TYPE_CLASS:
        _open_template_file_h = open_class_template_file_h;
        _open_template_file_c = open_class_template_file_c;
        
        break;
    case TYPE_INTERFACE:
        _open_template_file_h = open_interface_template_file_h;
        _open_template_file_c = open_interface_template_file_c;
        
        break;
    default:
        printf("Not supported type!\n");
        return 1;
    }
    return 0;
}
