
#ifndef _sms_h_
#define _sms_h_

#ifndef _sms_c_
typedef void *psms_t;
#endif

psms_t sms_destroy(psms_t sms);
psms_t sms_new(char *src, char *dst, char *msg);
int    sms_status_set(psms_t sms, int status);
int    sms_id_set(psms_t sms, char *id);
int    sms_is_rpt(psms_t s);
//prpt_t sms_rpt_get(psms_t sms, prpt_t rpt);

#endif
