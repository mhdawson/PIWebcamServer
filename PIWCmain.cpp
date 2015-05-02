// Copyright 2014-2015 the project authors as listed in the AUTHORS file.
// All rights reserved. Use of this source code is governed by the
// license that can be found in the LICENSE file.

#include "PIWC.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

   // validate we got the required parameters 
   if (2 > argc) {
      printf("Missing mqttBroker parameter - exiting\n");
      return -1;
   }

   PIWC cameraService = PIWC(argv[1]);
   cameraService.listenForMessages((char*)"house/camera/capture");
}
