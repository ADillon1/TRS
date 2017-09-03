# Introduction
TRS is a dynamic reflection and automated serialization system designed for C++11.

This system was designed to provide an accessible, extendible, and maintainable stand alone type instrospection system for C++ projects.

## Features
1. Header only and single file for easy integration into any projects.
2. Designed for use on Windows and Linux platform.
3. Simple template interface for registering, managing, and referencing data type information.
4. Default JSON like serialization for registered types to and from human readible text files.
5. Automated serialization upon registration of both basic and abstract types.
6. Runtime type information useful for debugging tools.
7. Dependecy free.

# Usage
## Example
```C++
#include "trs.hpp"  // Meta, Variable
#include <iostream> // cout
#include <fstream>  // ofstream, ifstream

struct foo
{
  int bar;
  foo(int val) : bar(val) {}
};

int main(void)
{
  using namespace std;
  Meta::Register<int>("integer");   // Basic type registration
  Meta::Register<foo>("foo");       // Class/struct registration
  Meta::Register("bar", &foo::bar); // Register abstract members

  MetaInfo intMeta = Meta::Get<int>();             // Get meta struct by typename. 
  MetaInfo fooMeta = Meta::Get("foo");             // get meta by string identifier.
  MemberInfo invalInfo = fooMeta.FindMember("NA"); // finding a member info struct.

  //valid member/meta check.
  if (!invalInfo.Valid()) 
    cout << "NA is not a registered member!" << endl;

  foo data1(10); // data to store, serialize
  foo data2(1);  // data to store, deserialize

  Variable var1 = data1; // Store data generically.
  Variable var2 = data2;

  //Serialization of data in memory to file.
  ofstream ofile("filename.txt");
  var1.Serialize(ofile);
  ofile.close();
  
  cout << "Lets deserialize a " << fooMeta.name << endl;

  // Deserialization of file into memory.
  ifstream  ifile("filename.txt");
  var2.Deserialize(ifile);
  ifile.close();

  cout << var2; // output data to standard out.
  return 0;
}
```
## Output
```
NA is not a registered member!
Lets deserialize a foo
foo
{
  bar = 10
}
```

# Building
Tested on:
- [ ] Visual Studio 2017
- [x] Visual Studio 2015
- [ ] Visual Studio 2013
- [x] g++ 4.8.4

## Instructions
### Premake5
Run [premake5](https://github.com/premake/premake-core/wiki/Using-Premake "Premake5's Documentation") from the premake directory using the action with the preferred build enviornment. Resulting solutions/make files will be output into the \build directory.

Compiling/Linking using your preferred build enviornment, objects will appear in /temp, while the build target is /bin.

The clean action can be used to automatically remove everything but the source and premake5 when you are done editing.

### Manual notes
#### Windows
Define [CRT_SECURE_NO_WARNINGS](https://stackoverflow.com/questions/16883037/remove-secure-warnings-crt-secure-no-warnings-from-projects-by-default-in-vis "yep") in the visual studio solution properties to quite warnings about safe versions of c functions, assuming they exist.

#### Linux
[Enable C++11](https://stackoverflow.com/questions/10363646/compiling-c11-with-g "Lol") using the appropriate compile flag.




