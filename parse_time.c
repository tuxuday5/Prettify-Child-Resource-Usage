#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<errno.h>
#include<math.h>
#include "parse_time.h"

void errExit(const char* s) {
  perror(s);
  exit(EXIT_FAILURE);
}

int Max(int x,int y) { 
  return x >= y ? x : y;
}

int Min(int x,int y) { 
  return x < y ? x : y;
}

void DumpSoFarStatsBuffer(struct StatsBuffer *s) {
  fprintf(OUT_STREAM,"%s",s->buffer);
}

void DumpData(char c) {
  fprintf(OUT_STREAM,"%c",c);
}

void ResetStatsBufferSepValues(struct StatsBuffer *s) {
  s->in_sep = FALSE ;
  s->sep_buf_idx = 0 ;
}

void ResetStatsBuffer(struct StatsBuffer *s) {
  memset(s,'\0',sizeof(struct StatsBuffer));
  s->buf_size = BUF_SIZE;
  s->last_idx=0;
  s->begun = s->end = FALSE;
  ResetStatsBufferSepValues(s);
}

void ResetDataBuffer(struct DataBuffer *d) {
  memset(d,'\0',sizeof(struct DataBuffer));
  d->header_idx= d->value_idx = 0;
  d->header_buf_size = d->value_buf_size = BUF_SIZE;
}

void PrintFieldBuffers_plain( struct FieldBuffer **b) {
  struct FieldBuffer *s;
  for(short int i=0;(s=b[i]);i++) {
    fprintf(OUT_STREAM,"\nFIELD NO ->%d<-\n",s->field_no);
    fprintf(OUT_STREAM,"header=%s\n",s->header);
    fprintf(OUT_STREAM,"value=%s\n",s->value);
    fprintf(OUT_STREAM,"header_width=%d\n",s->header_width);
    fprintf(OUT_STREAM,"value_width=%d\n",s->value_width);
    fprintf(OUT_STREAM,"max_width_for_field=%d\n",s->max_width_for_field);
    fprintf(OUT_STREAM,"value_type=%d\n",s->value_type) ;
  }
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

  //fprintf(OUT_STREAM,"%c",OP_VALUE_FIELD_SEP_CHAR);
  for(short int i=0;(s=b[i]);i++) {
    PrintBufferToLen(s->header,s->max_width_for_field);
    fprintf(OUT_STREAM,"%c",OP_VALUE_FIELD_SEP_CHAR);
  }
  fprintf(OUT_STREAM,"\n");

  PrintOutputSepLine(b,OP_HEADER_CHAR,OP_HEADER_FIELD_SEP_CHAR);
  fprintf(OUT_STREAM,"\n");

  //fprintf(OUT_STREAM,"%c",OP_VALUE_FIELD_SEP_CHAR);
  for(short int i=0;(s=b[i]);i++) {
    PrintBufferToLen(s->value,s->max_width_for_field);
    fprintf(OUT_STREAM,"%c",OP_VALUE_FIELD_SEP_CHAR);
  }
  fprintf(OUT_STREAM,"\n");

  PrintOutputSepLine(b,OP_FOOTER_CHAR,OP_FOOTER_FIELD_SEP_CHAR);
  fprintf(OUT_STREAM,"\n");
}

void PrintDataBuffer( struct DataBuffer *d) {
  fprintf(OUT_STREAM,"header_buf=%s\n",d->header_buf);
  fprintf(OUT_STREAM,"value_buf=%s\n",d->value_buf);

  fprintf(OUT_STREAM,"header_buf_size=%d\n",d->header_buf_size);
  fprintf(OUT_STREAM,"value_buf_size=%d\n",d->value_buf_size);

  fprintf(OUT_STREAM,"header_idx=%d\n",d->header_idx);
  fprintf(OUT_STREAM,"value_idx=%d\n",d->value_idx);

  fprintf(OUT_STREAM,"header_field_count=%d\n",d->header_field_count);
  fprintf(OUT_STREAM,"value_field_count=%d\n",d->value_field_count);
}


void PrintStatsBuffer(struct StatsBuffer *s) {
  fprintf(OUT_STREAM,"buffer=%s\n",s->buffer);
  fprintf(OUT_STREAM,"buf_size=%d\n",s->buf_size);
  fprintf(OUT_STREAM,"last_idx=%d\n",s->last_idx);
  fprintf(OUT_STREAM,"in_sep=%d\n",s->in_sep);
  fprintf(OUT_STREAM,"sep_buf_idx=%d\n",s->sep_buf_idx);
  fprintf(OUT_STREAM,"begun=%d\n",s->begun);
  fprintf(OUT_STREAM,"end=%d\n",s->end);
}

void CheckAndSetEndForStatsBuffer(struct StatsBuffer *s) {
  int len=strlen(SEP);

  if(! s->last_idx)
    return ;

  if(strncmp(SEP,&(s->buffer[s->last_idx-len]),len)==0) {
    s->begun=FALSE;
    s->end=TRUE;
  }
}

enum FieldType GetFieldType(char *f) { //TODO:
  return STRING;
}

int UpdateFieldCountFor(char *buf) {
  int len = strlen(FIELD_SEP);
  int retVal = 0; 
  char *b,*e;

  for( b=strstr(buf,FIELD_SEP);b && (e=strstr(b+len,FIELD_SEP));b=e,retVal++);

  return retVal;
}

