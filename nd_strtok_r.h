#include "token.h"


// #ifndef _MAX_TOKEN_LENGTH
// #ifndef MAX_TOKEN_LENGTH
// #define _MAX_TOKEN_LENGTH 64
// #else 
// #define _MAX_TOKEN_LENGTH MAX_TOKEN_LENGTH 
// #endif
// #endif 
// #pragma message("cpp MAX_TOKEN_LENGTH="_MAX_TOKEN_LENGTH )

namespace NotDestroyed {

Token strtok_r(const char * str, const char divider, const char ** rest){
    if ( str == nullptr || str[0] == '\0' ) { 
        debugPrintf("str = nullptr or NULL_STR \n");
        *rest = nullptr; 
        return Token();
    }
    debugPrintf("str = '%s'\n", str );

    *rest = strchr( str, divider);
    
    long len;
    if ( *rest != nullptr ) {
        len = *rest-str;
        (*rest)++;
        {   
            #ifdef ND_STRTOK_R_DEBUG
            auto dup = strndup( str, len);
            debugPrintf("%s, %d\nRest %s\n", dup , len, *rest );
            free(dup);    
            #endif
        }
    } else {
        len = strlen(str);
        debugPrintf("%s, %d\nNo next divider\n", str, len);
    }
    return Token(str, len);
};

Token getToken(const char *str, const char div, const char * name){
    Token token;
    const char * rest = str;
    while( ( token = NotDestroyed::strtok_r(rest, div, &rest ))){
        if ( token.isEquals(name) ) return token; 
    }
    return Token();
    };

Token Token::_value() const {
    const char divider = ptr[length];
    if ( divider == '\0' ) return Token();

    auto rest = ptr + length +1;
    debugPrintf("Token %s, divider=%c, rest=%s\n", ptr, divider, rest );
    return NotDestroyed::strtok_r(rest, divider, &rest );
};

char* Token::value() const {
    if ( isValid() ){
        // const char divider = ptr[length];
        // auto rest = ptr + length +1;
        // debugPrintf("Token %s, divider=%c, rest=%s\n", ptr, divider, rest );
        // Token tok = NotDestroyed::strtok_r(rest, divider, &rest );
        // return tok.tokdup();
        return _value().tokdup();
    }
    return nullptr;
};
char* Token::value(char * dest) const {
    if ( isValid() ){
        // const char divider = ptr[length];
        // auto rest = ptr + length +1;
        // debugPrintf("Token %s, divider=%c, rest=%s\n", ptr, divider, rest );
        // Token tok = NotDestroyed::strtok_r(rest, divider, &rest );
        // return tok.tokcpy(*dest);
        return _value().tokcpy(dest);
    }
    return nullptr;
};

char * getTokenValue(const char *str, const char div, const char * name){
    return getToken(str,div,name).value();
};


}