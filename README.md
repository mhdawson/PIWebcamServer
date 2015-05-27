# PI Webcam server

This project receives messages to take a picture from an mqtt broker
and then runs a script to take a picture, upload it to a web server
and then send the name back to the requestor through a response using
mqtt

## Requires

fswebcam - apt-get install fswebcam


## To build:
TODO

## TODOs
- add more info about build dependencies
- add more comments to the code
- add more general documenation
- make mqtt topic configurable
- generalize to allow more control through mqtt message
- generatize to run other scripts ?
