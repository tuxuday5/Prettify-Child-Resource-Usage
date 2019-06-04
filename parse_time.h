#define DATA ".asds,{#adfmasdf{#}\
||User||System||Elapse||CPU||(Data||Max)K||Ip+Op||Kern-Swtch||Vol-Swtch||\
{#}\
||9.00||10123.10||11:12.00||99%||(9||2636)K||123130+1313||21231||11313||\
{#}\
"

#define UNKNOWN_HEADER_FILED "UNKWN"
#define UNKNOWN_VALUE_FILED "UNKWN"
#define BUF_SIZE 1024
#define FIELD_SIZE 128
#define SEP "{#}"
#define FIELD_SEP "||"
#define TRUE 1
#define FALSE 0
#define OUT_STREAM stderr

#define OP_HEADER_CHAR '-'
#define OP_HEADER_FIELD_SEP_CHAR '+'
#define OP_FOOTER_CHAR '-'
#define OP_FOOTER_FIELD_SEP_CHAR '+'
#define OP_VALUE_FIELD_SEP_CHAR '|'

enum FieldType {INT=1,FLOAT=2,TIME=3,STRING=4};

struct StatsBuffer {
  char buffer[BUF_SIZE];
  short int buf_size;
  short int last_idx;
  short int in_sep;
  short int sep_buf_idx;
  short int begun;
  short int end;
};

struct DataBuffer {
  char header_buf[BUF_SIZE];
  char value_buf[BUF_SIZE];

  short int header_buf_size;
  short int value_buf_size;

  short int header_idx;
  short int value_idx;

  short int header_field_count;
  short int value_field_count;
};

struct FieldBuffer {
  short int field_no;
  char header[FIELD_SIZE];
  char value[FIELD_SIZE];
  short int header_width;
  short int value_width;
  short int max_width_for_field;
  short int value_type ;
};

int Max(int,int);
int Min(int,int);
void DumpSoFarStatsBuffer(struct StatsBuffer *);
void DumpData(char);
void ResetStatsBufferSepValues(struct StatsBuffer *);
void ResetStatsBuffer(struct StatsBuffer *);
void ResetDataBuffer(struct DataBuffer *);
void PrintFieldBuffers_plain( struct FieldBuffer **);
void AlignFieldMaxWidthForGivenSize( struct FieldBuffer **,short int);
void PrintOutputSepLine( struct FieldBuffer **,char,char);
void PrintBufferToLen(char *,short int);
void PrintFieldBuffers( struct FieldBuffer **);
void PrintDataBuffer( struct DataBuffer *);
void PrintStatsBuffer(struct StatsBuffer *);
void CheckAndSetEndForStatsBuffer(struct StatsBuffer *);
enum FieldType GetFieldType(char *);
int UpdateFieldCountFor(char *);
void UpdateFieldCountsInDataBuffer(struct DataBuffer *);
void SetUpStatsBuffer(struct StatsBuffer *,char);
void SetUpDataBuffer(struct StatsBuffer *,struct DataBuffer *);
short int GetFieldCount(struct DataBuffer *);
struct FieldBuffer **AllocateFieldBuffers(struct DataBuffer *);
void FreeFieldBuffers(struct FieldBuffer **);
void SetUpFieldBuffers(struct FieldBuffer **,struct DataBuffer *);
int BeautifyTimeOp(struct StatsBuffer *,short int );
void errExit(const char* );
