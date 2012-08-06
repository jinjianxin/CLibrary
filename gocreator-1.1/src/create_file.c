/* @(#)create_file.c
 *
 * Created on: 10 12ëÅ‘Å¬ 2008  09:30:37
 *     Author: lixujia <lixujia.cn@gmail.com>
 */

#define _GNU_SOURCE

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "config_file.h"
#include "create_file.h"

#define ALPHA_IS_UPPER(x)       ('A' <= (x) && 'Z' >= (x))
#define ALPHA_IS_LOWER(x)       ('a' <= (x) && 'z' >= (x))
#define LOWER_ALPHA_TO_UPPER(x) ((x) = ('a' <= (x) && (x) <= 'z') ? ((x) - 'a' + 'A') : (x))
#define UPPER_ALPHA_TO_LOWER(x) ((x) = ('A' <= (x) && (x) <= 'Z') ? ((x) - 'A' + 'a') : (x))

static char* hash_table[0x3F];

static char* parent = NULL;
static char* parent_type = NULL;
static char* author = NULL;
static char* email  = NULL;
static char* iname  = NULL;
static char* timestamp = NULL;
static char* filename = NULL;
static char* packname = NULL;
static char* widgname = NULL;
static char* packname_upper = NULL;
static char* widgname_upper = NULL;

static char* headerflag = "_(@ packname_upper @)_(@ widgname_upper @)_H";


static int init_hash_table(void){
    char* tem1[] = { parent , parent_type, author , email , timestamp , iname , timestamp , filename , packname , widgname , packname_upper , widgname_upper , headerflag};
    char* tem2[] = {"parent","parent_type","author","email","time"     ,"iname","timestamp","filename","packname","widgname","packname_upper","widgname_upper","headerflag"};
    memset(hash_table,0,sizeof(char*)*0x3F);

    int i;
    for(i = sizeof(tem1)/sizeof(char*)-1;i>=0;--i){
        unsigned char index;
        index = 0x3F&make_hash(tem2[i]);
        
        if(NULL != hash_table[index]){
            fprintf(stderr,"Hash table make value conflict!\n");
            exit(1);
        }
        hash_table[index] = tem1[i];
    }
    
    return 0;
}

char* hash_table_get_value(char* var){
    char* r;

    r = hash_table[0x3F&make_hash(var)];
    if(NULL == r){
        fprintf(stderr,"hash table: \"%s\" reachs NULL pointer!",var);
        exit(1);
    }
    
    return r;
}

static int init_global_var(char* path,char** name,char* _parent){
    int filenamelen = 0;
    int inamelen = 0;
    
    if(NULL == path || NULL == name || NULL == _parent){
        fprintf(stderr,"The parameter cannot leave NULL.\n");
        
        return 1;
    }
    
    if(NULL == author || NULL == email){
        config_content* cc;
        cc = read_config_file();
        author = strdup(cc->author);
        email  = strdup(cc->email);
    }

    if(NULL == name[0] || NULL == name[1]){
        fprintf(stderr,"The Class Name is assumed at least two part: NamespaceFunction.\n");
        return 1;
    }

    char** index;
    index = name;
    while(NULL != *index){
        filenamelen += strlen(*index)+1;
        inamelen    += strlen(*index);
        ++index;
    }
    
    filename = (char*)malloc(filenamelen*sizeof(char));
    iname    = (char*)malloc((inamelen+1)*sizeof(char));
    memset(iname,'\0',inamelen+1);
    memset(filename,'\0',filenamelen);
    index = name;
    while(NULL != *index){
        strcat(filename,*index);
        strcat(filename,"-");
        strcat(iname,*index);
        
        ++index;
    }
    
    filename[filenamelen-1] = '\0';
    
    int widgnamelen = 0;
    
    index = name;
    packname = strdup(*index);
    
    ++index;
    while(NULL != *index){
        widgnamelen += strlen(*index)+1;
        ++index;
    }
    widgname = (char*)malloc(sizeof(char)*widgnamelen);
    
    memset(widgname,'\0',widgnamelen);
    
    index = name;
    while(NULL != *++index){
        strcat(widgname,*index);
        strcat(widgname,"-");
    }
    widgname[widgnamelen - 1] = '\0';
    
    packname_upper = strdup(packname);
    widgname_upper = strdup(widgname);
    
    int i;
    for(i=strlen(packname_upper) - 1;i>=0;--i){
        LOWER_ALPHA_TO_UPPER(packname_upper[i]);
        UPPER_ALPHA_TO_LOWER(packname[i]);
    }
    
    for(i=strlen(widgname_upper) - 1;i>=0;--i){
        LOWER_ALPHA_TO_UPPER(widgname_upper[i]);
        UPPER_ALPHA_TO_LOWER(widgname[i]);
    }
    
    parent = _parent;
    int parent_type_len = 0;
    for(i=strlen(parent) - 1;i>=0;--i){
        if('a' <= parent[i] && parent[i] <= 'z')
            ++parent_type_len;
        else if('A' <= parent[i] && parent[i] <= 'Z')
            parent_type_len += 2;
        else{
            fprintf(stderr,"Illegal parent class name: %s!\n",parent);
            exit(1);
        }
    }
    parent_type_len += 5;
    parent_type = (char*)malloc(sizeof(char)*parent_type_len);
    memset(parent_type,0,parent_type_len);
    i = 0;
    if(!ALPHA_IS_UPPER(parent[i])){
        fprintf(stderr,"Illegal parent class name: %s!\n",parent);
        exit(1);
    }
    while(++i < strlen(parent) && !ALPHA_IS_UPPER(parent[i]));
    if(strlen(parent) == i){
        fprintf(stderr,"Illegal parent class name: %s!\n",parent);
        exit(1);
    }
    memcpy(parent_type,parent,i);
    strcat(parent_type,"_TYPE");
    int j;
    for(j=strlen(parent_type);j<parent_type_len && i < strlen(parent);++i,++j){
        if(ALPHA_IS_UPPER(parent[i])){
            parent_type[j++] = '_';
        }
        parent_type[j] = parent[i];
    }
    for(i=parent_type_len-1;i>=0;--i){
        LOWER_ALPHA_TO_UPPER(parent_type[i]);
    }
    
    time_t timep;
    time(&timep);
    timestamp = asctime(localtime(&timep));
    char* tem;
    tem = strchr(timestamp,'\n');
    if(NULL != tem) *tem = '\0';
    
    return 0;
}

