#include <nan.h>
#include <v8.h>
#include <RF24.h>
#include <RF24Network.h>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <time.h>
#include <string>
using namespace Nan;
using namespace v8;

RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);
RF24Network network(radio);


struct payload_t {                  // Structure of our payload
  char msg[24];
};

struct payload_pi {
  uint16_t fromNode;
  char msg[24];
};

//--------------------------------------------------------------------------
//Below functions are just replica of RF24Network functions.
//No need to use these functions in you app.
// NAN_METHOD(BeginRadio) {
//   radio.begin();
// }
//
// NAN_METHOD(BeginNetwork){
//   uint16_t channel = info[0]->Uint32Value();
//   uint16_t thisNode = info[0]->Uint32Value();
//   network.begin(channel,thisNode);
// }
//
// NAN_METHOD(Update) {
//   network.update();
// }
//
// NAN_METHOD(Available) {
//   v8::Local<v8::Boolean> status = Nan::New(network.available());
//   info.GetReturnValue().Set(status);
// }

NAN_METHOD(Read) {
  payload_t payload;
  RF24NetworkHeader header;
  network.read(header,&payload,sizeof(payload));
  info.GetReturnValue().Set(Nan::New(payload.msg).ToLocalChecked());
}
//--------------------------------------------------------------------------------

NAN_METHOD(Begin){
  if (info.Length() < 2)
      return Nan::ThrowTypeError("Should pass Channel and Node id");

  uint16_t channel = info[0]->Uint32Value();
  uint16_t thisNode = info[1]->Uint32Value();

	radio.begin();
	delay(5);
	network.begin(channel, thisNode);
}

NAN_METHOD(Write){
  if (info.Length() < 2)
      return Nan::ThrowTypeError("Should pass Receiver Node Id and Message");

  uint16_t otherNode = info[0]->Uint32Value();
  v8::String::Utf8Value message(info[1]->ToString());
  std::string msg = std::string(*message);
  payload_t payload;
  strncpy(payload.msg, msg.c_str(),24);

  RF24NetworkHeader header(otherNode);
  bool ok = network.write(header,&payload, sizeof(payload));
  info.GetReturnValue().Set(ok);
}

NAN_METHOD(WriteBuffer){
  if (info.Length() < 3)
      return Nan::ThrowTypeError("Should pass Receiver Node Id, Buffer and buffer size");

  uint16_t otherNode = info[0]->Uint32Value();
  uint16_t size = info[2]->Uint32Value();
  uint8_t * buffer = (uint8_t*)node::Buffer::Data(info[1]->ToObject());

  RF24NetworkHeader header(otherNode);
  bool ok = network.write(header, buffer, size);
  info.GetReturnValue().Set(ok);
}

void keepListen(void *arg) {
	// while(1)
	// {
	// 	network.update();
	// 	while (network.available()) {     // Is there anything ready for us?
	// 		  RF24NetworkHeader header;
  //  			payload_t payload;
  // 			network.read(header,&payload,sizeof(payload));
  //
  //       payload_pi localPayload;
  //       localPayload.fromNode = header.from_node;
  //       strncpy(localPayload.msg, payload.msg, 24);
  //       async->data = (void *) &localPayload;
  //       uv_async_send(async);
	// 	}
	// 	delay(2000);
	// }
}

void doCallback(uv_async_t *handle){
  // payload_pi* p = (struct payload_pi*)handle->data;
  // v8::Handle<v8::Value> argv[2] = {
  //     Nan::New(p->fromNode),
  //     Nan::New(p->msg).ToLocalChecked()
  //   };
  // cbPeriodic->Call(2, argv);
}

class ProgressWorker : public AsyncProgressWorker {
 public:
  ProgressWorker(Callback *onMessage, Callback *onFinish, int milliseconds)
    : AsyncProgressWorker(onFinish), progress(onMessage), milliseconds(milliseconds) {}

  ~ProgressWorker() {
    delete progress;
  }

