load "stdlibcore.ring"

cPathSep = "/"

if isWindows()
	cPathSep = "\\"
ok

# Remove the simplejson.ring file from the load directory
remove(exefolder() + "load" + cPathSep + "simplejson.ring")

# Remove the simplejson.ring file from the Ring2EXE libs directory
remove(exefolder() + ".." + cPathSep + "tools" + cPathSep + "ring2exe" + cPathSep + "libs" + cPathSep + "simplejson.ring")

# Change current directory to the samples directory
chdir(exefolder() + ".." + cPathSep + "samples")

# Remove the UsingSimpleJSON directory if it exists
if direxists("UsingSimpleJSON")
	OSDeleteFolder("UsingSimpleJSON")
ok