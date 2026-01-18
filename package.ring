aPackageInfo = [
	:name = "SimpleJSON",
	:description = "JSON parsing and manipulation library for Ring programming language.",
	:folder = "simplejson",
	:developer = "ysdragon",
	:email = "youssefelkholey@gmail.com",
	:license = "MIT",
	:version = "1.2.0",
	:ringversion = "1.23",
	:versions = 	[
		[
			:version = "1.2.0",
			:branch = "master"
		],
		[
			:version = "1.1.0",
			:branch = "1.1.0"
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
		"examples/01_basic_decode.ring",
		"examples/02_basic_encode.ring",
		"examples/03_nested_structures.ring",
		"examples/04_json_patch.ring",
		"examples/05_merge_patch.ring",
		"examples/06_binary_formats.ring",
		"examples/07_ndjson.ring",
		"examples/08_validation_and_info.ring",
		"examples/09_formatting.ring",
		"examples/10_object_manipulation.ring",
		"examples/11_path_queries.ring",
		"examples/12_file_io.ring",
		"examples/13_base64.ring",
		"examples/14_json_comments.ring",
		"examples/15_csv_conversion.ring",
		"examples/16_toml_conversion.ring",
		"LICENSE",
		"README.md",
		"src/ring_simplejson.cpp",
		"src/utils/color.ring",
		"src/utils/install.ring",
		"src/utils/uninstall.ring",
		"tests/simplejson_test.ring"
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
		"lib/linux/arm64/libring_simplejson.so",
		"lib/linux/musl/amd64/libring_simplejson.so",
		"lib/linux/musl/arm64/libring_simplejson.so"
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
