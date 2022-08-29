#ifndef __CHECKAPP_H
#define __CHECKAPP_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>
#include <locale.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>

#include <include/sds.h>
#include <include/sdsalloc.h>
#include <include/sqlite3.h>
#include <include/charsetchange.h>

#define BTN_NO 7

typedef struct sds_array {
    int array_len;
    sds *data;
}sds_array;

typedef enum broker_style {
    BROKER_STYLE_YG   = 0,
    BROKER_STYLE_IB   = 1,
    BROKER_STYLE_JJR  = 2,
    BROKER_STYLE_NONE = 100,
    BROKER_STYLE_INIT = 101
}broker_style;

enum message_no {
    MESSAGE_NONE       = 0,
    MESSAGE_DELETION_0,
    MESSAGE_DELETION_1,
    WRITE_SUCCESSFUL,
    WRITE_UNSUCCESSFUL,
    WRITE_JJR_SUCCESSFUL,
    WRITE_JJR_UNSUCCESSFUL,
    RETURN_VISIT_UNSUCCESSFUL,
    REOPEN_CUSTOMER,
    BOTH_THS_AND_BROKER,
    APPROPRIATENESS_SCORE_TIPS,
    APPROPRIATENESS_DO_TIPS,
    BANK_CODE_EXIST,
    ID_NUMBER_EXIST,
    BROKER_CODE_ERROR,
    BROKER_NAME_ERROR,
    RELEVANCE,
    HCEZT_NOT_IB,     
};

enum field_type_enum{
    M_SQLITE_INT  = 0,
    M_SQLITE_TEXT,
    M_SQLITE_REAL,
    M_SQLITE_BLOB
};

enum sql_command_style{
    CHECK_BANK_CODE = 0,
    CHECK_RELEVANCE,
    UPDATE_OPENDATE,
    JJRCUSTOMER_OPTION,
    SET_SERVICE_STAFF,
    CHECK_BROKER_MESSAGE,
    IS_CANCEL_CUSTOMER,
    INSERT_CUSTOMER,
    INSERT_JJRCUSTOMER,
};

const char *channel_list[] = {
    "hcezt",
    "thsfutures"
};

const char *db_path_list[] = {
    "base.db"
};

const char *department_list[] = {
    "网金",
    "总部",
    "贵州分公司",
    "大连分公司",
    "山东分公司",
    "业务发展总部",
    "华创证券总部",
    "IBHC001"
};

const char *tablename_list[] = {
    "BASEDATA",
    "TEMPDATA",
    "BROKERDATA",
    "JJRCUSTOMER",
    "RELATIONSHIPTOIB",
    "RELEVANCELIST",

};

const char *other_const_string[] = {
    "二次开户",
    "提示信息",
    "有效",
    "app.log",
    "a+"
};
#define DB_PATH      db_path_list[0]
#define HCEZT        channel_list[0]
#define THS          channel_list[1]
#define WJ           department_list[0]
#define HCZQZBNAME   department_list[6]
#define HCZQZBCODE   department_list[7]

#define BASEDATA         tablename_list[0]
#define TEMPDATA         tablename_list[1]
#define BROKERDATA       tablename_list[2]
#define JJRCUSTOMER      tablename_list[3]
#define RELATIONSHIPTOIB tablename_list[4]
#define RELEVANCELIST    tablename_list[5]

#define REOPEN_CUSTOMER_M  other_const_string[0]
#define TIPS_TITLE         other_const_string[1]
#define APPROPRIATENESS_DO other_const_string[2]
#define LOG_FILE_NAME      other_const_string[3]
#define WRITE_LOG_STYLE    other_const_string[4]


struct table_field{
    char field_name[32];
    enum field_type_enum  field_type;
};



struct sqlite_table_basedata{
    struct table_field *customer_code;
    struct table_field *customer_name;
    struct table_field *ID_number;
    struct table_field *open_date;
    struct table_field *broker_name;
    struct table_field *broker_code;
    struct table_field *service_name;
    struct table_field *service_code;
    struct table_field *department_real;
    struct table_field *department_IB;
    struct table_field *fee_adjust;
    struct table_field *appropriateness_score;
    struct table_field *risk_rank;
    struct table_field *bank_name;
    struct table_field *bank_code;
    struct table_field *cellphone_number;
    struct table_field *channel_name;
    struct table_field *other_message;
};

