#!/bin/bash

cc main.c -l ssl -l crypto -Dtrain=1 -o crun
