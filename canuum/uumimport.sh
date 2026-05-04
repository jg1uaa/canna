#!/bin/sh
date=20021221
cd freewnn-uum
CVS_RSH=ssh cvs import -I ! canna/canuum FREEWNN freewnn_$date
