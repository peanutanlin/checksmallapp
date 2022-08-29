#include <checkapp.h>

#define M_SIZE 1024
static char sql[M_SIZE];

int db_init(const char *db_name)
{
    return sqlite3_open_v2(db_name,&db,SQLITE_OPEN_READWRITE,NULL);
}

void LOG(FILE *fp, char *s)
{
    fprintf(fp, "%s:%s\n", make_time(time(NULL), 0), s);
}

int is_al_num(char *s, int len)
{
    int real_len = 0;
    for(int i = 0; i < len; i++)
    {
        if(isalnum(s[i])) real_len++;
    }
    return real_len;
}

int rj_et_cnt(char *s, int len)
{
    int real_len = 0;
    for(int i = 0; i < len; i++)
    {
        if(s[i] != 13 && s[i] != 32) real_len++;//回车的ASCII码为13,空格的ASCII码是32
    }
    return real_len;
}

char *Left(char *s,int n)
{
    char *res = (char *)calloc(1,sizeof(char)*(n + 1));
    strncpy_m(res, s, n);
    return res;
}

int const_string_to_messagebox(const char *s, int btn_style)
{
    int r = MessageBox(NULL, UTF8ToANSI(s), UTF8ToANSI(TIPS_TITLE), btn_style);
    return r;
}

void sql_reset(char *sql)
{
    memset(sql, '\0', M_SIZE);
}

char *make_time(time_t t, int q)
{
    struct tm *info = localtime(&t);
    char *time_fmt = (char *)calloc(1, sizeof(char) * M_SIZE);
    int y  = info->tm_year + 1900;
    int m  = info->tm_mon  + 1;
    int d  = info->tm_mday;
    int h  = info->tm_hour;
    int mm = info->tm_min;
    int s  = info->tm_sec;
    switch(q)
    {
        case 0 : snprintf(time_fmt, M_SIZE, "%d/%d/%d %d:%d",y,m,d,h,mm); break;
        case 1 : snprintf(time_fmt, M_SIZE, "%d/%d/%d",y,m,d); break;
    }
    return time_fmt;
}

size_t strcat_v2(char **dst_out, ...)
{
    size_t len = 0, len_sub;
    va_list argp;
    char *src;
    char *dst = NULL, *dst_p;

    if(*dst_out) free(*dst_out);
    *dst_out = NULL;

    va_start(argp, dst_out);
    for (;;)
    {
        if ((src = va_arg(argp, char *)) == NULL) break;
        len += strlen(src);
    }
    va_end(argp);
    if (len == 0) return 0;
    dst = (char *)malloc(sizeof(char) * (len + 1));
    if (dst == NULL) return -1;
    dst_p = dst;

    va_start(argp, dst_out);
    for (;;)
    {
        if ((src = va_arg(argp, char *)) == NULL) break;
        len_sub = strlen(src);
        memcpy(dst_p, src, len_sub);
        dst_p += len_sub;
    }
    va_end(argp);
    *dst_p = '\0';
    *dst_out = dst;
    return len;
}

void strncpy_m(char *dest, const char *src, int n)
{
    if(src) strncpy(dest, src, n);
}

/*****************************************************************************************************************************************************************/
int get_opendate(void)
{
    char *open_date = make_time(time(NULL), 1);
    strncpy_m(p->open_date, open_date, sizeof(p->open_date));
    free(open_date);
    return MESSAGE_NONE;
}

int channel_option(void)
{
    if(sdscmp(sdsnew(p->channel_name), sdsnew(HCEZT)) == 0)
    {
        if(is_al_num(p->broker_code, strlen(p->broker_code)) == 0)
        {
            strncpy_m(p->broker_code,     HCZQZBCODE, sizeof(p->broker_code));
            strncpy_m(p->broker_name,     HCZQZBNAME, sizeof(p->broker_name));
            memset(p->department_real, '\0', strlen(p->department_real));
            strncpy_m(p->department_real, WJ,         sizeof(p->department_real));
            strncpy_m(p->department_IB,   HCZQZBNAME, sizeof(p->department_real));
            p->broker_style_code = BROKER_STYLE_IB;
        }
        else if(p->broker_style_code != BROKER_STYLE_IB)
        {
            return HCEZT_NOT_IB;
        }
    }
    if(sdscmp(sdsnew(p->channel_name), sdsnew(THS)) == 0)
    {
        memset(p->department_real, '\0', strlen(p->department_real));
        strncpy_m(p->department_real, WJ, sizeof(p->department_real));
        if(is_al_num(p->broker_code, strlen(p->broker_code))) 
        {
            return BOTH_THS_AND_BROKER;
        }
    }
    return MESSAGE_NONE;
}

