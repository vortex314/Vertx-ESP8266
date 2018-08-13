#include <vertx.h>

class Consumer
{

public:
    static LinkedList<Consumer*> _consumers;
    uid_t _eventUid;
    EventHandler _handler;

    Consumer(uid_t eventUid,EventHandler handler) : _eventUid(eventUid),_handler(handler)
    {
        _consumers.add(this);
    };
};

LinkedList<Consumer*> Consumer::_consumers;

namespace std
{
void __throw_bad_function_call()
{
    ERROR(" bad function call, invalid std::function<> ");
    while(1);
};
};

#include <CborQueue.h>

EventBus::EventBus(uint32_t size) :_queue(size)
{

}

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
    Erc erc;
    uid_t uid = H(address);
    message.put(EB_DST,uid);
    taskENTER_CRITICAL(  );
    erc = _queue.put(message);
    taskEXIT_CRITICAL(  );
    if ( erc ) ERROR(" cannot publish to eventbus !!");
    return E_OK;
}

Erc EventBus::publish(EventLabel label)
{
    Message msg(10);
    INFO(" publish event : '%s'",label);
    return publish(label,msg);
}

Message _rxd(256);

void EventBus::eventLoop()
{
    Erc erc;
    Consumer* consumer;
    uid_t uidDst;

    taskENTER_CRITICAL(  );
    erc = _queue.get(_rxd);
    taskEXIT_CRITICAL(  );

    if ( erc ==0  && _rxd.get(EB_DST,uidDst) ) {
        Consumer::_consumers.forEach([uidDst](Consumer* consumer){
            if ( uidDst == consumer->_eventUid) {
                consumer->_handler(_rxd);
            }
        });
    }
}
