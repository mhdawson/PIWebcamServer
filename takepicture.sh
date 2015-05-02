# configure for the webcams connected to you pi
fswebcam -d /dev/video0 -r "640x480" $1
fswebcam -d /dev/video1 -r "1280x1024" 2-$1

# configure for your web server replacing 'uesr' and'server' 
scp -P 20022 $1 user@server:pictures/$1
scp -P 20022 2-$1 user@server:pictures/2-$1
rm $1
rm 2-$1
