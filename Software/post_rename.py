Import("env")

my_flags = env.ParseFlags(env['BUILD_FLAGS'])
defines = {k: v for (k, v) in my_flags.get("CPPDEFINES")}

build_tag = "LoRaMesh"

print(my_flags)
deps = my_flags.get('CPPDEFINES')
print(deps)
version_tag = defines.get("SW_VERSION")

env.Replace(PROGNAME="../../../Generated/%s_%s" % (build_tag, version_tag))