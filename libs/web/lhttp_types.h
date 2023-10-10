#ifndef LHTTP_TYPES_H
#define LHTTP_TYPES_H

//http request metod
enum HttpReqMetod {hrmGet = 501, hrmPost, hrmUnknown = -1};

//http request metod
enum HttpProtocolType {hptHttp = 521, hptHttps, htpUnknown = -1};

//http reply error
enum HttpReplyError {hreOk = 0,
                     hreWrongReqParams = 531,
                     hreReplyNull,
                     hreServerErr,
                     hreInvalidMetod,
                     hreUnknown = -1};




#endif


