#include "Lib.h"


Lib::Lib(){};
Lib::~Lib(){};

string Lib::getDate(string format, bool needUsec)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time_t t;
    t = tv.tv_sec;
    char tmp[20];
    strftime(tmp, sizeof(tmp), format.c_str(), localtime(&t));
    string s(tmp);
    if (needUsec) {
        s += "." + itos(tv.tv_usec);
    }
    return s;
};

char * Lib::stoc(string str)
{
    const char * s = str.c_str();
    char * ch = new char[strlen(s) + 1];
    strcpy(ch, s);
    return ch;
};

int Lib::stoi(string s)
{
    return atoi(s.c_str());
}

double Lib::stod(string s)
{
    return atof(s.c_str());
}

string Lib::dtos(double dbl)
{
    std::ostringstream strs;
    strs << dbl;
    std::string str = strs.str();
    return str;
};

string Lib::itos(int num)
{
    char s[10];
    sprintf(s, "%d", num);
    return string(s);
}

vector<string> Lib::split(const string& s, const string& delim)
{
    vector<string> elems;
    size_t pos = 0;
    size_t len = s.length();
    size_t delim_len = delim.length();
    if (delim_len == 0) return elems;
    while (pos < len)
    {
        int find_pos = s.find(delim, pos);
        if (find_pos < 0)
        {
            elems.push_back(s.substr(pos, len - pos));
            break;
        }
        elems.push_back(s.substr(pos, find_pos - pos));
        pos = find_pos + delim_len;
    }
    return elems;
}

int Lib::code_convert(string from_charset, string to_charset, char *inbuf, size_t inlen,
        char *outbuf, size_t outlen) {
    iconv_t cd;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(stoc(to_charset), stoc(from_charset));
    if (cd == 0)
        return -1;
    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        return -1;
    iconv_close(cd);
    *pout = '\0';

    return 0;
}

string Lib::g2u(string strGBK) {

    char *inbuf = stoc(strGBK);
    char buf[81];
    code_convert("gb2312", "utf-8", inbuf, strlen(inbuf), buf, sizeof(buf));
    return string(buf);
}
