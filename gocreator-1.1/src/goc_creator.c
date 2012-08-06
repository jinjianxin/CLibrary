/* @(#)goc_creator.c
 *
 * Created on: 09 12月 2008  10:10:22
 *     Author: lixujia <lixujia.cn@gmail.com>
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <mcheck.h>

#include "utils.h"
#include "create_file.h"
#include "goc_creator.h"
#include "config_file.h"

static char* parent = NULL;
static char** iface = NULL;
static char** npart = NULL;
static char* name   = NULL;
static char* directory = NULL;

struct option longopts[] = {
    {"parent"   ,1,NULL,'p'},
    {"interface",1,NULL,'i'},
    {"name"     ,1,NULL,'n'},
    {"directory",1,NULL,'d'},
    {"type"     ,1,NULL,'t'},
    {"help"     ,0,NULL,'h'},
    {NULL,0,NULL,0}
};

char shortopts[] = "p:i:n:d:t:h";

void print_help(void){
    printf("Usage: gocreator [options] ClassName\n\n");
    printf("  -p, --parent=Parent      appoint the parent class, the default is GObject.\n");
    printf("  -t, --type=TYPE          you can choose class or interface to be created.\n");
    printf("  -d, --directory=DIR      appoint the path where you want the output file be placed.\n");
    printf("  -h, --help               show this help message.\n");
    
    return;
}

void test_display(void){
    int i = 0;

    while(NULL != iface[i]){
        printf("%s\n",iface[i]);
        ++i;
    }
}

static int deal_iface(char*** iface,char* optarg){
    char* str = strdup(optarg);
    int flag = 0;
    int len  = 0;
    int counter = 0;
    int i;
    
    len = strlen(optarg);

    for(i=0;i<len;++i){
        if(' ' == str[i] && flag){
            flag = 0;
            str[i] = '\0';
        }
        else if(' ' != str[i] && !flag){
            ++counter;
            flag = 1;
        }
        else if(' ' == str[i] && !flag){
            str[i] = '\0';
        }
    }

    *iface = (char**)malloc(sizeof(char*)*(counter+1));
    (*iface)[counter] = NULL;
    
    for(i=0,flag=0;i<len;++i){
        if('\0' == str[i] && flag){
            flag = 0;
        }
        else if('\0' != str[i] && !flag){
            (*iface)[--counter] = strdup(str+i);
            flag = 1;
        }
    }
    free(str);
    
    return 0;
}

int main(int argc,char** argv){
    int c;

    mtrace();
    
    if(1 == argc){
        print_help();
        exit(0);
    }

    while(-1 != (c=getopt_long(argc,argv,shortopts,longopts,NULL))){
        switch(c){
        case 'p':
            parent = strdup(optarg);
            break;
        case 'i':
            deal_iface(&iface,optarg);
            test_display();
            
            break;
        case 'n':
            name = strdup(optarg);
            break;
        case 'd':
            directory = strdup(optarg);
            break;
        case 't':
            if(0 == strcasecmp(optarg,"CLASS"))
                set_type(TYPE_CLASS);
            else if(0 == strcasecmp(optarg,"INTERFACE"))
                set_type(TYPE_INTERFACE);
            else
                set_type(TYPENUMBER);
            printf("===============================\n");
            
            break;
        case 'h':
        default:
            print_help();
            
            break;
        }
    }
    if(NULL == parent){
        parent = strdup("GObject");
    }
    if(NULL == directory){
        directory = strdup("./");
    }
    if(NULL == name && optind >= argc){
        fprintf(stderr,"You need to appoint a Class name!\n");
        print_help();
        exit(0);
    }
    else if(NULL == name){
        name = strdup(argv[optind++]);
    }
    npart = analyze_class_name(name);

    create_files(directory,npart,parent);
    
    free(parent);
    free_2D_charp(iface);
    free(name);
    free(directory);
    
    return 0;
}

