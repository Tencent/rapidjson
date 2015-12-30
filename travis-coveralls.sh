#!/bin/bash

pip install --user cpp-coveralls
coveralls -r .. --gcov-options '\-lp' -e thirdparty -e example -e test -e build/CMakeFiles -e include/rapidjson/msinttypes -e include/rapidjson/internal/meta.h -e include/rapidjson/error/en.h
