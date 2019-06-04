#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "p_stat_print.h"


void errExit(const char* s) {
  perror(s);
  exit(EXIT_FAILURE);
}

int Max(int x,int y) { 
  return x >= y ? x : y;
}

int Min(int x,int y) { 
  return x <= y ? x : y;
}

struct FieldBuffer **AllocateFieldBuffers(short int size) {
  short int fieldCount ;
  struct FieldBuffer **f;

  fieldCount = size;
  f = malloc(sizeof(struct FieldBuffer*)*fieldCount+1);
  if( !f) {
    errExit("malloc(sizeof(struct FieldBuffer*)*fieldCount)");
  }

  for( int i=0;i < fieldCount;i++) {
    f[i] = malloc(sizeof(struct FieldBuffer));

    if( ! f[i] ) {
      for(i--;i>=0;i--)
        free(f[i]);
      free(f); 
      errExit("f[i] = malloc(sizeof(struct FieldBuffer))");
    }
  }

  f[fieldCount] = NULL;

  return f;
}

void FreeFieldBuffers(struct FieldBuffer **f) {
  for(short int i=0;f[i];i++) 
    free(f[i]);

  free(f);
}

int FillResourceUsage( struct FieldBuffer **f,struct timeval *e) {
  struct rusage ru;
  struct timeval total_cpu_usage;
  short int i = 0;
  float elap_in_secs = e->tv_sec + (e->tv_usec/MICRO_SECOND);
  float total_cpu_in_secs ;
  float percent ;

  if( getrusage(RUSAGE_CHILDREN,&ru) < 0)
    errExit("getrusage");

  timeradd(&ru.ru_utime,&ru.ru_stime,&total_cpu_usage);
  total_cpu_in_secs = total_cpu_usage.tv_sec + (total_cpu_usage.tv_usec/MICRO_SECOND);
  percent = (total_cpu_in_secs / elap_in_secs)*100;

  f[i]->field_no = i+1;
  strcpy(f[i]->header,"Usr CPU");
  sprintf(f[i]->value,"%ld(s).%lu(us)",ru.ru_utime.tv_sec,(ru.ru_utime.tv_usec/USEC_ROUND_TO));
  i++;

  f[i]->field_no = i+1;
  strcpy(f[i]->header,"Sys CPU");
  sprintf(f[i]->value,"%ld(s).%lu(us)",ru.ru_stime.tv_sec,(ru.ru_stime.tv_usec/USEC_ROUND_TO));
  i++;

  f[i]->field_no = i+1;
  strcpy(f[i]->header,"Elap Time");
  sprintf(f[i]->value,"%ld(s).%lu(us)",e->tv_sec,(e->tv_usec/USEC_ROUND_TO));
  i++;

  f[i]->field_no = i+1;
  strcpy(f[i]->header,"CPU %");
  sprintf(f[i]->value,"%0.1f",percent);
  i++;

  f[i]->field_no = i+1;
  strcpy(f[i]->header,"RSS Max");
  sprintf(f[i]->value,"%ldKB",ru.ru_maxrss);
  i++;

  f[i]->field_no = i+1;
  strcpy(f[i]->header,"Soft PgFaults");
  sprintf(f[i]->value,"%ld",ru.ru_minflt);
  i++;

  f[i]->field_no = i+1;
  strcpy(f[i]->header,"Hard PgFaults");
  sprintf(f[i]->value,"%ld",ru.ru_majflt);
  i++;

  f[i]->field_no = i+1;
  strcpy(f[i]->header,"Read FS");
  sprintf(f[i]->value,"%ld",ru.ru_inblock);
  i++;

  f[i]->field_no = i+1;
  strcpy(f[i]->header,"Write FS");
  sprintf(f[i]->value,"%ld",ru.ru_oublock);
  i++;

  f[i]->field_no = i+1;
  strcpy(f[i]->header,"Vol Switch");
  sprintf(f[i]->value,"%ld",ru.ru_nvcsw);
  i++;

  f[i]->field_no = i+1;
  strcpy(f[i]->header,"InVol Switch");
  sprintf(f[i]->value,"%ld",ru.ru_nivcsw);
  i++;

  for(i=0;i<RES_COUNT;i++)
    f[i]->max_width_for_field = Max(strlen(f[i]->header),strlen(f[i]->value));

  return i;
}

void AlignFieldMaxWidthForGivenSize( struct FieldBuffer **b,short int w) {
  short int i,fieldCount;
  short int totalFieldWidth=0;
  float s=w,scaleFactor;

  for(i=0;b[i];i++)
    totalFieldWidth += b[i]->max_width_for_field;

  fieldCount = i;
  s -= (fieldCount+1); //to accomodate for sep between fields+1
  scaleFactor = s/totalFieldWidth;
  //scaleFactor = round(s/fieldCount);

  for(i=0;b[i];i++) {
    //b[i]->max_width_for_field = scaleFactor;
    b[i]->max_width_for_field = round(b[i]->max_width_for_field*scaleFactor);
  }
}

void PrintOutputSepLine( struct FieldBuffer **b,char ch,char sep) {
  fprintf(OUT_STREAM,"%c",sep);
  for(short int i=0;b[i];i++) {
    for( short int j=b[i]->max_width_for_field;j;j--)
      fprintf(OUT_STREAM,"%c",ch);
    fprintf(OUT_STREAM,"%c",sep);
  }
}

void PrintBufferToLen(char *b,short int len) {
  short int l=Min(len,strlen(b));
  short int i;

  for(i=0;i<l;i++)
    fprintf(OUT_STREAM,"%c",b[i]);

  if(len > l) {
    for(i=len-l;i;i--)
      fprintf(OUT_STREAM," ");
  }
}

void PrintFieldBuffers( struct FieldBuffer **b) {
  struct FieldBuffer *s;

  PrintOutputSepLine(b,OP_HEADER_CHAR,OP_HEADER_FIELD_SEP_CHAR);
  fprintf(OUT_STREAM,"\n");

  fprintf(OUT_STREAM,"%c",OP_VALUE_FIELD_SEP_CHAR);
  for(short int i=0;(s=b[i]);i++) {
    PrintBufferToLen(s->header,s->max_width_for_field);
    fprintf(OUT_STREAM,"%c",OP_VALUE_FIELD_SEP_CHAR);
  }
  fprintf(OUT_STREAM,"\n");

  PrintOutputSepLine(b,OP_HEADER_CHAR,OP_HEADER_FIELD_SEP_CHAR);
  fprintf(OUT_STREAM,"\n");

  fprintf(OUT_STREAM,"%c",OP_VALUE_FIELD_SEP_CHAR);
  for(short int i=0;(s=b[i]);i++) {
    PrintBufferToLen(s->value,s->max_width_for_field);
    fprintf(OUT_STREAM,"%c",OP_VALUE_FIELD_SEP_CHAR);
  }
  fprintf(OUT_STREAM,"\n");

  PrintOutputSepLine(b,OP_FOOTER_CHAR,OP_FOOTER_FIELD_SEP_CHAR);
  fprintf(OUT_STREAM,"\n");
}
