# vertx-esp32

The purpose is to have a framework for small embedded devices that have the ease of integration and development.

This framework is based on the ideas that also live  in Vertx and Node.js.
## Features :
 

 - Loosely coupled and autonomous objects . They have no interface towards each othe except a common eventbus. 
 - Events have a string signature which is send as a hash for fast correlation between publisher and subscriber. Example :"mqtt/connected" is send to help the programmer remember what he send and received, at compile time only a 16bit int is sued.
 - An eventbus to send message between different objects 
 - Eventbus addressing is based on addresses formulated as strings, internally these are converted to unique id's ( 16 bit ) which are more performant to do the routing.
 -  Use of lambda's to specify event handlers , keeps the code of event handling easily centralized in the code.
 -  Verticles / Objects are eventLoop tasks ( VerticleCoRoutine ) or independent FreeRtos tasks ( VerticleTask ). Attention need to be paid when handlers are async invoked between Freertos tasks. Concurrency can be an issue. So the eventhandler of the task is mostly limited to provide a signal to itself.
 - Serialization of messages on the Eventbus are based on CBOR binary serialization which proves to be efficient 
 - .The framework comes with a list of  predefined Objects / Actors :
-- Wifi : to signal connection setup and disconnection
-- Mqtt : assure publishing of events to mqtt server and maintain connection
-- Config : assures store of persistent config items
-- Telnet : CLI interface for chaning config
-- Monitor : reports task state to logger
--  Hardware : abstraction interface for peripherals
-- DWM1000_Anchor
-- DWM1000_Tag



> Written with [StackEdit](https://stackedit.io/).
