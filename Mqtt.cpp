#include "Mqtt.h"
#include <Property.h>

//#define MQTT_PORT 1883
#define MQTT_HOST "limero.ddns.net"
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASS ""

enum {
	WIFI_CONNECTED=0,   // 0
	WIFI_DISCONNECTED,  // 1
	MQTT_CONNECTED, // 2
	SIG_MQTT_DISCONNECTED, // 3
	MQTT_SUBSCRIBED, // 4
	MQTT_PUBLISHED, // 5
	MQTT_INCOMING, // 6
	SIG_MQTT_FAILURE, // 7
	MQTT_DO_PUBLISH // 8
};

Mqtt* Mqtt::_mqtt=0;

void  Mqtt::topic_received_cb(mqtt_message_data_t *md)
{
	mqtt_message_t *message = md->message;
	_mqtt->_topicRxd.clear();
	_mqtt->_messageRxd.clear();
	_mqtt->_topicRxd.write((uint8_t*)md->topic->lenstring.data,0,(uint32_t)md->topic->lenstring.len);
	_mqtt->_messageRxd.write(((uint8_t*)(message->payload)),0,(uint32_t)message->payloadlen);
	_mqtt->onMessageReceived(_mqtt->_topicRxd,_mqtt->_messageRxd);
}

void Mqtt::onMessageReceived(Str& topicStr, Str& msg)
{
	INFO("Received: %s = %s ",topicStr.c_str(),msg.c_str());
	uint32_t offsetSlash[5];
	char topic[50];

	strncpy(topic,topicStr.c_str(),sizeof(topic)-1);
	const char* first=topic;
	const char* ptr = first;
	uint32_t idx=0;
	
	while ( ptr && (idx < 5) ) {
		ptr = strchr(ptr,'/');
		if ( ptr ) {
			offsetSlash[idx]=ptr-first;
			ptr++;
		} else {
			offsetSlash[idx]=0;
			break;
		}
		idx++;
	}

	if ( (idx > 1 ) && offsetSlash[0] && offsetSlash[1] && offsetSlash[2] && offsetSlash[3]==0 ) { // found 3 slashes
		topicStr = &topic[offsetSlash[1]+1];
		INFO("property set %s = %s",topicStr.c_str(),msg.c_str());
		_messageEb.clear();
		_messageEb.put(H("value"),msg);
		_messageEb.put(H("key"),H(topicStr.c_str()));
		eb.publish("property/set",_messageEb);
	} else {
		INFO(" no property or command found in %s , slashes at : %d %d %d %d %d ",
		     topic,
		     offsetSlash[0],
		     offsetSlash[1],
		     offsetSlash[2],
		     offsetSlash[3],
		     offsetSlash[4]);
	}
}

Mqtt::Mqtt(const char* name):VerticleTask(name,512,5)
	, _topicAlive(40)
	,_topicsForDevice(40)
	,_topicRxd(50)
	, _messageRxd(100)
	,_topic(50)
	,_message(100)
	,_topicTxd(50)
	,_messageTxd(100)
	,_messageEb(100)
{
	_wifiConnected=false;
	_mqttConnected=false;
	_busyTxd=false;
	UID.add("topic");
	UID.add("message");
	_mqtt=this;
}

static bool alive=true;

void Mqtt::start()
{
	_topicAlive = "src/";
	_topicAlive += Sys::hostname();
	_topicAlive += "/system/alive";
	_topicsForDevice = "dst/";
	_topicsForDevice += Sys::hostname();
	_topicsForDevice += "/#";

	eb.on("wifi/connected",[this](Message& msg) {
		signal(WIFI_CONNECTED);
		_wifiConnected=true;
	});

	eb.on("wifi/disconnected",[this](Message& msg) {
		signal(WIFI_DISCONNECTED);
		_wifiConnected=false;
	});

	eb.on("mqtt/publish",[this](Message& msg) {
		if ( !_mqttConnected ) return;
		if ( !_busyTxd ) {
			if ( msg.get(H("topic"),_topicTxd) && msg.get(H("message"),_messageTxd)) {
				_busyTxd=true;
				signal(MQTT_DO_PUBLISH);
			}
		} else {
			WARN(" Publish lost : BUSY.");
		}
	});

	VerticleTask::start();
}


