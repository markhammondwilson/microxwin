#!/usr/bin/awk
# $Id: rgb.awk 1.1 1994/01/15 05:24:00 ulrich Exp $
#
# scan rgb.txt and create rgb.h
#
{ if (NF == 4) print "{" $1 "," $2 "," $3 ",\"" $4 "\"},"
  if (NF == 5) print "{" $1 "," $2 "," $3 ",\"" $4 " " $5 "\"}," }
