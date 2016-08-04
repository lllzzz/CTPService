#include "Q.h"

Q::Q(){}
Q::~Q(){}

void Q::push(string key, string data)
{
    string pushPath = C::get("q_path");

    ofstream pusher;
    int s = Lib::stoi(Lib::getDate("%S"));
    s = s - s % 5;
    string ss = s > 10 ? Lib::itos(s) : "0" + Lib::itos(s);
    string pushFile = pushPath + key + "_" + Lib::getDate("%Y%m%d_%H%M") + ss + ".push";
    pusher.open(pushFile.c_str(), ios::app);
    pusher << data << endl;
    pusher.close();
}
