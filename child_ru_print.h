#define OUT_STREAM stderr
#define OP_HEADER_CHAR '-'
#define OP_HEADER_FIELD_SEP_CHAR '+'
#define OP_FOOTER_CHAR '-'
#define OP_FOOTER_FIELD_SEP_CHAR '+'
#define OP_VALUE_FIELD_SEP_CHAR '|'

#define UNKNOWN_HEADER_FILED "UNKWN"
#define UNKNOWN_VALUE_FILED "UNKWN"
#define BUF_SIZE 1024
#define FIELD_SIZE 128
#define DEFAULT_WIDTH 80
#define MICRO_SECOND ((float)1000000.0)
#define RES_COUNT 11
#define USEC_ROUND_TO ((long)10000)

struct FieldBuffer {
  short int field_no;
  char header[FIELD_SIZE];
  char value[FIELD_SIZE];
  short int header_width;
  short int value_width;
  short int max_width_for_field;
  short int value_type ;
};

void errExit(const char* );
int Max(int ,int ) ;
int Min(int ,int)  ;
struct FieldBuffer **AllocateFieldBuffers(short int );
void FreeFieldBuffers(struct FieldBuffer **) ;
int FillResourceUsage( struct FieldBuffer **,struct timeval *);
void AlignFieldMaxWidthForGivenSize( struct FieldBuffer **,short int );
void PrintOutputSepLine( struct FieldBuffer **,char ,char ) ;
void PrintBufferToLen(char *,short int ) ;
void PrintFieldBuffers( struct FieldBuffer **);
