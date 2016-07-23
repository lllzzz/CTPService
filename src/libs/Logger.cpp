#include "Logger.h"

Logger::Logger(string id)
{
    _path = C::get("log_path");
    _id = id;
}

Logger::~Logger(){}


void Logger::_clear()
{
    _info.clear();
}

void Logger::_write(int type, string logName)
{
    string typeStr = "INFO";
    switch (type) {
        case TYPE_ERR:
            typeStr = "ERROR";
            break;
        case TYPE_REQ:
            typeStr = "REQUEST";
            break;
        default:
            typeStr = "INFO";
            break;
    }
    ofstream fh;
    string logPath = _path + _id + "_" + Lib::getDate("%Y%m%d") + ".log";
    fh.open(logPath.c_str(), ios::app);

    fh << Lib::getDate("%Y%m%d-%H:%M:%S", true) << "|";
    fh << "[" + typeStr + "]" << "|";
    fh << logName << "|";

    std::map<string, string>::iterator it;
    for (it = _info.begin(); it != _info.end(); ++it)
    {
        fh << it->first << "|" << it->second << "|";
    }
    fh << endl;
    fh.close();
    _clear();
}

void Logger::push(string key, string val)
{
    _info[key] = val;
}

void Logger::info(string name)
{
    _write(TYPE_INFO, name);
}

void Logger::error(string name, CThostFtdcRspInfoField *info, int id, int isLast)
{
    push("ErrCode", Lib::itos(info->ErrorID));
    push("ErrMsg", string(info->ErrorMsg));
    push("ReqID", Lib::itos(id));
    push("IsLast", Lib::itos(isLast));
    _write(TYPE_ERR, name);
}

void Logger::request(string name, int reqID, int reqCode)
{
    push("ReqID", Lib::itos(reqID));
    push("ReqCode", Lib::itos(reqCode));
    _write(TYPE_REQ, name);
}