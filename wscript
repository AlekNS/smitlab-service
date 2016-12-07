#! /usr/bin/env python
# coding: utf-8

import sys
import config

VERSION='0.2.3'
APPNAME='SmitlabService'

top = '.'
out = 'build'

poco_incs=config.poco['incs']
poco_libs=config.poco['libs']
poco_libs_shared=config.poco['shared']

def options(opt):
    opt.load('compiler_c c compiler_cxx cxx')

def configure(conf):
    conf.load('compiler_c c compiler_cxx cxx')
    # conf.setenv('platform')
    if sys.platform.find('win') < 0:
        conf.env.CXXFLAGS = '-g'
        conf.env.CFLAGS   = '-g'
    else:
        conf.env.append_unique('DEFINES', [
            "WIN32",
            "NDEBUG",
            "_ASTM_DEBUG",
            "_WINDOWS",
            "_USRDLL",
        ])
        conf.env.CXXFLAGS   = ['/Ob1', '/Oi', '/Ot', '/Oy',
                                '/GA', '/GF',
                                '/EHsc', '/MD']
        conf.env.LINKFLAGS    = ['/INCREMENTAL:NO', '/OPT:REF', '/OPT:ICF']

    conf.env.append_unique('DEFINES', ['POCO_NO_AUTOMATIC_LIBS', "_ASTM_DEBUG",] + config.poco['defs'])


    conf.env['libs'] = []
    conf.env['libs_link'] = []
    conf.env['libs_shared'] = []
    conf.env['libs_shared_link'] = []

    conf.env.append_value('libs_shared', poco_libs_shared)
    conf.env.append_value('libs_shared_link',[
        'PocoUtil',
        'PocoNetSSL',
        'PocoCrypto',
        'PocoNet',
        'PocoXML',
        'PocoDataSQLite',
        'PocoData',
        'PocoFoundation',
    ])

    conf.env.append_value('libs_link',['lualib'])

    if sys.platform.find('win') < 0:
        conf.env.append_value('libs_shared_link', [
            'pthread',
            'dl',
            'rt',
        ])

def build(bld):

    includes = [
       '.',
       'lualib',
       'serialport',
    ]

    includes.extend(poco_incs)

    bld(
        export_includes = includes,
        name = 'includes',
    )

    # lua lib
    lua_cflags = ''
    lua_ar     = ''
    if sys.platform.find('linux') == 0:
        lua_cflags = '-DLUA_USE_LINUX'
        lua_ar     = ' ar'

    bld.stlib(
        target='lualib',
        features='c' + lua_ar,
        source=[
           'lualib/lcode.c',
           'lualib/lctype.c',
           'lualib/ldebug.c',
           'lualib/ldo.c',
           'lualib/ldump.c',
           'lualib/lfunc.c',
           'lualib/lgc.c',
           'lualib/llex.c',

           'lualib/lmem.c',
           'lualib/lobject.c',
           'lualib/lopcodes.c',
           'lualib/lparser.c',
           'lualib/lstate.c',
           'lualib/lstring.c',
           'lualib/ltable.c',

           'lualib/ltm.c',
           'lualib/lundump.c',
           'lualib/lvm.c',
           'lualib/lzio.c',

           'lualib/lauxlib.c',
           'lualib/lbaselib.c',
           'lualib/lbitlib.c',
           'lualib/lcorolib.c',
           'lualib/ldblib.c',
           'lualib/liolib.c',
           'lualib/lmathlib.c',
           'lualib/loslib.c',
           'lualib/lstrlib.c',
           'lualib/ltablib.c',
           'lualib/loadlib.c',
           'lualib/linit.c',
           'lualib/lapi.c',
        ],
        includes=['includes'],
        cflags=lua_cflags
    )

    # analyzers
    bld.objects(
        target='analyzer',
        source=[
            'analyzer/Analyzer.cpp',
            'analyzer/AnalyzerDispatcher.cpp',
            'analyzer/AnalyzerModule.cpp',
            'analyzer/AnalyzerStorage.cpp',
        ],
        use=['includes']
    )

    # common
    bld.objects(
        target='common',
        source=[
            'common/DbCreateSQLite.cpp',
            'common/JsonEmitter.cpp',
            'common/stdafx.cpp',
            'common/IOAdapter.cpp',
        ],
        use=['includes']
    )

    # common
    serial_port_impl = 'serialport/serial/src/impl/unix.cc'

    if sys.platform.find('win') >= 0:
        serial_port_impl = 'serialport/serial/src/impl/win.cc'

    bld.objects(
        target='serialport',
        includes=[
            'serialport',
        ],
        source=[
            'serialport/SerialPort.cpp',
            'serialport/serial/src/serial.cc',
            serial_port_impl,
        ],
        use=['includes']
    )

    # server
    bld.objects(
        target='server',
        source=[
            'server/AnalyzerRequests.cpp',
            'server/InterpreterHolder.cpp',
            'server/AnalyzerServer.cpp',
            'server/ServerModule.cpp',
            'server/ServerRequestHandlerFactory.cpp',
        ],
        use=['includes']
    )

    #
    # Program
    #

    if sys.platform.find('win') < 0:
        bld(rule='cp ${SRC} ${TGT}', source="SmitlabService.xml.example", target="SmitlabService.xml")
    else:
        bld(rule='copy /Y ${SRC} ${TGT}', source="SmitlabService.xml.example", target="SmitlabService.xml")

    bld.program(
        features='cxx cxxprogram',
        target='SmitlabService',
        source=[
            'service/SmitlabService.cpp',
        ],

        libpath  = bld.env.libs_shared,
        lib      = bld.env.libs_shared_link,

        use      = [
            'includes',
            'analyzer',
            'common',
            'serialport',
            'server',
            'lualib',
        ]
    )

    bld.recurse('instruments')
