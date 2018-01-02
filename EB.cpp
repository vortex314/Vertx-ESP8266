#include <vertx.h>

class Consumer : public LinkedList<Consumer> {
Address _address;
MessageHandler _handler;
public:
Consumer(Address& address,MessageHandler& handler) :_address(address),_handler(handler){
add(this);
}
Address& address(){return _address;};
MessageHandler& handler(){ return _handler;};
};



EventBus::EventBus(uint32_t size){

}

Erc EventBus::consumer(Address address,MessageHandler& handler){
  new Consumer(address,handler);
  return E_OK;
}

Erc EventBus::publish(Address address,Message& message){
return E_OK;
}

void EventBus::eventLoop(){
  // get next message and address and type
  // if type==send -> go directly 
  INFO(" eventLoop ");
  return;
  Address addr("testing");
  Message message;
  Consumer* cursor;
  for(cursor= Consumer::first();cursor!=0;cursor=cursor->next()) {
    if ( addr.match(cursor->address()))
      cursor->handler().handle(message);
  }
}



