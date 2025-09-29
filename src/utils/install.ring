/*
	SimpleJSON Library Install Script
	----------------------------------
	This script installs SimpleJSON library for the current platform.
	It detects the OS and architecture, then copies or symlinks the library to the 
	appropriate system location.
*/

load "stdlibcore.ring"
load "src/utils/color.ring"

# Default library settings
cLibPrefix = "lib"
cPathSep = "/"

# Platform detection and configuration
switch true
	on isWindows()
		cLibPrefix = ""
		cPathSep = "\\"
		cLibExt = ".dll"
		cOSName = "windows"
	on isLinux()
		cLibExt = ".so"
		cOSName = "linux"
	on isFreeBSD()
		cLibExt = ".so"
		cOSName = "freebsd"
	on isMacOSX()
		cLibExt = ".dylib"
		cOSName = "macos"
	other
		? colorText([:text = "Error: Unsupported operating system detected!", :color = :BRIGHT_RED, :style = :BOLD])
		return

off

# Get system architecture
cArchName = getarch()
switch cArchName
	on "x86"
		cArchName = "i386"
	on "x64"
		cArchName = "amd64"
	on "arm64"
		cArchName = "arm64"
	other
		? colorText([:text = "Error: Unsupported architecture: " + cArchName, :color = :BRIGHT_RED, :style = :BOLD])
		return
off

# Construct the package path
cPackagePath = exefolder() + ".." + cPathSep + "tools" + cPathSep + "ringpm" + cPathSep + "packages" + cPathSep + "simplejson"

# Construct the library path
cLibPath = cPackagePath + cPathSep + "lib" + cPathSep + 
		cOSName + cPathSep + cArchName + cPathSep + cLibPrefix + "ring_simplejson" + cLibExt

# Verify library exists
if not fexists(cLibPath)
	? colorText([:text = "Error: SimpleJSON library not found!", :color = :BRIGHT_RED, :style = :BOLD])
	? colorText([:text = "Expected location: ", :color = :YELLOW]) + colorText([:text = cLibPath, :color = :CYAN])
	? colorText([:text = "Please ensure the library is built for your platform (" + cOSName + "/" + cArchName + ")", :color = :BRIGHT_MAGENTA])
	? colorText([:text = "You can refer to README.md for build instructions: ", :color = :CYAN]) + colorText([:text = cPackagePath + cPathSep + "README.md", :color = :YELLOW])
	return
ok

# Install library based on platform
try
	if isWindows()
		systemSilent("copy /y " + '"' + cLibPath + '" "' + exefolder() + '"')
	else
		cLibDir = exefolder() + ".." + cPathSep + "lib"
		if isFreeBSD() or isMacOSX()
			cDestDir = "/usr/local/lib"
		but isLinux()
			cDestDir = "/usr/lib"
		ok
		cCommand1 = 'ln -sf "' + cLibPath + '" "' + cLibDir + '"'
		cCommand2 = 'which sudo >/dev/null 2>&1 && sudo ln -sf "' + cLibPath + '" "' + cDestDir + 
				'" || (which doas >/dev/null 2>&1 && doas ln -sf "' + cLibPath + '" "' + cDestDir + 
				'" || ln -sf "' + cLibPath + '" "' + cDestDir + '")'

		system(cCommand1)
		system(cCommand2)
	ok

	# Copy examples to the samples/UsingSimpleJSON directory
	cCurrentDir = currentdir()
	cExamplesPath = cPackagePath + cPathSep + "examples"
	cSamplesPath = exefolder() + ".." + cPathSep + "samples" + cPathSep + "UsingSimpleJSON"

	# Ensure the samples directory exists and create it if not
	if not direxists(exefolder() + ".." + cPathSep + "samples")
		makeDir(exefolder() + ".." + cPathSep + "samples")
	ok

	# Create the UsingSimpleJSON directory
	makeDir(cSamplesPath)

	# Change to the samples directory
	chdir(cSamplesPath)

	# Loop through the examples and copy them to the samples directory
	for item in dir(cExamplesPath) 
		if item[2]
			OSCopyFolder(cExamplesPath + cPathSep, item[1])
		else
			OSCopyFile(cExamplesPath + cPathSep + item[1])
		ok
	next
	
	# Change back to the original directory
	chdir(cCurrentDir)

	# Check if simplejson.ring exists in the exefolder
	if fexists(exefolder() + "simplejson.ring")
		# Remove the existing simplejson.ring file
		remove(exefolder() + "simplejson.ring")

		# Write the load command to the simplejson.ring file
		write(exefolder() + "load" + cPathSep + "simplejson.ring", `load "/../../tools/ringpm/packages/simplejson/lib.ring"`)
	ok
	
	# Ensure the Ring2EXE libs directory exists
	if direxists(exefolder() + ".." + cPathSep + "tools" + cPathSep + "ring2exe" + cPathSep + "libs")
		# Write the library definition to the simplejson.ring file for Ring2EXE
		write(exefolder() + ".." + cPathSep + "tools" + cPathSep + "ring2exe" + cPathSep + "libs" + cPathSep + "simplejson.ring", getRing2EXEContent())
	ok
	
	? colorText([:text = "Successfully installed Ring SimpleJSON!", :color = :BRIGHT_GREEN, :style = :BOLD])
	? colorText([:text = "You can refer to samples in: ", :color = :CYAN]) + colorText([:text = cSamplesPath, :color = :YELLOW])
	? colorText([:text = "Or in the package directory: ", :color = :CYAN]) + colorText([:text = cExamplesPath, :color = :YELLOW])
catch
	? colorText([:text = "Error: Failed to install Ring SimpleJSON!", :color = :BRIGHT_RED, :style = :BOLD])
	? colorText([:text = "Details: ", :color = :YELLOW]) + colorText([:text = cCatchError, :color = :CYAN])
done


func getRing2EXEContent
	return `aLibrary = [:name = :simplejson,
	 :title = "SimpleJSON",
	 :windowsfiles = [
		"ring_simplejson.dll"
	 ],
	 :linuxfiles = [
		"libring_simplejson.so"
	 ],
	 :macosxfiles = [
		"libring_simplejson.dylib"
	 ],
	 :freebsdfiles = [
	 	"libring_simplejson.so"
	 ],
	 :ubuntudep = "",
	 :fedoradep = "",
	 :macosxdep = ""
	]`