struct sqlite_table_brokerdata{
    struct table_field *broker_code;
    struct table_field *broker_name;
    struct table_field *broker_style;
    struct table_field *broker_style_code;
    struct table_field *department_IB;
    struct table_field *department_real;
    struct table_field *fee_adjust;
    struct table_field *other_message;
};

struct sqlite_table_relationshiptoib{
    struct table_field *index;
    struct table_field *IB_assessment_unit;
    struct table_field *department_IB;
    struct table_field *department_real;
    struct table_field *service_name;
    struct table_field *service_code;
    struct table_field *department_IB_style;
    struct table_field *IB_seniority;
    struct table_field *IB_seniority_style;
    struct table_field *IB_post_name;
    struct table_field *other_message;
};

struct sqlite_table_relevancelist{
    struct table_field *ID_number;
    struct table_field *relevance;
};

struct sqlite_table_jjrcustomer{
    struct table_field *index;
    struct table_field *customer_code;
    struct table_field *customer_name;
    struct table_field *cellphone_number;
    struct table_field *broker_name;
    struct table_field *broker_code;
    struct table_field *fee_adjust;
    struct table_field *department_real;
    struct table_field *submit_open_acc_time;
    struct table_field *enble_return_visit_time;
    struct table_field *return_visit_successful_time;
    struct table_field *return_visit_successful_mark;
    struct table_field *open_date;
    struct table_field *notice_sign_style;
    struct table_field *notice_sign_time;
    struct table_field *other_message;
};


struct customer_message{
    char customer_code[32];//客户号-0
    char customer_name[128];//客户姓名-1
    char ID_number[32];//身份证号-2
    char open_date[32];//开户日期-3
    char broker_name[128];//推荐人姓名-4
    char broker_code[32];//推荐人代码-5
    char service_name[32];//服务人员姓名-6
    char service_code[32];//服务人员代码-7
    char department_real[64];//客户所属部门-8
    char department_IB[64];//客户所属IB营业部-9
    char department_open[64];//客户开户营业部-10
    int fee_adjust;//手续费调整标志
    char risk_rank[32];//适当性风险等级-12
    char bank_name[32];//开户行-13
    char bank_code[32];//银行账号-14
    char period_ID[32];//身份证有效期-15
    char cell_number[32];//手机号-16
    char check_name[32];//复核人员姓名-17
    char approp_do[32];//适当性有效性-18
    char channel_name[32];//渠道号-19
    char other_message[128];//其他备注信息-20

    int  approp_score;//适当性得分-11
    int  broker_style_code;//经纪人类型代码
};

static sqlite3 *db = NULL;
static sqlite3_stmt *stmt = NULL;

static struct customer_message *p = NULL;

static struct sqlite_table_basedata         *basedata_field         = NULL;
static struct sqlite_table_brokerdata       *brokerdata_field       = NULL;
static struct sqlite_table_relationshiptoib *relationshiptoib_field = NULL;
static struct sqlite_table_relevancelist    *relevancelist_field    = NULL;
static struct sqlite_table_jjrcustomer      *jjrcustomer_field      = NULL;

struct table_field *init_single_field(const char *, enum field_type_enum);
struct sqlite_table_basedata *init_basedata_field(void);
struct sqlite_table_brokerdata *init_brokerdata_field(void);
struct sqlite_table_relationshiptoib *init_relationshiptoib_field(void);
struct sqlite_table_relevancelist *init_relevancelist_field(void);
struct sqlite_table_jjrcustomer *init_jjrcustomer_field(void);

char *get_clipboard_data(void);
int  get_customer_data_from_string(char *);
char *Left(char *s,int n);
void check_sqlite_retcode(sqlite3 *, int);
int  db_init(const char *);
int  mainloop(const char *);
int tips_and_log(enum message_no);

int insert_customer_data(const char *, int);
int jjr_customer_option(void);
int insert_jjrcustomer_into_db(void);
char *make_time(time_t, int);
int rj_et_cnt(char *, int);
size_t strcat_v2(char **d, ...);
int is_al_num(char *, int);
void sql_reset(char *);
void strncpy_m(char *, const char *, int);

int get_opendate(void);
int channel_option(void);

void make_sql_command(enum sql_command_style, const char *, void *);
int check_relevance(void);
int is_cancel_customer(void);
int check_broker_message(void);
int check_bank_code(void);
int const_string_to_messagebox(const char *, int);

void set_service_staff(void);

void free_sds_array(sds_array *);
sds_array *collect_data(char *);
void real_collect_data(sds, sds *, char *, int(*option)(char *, int), int, int);
void printStruct(void);
#endif
