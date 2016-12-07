
#ifndef STRINGTOOLS_H
#define STRINGTOOLS_H


namespace common {

    
template <class S>
S& trimRightZeroes(S& str)
{
    int pos = int(str.size()) - 1;

    while (pos >= 0 && str[pos] != '.') --pos;

    if(pos < 0) return str;

    pos = int(str.size()) - 1;

    while (pos >= 0) {
        if(str[pos] == '0')
            --pos;
        else if(str[pos] == '.') {
            --pos;
            break;
        }
        else {
            break;
        }
    }
    str.resize(pos + 1);

    return str;
}


}


#endif

