#include "./trs/trs.hpp" //Meta, VarPtr, Function
#include <iostream> //cout
#include <fstream> // ofstream, ifstream

struct Unregistered_Class {};

#define PASTE( _, __ )  _##__
#define INTERMEDIATE( _ ) PASTE( test, _ )
#define GENERATE_NAME( ) INTERMEDIATE( __COUNTER__ )
#define TEST GENERATE_NAME()

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

  int* Get() { return &int_val; }
  void Set(int i) { int_val = i; }
};

int test_fn(std::string a, float b) { return (int)b; }

void test_fn2() { std::cout << "inside static function 2!" << std::endl;  }

template <typename T>
void TestType(const char * name, const char * actualName)
{
  using namespace std;

  MetaInfo meta = trs::get_type<T>();
  if (meta == trs::get_type(name))
    cout << "Type Name: " << meta.name << " Size: " << meta.size << endl;
  else 
    cout << "ERROR: Type " << actualName << " is not registered properly!" << endl;
}

template <typename T>
void TestProperty(const char * memName, const char *className)
{
  using namespace std;

  MetaInfo meta = trs::get_type<T>();

  if (meta.valid())
  {
    PropertyInfo mem = meta.properties.find(memName);
    if (mem.valid())
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

void VarPtrTest(trs::VarPtr var)
{
  var.serialize(std::cout) << std::endl;
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
test_struct abstract_class;

// Automated POD type get check
void TEST()
{
  using namespace std;
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
void TEST()
{
  using namespace std;
  cout << "===== Abstract Registeration =====" << endl;
  
  static char name[] = "test_struct";
  trs::reflect<test_struct>(name)
    .constructor<>()
    .property("char_val", &test_struct::char_val)
    .property("c_str_val", &test_struct::c_str_val)
    .property("u_char_val", &test_struct::u_char_val)
    .property("int_val", &test_struct::int_val)
    .property("u_in_val", &test_struct::u_in_val)
    .property("short_val", &test_struct::short_val)
    .property("u_short_val", &test_struct::u_short_val)
    .property("long_val", &test_struct::long_val)
    .property("u_long_val", &test_struct::u_long_val)
    .property("float_val", &test_struct::float_val)
    .property("double_val", &test_struct::double_val)
    .property("long_double_val", &test_struct::long_double_val)
    .property("bool_val", &test_struct::bool_val)
    .property("string_val", &test_struct::string_val)
    .function("get", &test_struct::Get)
    .function("set", &test_struct::Set);

  TEST_TYPE(test_struct, name);
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
void TEST()
{
  using namespace std;
  cout << "===== Invalid Meta =====" << endl;

  MetaInfo meta1 = trs::get_type("SomeClassThatDoesn'tExist");
  MetaInfo meta2 = trs::get_type<Unregistered_Class>();

  cout << "Through string: ";
  if (!meta1.valid())
    cout << "Pass." << endl;
  else
    cout << "Something went wrong with registration." << endl;

  cout << "Through template: ";
  if (!meta2.valid())
    cout << "Pass." << endl;
  else
    cout << "Something went wrong with registration." << endl;
}

// Invalid Property Test.
void TEST()
{
  using namespace std;
  cout << "===== Invalid Property =====" << endl;

  MetaInfo meta = trs::get_type("test_struct");

  if (meta.valid())
  {
    cout << "Attempt to get:" << endl;
    PropertyInfo real = meta.properties.find("int_val");
    PropertyInfo fake = meta.properties.find("SomePropertyThatDoesn'tExist");

    cout << "Valid Property: ";
    if (real.valid())
      cout << "Pass." << endl;
    else
      cout << "ERROR: Something went wrong with Property registration." << endl;

    cout << "Invalid Property: ";
    if (!fake.valid())
      cout << "Pass." << endl;
    else
      cout << "ERROR: Something went wrong with Property registration." << endl;
  }
  else
    cout << "Unable to get test struct meta." << endl;
}
void TEST()
{
  using namespace std;
  cout << "===== VarPtr Constructor Test =====" << endl;
  ADD_STACK_VarPtrS();
  trs::VarPtr data[14] = {
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
    cout << "VarPtr type: " << data[i].type().name << " with size: " << data[i].type().size << endl;
}

void TEST()
{
  using namespace std;
  cout << "===== VarPtr Default constructor =====" << endl;

  trs::VarPtr var;
  if (!var.valid())
    cout << "Pass." << endl;
}

void TEST()
{
  using namespace std;
  std::string data = "Hello World";
  cout << "===== By Value VarPtr Passing =====" << endl;

  trs::VarPtr var = data;
  cout << "Value before by value pass: ";
  var.serialize(cout) << endl;
  cout << "Value after by value pass: ";
  VarPtrTest(var);
}

void TEST()
{
  using namespace std;
  cout << "===== Ignore Reference Types =====" << endl;

  int i = 10;
  int &r = i;

  trs::VarPtr var = r; // store reference to int

  int b = var.value<int &>();
  int c = var.value<int> (); // if this compiles you pass.

  cout << "Pass." << endl;
}

void TEST()
{
  using namespace std;
  cout << "===== Pointer/Reference Agnosticism =====" << endl;

  int s = 1;             // stack memory
  int *h = new int(2);   // heap memory
  
  trs::VarPtr var = s;
  cout << "Stack Memory" << endl;
  cout << "int: " << var.value<int>() << endl;
  cout << "const int: " << var.value<const int>() << endl;
  cout << "int &: " << var.value<int &>() << endl;
  cout << "const int &: " << var.value<const int &>() << endl;
  cout << "int *: " << var.value<int *>() << endl;
  cout << "const int *: " << var.value<const int *>() << endl << endl;

  var = h;
  cout << "Heap Memory" << endl;
  cout << "int: " << var.value<int>() << endl;
  cout << "const int: " << var.value<const int>() << endl;
  cout << "int &: " << var.value<int &>() << endl;
  cout << "const int &: " << var.value<const int &>() << endl;
  cout << "int *" << var.value<int *>() << endl;
  cout << "const int *: " << var.value<const int *>() << endl;

  delete (h);
}

void TEST()
{
  using namespace std;
  cout << "===== Pointer handling =====" << endl;
  int * pint = new int(3);

  trs::VarPtr p = pint;

  int three = p.value<int>();
  int &rthree = p.value<int>();

  int *pthree = p.value<int *>();

  assert(three == *pint); // assert that this is all the same data
  assert(pthree == pint);
  assert(pint == &rthree);

  cout << "If you see this you pass." << endl;

  delete(pint);
}

void TEST()
{
  using namespace std;
  cout << "===== Type conversion test  =====" << endl;
  char str1[] = "I am non-const char";
  const char *str2 = "hello";
  std::string str3 = "world";

  trs::VarPtr var1 = str1;
  trs::VarPtr var2 = str2;
  trs::VarPtr var3 = str3;
  trs::VarPtr var4 = str3.c_str();

  var1.serialize(cout) << ' ';
  var2.serialize(cout) << ' ';
  var3.serialize(cout) << ' '; 
  var4.serialize(cout) << endl;
}

void TEST()
{
  using namespace std;
  cout << "===== Member Function Test  =====" << endl;
  MetaInfo meta = trs::get_type<test_struct>();
  
  test_struct *d = new test_struct();
  FunctionInfo getter = meta.functions.find("get");
  FunctionInfo setter = meta.functions.find("set");
  
  trs::VarPtr ptr = (int *)nullptr; // set it equal to nullptr
  getter(ptr, d);
  cout << "before: ";
  ptr.serialize(cout) << endl;
  setter(trs::VarPtr(), d, 500);
  getter(ptr, d);
  
  cout << "after: ";
  ptr.serialize(cout) << endl;
  delete d;
}

void TEST()
{
  using namespace std;
  cout << "===== Static Function Test  =====" << endl;
  trs::reflect("static_func", test_fn);
  trs::reflect("static_func2", test_fn2);
  std::string a = "test!";
  float b = 10.0f;
  trs::VarPtr ret = 10;

  FunctionInfo fn = trs::get_func("static_func");
  FunctionInfo fn2 = trs::get_func("static_func2");

  if (fn.valid())
  {
    int i = 10;
    trs::VarPtr ret = i;
    fn(ret, nullptr, a, b);
    std::cout << "return from fn: " << ret.value<int>() << endl;
  }
  else
    cout << "ERROR: registering static_fn." << endl;

  if (fn2.valid())
    fn2(trs::VarPtr());
  else
    cout << "ERROR: registering static_fn2." << endl;
}

struct test
{
  float a;
  int b;
  int &get() { return b; }
  void set(float val) { a = val; }
};

void TEST()
{
  using namespace std;
  //int and float auto registered.

  trs::reflect<test>("test_struct")
    .constructor<>()               // add generic construction.
    .property("a", &test::a)
    .property("b", &test::b)
    .function("get", &test::get)   // add functions
    .function("set", &test::set);

  MetaInfo meta = trs::get_type<test>();

  if (meta.valid())
  {
    for (auto &it : meta.properties)
      cout << it.second.name;

    for (auto &it : meta.functions)
      cout << it.second.name;

    test t;
    int i = 10;
    trs::VarPtr ret = i;
    meta.properties.find("a").valid();
    meta.functions.find("get")(ret, &t);
  }
}
void TEST() 
{

}

void TEST() 
{

}

void TEST() 
{

}

void TEST() 
{

}

void TEST() 
{

}

void TEST() 
{

}
void TEST() 
{

}

std::vector<void(*)()> tests = {
  test0, test1, test2, test3, test4, test5, test6,
  test7, test8, test9, test10, test11, test12, test13,
  test14, test15, test16, test17, test18, test19, test20
};

int main(int argc, char *argv[])
{
  using namespace std;
  if (argc == 2 && (atoi(argv[1]) >= 0 && atoi(argv[1]) < (int)tests.size()))
  {
    tests[atoi(argv[1])]();
    return 0;
  }

  for (size_t i = 0; i < (int)tests.size(); ++i)
  {
    cout << "===== TEST " << i << " =====" << endl;
    tests[i]();
    cout << endl;
  }

  return 0;
}

/* TODO items:
* Add constructor support
* Add in new, shallow copy, deep copy, and delete auto generated functions
* Add support for standard allocators.
* Add enforcement of const correctness
* Add in custom assert system for low level errors (and unhandled high level errors)
* Add in optional redefining of assert to custom assertion system
* Add in exception handling for high level user errors.
* Make serialization optional for certain types (const types, etc)
* Enforce type saftey for serialization
* Reimplement static type array and auto registeration on Property registration and VarPtr assignment
* Convert Meta from class to namespace, place VarPtr inside of it. (maybe)
* Generic Getter and Setter for class Propertys
*/