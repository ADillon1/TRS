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

  MetaInfo c = Meta::Get("char"); // automatically registered types.
  MetaInfo uc = Meta::Get("unsigned char");
  MetaInfo i = Meta::Get("int");
  MetaInfo ui = Meta::Get("unsigned int");
  MetaInfo s = Meta::Get("short");
  MetaInfo usi = Meta::Get("unsigned short");
  MetaInfo li = Meta::Get("long");
  MetaInfo uli = Meta::Get("unsigned long");
  MetaInfo fp = Meta::Get("float");
  MetaInfo df = Meta::Get("double");
  MetaInfo ld = Meta::Get("long double");
  MetaInfo b = Meta::Get("bool");
  MetaInfo str = Meta::Get("string");

  Meta::Register<foo>("foo"); // Class/struct registration
  Meta::Register("bar", &foo::bar); // Register abstract members

  MetaInfo intMeta = Meta::Get<signed int>(); // Get meta struct by typename. 
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