void UpdateFieldCountsInDataBuffer(struct DataBuffer *d) {
  if( ! d->header_idx )
    return ;
  d->header_field_count = UpdateFieldCountFor(d->header_buf);
  d->value_field_count = UpdateFieldCountFor(d->value_buf);
}

void SetUpStatsBuffer(struct StatsBuffer *s,char c) {
  if( (s->begun==FALSE) && (c == SEP[0]) ) { // starting, check for sep
    ResetStatsBuffer(s);
    s->begun = s->in_sep = TRUE;
    s->buffer[s->last_idx] = c;
    s->last_idx++;
    s->sep_buf_idx++;
  } else if( s->in_sep ) {
    if( c == SEP[s->sep_buf_idx]) {
      s->sep_buf_idx++;
      s->buffer[s->last_idx] = c;
      s->last_idx++;
      if( strlen(s->buffer) == strlen(SEP) ) {
        ResetStatsBufferSepValues(s);
      }
    } else { // some chars of data match sep, but not completely. so output buffer & reset.
      DumpSoFarStatsBuffer(s);
      ResetStatsBuffer(s);
    }
  } else if( s->begun ) { // sep has been done, now deal with stats data
    s->buffer[s->last_idx] = c;
    s->last_idx++;
  } else {
    DumpData(c);
  }
}

void SetUpDataBuffer(struct StatsBuffer *s,struct DataBuffer *d) {
  char *buffer,*startSep,*endSep,data[BUF_SIZE];
  short int len=strlen(SEP);

  ResetDataBuffer(d);

  if(! s->last_idx)
    return ;

  strncpy(data,s->buffer,s->last_idx);

  buffer   = data;

  if( !(startSep = strstr(buffer,SEP))) { // no sep!! :(, time op not per recommondation
    ResetDataBuffer(d);
    return;
  }

  buffer  += len;
  if( !(endSep   = strstr(buffer,SEP))) {
    ResetDataBuffer(d);
    return;
  }

  d->header_idx = endSep-buffer;
  strncpy(d->header_buf,buffer,d->header_idx);

  buffer   = startSep = endSep;
  buffer  += len;
  if( !(endSep   = strstr(buffer,SEP))) {
    ResetDataBuffer(d);
    return;
  }

  d->value_idx = endSep-buffer;
  strncpy(d->value_buf,buffer,d->value_idx);
}

short int GetFieldCount(struct DataBuffer *d) {
  return Max(d->header_field_count,d->value_field_count);
}

struct FieldBuffer **AllocateFieldBuffers(struct DataBuffer *d) {
  short int fieldCount ;
  struct FieldBuffer **f;

  fieldCount = GetFieldCount(d);
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

void SetUpFieldBuffers(struct FieldBuffer **b,struct DataBuffer *d) {
  short int len=strlen(FIELD_SEP);
  char *headerStartSep,*headerEndSep,*headerBuffer;
  char *valueStartSep,*valueEndSep,*valueBuffer;
  struct FieldBuffer *f;

  headerBuffer = d->header_buf;
  valueBuffer = d->value_buf;
  for(short int fieldNo=0;(f=b[fieldNo]);fieldNo++) {

    headerStartSep = strstr(headerBuffer,FIELD_SEP);
    if(headerStartSep) {
      headerBuffer += len;
      headerEndSep = strstr(headerBuffer,FIELD_SEP);
      if(headerEndSep) {
        f->header_width= headerEndSep-headerBuffer;
        strncpy(f->header,headerBuffer,f->header_width);
        headerBuffer+=f->header_width;
      } else {
        f->header_width= strlen(UNKNOWN_HEADER_FILED);
        strncpy(f->header,UNKNOWN_HEADER_FILED,f->header_width);
      }
    }

    valueStartSep = strstr(valueBuffer,FIELD_SEP);
    if(valueStartSep) {
      valueBuffer += len;
      valueEndSep = strstr(valueBuffer,FIELD_SEP);
      if(valueEndSep) {
        f->value_width= valueEndSep-valueBuffer;
        strncpy(f->value,valueBuffer,f->value_width);
        valueBuffer+=f->value_width;
      } else {
        f->value_width= strlen(UNKNOWN_VALUE_FILED);
        strncpy(f->value,UNKNOWN_VALUE_FILED,f->value_width);
      }
    }

    if( headerEndSep || valueEndSep ) 
      f->field_no = fieldNo+1;

    f->max_width_for_field = Max(f->header_width,f->value_width);
  }
}

int BeautifyTimeOp(struct StatsBuffer *s,short int width) {
  struct DataBuffer dBuf;
  struct FieldBuffer **fBuf;
  struct StatsBuffer sBuf;

  memcpy(&sBuf,s,sizeof(struct StatsBuffer));

  /*
  ResetStatsBuffer(&sBuf);
  for(int i=0;i < strlen(DATA) ; i++) 
    SetUpStatsBuffer(&sBuf,DATA[i]);
    */

  CheckAndSetEndForStatsBuffer(&sBuf);

  SetUpDataBuffer(&sBuf,&dBuf);
  UpdateFieldCountsInDataBuffer(&dBuf);

  if( dBuf.header_idx )  {
    fBuf = AllocateFieldBuffers(&dBuf);
    SetUpFieldBuffers(fBuf,&dBuf);
    AlignFieldMaxWidthForGivenSize(fBuf,width);
    PrintFieldBuffers(fBuf);
    FreeFieldBuffers(fBuf);
  }

  return 0;
}
