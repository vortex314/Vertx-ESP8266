#include <vertx.h>

class Consumer : public LinkedList<Consumer>
{
public:
    uid_t _eventUid;
    EventHandler _handler;

    Consumer(uid_t eventUid,EventHandler handler) : _eventUid(eventUid),_handler(handler) {
        add(this);
    };
};

Message::Message(uint32_t size)
{
    _start = new uint32_t[(size/4)+1];
    _size=size;
    _nextFree=0;
    _lastToken=0;
}

EventBus::EventBus(uint32_t size)
{

}



namespace std
{
void __throw_bad_function_call()
{
    ERROR(" bad function call, invalid std::function<> ");
    while(1);
};
};

Erc EventBus::on(EventLabel address,EventHandler f)
{
    return consumer(address,f);
}

Erc EventBus::consumer(EventLabel address,EventHandler f)
{
    uid_t uid=UID.add(address);
    new Consumer(uid,f);
    return E_OK;
}

Erc EventBus::publish(EventLabel address,Message& message)
{
    Message msg(10);
    Consumer* consumer;
    uid_t uid=UID.add(address);
//    int count=0;
    if ( Consumer::first()==0) ERROR(" no consumers found ");
    for(consumer=Consumer::first(); consumer; consumer=consumer->next()) {
//       INFO(" %d : compare %s :%s",count++,UID.label(uid),UID.label(consumer->_eventUid));
        if ( uid == consumer->_eventUid)
            consumer->_handler(msg);
    }
    return E_OK;
}

Erc EventBus::publish(EventLabel label)
{
    Message msg(0);
    INFO(" publish event : '%s'",label);
    return publish(label,msg);
}

void EventBus::eventLoop()
{
    // get next message and address and type
    // if type==send -> go directly
    /*   INFO(" eventLoop ");
       Message msg(10);
       Consumer* consumer;
       for(consumer=Consumer::first(); consumer; consumer=consumer->next()) {
           consumer->_handler(msg);
       }*/
    return;

}
