#include "trs.hpp" //Meta, VarPtr, Function
#include <iostream> //cout
#include <fstream> // ofstream, ifstream

struct Unregistered_Class {};

struct test_struct
{
  char char_val;
  const char * c_str_val;
  unsigned char u_char_val;
  int int_val;
  unsigned int u_in_val;
  short int short_val;
  unsigned short int u_short_val;
  long int long_val;
  unsigned long int u_long_val;
  float float_val;
  double double_val;
  long double long_double_val;
  bool bool_val;
  std::string string_val;

  test_struct() :
    char_val('a'),
    c_str_val("Hello World"),
    u_char_val(32),
    int_val(-199),
    u_in_val(1),
    short_val(1),
    u_short_val(25),
    long_val(50),
    u_long_val(100),
    float_val((float)(4.0*atan(1))),
    double_val(4.0*atan(1)),
    long_double_val(4.0*atan(1)),
    bool_val(false),
    string_val("test string")
  {}

  int Get() { return int_val; }
  void Set(int i) { int_val = i; }
};

int test_fn(std::string a, float b)
{
  return 5;
}

template <typename T>
void TestType(const char * name, const char * actualName)
{
  using namespace Trs;
  using namespace std;

  MetaInfo meta = Meta::GetType<T>();
  if (meta == Meta::GetType(name))
    cout << "Type Name: " << meta.name << " Size: " << meta.size << endl;
  else 
    cout << "ERROR: Type " << actualName << " is not registered properly!" << endl;
}

template <typename T>
void TestProperty(const char * memName, const char *className)
{
  using namespace Trs;
  using namespace std;

  MetaInfo meta = Meta::GetType<T>();

  if (meta.Valid())
  {
    PropertyInfo mem = meta.properties.find(memName);
    if (mem.Valid())
    {
      cout << "Property Name: " <<
        mem.name <<
        " Property Type Name: " <<
        mem.meta.name <<
        " Property offset: " <<
        mem.offset << endl;
    }
    else
      cout << "ERROR: " << className << " has no Property " << memName << "." << endl;
  }
  else
    cout << "ERROR: " << className << " is not a registered abstract class." << endl;
}

void VarPtrTest(Trs::VarPtr var)
{
  std::cout << var << std::endl;
}

#define TEST_TYPE(TYPE, NAME) TestType<TYPE>( NAME, #TYPE )

#define TEST_Property(TYPE, MEM_NAME) TestProperty<TYPE>( MEM_NAME, #TYPE )

#define ADD_STACK_VarPtrS() \
char char_val = 'a';\
const char * c_str_val = "hello world";\
unsigned char u_char_val = 'a';\
int int_val = 10000;\
unsigned int u_in_val = 100000;\
short int short_val = 128;\
unsigned short int u_short_val = 150;\
long int long_val = 124632;\
unsigned long int u_long_val = 150;\
float float_val = (float)(4 * atan(1));\
double double_val = 4 * atan(1);\
long double long_double_val = 4 * atan(1);\
bool bool_val = false;\
std::string string_val = "hello world";\
test_struct abstract_class

// Automated POD type get check
void test0() 
{
  using namespace std;
  cout << "===== TEST 0 =====" << endl;
  cout << "===== Auto Registered Types =====" << endl;
  TEST_TYPE(char, "char");
  TEST_TYPE(const char *, "const char *");
  TEST_TYPE(unsigned char, "unsigned char");
  TEST_TYPE(int, "int");
  TEST_TYPE(unsigned int, "unsigned int");
  TEST_TYPE(short int, "short");
  TEST_TYPE(unsigned short int, "unsigned short");
  TEST_TYPE(long int, "long");
  TEST_TYPE(unsigned long int, "unsigned long");
  TEST_TYPE(float, "float");
  TEST_TYPE(double, "double");
  TEST_TYPE(long double, "long double");
  TEST_TYPE(bool, "bool");
  TEST_TYPE(std::string, "string");
}

