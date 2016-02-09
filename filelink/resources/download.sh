#!/bin/sh

JQVER=2.2.0
BSTRAPVER=3.3.6

mkdir js
cd js
wget code.jquery.com/jquery-$JQVER.min.js
mv jquery-$JQVER.min.js jquery.min.js
wget https://maxcdn.bootstrapcdn.com/bootstrap/$BSTRAPVER/js/bootstrap.min.js
cp ../upload.js .
#wget http://malsup.github.com/jquery.form.js

cd ..
mkdir css
cd css
wget https://maxcdn.bootstrapcdn.com/bootstrap/$BSTRAPVER/css/bootstrap.min.css
wget https://maxcdn.bootstrapcdn.com/bootstrap/$BSTRAPVER/css/bootstrap-theme.min.css

cd ..
mkdir fonts
cd fonts
wget https://maxcdn.bootstrapcdn.com/bootstrap/$BSTRAPVER/fonts/glyphicons-halflings-regular.eot
wget https://maxcdn.bootstrapcdn.com/bootstrap/$BSTRAPVER/fonts/glyphicons-halflings-regular.svg
wget https://maxcdn.bootstrapcdn.com/bootstrap/$BSTRAPVER/fonts/glyphicons-halflings-regular.ttf
wget https://maxcdn.bootstrapcdn.com/bootstrap/$BSTRAPVER/fonts/glyphicons-halflings-regular.woff
wget https://maxcdn.bootstrapcdn.com/bootstrap/$BSTRAPVER/fonts/glyphicons-halflings-regular.woff2
