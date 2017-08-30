#include "trs.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

struct TestStruct
{
  int integer;
  float floatingPoint;
  double doublefloat;
  char character;
  char characterArray[10];
  int integerArray[10];
  bool boolean;
  bool booleanArray[10];
  std::string characterString;
  TestStruct() : integer(0),
    floatingPoint(10.0f),
    doublefloat(10.0),
    character('c'),
    boolean(true)
  {
    for (size_t i = 0; i < 10; ++i)
    {
      integerArray[i] = i + 1;
      booleanArray[i] = false;
      characterArray[i] = 'a';
    }
  }
};

struct TestStruct2
{
  TestStruct abstract;
};

int main(void)
{
  using namespace std;

  Meta::Register<int>("integer"); // basic type registration
  Meta::Register<float>("float");
  Meta::Register<double>("double");
  Meta::Register<bool>("bool");
  Meta::Register<char>("char");
  Meta::Register<char *>("character pointer");
  Meta::Register<string>("string"); // string is a little flimsy, will only deserialize a token atm.

  Meta::Register<TestStruct>("TestStruct");
  Meta::Register("integer", &TestStruct::integer);
  Meta::Register("floatingPoint", &TestStruct::floatingPoint);
  Meta::Register("doubleFloat", &TestStruct::doublefloat);
  Meta::Register("character", &TestStruct::character);
  Meta::Register("characterArray", &TestStruct::characterArray);
  Meta::Register("integerArray", &TestStruct::integerArray);
  Meta::Register("boolean", &TestStruct::boolean);
  Meta::Register("booleanArray", &TestStruct::booleanArray);

  Meta::Register<TestStruct2>("TestStruct2");
  Meta::Register("abstract", &TestStruct2::abstract);

  MetaInfo testMeta = Meta::Get("TestStruct");
  MetaInfo explosionMeta = Meta::Get("something?");
  MetaInfo stringMeta = Meta::Get<std::string>();
  MemberInfo memfake = testMeta.FindMember("j");
  
  if (!stringMeta.Valid())
    printf("this is not a registered type!\n");

  if (!explosionMeta.Valid())
    printf("this is not a registered type!\n");

  if (!memfake.Valid())
    printf("this is not a registered member!\n");

  // test arrays.
  std::string name = "Hello World!";
  Variable var1 = name;
  cout << var1 << endl;

  return 0;
}
