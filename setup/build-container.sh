#!/bin/bash
cd ./docker/rpi-cross-compile
podman build -t rpi-cross-compile:latest .
echo "Cross-compilation container ready!"
