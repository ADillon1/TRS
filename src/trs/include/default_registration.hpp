#ifndef _TRS_DEFAULT_REG_HPP
#define _TRS_DEFAULT_REG_HPP


namespace _trs_backend
{
  /** Plain old data struct
  Used to auto Register POD types for the user at compile time.
  */
  template <typename T>
  struct POD { POD(const char * name) { trs::reflect<T>(name); } };

  /***** Auto Registered Types *****/
  POD<char> c("char");
  POD<const char *> cc("const char *");
  POD<unsigned char> uc("unsigned char");
  POD<int> i("int");
  POD<unsigned int> ui("unsigned int");
  POD<short int> si("short");
  POD<unsigned short int> usi("unsigned short");
  POD<long int> li("long");
  POD<unsigned long int> uli("unsigned long");
  POD<float> fp("float");
  POD<double> df("double");
  POD<long double> ld("long double");
  POD<bool> b("bool");
  POD<std::string> str("string");
}    

#endif