aPackageInfo = [
	:name = "SimpleJSON",
	:description = "JSON parsing and manipulation library for Ring programming language.",
	:folder = "simplejson",
	:developer = "ysdragon",
	:email = "youssefelkholey@gmail.com",
	:license = "MIT License",
	:version = "1.0.0",
	:ringversion = "1.23",
	:versions = 	[
		[
			:version = "1.0.0",
			:branch = "master"
		]
	],
	:libs = 	[
		[
			:name = "",
			:version = "",
			:providerusername = ""
		]
	],
	:files = 	[
		"lib.ring",
		"main.ring",
		".clang-format",
		"CMakeLists.txt",
		"examples/example1.ring",
		"examples/example2.ring",
		"examples/example3.ring",
		"LICENSE",
		"README.md",
		"src/ring_simplejson.c",
		"src/utils/color.ring",
		"src/utils/install.ring",
		"src/utils/uninstall.ring"
	],
	:ringfolderfiles = 	[

	],
	:windowsfiles = 	[
		"lib/windows/i386/ring_simplejson.dll",
		"lib/windows/amd64/ring_simplejson.dll",
		"lib/windows/arm64/ring_simplejson.dll"
	],
	:linuxfiles = 	[
		"lib/linux/amd64/libring_simplejson.so",
		"lib/linux/arm64/libring_simplejson.so"
	],
	:ubuntufiles = 	[

	],
	:fedorafiles = 	[

	],
	:macosfiles = 	[
		"lib/macos/amd64/libring_simplejson.dylib",
		"lib/macos/arm64/libring_simplejson.dylib"
	],
	:freebsdfiles = 	[
		"lib/freebsd/amd64/libring_simplejson.so",
		"lib/freebsd/arm64/libring_simplejson.so"
	],
	:windowsringfolderfiles = 	[

	],
	:linuxringfolderfiles = 	[

	],
	:ubunturingfolderfiles = 	[

	],
	:fedoraringfolderfiles = 	[

	],
	:freebsdringfolderfiles = 	[

	],
	:macosringfolderfiles = 	[

	],
	:run = "ring main.ring",
	:windowsrun = "",
	:linuxrun = "",
	:macosrun = "",
	:ubunturun = "",
	:fedorarun = "",
	:setup = "ring src/utils/install.ring",
	:windowssetup = "",
	:linuxsetup = "",
	:macossetup = "",
	:ubuntusetup = "",
	:fedorasetup = "",
	:remove = "ring src/utils/uninstall.ring",
	:windowsremove = "",
	:linuxremove = "",
	:macosremove = "",
	:ubunturemove = "",
	:fedoraremove = ""
]