// Abstract Class Registration check.
void test1() 
{
  using namespace std;
  using namespace Trs;
  cout << "===== TEST 1 =====" << endl;
  cout << "===== Abstract Registeration =====" << endl;

  char name[] = "test_struct";
  Meta::Register<test_struct>(name); // issue.
  TEST_TYPE(test_struct, name);

  Meta::Register("char_val", &test_struct::char_val);
  Meta::Register("c_str_val", &test_struct::c_str_val);
  Meta::Register("u_char_val", &test_struct::u_char_val);
  Meta::Register("int_val", &test_struct::int_val);
  Meta::Register("u_in_val", &test_struct::u_in_val);
  Meta::Register("short_val", &test_struct::short_val);
  Meta::Register("u_short_val", &test_struct::u_short_val);
  Meta::Register("long_val", &test_struct::long_val);
  Meta::Register("u_long_val", &test_struct::u_long_val);
  Meta::Register("float_val", &test_struct::float_val);
  Meta::Register("double_val", &test_struct::double_val);
  Meta::Register("long_double_val", &test_struct::long_double_val);
  Meta::Register("bool_val", &test_struct::bool_val);
  Meta::Register("string_val", &test_struct::string_val);

  TEST_Property(test_struct, "char_val");
  TEST_Property(test_struct, "c_str_val");
  TEST_Property(test_struct, "u_char_val");
  TEST_Property(test_struct, "int_val");
  TEST_Property(test_struct, "u_in_val");
  TEST_Property(test_struct, "short_val");
  TEST_Property(test_struct, "u_short_val");
  TEST_Property(test_struct, "long_val");
  TEST_Property(test_struct, "u_long_val");
  TEST_Property(test_struct, "float_val");
  TEST_Property(test_struct, "double_val");
  TEST_Property(test_struct, "long_double_val");
  TEST_Property(test_struct, "bool_val");
  TEST_Property(test_struct, "string_val");
}

// Invalid Meta Test
void test2() 
{
  using namespace std;
  using namespace Trs;
  cout << "===== TEST 2 =====" << endl;
  cout << "===== Invalid Meta =====" << endl;

  MetaInfo meta1 = Meta::GetType("SomeClassThatDoesn'tExist");
  MetaInfo meta2 = Meta::GetType<Unregistered_Class>();

  cout << "Through string: ";
  if (!meta1.Valid())
    cout << "Pass." << endl;
  else
    cout << "Something went wrong with registration." << endl;

  cout << "Through template: ";
  if (!meta2.Valid())
    cout << "Pass." << endl;
  else
    cout << "Something went wrong with registration." << endl;
}

// Invalid Property Test.
void test3() 
{
  using namespace std;
  using namespace Trs;
  cout << "===== TEST 3 =====" << endl;
  cout << "===== Invalid Property =====" << endl;

  MetaInfo meta = Meta::GetType("test_struct");

  if (meta.Valid())
  {
    cout << "Attempt to get:" << endl;
    PropertyInfo real = meta.properties.find("int_val");
    PropertyInfo fake = meta.properties.find("SomePropertyThatDoesn'tExist");

    cout << "Valid Property: ";
    if (real.Valid())
      cout << "Pass." << endl;
    else
      cout << "ERROR: Something went wrong with Property registration." << endl;

    cout << "Invalid Property: ";
    if (!fake.Valid())
      cout << "Pass." << endl;
    else
      cout << "ERROR: Something went wrong with Property registration." << endl;
  }
  else
    cout << "Unable to get test struct meta." << endl;
}
void test4() 
{
  using namespace std;
  using namespace Trs;

  ADD_STACK_VarPtrS();

  cout << "===== TEST 4 =====" << endl;
  cout << "===== VarPtr Constructor Test =====" << endl;

  VarPtr data[14] = {
    char_val,
    c_str_val,
    u_char_val,
    int_val,
    u_in_val, 
    short_val,
    u_short_val,
    long_val,
    u_long_val,
    float_val,
    double_val,
    bool_val,
    string_val,
    abstract_class
  };


  for (size_t i = 0; i < 14; ++i)
    cout << "VarPtr type: " << data[i].Type().name << " with size: " << data[i].Type().size << endl;
}

void test5() 
{
  using namespace std;
  using namespace Trs;

  cout << "===== TEST 5 =====" << endl;
  cout << "===== VarPtr Default constructor =====" << endl;

  VarPtr var;
  if (!var.Valid())
    cout << "Pass." << endl;
}

void test6() 
{
  using namespace std;
  using namespace Trs;

  std::string data = "Hello World";
  cout << "===== TEST 6 =====" << endl;
  cout << "===== By Value VarPtr Passing =====" << endl;

  VarPtr var = data;
  cout << "Value before by value pass: ";
  cout << var << endl;
  cout << "Value after by value pass: ";
  VarPtrTest(var);
}

