#pragma once
#include <unordered_map> // MetaMap
#include <assert.h>      // Assert
#include <istream>       // Deserialization
#include <ostream>       // Serialization
#include <cstring>       // sprintf
#include <string>        // string

/***** Reflection Forward Declarations *****/
namespace Reflection { class MetaData;  class Member; }
class Meta;
class Variable;

/***** Serialization Forward Declares *****/
namespace Serialization
{
  void GenericSerialize(std::ostream &os, Variable var);
  void GenericDeserialize(std::istream &os, Variable var);
  template <typename T> void Serialize(std::ostream &os, Variable var);
  template <typename T> void Deserialize(std::istream &is, Variable var);
  template <typename T, size_t size> void SerializeArray(std::ostream &, Variable);
  template <typename T, size_t size> void DeserializeArray(std::istream &, Variable);
  template <> void Serialize<std::string>(std::ostream &os, Variable var);
  template <> void Deserialize<std::string>(std::istream &is, Variable var);
}

/***** Serialization Declarations *****/
typedef void(*SerializeFN)(std::ostream &, Variable);
typedef void(*DeserializeFN)(std::istream &, Variable);

/***** Info Types for User *****/
typedef const Reflection::MetaData & MetaInfo;
typedef const Reflection::Member & MemberInfo;

/***** Reflection Backend *****/
namespace Reflection
{
  typedef std::unordered_map<std::string, MetaInfo> MetaMap; // map typedefs
  typedef std::pair<std::string, MetaInfo> KeyValue;

  class Member
  {
    friend class ::Meta;
    friend class MetaData;
    friend void Serialization::GenericSerialize(std::ostream &os, Variable var);

    Member() = delete;
    Member(const char * name, size_t offset, MetaInfo meta) : name(name), offset(offset), meta(meta) {}
    Member(const MetaData &) = delete;
    Member & operator=(const MetaData &) = delete;

    const Member * next;
  public:

    bool Valid() const { return this != nullptr; }

    const char *name;
    size_t offset;
    MetaInfo meta;
  };

  class MetaData
  {
    friend class ::Meta;
    template <typename T, bool a = std::is_class<T>::value> friend struct Setup;
    friend void Serialization::GenericSerialize(std::ostream &os, Variable var);
    friend class ::Variable;

    MetaData() : _members(nullptr), name("Unregistered"), size(0){}
    MetaData(const char *name, size_t size) : _members(nullptr), name(name), size(size){}
    MetaData(const MetaData &) = delete;
    MetaData & operator=(const MetaData &) = delete;
    void AddMember(Member * member)
    {
      member->next = _members;
      _members = member;
    }

    const Member * _members;
    SerializeFN _serialize;
    DeserializeFN _deserialize;

  public:
    ~MetaData() {}
    bool operator==(const MetaData &rhs) const { return this == &rhs; }
    bool Valid() const { return this != nullptr && size != 0; }
    MemberInfo FindMember(std::string &str) const { return FindMember(str.c_str()); }
    MemberInfo FindMember(const char * name) const
    {
      const Member *mem = _members;
      while (mem)
      {
        if (strcmp(mem->name, name) == 0)
          return *mem;
        mem = mem->next;
      }

      return *mem;
    }

    const char * name;
    size_t size;
  };

  template <typename T, bool a>
  struct Setup
  {
    static void Serializers(MetaData &meta)
    {
      meta._serialize = Serialization::GenericSerialize;
      meta._deserialize = Serialization::GenericDeserialize;
    }
  };

  template <typename T, size_t size>
  struct Setup<T[size], false>
  {
    static void Serializers(MetaData &meta)
    {
      meta._serialize = Serialization::SerializeArray<T, size>;
      meta._deserialize = Serialization::DeserializeArray<T, size>;
    }
  };

  template <typename T>
  struct Setup<T, false> 
  { 
    static void Serializers(MetaData &meta)
    { 
      meta._serialize = Serialization::Serialize<T>;
      meta._deserialize = Serialization::Deserialize<T>;
    } 
  };

  template <>
  struct Setup<std::string, true>
  {
    static void Serializers(MetaData &meta)
    {
      meta._serialize = Serialization::Serialize<std::string>;
      meta._deserialize = Serialization::Deserialize<std::string>;
    }
  };
}

/***** Reflection Interface *****/
class Meta
{
  static Reflection::MetaMap &GetMap() // hidden map for string hashing.
  {
    static Reflection::MetaMap map;
    return map;
  }

  template <typename T>
  static Reflection::MetaData &_Get() // hidden non-const meta getter of meta.
  {
    static Reflection::MetaData meta;
    return meta;
  }

public:
  template <typename T>
  static MetaInfo Get() { return _Get<T>(); } // const version available to user.

  static MetaInfo Get(std::string &name) { return Get(name.c_str()); } // string hashing

  static MetaInfo Get(const char *name) // string literal hashing for meta
  { 
    auto it = GetMap().find(name);
    return it != GetMap().end() ? it->second : *(Reflection::MetaData *)nullptr;
  }

  template <typename T>
  static constexpr void Register(const char * name) // register and setup meta.
  {
    using namespace Reflection;
    _Get<T>().name = name;
    _Get<T>().size = sizeof(T);
    Setup<T>::Serializers(_Get<T>());
    GetMap().emplace(KeyValue(name, _Get<T>()));
  }

