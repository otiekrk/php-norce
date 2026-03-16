#!/bin/bash

docker build -t norce .
docker run --name=norce -p 8000:8000 norce