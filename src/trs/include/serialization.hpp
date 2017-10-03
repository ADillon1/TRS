#ifndef _TRS_SERIALIZATION_HPP
#define _TRS_SERIALIZATION_HPP

namespace _trs_backend
{
  struct Padding
  {
    static int &GetLevel()
    {
      static int level = 0;
      return level;
    }

    static void Increase() { GetLevel() += 2; }

    static std::ostream &Add(std::ostream &os)
    {
      for (int i = 0; i < GetLevel(); ++i) os << " ";
      return os;
    }

    static void Decrease() { GetLevel() -= 2; }
  };

  std::string ReadToken(std::istream &is, char token = ' ')
  {
    std::string ret;
    char temp = is.get();
    while (ret.empty() || (temp != token && temp != '\n'))
    {
      if (temp != token && temp != '\n') ret += temp;
      temp = is.get();
    }
    return ret;
  }

  void *PropertyPtr(void *data, size_t offset) { return ((char *)data) + offset; }

  void GenericSerialize(std::ostream &os, VarPtr var)
  {
    using namespace std;
    os << var.type().name << endl;
    Padding::Add(os) << "{" << endl;
    Padding::Increase();

    for (auto & it : var.type().properties)
    {
      Padding::Add(os) << it.second.name << " = ";
      VarPtr(PropertyPtr(var.data(), it.second.offset), &it.second.meta).serialize(os) << endl;
    }


    Padding::Decrease();
    Padding::Add(os);
    os << "}" << endl;
  }

  void GenericDeserialize(std::istream &is, VarPtr var)
  {
    std::string name = ReadToken(is);
    assert(name == var.type().name);
    name = ReadToken(is); // {
    name = ReadToken(is); // first mem

    while (name != "}")
    {
      if (is.eof()) break;
      PropertyInfo mem = var.type().properties.find(name);
      if (mem.valid())
      {
        name = ReadToken(is); // = 
        VarPtr(PropertyPtr(var.data(), mem.offset), &mem.meta).deserialize(is);
      }
      name = ReadToken(is); // next Property
    }
  }

  template <typename T>
  void Serialize(std::ostream &os, VarPtr var) { os << var.value<T>(); }

  template <>
  void Serialize<bool>(std::ostream &os, VarPtr var) { os << (var.value<bool>() ? "true" : "false"); }

  template <>
  void Serialize<char *>(std::ostream &os, VarPtr var) { os << '"' << var.value<char *>() << '"'; }

  template <>
  void Serialize<const char *>(std::ostream &os, VarPtr var) { os << '"' << var.value<const char *>() << '"'; }

  template <>
  void Serialize<std::string>(std::ostream &os, VarPtr var) { os << '"' << var.value<std::string>().c_str() << '"'; }

  template <typename T>
  void Deserialize(std::istream &is, VarPtr var) { is >> var.value<T>(); }

  template <>
  void Deserialize<bool>(std::istream &is, VarPtr var) { var.value<bool>() = (ReadToken(is) == "true" ? true : false); }

  template <>
  void Deserialize<char *>(std::istream &is, VarPtr var) { /* Don't deserialize char pointer's. */ }

  template <>
  void Deserialize<const char *>(std::istream &is, VarPtr var) { /* Don't deserialize char pointer's. */ }

  template <>
  void Deserialize<std::string>(std::istream &is, VarPtr var)
  {
    is.get(); // first "
    var.value<std::string>() = ReadToken(is, '"');
  }
}

#endif