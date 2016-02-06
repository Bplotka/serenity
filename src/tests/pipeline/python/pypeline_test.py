#!/usr/bin/python

import mesos_pb2
import serenity_pb2

class PypelineTest(object):
  def __init__(self):
    print "Initialized"

  def run(self, *args):
    usagePy = mesos_pb2.ResourceUsage()
    usagePy.ParseFromString(args[0])

    print usagePy