void test7() 
{
  using namespace std;
  using namespace Trs;

  cout << "===== TEST 7 =====" << endl;
  cout << "===== Ignore Reference Types =====" << endl;

  int i = 10;
  int &r = i;

  VarPtr var = r; // store reference to int

  int b = var.Value<int &>();
  int c = var.Value<int> (); // if this compiles you pass.

  cout << "Pass." << endl;
}

void test8() 
{
  using namespace std;
  using namespace Trs;

  cout << "===== TEST 8 =====" << endl;
  cout << "===== Pointer/Reference Agnosticism =====" << endl;

  int s = 1;             // stack memory
  int *h = new int(2);   // heap memory
  
  VarPtr var = s;
  cout << "Stack Memory" << endl;
  cout << "int: " << var.Value<int>() << endl;
  cout << "const int: " << var.Value<const int>() << endl;
  cout << "int &: " << var.Value<int &>() << endl;
  cout << "const int &: " << var.Value<const int &>() << endl;
  cout << "int *: " << var.Value<int *>() << endl;
  cout << "const int *: " << var.Value<const int *>() << endl << endl;

  var = h;
  cout << "Heap Memory" << endl;
  cout << "int: " << var.Value<int>() << endl;
  cout << "const int: " << var.Value<const int>() << endl;
  cout << "int &: " << var.Value<int &>() << endl;
  cout << "const int &: " << var.Value<const int &>() << endl;
  cout << "int *" << var.Value<int *>() << endl;
  cout << "const int *: " << var.Value<const int *>() << endl;

  delete (h);
}

void test9()
{
  using namespace std;
  using namespace Trs;

  cout << "===== TEST 9 =====" << endl;
  cout << "===== Pointer handling =====" << endl;
  int * pint = new int(3);

  VarPtr p = pint;

  int three = p.Value<int>();
  int &rthree = p.Value<int>();

  int *pthree = p.Value<int *>();

  assert(three == *pint); // assert that this is all the same data
  assert(pthree == pint);
  assert(pint == &rthree);

  cout << "If you see this you pass." << endl;

  delete(pint);
}

void test10() 
{
  using namespace std;
  using namespace Trs;

  char str1[] = "I am non-const";
  const char *str2 = "hello";
  std::string str3 = "world";

  VarPtr var1 = str1;
  VarPtr var2 = str2;
  VarPtr var3 = str3;
  VarPtr var4 = str3.c_str();

  cout << var1 << var2 << var3 << var4 << endl;
}

void test11() 
{
  Trs::Meta::Register("get", &test_struct::Get);
  Trs::Meta::Register("set", &test_struct::Set);
  MetaInfo meta = Trs::Meta::GetType<test_struct>();

  int ret = 0;
  test_struct *d = new test_struct();
  FunctionInfo getter = meta.functions.find("get");
  FunctionInfo setter = meta.functions.find("set");
  Trs::VarPtr * args = nullptr;

  getter(ret, d);
  setter(Trs::VarPtr(), d, 1000);
  getter(ret, d);

  Trs::VarPtr a;
  Trs::VarPtr b = a;

  Trs::VarPtr c = b.Value<Trs::VarPtr>();
}

void test12() {}
void test13() {}
void test14() {}
void test15() {}
void test16() {}
void test17() {}
void test18() {}
void test19() {}

void(*tests[])() = {
  test0, test1, test2, test3, test4, test5, test6,
  test7, test8, test9, test10, test11, test12, test13,
  test14, test15, test16, test17, test18, test19
};

int main(int argc, char *argv[])
{
  if (argc == 2 && (atoi(argv[1]) >= 0 && atoi(argv[1]) < 20))
  {
    tests[atoi(argv[1])]();
    return 0;
  }

  for (size_t i = 0; i < 20; ++i)
    tests[i]();

  return 0;
}

/* TODO items:
* Redesign Meta Interface
* Add in new, shallow copy, deep copy, and delete auto generated functions
* Unit tests for functions
* Add enforcement of const correctness
* Add in custom assert system for low level errors (and unhandled high level errors)
* Add in optional redefining of assert to custom assertion system
* Add in exception handling for high level user errors.
* Make serialization optional for certain types (const types, etc)
* Enforce type saftey for serialization
* Reimplement static type array and auto registeration on Property registration and VarPtr assignment
* Remove Trs namespace
* Change VarPtr to VarPtr
* Convert Meta from class to namespace, place VarPtr inside of it. (maybe)
* Generic Getter and Setter for class Propertys
*/