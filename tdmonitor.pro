TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = lib prodconsumer

prodconsumer.depends = lib