bool Mqtt::do_connect()
{
	_mqttConnected=false;
	_ret         = 0;
	_client   = mqtt_client_default;
	_data = mqtt_packet_connect_data_initializer;
	mqtt_network_new( &_network );
	ZERO(_mqtt_client_id);
	strcpy(_mqtt_client_id, Sys::hostname());

	INFO("connecting to MQTT server %s:%d ... ",MQTT_HOST,MQTT_PORT);
	_ret = mqtt_network_connect(&_network, MQTT_HOST, MQTT_PORT);
	if( _ret ) {
		INFO("connect error: %d", _ret);
		return false;
	}
	mqtt_client_new(&_client, &_network, 5000, _mqtt_buf, 100,
	                _mqtt_readbuf, 100);

	_data.willFlag       = 0;
	_data.MQTTVersion    = 3;
	_data.clientID.cstring   = _mqtt_client_id;
	_data.username.cstring   = (char*)MQTT_USER;
//   _data.username.lenstring=0;
	_data.password.cstring   = (char*)MQTT_PASS;
//    _data.password.lenstring=0;
	_data.keepAliveInterval  = 20;
	_data.cleansession   = 0;
	_data.will.topicName.cstring = (char*)_topicAlive.data();
	_data.will.message.cstring = (char*)"false";
	INFO("Send MQTT connect ... ");
	_ret = mqtt_connect(&_client, &_data);
	if(_ret) {
		INFO("error: %d", _ret);
		mqtt_network_disconnect(&_network);
		return false;
	};
	eb.publish("mqtt/connected");
	_mqttConnected=true;
	return true;
}


void Mqtt::run()
{
	while(true) {
		while (!_wifiConnected) {
			waitSignal(1000);
		}
		while(_wifiConnected) {
			while (!_mqttConnected) {
				do_connect();
			};
			mqtt_yield(&_client, 10);
			INFO(" subscribing %s ",_topicsForDevice.c_str());
			mqtt_subscribe(&_client, _topicsForDevice.c_str(), MQTT_QOS1, topic_received_cb);
			mqtt_yield(&_client, 10);
			INFO("subscribed");
			while(_mqttConnected) {
				waitSignal(10);
				if ( hasSignal(MQTT_DO_PUBLISH)) {
					mqtt_message_t message;
					message.payload = _messageTxd.data();
					message.payloadlen = _messageTxd.length();
					message.dup = 0;
					message.qos = MQTT_QOS0;
					message.retained = 0;
					_ret = mqtt_publish(&_client, _topicTxd.c_str(), &message);
					_busyTxd=false;
					INFO("PUB %s:%s",_topicTxd.c_str(),_messageTxd.c_str());
					if (_ret != MQTT_SUCCESS ) {
						INFO("error while publishing message: %d", _ret );
						break;
					}
					_ret = mqtt_yield(&_client, 10);
					if (_ret == MQTT_DISCONNECTED) {
						_mqttConnected=false;
					}
				}

			}
			INFO("Connection dropped, request restart\n\r");
			mqtt_network_disconnect(&_network);
			_mqttConnected=false;
			_busyTxd=false;
			taskYIELD();
		}
	}
}
/*
//   TimerHandle_t th = xTimerCreate("mqtt",100/portTICK_PERIOD_MS,pdTRUE,this,timerHandler);
//   xTimerStart(th,0);
while (true) { // FSM MQTT Connection
// WIFI_DISCONNECTED
WIFI_DISCONNECTED: {
while (true) {
waitSignal(1000);
if ( hasSignal( WIFI_CONNECTED)) {
INFO(" wifi connected ");
goto WIFI_CONNECTED;
}
}
};
WIFI_CONNECTED : {
MQTT_DISCONNECTED : {
_mqttConnected=false;
//              mqtt_disconnect(_client);MESSAGE: Entering directory `/home/lieven/workspace/vertx-esp8266'
/bin/sh -c 'make -j4'
----------Building project:[ vertx-esp8266 - Debug_Linux ]----------
C++ /home/lieven/workspace/vertx-esp8266/Mqtt.cpp
/home/lieven/workspace/vertx-esp8266/Mqtt.cpp: In member function 'bool Mqtt::do_connect()':

while ( true) {
do_connect();
waitSignal(2000);
if ( hasSignal(WIFI_DISCONNECTED ) )goto WIFI_DISCONNECTED;
if ( hasSignal(MQTT_CONNECTED) ) goto MQTT_CONNECTED;
if ( hasSignal(MQTT_FAILURE )) {
waitSignal(1000);
}
waitSignal(1000);
};
MQTT_CONNECTED  : {
mqtt_set_inpub_callback(_client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, this);
_mqttConnected=true;
eb.publish("mqtt/connected");
while(true) {

err_t err = mqtt_subscribe(_client, "src/+/system/alive", 1, mqtt_sub_request_cb, this);
if ( err ) {
ERROR(" subscribe failed ");
}
waitSignal(1000);
if ( hasSignal(WIFI_DISCONNECTED) ) goto WIFI_DISCONNECTED;
if ( hasSignal(MQTT_DISCONNECTED) ) goto MQTT_DISCONNECTED;
if ( hasSignal(MQTT_SUBSCRIBED) ) goto MQTT_PUBLISHING;
waitSignal(1000);
}
}
MQTT_PUBLISHING : {
while(true) {
waitSignal(1000);
if ( hasSignal(WIFI_DISCONNECTED ) )goto WIFI_DISCONNECTED;
if ( hasSignal(MQTT_DISCONNECTED) ) goto MQTT_DISCONNECTED;
if ( hasSignal(MQTT_DO_PUBLISH) ) {
publish(_topicTxd,_messageTxd);
}
}
}
}
}
}
}*/
