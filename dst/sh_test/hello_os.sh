#!/bin/bash
echo `sed -n 8p $1` > $2
echo `sed -n 32p  $1` >> $2
echo `sed -n 128p $1` >> $2
echo `sed -n 512p $1` >> $2
echo `sed -n 1024p $1` >> $2