  void Execute (const AsyncProgressWorker::ExecutionProgress& progress) {
    int payloadSize = 3;
    char *payload;
  	while(run)
  	{
  		network.update();
  		while (network.available()) {     // Is there anything ready for us?
  			  RF24NetworkHeader header;
          payload = (char *)malloc(payloadSize);
    			network.read(header, payload, payloadSize);
          // //
          // // // payload_pi localPayload;
          // // // localPayload.fromNode = header.from_node;
          // // // strncpy(localPayload.msg, payload.msg, 24);
          progress.Send(reinterpret_cast<const char*>(payload), payloadSize);
  		}
  		delay(milliseconds);
  	}
    // for (int i = 0; i < 10; ++i) {
    //   progress.Send(reinterpret_cast<const char*>(&i), sizeof(int));
    //   delay(milliseconds);
    // }
  }

  void Stop() {
    run = false;
  }

  void HandleProgressCallback(const char *data, size_t count) {
    Nan::HandleScope scope;

    v8::Local<v8::Value> argv[] = {
        Nan::NewBuffer(const_cast<char*>(data), count).ToLocalChecked()
    };
    progress->Call(1, argv);
  }

 private:
  Callback *progress;
  int milliseconds;
  bool run = true;
};

ProgressWorker *readWorker;

NAN_METHOD(ReadAsync){
  const int ON_MESSAGE = 0;
  const int ON_FINISH = 1;
  const int DELAY = 2;
  int delay = 200;

  if (info.Length() < 1) {
    return Nan::ThrowTypeError("Should pass at least 2 arguments: onMessage, onFinish[, delay]");
  }
  if (!info[ON_MESSAGE]->IsFunction()) {
    return Nan::ThrowTypeError("onMessage should be a function");
  }

  if (!info[ON_FINISH]->IsFunction()) {
    return Nan::ThrowTypeError("onFinish should be a function");
  }

  if (info.Length() > 2) {
    if (!info[DELAY]->IsNumber()) {
      return Nan::ThrowTypeError("delay should be a number");
    }

    if (info[DELAY]->Uint32Value() < 200) {
      return Nan::ThrowTypeError("delay should be a least 200");
    }
    delay = info[DELAY]->Uint32Value();
  }

  readWorker = new ProgressWorker(
    new Nan::Callback(info[ON_MESSAGE].As<Function>()),
    new Nan::Callback(info[ON_FINISH].As<Function>()),
    delay
  );

  AsyncQueueWorker(readWorker);
}

NAN_METHOD(PrintDetails) {
  radio.printDetails();
}

NAN_METHOD(Close){
  if (readWorker != NULL) {
    readWorker->Stop();
  }
}


NAN_MODULE_INIT(Init){

    Nan::Set(target, New<String>("begin").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(Begin)).ToLocalChecked());

    // Nan::Set(target, New<String>("beginRadio").ToLocalChecked(),
    //     GetFunction(New<FunctionTemplate>(BeginRadio)).ToLocalChecked());
    //
    // Nan::Set(target, New<String>("beginNetwork").ToLocalChecked(),
    //     GetFunction(New<FunctionTemplate>(BeginNetwork)).ToLocalChecked());
    //
    // Nan::Set(target, New<String>("update").ToLocalChecked(),
    //     GetFunction(New<FunctionTemplate>(Update)).ToLocalChecked());

    Nan::Set(target, New<String>("printDetails").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(PrintDetails)).ToLocalChecked());

    // Nan::Set(target, New<String>("available").ToLocalChecked(),
    //     GetFunction(New<FunctionTemplate>(Available)).ToLocalChecked());

    Nan::Set(target, New<String>("read").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(Read)).ToLocalChecked());

    Nan::Set(target, New<String>("readAsync").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(ReadAsync)).ToLocalChecked());

    Nan::Set(target, New<String>("write").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(Write)).ToLocalChecked());

    Nan::Set(target, New<String>("writeBuffer").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(WriteBuffer)).ToLocalChecked());

    Nan::Set(target, New<String>("close").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(Close)).ToLocalChecked());
}

NODE_MODULE(nrf24Node, Init)
