/* @(#)utils.c
 *
 * Created on: 09 12月 2008  11:26:59
 *     Author: lixujia <lixujia.cn@gmail.com>
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"

#define IS_UPPER_CHAR(x) (((x) <= 'Z') && ((x) >= 'A'))
#define IS_LOWER_CHAR(x) (((x) <= 'z') && ((x) >= 'a'))

char** analyze_class_name(char* name){
    int i;
    int parts = 0;
    char* str = NULL;
    char** result = NULL;
    
    if (!IS_UPPER_CHAR(name[0])){
        printf("The class name appointed is illegal!\n");
        printf("(%s)\n",name);
        return NULL;
    }
    
    str = strdup(name);
    for(i=0;'\0' != name[i];++i){
        if(IS_UPPER_CHAR(name[i])){
            ++parts;
        }
        else if(!IS_LOWER_CHAR(name[i])){
            printf("The class name appointed is illegal!\n");
            return NULL;
        }
    }
    
    result = (char**)malloc(sizeof(char*)*(parts+1));
    result[parts] = NULL;
    
    for(i=strlen(str)-1;i>=0;--i){
        if(IS_UPPER_CHAR(str[i])){
            result[--parts] = strdup(str+i);
            str[i] = '\0';
        }
    }
    
    free(str);
    return result;
}

int free_2D_charp(char** p){
    int i;

    if(NULL == p)
        return 0;
    
    for(i=0;NULL != p[i];++i){
        free(p[i]);
    }

    free(p);
    
    return 0;
}

unsigned char make_hash(char* str){
    int len;
    unsigned char result = 0;
    
    len = strlen(str);
    result = str[0];
    for(--len;len>=0;--len){
        result += str[len];
    }
    return (result^(result>>1))&0x3F;
}

