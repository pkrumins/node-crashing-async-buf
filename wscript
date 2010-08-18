import Options
from os import unlink, symlink, popen
from os.path import exists 

srcdir = "."
blddir = "build"
VERSION = "0.0.1"

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.target = "retbuf"
  obj.source = "retbuf-async.cpp"

def shutdown():
  if Options.commands['clean']:
    if exists('retbuf.node'): unlink('retbuf.node')
  else:
    if exists('build/default/retbuf.node') and not exists('retbuf.node'):
      symlink('build/default/retbuf.node', 'retbuf.node')