  template <typename T, size_t size> // Register meta array
  static MetaInfo RegisterArray()
  {
    using namespace Reflection;
    assert(Meta::Get<T>().Valid()); // Array type must be registered first!
    char name[24];
    sprintf(name, "%s[%zu]", Meta::Get<T>().name, size);
    Meta::Register<T[size]>(name);
    return Meta::Get<T[size]>();
  }

  template <class T, typename M>
  static constexpr void Register(const char * name, M T::* member) // register meta members.
  {
    using namespace Reflection;
    Meta::_Get<T>().AddMember(new Member(name, uintptr_t(&((T*)nullptr->*member)), Meta::Get<M>()));
  }

  template <class T, typename M, size_t size>
  static void Register(const char * name, M (T::* member)[size]) // register meta member arrays.
  {
    using namespace Reflection;
    if (!Meta::Get<M[size]>().Valid()) // register array type if not registered.
      Meta::RegisterArray<M, size>();
    Meta::_Get<T>().AddMember(new Member(name, uintptr_t(&((T*)nullptr->*member)), Meta::Get<M[size]>()));
  }
};

class Variable
{
  friend void Serialization::GenericSerialize(std::ostream &, Variable);
  friend void Serialization::GenericDeserialize(std::istream &, Variable);

  Variable(void *data, const Reflection::MetaData *meta) : data(data), meta(meta) {}

  void *data;
  const Reflection::MetaData *meta;
public:
  
  template <typename T>
  Variable(const T &val) : data(const_cast< T*>(&val)), meta(&Meta::Get<T>()) {}
  template <typename T, size_t size>
  Variable(const T (&val)[size]) 
    : data(const_cast< T*>(&val[0])), 
      meta(Meta::Get<T[size]>().Valid() ? &Meta::Get<T[size]>() : &Meta::RegisterArray<T, size>()) {}
  Variable (const Variable &var) : data(var.data), meta(var.meta) {}
  void Serialize(std::ostream &os) { if (meta->_serialize) meta->_serialize(os, *this); }
  void Deserialize(std::istream &is) { if (meta->_deserialize) meta->_deserialize(is, *this); }
  MetaInfo Type() const { return *meta; }
  template <typename T>
  T & Value()
  {
    assert(Meta::Get<T>() == *meta); // run time type-checking. can be ignored using macros.
    return (*reinterpret_cast<T *>(data));
  }
};

std::ostream & operator<<(std::ostream &os, Variable var)
{
  var.Serialize(os);
  return os;
}

std::istream & operator >> (std::istream &is, Variable var)
{
  var.Deserialize(is);
  return is;
}

/***** Serialization Backend *****/
namespace Serialization
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
    while (ret.empty() ||(temp != token && temp != '\n'))
    {
      if (temp != token && temp != '\n') ret += temp;
      temp = is.get();
    }
    return ret;
  }

  void *MemberPtr(void *data, size_t offset) { return ((char *)data) + offset; }

  void GenericSerialize(std::ostream &os, Variable var)
  {
    using namespace std;
    os << var.meta->name << endl;
    Padding::Add(os) << "{" << endl;
    Padding::Increase();
    const Reflection::Member * mem = var.meta->_members;
    while (mem)
    {
      Padding::Add(os) << mem->name << " = " 
        << Variable(MemberPtr(var.data, mem->offset), &mem->meta) << endl;
      mem = mem->next;
    }
    Padding::Decrease();
    Padding::Add(os);
    os << "}" << endl;
  }

  void GenericDeserialize(std::istream &is, Variable var)
  {
    std::string name = ReadToken(is);
    assert(name == var.Type().name);
    name = ReadToken(is); // {
    name = ReadToken(is); // first mem
    
    while (name != "}")
    {
      if (is.eof()) break;
      MemberInfo mem = var.Type().FindMember(name);
      if (mem.Valid())
      {
        name = ReadToken(is); // = 
        Variable(MemberPtr(var.data, mem.offset), &mem.meta).Deserialize(is);
      }
      name = ReadToken(is); // next member
    }
  }

  template <typename T>
  void Serialize(std::ostream &os, Variable var) { os << var.Value<T>(); }

  template <typename T, size_t size>
  void SerializeArray(std::ostream &os, Variable var)
  {
    os << "[";
    for (size_t i = 0; i < size; ++i)
       os << " " << Variable(var.Value<T[size]>()[i]);
    os << " ]";
  }

  template <>
  void Serialize<bool>(std::ostream &os, Variable var) { os << (var.Value<bool>() ? "true" : "false"); }

  template <>
  void Serialize<char *>(std::ostream &os, Variable var) { os << '"' << var.Value<char *>() << '"'; }

  template <> 
  void Serialize<std::string>(std::ostream &os, Variable var) { os << '"' << var.Value<std::string>().c_str() << '"'; }

  template <typename T>
  void Deserialize(std::istream &is, Variable var) { is >> var.Value<T>(); }

  template <typename T, size_t size>
  void DeserializeArray(std::istream &is, Variable var)
  {
    is.get(); // first [
    for (size_t i = 0; i < size; ++i)
      is >> Variable(var.Value<T[size]>()[i]);
  }

  template <>
  void Deserialize<bool>(std::istream &is, Variable var) { var.Value<bool>() = (ReadToken(is) == "true" ? true : false); }

  template <>
  void Deserialize<char *>(std::istream &is, Variable var) { /* Don't deserialize char pointer's */ }

  template <>
  void Deserialize<std::string>(std::istream &is, Variable var) 
  {
    is.get(); // first "
    var.Value<std::string>() = ReadToken(is, '"');
  }
}