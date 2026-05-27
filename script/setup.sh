#!/bin/bash
set -e
apt-get update --fix-missing
apt-get install -y --fix-missing cmake g++ make