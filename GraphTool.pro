TEMPLATE = subdirs

SUBDIRS = \
    app \
    crashreporter \
    plugins \
    thirdparty

app.subdir = source/app
crashreporter.subdir = source/crashreporter
plugins.subdir = source/plugins
thirdparty.subdir = source/thirdparty

app.depends = thirdparty
plugins.depends = thirdparty
