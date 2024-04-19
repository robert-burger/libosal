from conan import ConanFile
from conan.tools.gnu import Autotools, AutotoolsToolchain
import re

class MainProject(ConanFile):
    name = "libosal"
    license = "GPLv3"
    author = "Robert Burger <robert.burger@dlr.de>"
    url = f"https://rmc-github.robotic.dlr.de/common/{name}"
    description = """"This library provides an Operating System Abstraction Layer 
                      (OSAL) for other programs so they do not need to take care 
                      about the underlying implementation"""
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = ["*", "!.gitignore", "!bindings"]
    options = {"shared": [True, False]}
    default_options = {"shared": True}
    
    def generate(self):
        tc = AutotoolsToolchain(self)
        tc.autoreconf_args = [ "--install", ]        
        if self.settings.os == "pikeos":
            tc.update_configure_args({
                "--host": "%s-%s" % (self.settings.arch, self.settings.os),  # update flag '--host=my-gnu-triplet
            })
        tc.generate()

    def build(self):
        autotools = Autotools(self)
        autotools.libs=[]
        autotools.include_paths=[]
        autotools.library_paths=[]

        args = []

        if self.settings.build_type == "Debug":
            autotools.flags = ["-O0", "-g"]
            args.append("--enable-assert")
        else:
            autotools.flags = ["-O2"]
            args.append("--disable-assert")

        if self.options.shared:
            args.append("--enable-shared")
            args.append("--disable-static")
        else:
            args.append("--disable-shared")
            args.append("--enable-static")

        autotools.autoreconf()
        autotools.configure(args=args)
        autotools.make()

    def package(self):
        autotools = Autotools(self)
        autotools.install()

    def package_info(self):
        self.cpp_info.includedirs = ['include']
        self.cpp_info.libs = ["osal"]
        self.cpp_info.bindirs = ['bin']
        self.cpp_info.resdirs = ['share']
