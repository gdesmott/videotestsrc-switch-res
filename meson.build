project('videotestsrc-switch-res', 'c', license : 'LGPL')

cc = meson.get_compiler('c')

gst_dep = dependency('gstreamer-1.0',
    fallback : ['gstreamer', 'gst_dep'])

executable('videotestsrc-switch-res', 'videotestsrc-switch-res.c', dependencies : [gst_dep])
