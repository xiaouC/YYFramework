import os
import platform

pbc = SConscript( 'src/pbc/SConscript' )


bits = platform.architecture()[0]

cpp_path = [ 'src/pbc',
         'src/Common/../',
         'src/toluaAPI/lua-cjson-2.1.0',
         'src/protocol/src/',
         'protobuf-2.4.1/src',
         '../MCC/cocos2dx',
         '../MCC/cocos2dx/include',
         '../MCC/cocos2dx/support',
         '../MCC/cocos2dx/platform',
         '../MCC/cocos2dx/platform/third_party/linux',
         '../MCC/cocos2dx/platform/linux',
         '../MCC/cocos2dx/kazmath/include',
         '../MCC/CocosDenshion/include',
         '../MCC/extensions',
         '../MCC/scripting/lua/cocos2dx_support',
         '../MCC/scripting/lua/lua',
         '../MCC/scripting/lua/tolua',
         '../MCC/external/chipmunk/include',
     ]

if bits == '64bit':
    cpp_path.insert( 0, '../MCC/cocos2dx/platform/third_party/linux/include64' )





lib_path = [ '../MCC/lib/linux/debug', '../MCC/cocos2dx/proj.linux', ]

if bits == '64bit':
    lib_path.append( '../MCC/cocos2dx/platform/third_party/linux/libraries/lib64' )
    lib_path.append( '../MCC/CocosDenshion/third_party/fmod/lib64/api/lib' )
else:
    lib_path.append( '../MCC/cocos2dx/platform/third_party/linux/libraries' )
    lib_path.append( '../MCC/CocosDenshion/third_party/fmod/api/lib' )



libs = [ 'lua',
     'cocosdenshion',
     'cocos2dx',
     'extension',
     'protobuf-lite',
     'curl',
     'glfw',
     'GLEW',
     'GL',
     'rt',
     'z',
     'freetype',
     'jpeg',
     'png',
     'fontconfig',
     'ldap',
     'rtmp',
     'idn',
     'pthread',
     'lber',
     'X11',
     'tiff',
     'webp',
     pbc,
 ]

if bits == '64bit':
    libs.append( 'fmodex64' )
else:
    libs.append( 'fmodex64' )



env = Environment(
        CCFLAGS=['-Wall', '-g3', '-pg'],
        CPPPATH = cpp_path,
        LIBPATH = lib_path,
        LIBS = libs,
    CPPDEFINES = [('COCOS2D_DEBUG', 2), 'LUA_DEBUG', 'LINUX', 'CC_UNDER_LINUX', 'RENDER_WITH_OPEN_GL', 'DEBUG', 'ENABLE_CJSON_GLOBAL'
                 #, 'PACK_RESOURCE'
                 ],
)

sources = Glob( 'src/*.cpp' )
sources += Glob( 'src/*/*.cpp')
sources += Glob( 'src/*/*/*.cpp')
sources += Glob( 'src/protocol/src/*.cc' )
sources += Glob( 'src/toluaAPI/*.c' )
sources += Glob( 'src/toluaAPI/lua-cjson-2.1.0/*.c' )

env.Program( 'main', sources )
