#!/usr/bin/env bash

JSON_PATH=$1

# NOTE(bplotka) valid for serenity deployment using
# serenity-formulas (https://github.com/Bplotka/serenity-formula)
source /opt/serenity/mesos/etc/mesos-slave
export MESOS_MASTER=$MESOS_MASTER

./smoke_test_framework --logging_level=INFO --json_path="${JSON_PATH}"