#pragma once
#include <Arduino.h>


//#define ND_STRTOK_R_DEBUG
#ifdef ND_STRTOK_R_DEBUG
#define debugPrintf(fmt, ...) { Serial.print(__PRETTY_FUNCTION__); Serial.println(__LINE__); Serial.printf((PGM_P)PSTR(fmt), ##__VA_ARGS__); Serial.flush();}
#define ptrPrint(ptr)   ptr?ptr:""
#else
#define debugPrintf
#define ptrPrint
#endif

//#define STR_ARRAY
// prep string concat vars

#ifdef STR_ARRAY
#define WM_STRING2(x) #x
#define WM_STRING(x) WM_STRING2(x)  

#ifndef MAX_TOKEN_LENGTH
#define _MAX_TOKEN_LENGTH 128
#else
#define _MAX_TOKEN_LENGTH MAX_TOKEN_LENGTH 
#endif

#pragma message("MAX_TOKEN_LENGTH=" WM_STRING(_MAX_TOKEN_LENGTH) )
#endif

namespace NotDestroyed {
//    static const int _MAX_TOKEN_LENGTH;

    class Token {
        private:
        const char * ptr;
        long length;

#ifdef STR_ARRAY
       char _cStrbuf[_MAX_TOKEN_LENGTH] = {0};
#else
        char * _cStrbuf = nullptr; 
        void clean(){ 
            if ( _cStrbuf ) {
                debugPrintf("delete '%s'\n", _cStrbuf );
                //free( _cStrbuf);
                delete[] _cStrbuf;
                _cStrbuf = nullptr;
            }
        };
#endif
        //
        public:
        //static void free(){ delete[] _cStrbuf; };
        Token(const char * ptr, const long length) :
            ptr(ptr), length(length){
                #ifdef STR_ARRAY
                debugPrintf("MAX_TOKEN_LENGTH=%d\n", _MAX_TOKEN_LENGTH);
                #endif
                debugPrintf("Constructor.\n\tptr '%s', length=%d\n", ptrPrint(ptr), length);
            };
        Token(const char * ptr) :
            Token(ptr,strlen(ptr))
            {}; 
        Token() :
            Token(nullptr,0)
            {};
        ~Token(){   
            debugPrintf("Destuctor.\n\t buf = '%s'\n",  _cStrbuf ? _cStrbuf : "" );
            #ifdef STR_ARRAY    
            
            #else
            clean();
            #endif
        };
        bool isValid() const { return ptr != nullptr && length != 0; };
        operator bool() const { return isValid(); };
        Token& operator=(const Token& srs ){
            length = srs.length;
            debugPrintf("Copy %d ptr '%s' from '%s'\n", length, ptrPrint(this->ptr), ptrPrint(srs.ptr));
            ptr = srs.ptr;
            debugPrintf("New ptr '%s'\n", ptrPrint(ptr));
            return *this;
        };

        Token& operator=(const char* _ptr){ 
                debugPrintf("Copy ptr '%s'<='%s'\n", ptrPrint(this->ptr), ptrPrint(_ptr));
                ptr = _ptr;
                length = strlen( _ptr);
                return *this;
        };

        // bool operator==(const char *_ptr){
        //     return ( length == strlen(_ptr) && strncmp(ptr,_ptr,length ) == 0 );
        // };
        bool isEquals(const char *_ptr){
            return ( length == strlen(_ptr) && strncmp(ptr,_ptr,length ) == 0 );
        };
        bool isEquals_P(const char* _ptr ){
            return ( length == strlen_P(_ptr) && strncmp_P(ptr,_ptr,length ) == 0 );
        };
        bool isEquals(const __FlashStringHelper* _ptr ){
            return ( length == strlen_P((PGM_P)_ptr) && strncmp_P(ptr,(PGM_P)_ptr,length ) == 0 );
        };
        bool operator==(const char *_ptr){
            return isEquals( _ptr );
            //     return ( length == strlen_P(_ptr) && strncmp_P(ptr,_ptr,length ) == 0 );
        };
        bool operator==(const __FlashStringHelper* _ptr ){
            return isEquals_P((PGM_P)_ptr );
        };
        bool operator==(const Token& other){
            return ( length == other.length && strncmp(ptr, other.ptr ,length ) == 0 );
        };

        // создает новуя строку в памяти
        // используйте free(str) для освобождения памяти
        char * tokdup()const { return ::strndup(ptr,length); };
        char * tokcpy(char * dest )const { 
            dest[length] = '\0';
            return ::strncpy(dest,ptr,length); };

        /// возвращает указатель на временный буффер
        /// копируйте буфер для дальнейшего использования
        char * c_str() {
            #ifdef STR_ARRAY
            return tokcpy(_cStrbuf);
            #else
            debugPrintf("Try to reserve %d bytes\n", length+1 );
            if ( ! _cStrbuf ) { 
                clean();
                _cStrbuf =  new char[length+1]();
            }
            if ( _cStrbuf ) {
                debugPrintf("Success\n");
                return tokcpy(_cStrbuf);
            } else {
                return nullptr;
            }
            #endif
        };
        String toStr()  {
            return String( c_str() );
        };
        // operator char*(){
        //   return c_str();  
        // };
        const long len() const { return length; };
        const char * rest() const { return ptr; };
        char* value() const ;
        char* value(char *dest) const; 
        Token _value() const;
    };

    Token strtok_r(const char * str, const char devider, const char ** rest);
    Token getToken(const char *str, const char div, const char * name); 
    char * getTokenValue(const char *str, const char div, const char * name);

};

