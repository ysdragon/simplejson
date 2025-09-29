# Load the SimpleJSON library based on the operating system.
if isWindows()
	loadlib("ring_simplejson.dll")
but isLinux() or isFreeBSD()
	loadlib("libring_simplejson.so")
but isMacOSX()
	loadlib("libring_simplejson.dylib")
else
	raise("Unsupported OS! You need to build the library for your OS.")
ok