char* mystrdup(char* ori,int len){
    char* result;
    int cp_len;
    
    if(0 == len)
        return NULL;
    
    result = (char*)malloc(sizeof(char)*(len+1));
    memset(result,0,len+1);

    cp_len = strlen(ori);
    cp_len = (cp_len > len) ? (len) : (cp_len);
    
    memcpy(result,ori,cp_len);
    
    return result;
}

static int out_put_func(FILE* outfp,char* template){
    int temp_len;
    int i;

    temp_len = strlen(template);
    
    for(i=0;i<temp_len;++i){
        if('(' == template[i] && (i+6) < temp_len && '@' == template[i+1] && ' ' == template[i+2]){
            char* tem;
            int end;
            i += 3;
            end = i;
            while(' ' != template[end] && end < temp_len) ++end;
            
            if(end == temp_len){
                fprintf(stderr,"Illegal mark, one marketer must in one line.\n");
                exit(1);
            }
            if('@' != template[end+1] || ')' != template[end+2]){
                fprintf(stderr,"Illegal mark, the variable must in (@  @) pair.\n");
                exit(1);
            }
            
            /* Cannot do the next line!
             * Because some times we come across literals */
//            template[end] = '\0';
            if(NULL != (tem = mystrdup(template+i,end-i))){
                out_put_func(outfp,hash_table_get_value(tem));
            }
            free(tem);
            
            i = end+2;
            continue;
        }
        putc(template[i],outfp);
    }
    
    return 0;
}

int create_file_h(char* path){
    FILE* outfp = NULL;
    FILE* infp  = NULL;
    int   getlen;
    char  buf[256];
    char* line = NULL;
    size_t line_len = 0;
    int i;
    
    for(i=strlen(filename)-1;i>=0;--i){
        UPPER_ALPHA_TO_LOWER(filename[i]);
    }
    
    memset(buf,0,256);
    snprintf(buf,256,"%s%s.h",path,filename);
    
    if(NULL == (outfp = fopen(buf,"w"))){
        fprintf(stderr,"Cannot open file \"%s\" to write.\n",buf);
        return 1;
    }
    infp = open_template_file_h();
    if(NULL == infp)
        return 1;

    while(-1 != (getlen = getline(&line,&line_len,infp))){
        out_put_func(outfp,line);
    }
    printf("Generate %s.h complete!\n",filename);
    
    return 0;
}

int create_file_c(char* path){
    FILE* outfp = NULL;
    FILE* infp  = NULL;
    int   getlen;
    char  buf[256];
    char* line  = NULL;
    size_t line_len = 0;

    memset(buf,0,256);
    snprintf(buf,256,"%s%s.c",path,filename);
    if(NULL == (outfp = fopen(buf,"w"))){
        fprintf(stderr,"Cannot open file \"%s\" to write.\n",buf);
        return 1;
    }
    infp = open_template_file_c();
    if(NULL == infp)
        return 1;

    while(-1 != (getlen = getline(&line,&line_len,infp))){
        out_put_func(outfp,line);
    }
    printf("Generate %s.c complete!\n",filename);
    
    return 0;
}

int create_files(char* path,char** name,char* parent){
    init_global_var(path,name,parent);
    init_hash_table();
    create_file_h(path);
    create_file_c(path);
    
    return 0;
}
