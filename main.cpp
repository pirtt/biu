#include "msg_handler.h"
#include <functional>
#include <iostream>


using namespace std;
using namespace biu;



class T : public MsgHandler {
    virtual void Test(Msg& msg, int err)
    {
        //cout << "Fuck" << endl;
    }
};

int main()
{

    T H;
    for (int i = 0; i < 1000; i++) {
        H.ProcessMsg(std::make_unique<Msg>(0));
    }
    H.Exit();
    cout << Msg::GetCount() << endl;
    cout << Msg::GetDoCount() << endl;
    return 0;
}