void free_sds_array(sds_array *t)
{
    if(t)
    {
        sds *d  = t->data;
        int len = t->array_len;
        for (int i = 0; i < len; ++i)
        {
            if(d[i]) free(d[i]);
        }
        free(t);
    }
}

sds_array *collect_data(char *cs)
{
    int ret = 0;
    sds_array *d = (sds_array *)calloc(1, sizeof(sds_array));
    sds *data = NULL;
    int data_len = 0;

    if(cs)
    {
        int sds_cnt = 0;
        sds ns = sdsnewlen(cs,strlen(cs));
        sds *s = sdssplitlen(ns,sdslen(ns),"\n",1,&sds_cnt);
        for(int i = 0; i < sds_cnt; ++i)
        {
            int cnt = 0;
            sds *tmp = sdssplitlen(s[i],sdslen(s[i]),"\t",1,&cnt);
            data_len = data_len + cnt;
            data = (sds *)realloc(data, data_len * sizeof(sds));
            for (int j = 0; j < cnt; ++j)
            {
                data[data_len - cnt + j] = sdsnewlen(NULL, strlen(tmp[j]));
                sdscpy(data[data_len - cnt + j], tmp[j]);
            }
        }
    }
    d->data = data;
    d->array_len = data_len;
    return d;
}

void real_collect_data(sds field_name, sds *d, char *des, int(*option)(char *, int), int s, int delta)
{
    if(sdscmp(d[s], field_name) == 0) 
    {
        strncpy_m(des, d[s + delta], option(d[s + delta],strlen(d[s + delta])));
    }
}

void check_sqlite_retcode(sqlite3 *db, int sqlite_retcode)
{

}

int insert_customer_data(const char *tablename, int mark)
{
    int sqlite_retcode = 0;
    make_sql_command(INSERT_CUSTOMER, tablename, NULL);
    sqlite_retcode = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite_retcode = sqlite3_step(stmt);
    check_sqlite_retcode(db,sqlite_retcode);
    if(mark){
        if(sqlite_retcode == SQLITE_DONE) 
        {
            tips_and_log(WRITE_SUCCESSFUL); 
        }
        else 
        {
            tips_and_log(WRITE_UNSUCCESSFUL);
        }
    }
    return 0;
}

int insert_jjrcustomer_into_db(void)
{
    int sqlite_retcode = 0;
    make_sql_command(INSERT_JJRCUSTOMER, JJRCUSTOMER, NULL);
    sqlite_retcode = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite_retcode = sqlite3_step(stmt);
    check_sqlite_retcode(db,sqlite_retcode);
    sqlite3_reset(stmt);
    return sqlite_retcode;
}

char *get_clipboard_data(void)
{
    OpenClipboard(NULL);
    char *cs = ANSIToUTF8(GetClipboardData(CF_TEXT));//CF_TEXT CF_OEMTEXT CF_UNICODETEXT
    CloseClipboard();
    if(strlen(cs) < 1500)
    {
        tips_and_log(MESSAGE_DELETION_0);
        cs = NULL;
    }
    return cs;
}

int check_bank_code(void)
{
    int sqlite_retcode = 0;
    make_sql_command(CHECK_BANK_CODE, BASEDATA, NULL);
    sqlite_retcode = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    int ret_row = 0;

    while ((sqlite_retcode = sqlite3_step(stmt)) == SQLITE_ROW) 
    {
        ret_row++;
    }
    check_sqlite_retcode(db, sqlite_retcode);
    sqlite3_reset(stmt);
    
    if (ret_row) 
    {
        return BANK_CODE_EXIST;
    }
    return MESSAGE_NONE;
}


int check_relevance(void)
{
    int sqlite_retcode = 0;
    make_sql_command(CHECK_RELEVANCE, RELEVANCELIST, NULL);
    sqlite_retcode = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    int ret_row = 0;
    char relevanceString[M_SIZE];
    while ((sqlite_retcode = sqlite3_step(stmt)) == SQLITE_ROW) 
    {
        ret_row++;
        strncpy_m(relevanceString, sqlite3_column_text(stmt, 0), sizeof(relevanceString));
    }
    check_sqlite_retcode(db, sqlite_retcode);
    sqlite3_reset(stmt);
    if (ret_row) 
    {
        return RELEVANCE;
    }
    return MESSAGE_NONE;
}

