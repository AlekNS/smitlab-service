import os

poco = {
    'defs'      : [ '_DEBUG' ],
    'incs'      : [ os.environ['POCO_INCLUDE'] ],
    'libs'      : [ os.environ['POCO_LIB'] ],
    'shared'    : [ os.environ['POCO_SHLIB'] ],
}
