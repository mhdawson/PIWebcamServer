// Copyright 2014-2015 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#ifndef _PIWC
#define _PIWC

#include <time.h>

extern "C" {
    #include "MQTTClient.h"
    #include "MQTTClientPersistence.h"
}

class PIWC {
   public:
      PIWC(char* mqttBroker);
      void listenForMessages(char* topic);

   private:
      char* _topic;
      char* _mqttBroker;
      MQTTClient _myClient;
      MQTTClient_connectOptions _mqttOptions;

      void takePicture();

      static int messageArrived(void *context, char* topicName, int topicLen, MQTTClient_message *message);
      static void connectionLost(void *context, char *cause);
};

#endif