void update_jjrcustomer_opendate(void)
{
    int sqlite_retcode = 0;
    make_sql_command(UPDATE_OPENDATE, JJRCUSTOMER, NULL);
    sqlite_retcode = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite_retcode = sqlite3_step(stmt);
    check_sqlite_retcode(db,sqlite_retcode);
    sqlite3_reset(stmt);
}

int jjr_customer_option(void)
{
    int sqlite_retcode = 0;
    int ret_row = 0;
    int return_visit_successful_mark_from_db = 0;
    char open_date_form_db[M_SIZE];
    make_sql_command(JJRCUSTOMER_OPTION, JJRCUSTOMER, NULL);
    sqlite_retcode = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    while ((sqlite_retcode = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        return_visit_successful_mark_from_db = sqlite3_column_int(stmt,  0);
        strncpy_m(open_date_form_db, sqlite3_column_text(stmt, 1), sizeof(open_date_form_db));
        ret_row++;
    }
    check_sqlite_retcode(db, sqlite_retcode);
    sqlite3_reset(stmt);
    
    if(ret_row &&  return_visit_successful_mark_from_db) 
    {
        update_jjrcustomer_opendate(); 
        return MESSAGE_NONE;
    }
    if(ret_row && !return_visit_successful_mark_from_db) 
    {
        return RETURN_VISIT_UNSUCCESSFUL;
    }
    if(!ret_row) {
        sqlite_retcode = insert_jjrcustomer_into_db();
        switch(sqlite_retcode)
        {
            case SQLITE_DONE : 
                return WRITE_JJR_SUCCESSFUL;
            default : 
                return WRITE_JJR_UNSUCCESSFUL;
        }
    }
}

void set_service_staff(void)
{
    int sqlite_retcode = 0;
    make_sql_command(SET_SERVICE_STAFF, RELATIONSHIPTOIB, NULL);
    sqlite_retcode = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    while ((sqlite_retcode = sqlite3_step(stmt)) == SQLITE_ROW) 
    {
        memset(p->department_real, '\0', sizeof(p->department_real));
        strncpy_m(p->department_real, sqlite3_column_text(stmt, 0), sizeof(p->department_real));
        strncpy_m(p->service_name,    sqlite3_column_text(stmt, 1), sizeof(p->service_name));
        strncpy_m(p->service_code,    sqlite3_column_text(stmt, 2), sizeof(p->service_code));
    }
    check_sqlite_retcode(db, sqlite_retcode);
    sqlite3_reset(stmt);
}

int check_broker_message(void)
{
    int sqlite_retcode = 0;
    make_sql_command(CHECK_BROKER_MESSAGE, BROKERDATA, NULL);
    sqlite_retcode = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    check_sqlite_retcode(db, sqlite_retcode);

    char broker_name_from_db[M_SIZE];
    int select_cnt = 0;
    while ((sqlite_retcode = sqlite3_step(stmt)) == SQLITE_ROW) 
    {
        select_cnt++;
        p->broker_style_code  = sqlite3_column_int(stmt,  0);
        p->fee_adjust = sqlite3_column_int(stmt,  4);
        strncpy_m(p->department_IB,    sqlite3_column_text(stmt, 1), sizeof(p->department_IB));
        memset(p->department_real, '\0', sizeof(p->department_real));
        strncpy_m(p->department_real,  sqlite3_column_text(stmt, 2), sizeof(p->department_real));
        strncpy_m(broker_name_from_db, sqlite3_column_text(stmt, 3), sizeof(broker_name_from_db));
    }
    check_sqlite_retcode(db, sqlite_retcode);
    sqlite3_reset(stmt);

    if(!select_cnt){
        return BROKER_CODE_ERROR;
    }

    if(is_al_num(p->broker_code, strlen(p->broker_code)))
    {
        if(sdscmp(sdsnew(p->broker_name), sdsnew(broker_name_from_db))) return BROKER_NAME_ERROR;
        switch(p->broker_style_code){
            case BROKER_STYLE_INIT : return BROKER_CODE_ERROR;
            case BROKER_STYLE_IB   : set_service_staff();  break;
            case BROKER_STYLE_JJR  : break;
        }
        
    }
    return MESSAGE_NONE;
}

int is_cancel_customer(void)
{
    int sqlite_retcode = 0;
    make_sql_command(IS_CANCEL_CUSTOMER, BASEDATA, NULL);
    sqlite_retcode = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    check_sqlite_retcode(db, sqlite_retcode);
    int ret_row = 0;
    while ((sqlite_retcode = sqlite3_step(stmt)) == SQLITE_ROW) 
    {
        ret_row++;
    }
    check_sqlite_retcode(db, sqlite_retcode);
    sqlite3_reset(stmt);
    
    if (ret_row) 
    {
        return ID_NUMBER_EXIST;
    }
    return MESSAGE_NONE;

}
/*****************************************************************************************************************************************************************/

int mainloop(const char *db_path){
    p = (struct customer_message *)calloc(1, sizeof(struct customer_message));
    p->broker_style_code = BROKER_STYLE_INIT;
    char *s = get_clipboard_data();

    if(s)
    {
        int db_init_ret        = db_init(db_path);
        basedata_field         = init_basedata_field();
        brokerdata_field       = init_brokerdata_field();
        relationshiptoib_field = init_relationshiptoib_field();
        relevancelist_field    = init_relevancelist_field();
        jjrcustomer_field      = init_jjrcustomer_field();
        if(get_customer_data_from_string(s) == 0)
        {
            insert_customer_data(BASEDATA, 1);
            insert_customer_data(TEMPDATA, 0);
        }

        free(basedata_field);
        free(brokerdata_field);
        free(relationshiptoib_field);
        free(relevancelist_field);
        free(jjrcustomer_field);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }
    free(p);
    return 0;
}


int main(int argc, char *argv[])
{
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    if(argc > 1) 
    {
        mainloop(argv[1]); 
    }
    else 
    {
        mainloop(DB_PATH);
    }
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int tips_and_log(enum message_no number_id)
{
    char *name = p->customer_name;
    char *code = p->customer_code;
    FILE *fp = fopen(LOG_FILE_NAME, WRITE_LOG_STYLE);
    int ret = 0;
    int btn_style = MB_OK;
    char *msg = NULL;
    switch(number_id)
    {
        case MESSAGE_DELETION_0         : strcat_v2(&msg, "0-信息复制不完整!", NULL); break;
        case MESSAGE_DELETION_1         : strcat_v2(&msg, "1-信息复制不完整!", NULL); break;
        case WRITE_SUCCESSFUL           : strcat_v2(&msg, code, name, "-写入成功!",                              NULL); break;
        case WRITE_UNSUCCESSFUL         : strcat_v2(&msg, code, name, "-写入失败!请检查!",                       NULL); break;
        case WRITE_JJR_SUCCESSFUL       : strcat_v2(&msg, code, name, "-写入待回访居间客户表成功!",              NULL); break;
        case WRITE_JJR_UNSUCCESSFUL     : strcat_v2(&msg, code, name, "-写入待回访居间客户表失败!请检查!",       NULL); break;
        case RETURN_VISIT_UNSUCCESSFUL  : strcat_v2(&msg, code, name, "-居间客户还未回访成功!请检查!",           NULL); break;
        case REOPEN_CUSTOMER            : strcat_v2(&msg, code, name, "-该客户为二次开户!",                      NULL); break;
        case BROKER_CODE_ERROR          : strcat_v2(&msg, code, name, "-该客户推荐人代码有误!请确认!",           NULL); break;
        case BROKER_NAME_ERROR          : strcat_v2(&msg, code, name, "-该客户推荐人姓名有误!请确认!",           NULL); break;
        case RELEVANCE                  : strcat_v2(&msg, code, name, "-该客户存在关联关系!请确认!",             NULL); break;
        case BOTH_THS_AND_BROKER        : strcat_v2(&msg, code, name, "-该客户为同花顺客户又存在推荐人!请确认!", NULL); btn_style = MB_YESNO; break;
        case APPROPRIATENESS_SCORE_TIPS : strcat_v2(&msg, code, name, "-该客户适当性得分小于37分!请确认!",       NULL); btn_style = MB_YESNO; break;
        case APPROPRIATENESS_DO_TIPS    : strcat_v2(&msg, code, name, "-该客户适当性有效性有误!请确认!",         NULL); btn_style = MB_YESNO; break;
        case BANK_CODE_EXIST            : strcat_v2(&msg, code, name, "-该客户银行卡已存在!请确认!",             NULL); btn_style = MB_YESNO; break;
        case ID_NUMBER_EXIST            : strcat_v2(&msg, code, name, "-该客户身份证号已存在!请确认!",           NULL); btn_style = MB_YESNO; break;
        case HCEZT_NOT_IB               : strcat_v2(&msg, code, name, "-该客户为E智通客户但推荐人非IB介绍人!",   NULL); btn_style = MB_YESNO; break;
    }
    if(msg)
    {
        LOG(fp, msg);
        ret = const_string_to_messagebox((const char *)msg, btn_style);
        free(msg);
    }
    fclose(fp);
    return ret;
}

void make_sql_command(enum sql_command_style st, const char *tablename, void *other_data)
{
    sql_reset(sql);
    switch(st)
    {
        case CHECK_BANK_CODE :      
            snprintf(sql, M_SIZE, "SELECT %s FROM %s WHERE %s=\"%s\"", 
                basedata_field->customer_code->field_name,
                tablename,
                basedata_field->bank_code->field_name, 
                p->bank_code);
            break;
        case CHECK_RELEVANCE :      
            snprintf(sql, M_SIZE, "SELECT %s FROM %s WHERE %s=\"%s\"",
                relevancelist_field->relevance->field_name,
                tablename,
                relevancelist_field->ID_number->field_name,
                p->ID_number);
            break;
        case UPDATE_OPENDATE :      
            snprintf(sql, M_SIZE, "UPDATE %s SET %s=\"%s\" WHERE %s=\"%s\"",
                tablename, 
                jjrcustomer_field->open_date->field_name, 
                p->open_date, 
                jjrcustomer_field->customer_code->field_name, 
                p->customer_code);
            break;
        case JJRCUSTOMER_OPTION :   
            snprintf(sql, M_SIZE, "SELECT %s,%s FROM %s WHERE %s=\"%s\"",
                jjrcustomer_field->return_visit_successful_mark->field_name,
                jjrcustomer_field->open_date->field_name,
                tablename,
                jjrcustomer_field->customer_code->field_name,
                p->customer_code);
            break;
        case SET_SERVICE_STAFF :    
            snprintf(sql, M_SIZE, "SELECT %s,%s,%s FROM %s WHERE %s=\"%s\"",
                relationshiptoib_field->department_real->field_name,
                relationshiptoib_field->service_name->field_name,
                relationshiptoib_field->service_code->field_name,
                tablename,
                relationshiptoib_field->department_IB->field_name,
                p->department_IB);
            break;
        case CHECK_BROKER_MESSAGE : 
            snprintf(sql, M_SIZE, "SELECT %s,%s,%s,%s,%s FROM %s WHERE %s=\"%s\"",
                brokerdata_field->broker_style_code->field_name,
                brokerdata_field->department_IB->field_name,
                brokerdata_field->department_real->field_name,
                brokerdata_field->broker_name->field_name,
                brokerdata_field->fee_adjust->field_name,
                tablename,
                brokerdata_field->broker_code->field_name,
                p->broker_code);
            break;
        case IS_CANCEL_CUSTOMER :   
            snprintf(sql, M_SIZE, "SELECT %s FROM %s WHERE %s=\"%s\"", 
                basedata_field->customer_code->field_name,
                tablename,
                basedata_field->ID_number->field_name, 
                p->ID_number);
            break;
        case INSERT_CUSTOMER :
            snprintf(sql, M_SIZE,"INSERT INTO %s (%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s) \
                VALUES (\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,%d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\")",
                        tablename,
                        basedata_field->customer_code->field_name,//1
                        basedata_field->customer_name->field_name,//2
                        basedata_field->ID_number->field_name,//3
                        basedata_field->open_date->field_name,//4
                        basedata_field->broker_name->field_name,//5
                        basedata_field->broker_code->field_name,//6
                        basedata_field->service_name->field_name,//7
                        basedata_field->service_code->field_name,//8
                        basedata_field->department_real->field_name,//9
                        basedata_field->department_IB->field_name,//10
                        basedata_field->fee_adjust->field_name,//11
                        basedata_field->appropriateness_score->field_name,//12
                        basedata_field->risk_rank->field_name,//13
                        basedata_field->bank_name->field_name,//14
                        basedata_field->bank_code->field_name,//15
                        basedata_field->cellphone_number->field_name,//16
                        basedata_field->channel_name->field_name,//17
                        basedata_field->other_message->field_name,//18
                        p->customer_code,//1
                        p->customer_name,//2
                        p->ID_number,//3
                        p->open_date,//4
                        p->broker_name,//5
                        p->broker_code,//6
                        p->service_name,//7
                        p->service_code,//8
                        p->department_real,//9
                        p->department_IB,//10
                        p->fee_adjust,//11
                        p->approp_score,//12
                        p->risk_rank,//13
                        p->bank_name,//14
                        p->bank_code,//15
                        p->cell_number,//16
                        p->channel_name,//17
                        p->other_message//18
                        );
            break;
        case INSERT_JJRCUSTOMER :
            time_t s_time = time(NULL);
            time_t e_time = s_time + 24*60*60;
            char *s_time_string = make_time(s_time, 0);
            char *e_time_string = make_time(e_time, 0);
            snprintf(sql, M_SIZE, "INSERT INTO %s (%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s) \
                VALUES (\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,\"%s\",\"%s\",\"%s\",\"%s\",%d,\"%s\",\"%s\",\"%s\",\"%s\")",
                tablename,
                jjrcustomer_field->customer_code->field_name,
                jjrcustomer_field->customer_name->field_name,
                jjrcustomer_field->cellphone_number->field_name,
                jjrcustomer_field->broker_name->field_name,
                jjrcustomer_field->broker_code->field_name,
                jjrcustomer_field->fee_adjust->field_name,
                jjrcustomer_field->department_real->field_name,
                jjrcustomer_field->submit_open_acc_time->field_name,
                jjrcustomer_field->enble_return_visit_time->field_name,
                jjrcustomer_field->return_visit_successful_time->field_name,
                jjrcustomer_field->return_visit_successful_mark->field_name,
                jjrcustomer_field->open_date->field_name,
                jjrcustomer_field->notice_sign_style->field_name,
                jjrcustomer_field->notice_sign_time->field_name,
                jjrcustomer_field->other_message->field_name,
                p->customer_code,
                p->customer_name,
                p->cell_number,
                p->broker_name,
                p->broker_code,
                p->fee_adjust,
                p->department_real,
                s_time_string,e_time_string,"",0,"","","","");
            free(s_time_string);
            free(e_time_string);
            break;
    }
    //PRINT(sql);
}


struct table_field *init_single_field(const char *field_name, enum field_type_enum field_type)
{
    struct table_field *p = (struct table_field *)calloc(1,sizeof(struct table_field));
    strncpy_m(p->field_name, field_name, sizeof(p->field_name));
    p->field_type = field_type;
    return p;
}

struct sqlite_table_basedata *init_basedata_field(void)
{
    struct sqlite_table_basedata *t = (struct sqlite_table_basedata *)calloc(1,sizeof(struct sqlite_table_basedata));
    if(t)
    {
        t->customer_code         = init_single_field("客户号",       M_SQLITE_TEXT);
        t->customer_name         = init_single_field("客户姓名",     M_SQLITE_TEXT);
        t->ID_number             = init_single_field("身份证号",     M_SQLITE_TEXT);
        t->open_date             = init_single_field("开户日期",     M_SQLITE_TEXT);
        t->broker_name           = init_single_field("推荐人姓名",   M_SQLITE_TEXT);
        t->broker_code           = init_single_field("推荐人代码",   M_SQLITE_TEXT);
        t->service_name          = init_single_field("服务人员姓名", M_SQLITE_TEXT);
        t->service_code          = init_single_field("服务人员代码", M_SQLITE_TEXT);
        t->department_real       = init_single_field("客户所属部门", M_SQLITE_TEXT);
        t->department_IB         = init_single_field("证券营业部",   M_SQLITE_TEXT);
        t->fee_adjust            = init_single_field("手续费调整",   M_SQLITE_INT);
        t->appropriateness_score = init_single_field("适当性得分",   M_SQLITE_INT);
        t->risk_rank             = init_single_field("适当性等级",   M_SQLITE_TEXT);
        t->bank_name             = init_single_field("开户银行",     M_SQLITE_TEXT);
        t->bank_code             = init_single_field("银行账号",     M_SQLITE_TEXT);
        t->cellphone_number      = init_single_field("手机号",       M_SQLITE_TEXT);
        t->channel_name          = init_single_field("渠道号",       M_SQLITE_TEXT);
        t->other_message         = init_single_field("备注",         M_SQLITE_TEXT);
    }
    else
    {
        return NULL;
    }
    return t;
}

struct sqlite_table_brokerdata *init_brokerdata_field(void)
{
    struct sqlite_table_brokerdata *t = (struct sqlite_table_brokerdata*)calloc(1, sizeof(struct sqlite_table_brokerdata));
    if(t)
    {
        t->broker_code       = init_single_field("推荐人代码",     M_SQLITE_TEXT);
        t->broker_name       = init_single_field("推荐人姓名",     M_SQLITE_TEXT);
        t->broker_style      = init_single_field("推荐人类型",     M_SQLITE_TEXT);
        t->broker_style_code = init_single_field("类型代码",       M_SQLITE_INT);
        t->department_IB     = init_single_field("证券营业部",     M_SQLITE_TEXT);
        t->department_real   = init_single_field("推荐人所属部门", M_SQLITE_TEXT);
        t->fee_adjust        = init_single_field("手续费调整",     M_SQLITE_INT);
        t->other_message     = init_single_field("备注",           M_SQLITE_TEXT);
    }
    else
    {
        return NULL;
    }
    return t;
}
struct sqlite_table_relationshiptoib *init_relationshiptoib_field(void)
{
    struct sqlite_table_relationshiptoib *t = (struct sqlite_table_relationshiptoib*)calloc(1, sizeof(struct sqlite_table_relationshiptoib));
    if(t)
    {
        t->index               = init_single_field("序号",         M_SQLITE_INT);
        t->IB_assessment_unit  = init_single_field("考核单元",     M_SQLITE_TEXT);
        t->department_IB       = init_single_field("证券营业部",   M_SQLITE_TEXT);
        t->department_real     = init_single_field("对接部门",     M_SQLITE_TEXT);
        t->service_name        = init_single_field("服务人员名称", M_SQLITE_TEXT);
        t->service_code        = init_single_field("服务人员代码", M_SQLITE_TEXT);
        t->department_IB_style = init_single_field("营业部类型",   M_SQLITE_TEXT);
        t->IB_seniority        = init_single_field("介绍资格",     M_SQLITE_TEXT);
        t->IB_seniority_style  = init_single_field("介绍资格类型", M_SQLITE_TEXT);
        t->IB_post_name        = init_single_field("开户人员名称", M_SQLITE_TEXT);
        t->other_message       = init_single_field("备注",         M_SQLITE_TEXT);
    }
    else
    {
        return NULL;
    }
    return t;
}

struct sqlite_table_relevancelist *init_relevancelist_field(void)
{
    struct sqlite_table_relevancelist *t = (struct sqlite_table_relevancelist*)calloc(1, sizeof(struct sqlite_table_relevancelist));
    if(t)
    {
        t->ID_number = init_single_field("证件号码", M_SQLITE_TEXT);
        t->relevance = init_single_field("关联关系", M_SQLITE_TEXT);
    }
    else
    {
        return NULL;
    }
    return t;
}

struct sqlite_table_jjrcustomer *init_jjrcustomer_field(void)
{
    struct sqlite_table_jjrcustomer *t = (struct sqlite_table_jjrcustomer *)calloc(1, sizeof(struct sqlite_table_jjrcustomer));
    if(t)
    {
        t->index                        = init_single_field("序号",             M_SQLITE_INT);
        t->customer_code                = init_single_field("客户号",           M_SQLITE_TEXT);
        t->customer_name                = init_single_field("客户姓名",         M_SQLITE_TEXT);
        t->cellphone_number             = init_single_field("手机号",           M_SQLITE_TEXT);
        t->broker_name                  = init_single_field("推荐人姓名",       M_SQLITE_TEXT);
        t->broker_code                  = init_single_field("推荐人代码",       M_SQLITE_TEXT);
        t->fee_adjust                   = init_single_field("手续费调整",       M_SQLITE_INT);
        t->department_real              = init_single_field("客户所属部门",     M_SQLITE_TEXT);
        t->submit_open_acc_time         = init_single_field("提交开户申请时间", M_SQLITE_TEXT);
        t->enble_return_visit_time      = init_single_field("可回访时间",       M_SQLITE_TEXT);
        t->return_visit_successful_time = init_single_field("回访成功时间",     M_SQLITE_TEXT);
        t->return_visit_successful_mark = init_single_field("回访是否成功",     M_SQLITE_INT);
        t->open_date                    = init_single_field("开户日期",         M_SQLITE_TEXT);
        t->notice_sign_style            = init_single_field("告知书签署方式",   M_SQLITE_TEXT);
        t->notice_sign_time             = init_single_field("告知书签署时间",   M_SQLITE_TEXT);
        t->other_message                = init_single_field("备注",             M_SQLITE_TEXT);
    }
    else
    {
        return NULL;
    }
    return t;
}
int get_customer_data_from_string(char *cs)
{
    int ret = 0;
    int check_ret = 0;
    int tips_and_log_ret = 0;
    sds_array *t = collect_data(cs);
    sds *d = t->data;
    int len = t->array_len;
    int is_jjr_customer = 0;
    for(int i = 0; i < len; ++i)
    {
        if(sdscmp(d[i], sdsnew("是否存在居间人")) == 0) 
        {
            if (sdscmp(d[i + 1], sdsnew("是")) == 0) 
            {
                is_jjr_customer = 1;
            }
        }
        real_collect_data(sdsnew("资金账户"), d, p->customer_code, is_al_num, i, 2);
        real_collect_data(sdsnew("身份证"),   d, p->ID_number,     is_al_num, i, 1);

        if(is_jjr_customer)
        {
            real_collect_data(sdsnew("居间人登记编号"), d, p->broker_code, is_al_num, i, 1);
            real_collect_data(sdsnew("居间人姓名"),     d, p->broker_name, rj_et_cnt, i, 1);
        }
        else
        {
            real_collect_data(sdsnew("推荐人编号"), d, p->broker_code, is_al_num, i, 1);
            real_collect_data(sdsnew("推荐人姓名"), d, p->broker_name, rj_et_cnt, i, 1);
        }
        real_collect_data(sdsnew("银行卡号"),       d, p->bank_code,       is_al_num, i, 1);
        real_collect_data(sdsnew("注册电话"),       d, p->cell_number,     is_al_num, i, 1);
        real_collect_data(sdsnew("姓名"),           d, p->customer_name,   rj_et_cnt, i, 1);
        real_collect_data(sdsnew("开户营业部"),     d, p->department_open, rj_et_cnt, i, 1);
        real_collect_data(sdsnew("风险等级"),       d, p->risk_rank,       rj_et_cnt, i, 1);
        real_collect_data(sdsnew("银行名称"),       d, p->bank_name,       rj_et_cnt, i, 1);
        real_collect_data(sdsnew("身份证有效期限"), d, p->period_ID,       rj_et_cnt, i, 1);
        real_collect_data(sdsnew("风险试题有效性"), d, p->approp_do,       rj_et_cnt, i, 1);
        real_collect_data(sdsnew("渠道"),           d, p->channel_name,    rj_et_cnt, i, 1);

        if(sdscmp(d[i], sdsnew("期货账户")) == 0) 
        {
            if(sdslen(d[i + 1]) >= 100) 
            {
                strncpy_m(p->other_message, REOPEN_CUSTOMER_M, strlen(REOPEN_CUSTOMER_M));
            }
        }

        if(sdscmp(d[i], sdsnew("得分")) == 0) 
        {
            int score_len = is_al_num(d[i + 1], sdslen(d[i + 1]));
            char *score_string = Left(d[i + 1], score_len);
            p->approp_score = atoi(score_string);
            if(p->approp_score < 37)
            {
                tips_and_log_ret = tips_and_log(APPROPRIATENESS_SCORE_TIPS);
                if(tips_and_log_ret == BTN_NO) 
                {
                    return tips_and_log_ret;
                }
            }
            free(score_string);
        }
    }
    /*for end*/

    if(sdscmp(sdsnew(p->other_message), sdsnew(REOPEN_CUSTOMER_M)) == 0) 
    {
        tips_and_log(REOPEN_CUSTOMER);
    }
    strncpy_m(p->department_real, p->department_open, strlen(p->department_open));
    
    if(is_al_num(p->broker_code, strlen(p->broker_code)))
    {
        check_ret = check_broker_message(); 
        if(check_ret) 
        {
            tips_and_log_ret = tips_and_log(check_ret); 
            return check_ret;
        }
    }

    check_ret = channel_option(); 
    if(check_ret) 
    {
        tips_and_log_ret = tips_and_log(check_ret); 
            if(tips_and_log_ret == BTN_NO) 
            {
                return check_ret;
            }
    }

    if(p->broker_style_code == BROKER_STYLE_JJR) 
    {
        check_ret = jjr_customer_option(); 
        tips_and_log(check_ret); 
        if(check_ret) 
        {
            return check_ret;
        }
    }

    check_ret = get_opendate();       
    if(check_ret) 
    {
        tips_and_log_ret = tips_and_log(check_ret); 
    }

    check_ret = check_relevance();    
    if(check_ret) 
    {
        tips_and_log_ret = tips_and_log(check_ret); 
    }

    check_ret = check_bank_code();    
    if(check_ret) 
    {
        tips_and_log_ret = tips_and_log(check_ret); 
        if(tips_and_log_ret == BTN_NO) 
        {
            return check_ret;
        }
    }
    
    check_ret = is_cancel_customer(); 
    if(check_ret) 
    {
        tips_and_log_ret = tips_and_log(check_ret); 
        if(tips_and_log_ret == BTN_NO) 
        {
            return check_ret;
        }
    }

    free_sds_array(t);
    return ret;
}
/////////////////////////////////////////////////////////