#!/bin/bash
echo `sed '8q;d' $1` > $2
echo `sed '32q;d' $1` >> $2
echo `sed '128q;d' $1` >> $2
echo `sed '512q;d' $1` >> $2
echo `sed '1024q;d' $1` >> $2



