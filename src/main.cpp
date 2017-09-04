#include "trs.hpp" //Meta, Variable
#include <iostream> //cout
#include <fstream> // ofstream, ifstream

struct foo
{
  int bar;
  foo(int val) : bar(val) {}
};

int main(void)
{
  using namespace std;
  Meta::Register<int>("integer"); // Basic type registration
  Meta::Register<foo>("foo"); // Class/struct registration
  Meta::Register("bar", &foo::bar); // Register abstract members

  MetaInfo intMeta = Meta::Get<int>(); // Get meta struct by typename. 
  MetaInfo fooMeta = Meta::Get("foo"); // get meta struct by string identifier.
  MemberInfo invalInfo = fooMeta.FindMember("NA"); // finding a member info struct.

    if (!invalInfo.Valid()) //valid member/meta check.
      cout << "NA is not a registered member!" << endl;

  foo data1(10); // data to store, serialize
  foo data2(1); // data to store, deserialize

  Variable var1 = data1; // Store data generically.
  Variable var2 = data2;

  //Serialization of data in memory to file.
  ofstream ofile("filename.txt");
  var1.Serialize(ofile);
  ofile.close();

  cout << endl << "Lets deserialize a " << fooMeta.name << endl;

  // Deserialization of file into memory.
  ifstream  ifile("filename.txt");
  var2.Deserialize(ifile);
  ifile.close();

  cout << var2; // output data to standard out.
  return 0;
}

/*
NA is not a registered member!
foo
{
bar = 10
}
*/