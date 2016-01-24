#!/usr/bin/python

import serenity_pb2

class PypelineTest(object):
  def __init__(self):
    print "A"

  def run(self, protoUsage):
    #usagePy = serenity_pb2.ResourceUsage()
    #usagePy.ParseFromString(protoUsage)

    #print usagePy
    print "B"
