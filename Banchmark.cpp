#include <EventBus.h>

#define REQ_ID H("R")
#define PAR1 H("I")

class Requester : public Actor
{
    uint32_t _request;
    uid_t _dst;

  public:
    Requester(const char *name) : Actor(name)
    {
        _request = 0;
        _dst = H("replier");
    };

    void sendRequest()
    {
        eb.request(_dst, REQ_ID, id()).addKeyValue(PAR1, _request);
        eb.send();
    }
    void setup()
    {
        sendRequest();
        uid.add("R");
        uid.add("I");
    }
    void onEvent(Cbor &msg)
    {
        Str str(40);
        Header h;
        h.fromCbor(msg);

        //   INFO(" requester : %s ", h.toStr(str).c_str());
        if (eb.isReply(_dst, REQ_ID))
        {
            msg.getKeyValue(PAR1, _request);
            sendRequest();
        }
    }
};

class Replier : public Actor
{
    uint32_t _request;
    uid_t _dst, _reqId;
    uint64_t _lastTime;
    uint32_t _lastCount;

  public:
    Replier(const char *name) : Actor(name)
    {
        _request = 0;
        _dst = H("requester");
        _lastTime = Sys::millis();
        _lastCount = 0;
        _reqId = H("I");
    };

    void setup()
    {
        timeout(1000);
    }
    void onEvent(Cbor &msg)
    {
        Str str(40);
        Header h;
        h.fromCbor(msg);

        // INFO(" replier : %s ", h.toStr(str).c_str());
        if (eb.isRequest(id(), REQ_ID))
        {
            msg.getKeyValue(PAR1, _request);
            eb.reply().addKeyValue(PAR1, ++_request);
            eb.send();
        }
        else if (eb.isTimeout())
        {
            uint32_t speed = ((_request - _lastCount) * 1000);
            speed /= (Sys::millis() - _lastTime);
            INFO("  %d msg/sec", speed);
            timeout(1000);
            _lastCount = _request;
            _lastTime = Sys::millis();
        }
    }
};

void doBenchmark()
{
    Requester *requester = new Requester("requester");
    Replier *replier = new Replier("replier");
    replier->setup();
    requester->setup();
}
