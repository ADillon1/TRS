/*! Templatized Reflection & Seriaization.
    TRS is a stand alone type introspection and serialization system for C++11.

    \author Andrew Dillon
    \date   9/3/2017

    \par Licensed under MIT, see LICENSE for details.
*/
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

/***** Serialization Forward Declarations *****/
namespace Serialization
{
  /****** Function Prototypes *****/
  void GenericSerialize(std::ostream &, Variable);
  void GenericDeserialize(std::istream &, Variable);
  template <typename T> void Serialize(std::ostream &, Variable);
  template <typename T> void Deserialize(std::istream &, Variable);
  template <> void Serialize<std::string>(std::ostream &, Variable);
  template <> void Deserialize<std::string>(std::istream &, Variable);
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
  /***** Map Type definition *****/
  typedef std::unordered_map<std::string, MetaInfo> MetaMap;
  typedef std::pair<std::string, MetaInfo> KeyValue;

  /** Meta class for class members.
      Stores information on abstract type elements.
  */
  class Member
  {
    friend class ::Meta;
    friend class MetaData;
    friend void Serialization::GenericSerialize(std::ostream &, Variable);

    /** Constructor

        \param name 
          Identifier of the class member.

        \param  offset
          Offset in bytes from the class pointer.

        \param meta
          Meta information of the member's type.
    */
    Member(const char * name, size_t offset, MetaInfo meta) 
      : name(name), offset(offset), meta(meta) {}

    Member() = delete;
    Member(const MetaData &) = delete;
    Member & operator=(const MetaData &) = delete;

    const Member * _next;
  public:

    /** Validity Check
        Tests whether the meta has been registered.

        \return 
          True if registered, false if not registered.
    */
    bool Valid() const { return this != nullptr; }

    const char *name;
    size_t offset;
    MetaInfo meta;
  };

  class MetaData
  {
    friend class ::Meta;
    template <typename T, bool a = std::is_class<T>::value> friend struct Setup;
    friend void Serialization::GenericSerialize(std::ostream &, Variable);
    friend class ::Variable;

    /** Private default constructor
        Defaults to unregistered type.
    */
    MetaData() : _members(nullptr), name("Unregistered"), size(0){}

    MetaData(const MetaData &) = delete;
    MetaData & operator=(const MetaData &) = delete;

    /** Adds a Member to the class's member list.
        Used internally.
        
        \param member
          Member node to add to the class's member list.
    
    */
    void AddMember(Member * member)
    {
      member->_next = _members;
      _members = member;
    }

    const Member * _members;
    SerializeFN _serialize;
    DeserializeFN _deserialize;

  public:

    /** Destructor
        Deletes member data if needed.
    */
    ~MetaData() 
    {
      while (_members)
      {
        const Member *temp = _members;
        _members = _members->_next;
        delete temp;
      }
    }

    /** Equality Operator
        Returns whether the two metas are the same type.

        /param rhs
          Right hand side meta reference.
    */
    bool operator==(MetaInfo rhs) const { return this == &rhs; }

    /** Validity Check
        Tests whether the meta has been registered.

        \return
          True if registered, false if not registered.
    */
    bool Valid() const { return this != nullptr && size != 0; }

    /** Finds a member of the given name.
        O(N) search, where N is the number of members.

        \param str
          String name of the member to find.

        \return
          A valid member if successful, an invalid member if not found.
    */
    MemberInfo FindMember(const std::string &str) const
    { 
      return FindMember(str.c_str());
    }

    /** Finds a member of the given name.
        O(N) search, where N is the number of members.

        \param str
          C string name of the member to find.

        \return
          A valid member if successful, an invalid member if not found.
    */
    MemberInfo FindMember(const char * name) const
    {
      const Member *mem = _members;
      while (mem)
      {
        if (strcmp(mem->name, name) == 0)
          return *mem;
        mem = mem->_next;
      }

      return *mem;
    }

    const char * name;
    size_t size;
  };

  /** Templatized setup struct.
      Default serialization function ptr assignment for classes.
  */
  template <typename T, bool a>
  struct Setup
  {
    /** Sets up serialization pointers for the given metadata.
        
        \param meta
          Meta instance to setup.
    */
    static void Serializers(MetaData &meta)
    {
      meta._serialize = Serialization::GenericSerialize;
      meta._deserialize = Serialization::GenericDeserialize;
    }
  };


  /** Templatized setup struct.
      Specialization for POD types.
  */
  template <typename T>
  struct Setup<T, false> 
  {
    /** Sets up serialization pointers for the given metadata.

        \param meta
          Meta instance to setup.
    */
    static void Serializers(MetaData &meta)
    { 
      meta._serialize = Serialization::Serialize<T>;
      meta._deserialize = Serialization::Deserialize<T>;
    } 
  };

  /** Templatized setup struct.
      Specialization for std::string, which we treat as a POD type.
  */
  template <>
  struct Setup<std::string, true>
  {
    /** Sets up serialization pointers for the given metadata.

        \param meta
          Meta instance to setup.
    */
    static void Serializers(MetaData &meta)
    {
      meta._serialize = Serialization::Serialize<std::string>;
      meta._deserialize = Serialization::Deserialize<std::string>;
    }
  };
}

/***** Reflection Interface *****/

/** Meta interface class.
    Used to register, get, and modify metainfo instances
*/
class Meta
{
  /** Private map singleton.
      Stores type name and metainfo instance as a key value pair.

      \return
        Reference to the map.
  */
  static Reflection::MetaMap &GetMap() // hidden map for string hashing.
  {
    static Reflection::MetaMap map;
    return map;
  }

public:

  /** Gets MetaData info for type T
      Templatized Getter for type information.

      \return
        Returns a meta instance of type T, only valid if registered.
  */
  template <typename T>
  static MetaInfo Get() 
  {
    static Reflection::MetaData meta;
    return meta;
  }

  /** Gets MetaData info by string
      Hashs map by registered string identifier for type.

      \param name
        type name identifier to search for

      \return
        A valid MetaInfo if registered, invalid MetaInfo if not.
  */
  static MetaInfo Get(std::string &name) 
  { 
    auto it = GetMap().find(name);
    return it != GetMap().end() ? it->second : *(Reflection::MetaData *)nullptr;
  }

  /** Gets MetaData info by c-string.
      Hashs map by registered string identifier for type.

      \param name
        type name identifier to search for

      \return
        A valid MetaInfo if registered, invalid MetaInfo if not.
  */
  static MetaInfo Get(const char *name) { return Get(std::string(name)); }

  /** Registration for type T.
      Registers Type T to the reflection & serialization systems.

      \param name
        unique name to use for string searching type T meta info.
  
  */
  template <typename T>
  static constexpr void Register(const char * name)
  {
    using namespace Reflection;
    const_cast<MetaData &>(Get<T>()).name = name;
    const_cast<MetaData &>(Get<T>()).size = sizeof(T);
    Setup<T>::Serializers(const_cast<MetaData &>(Get<T>()));
    GetMap().emplace(KeyValue(name, const_cast<MetaData &>(Get<T>())));
  }

  /** Registration for type T's members.
      Registers members to the reflection & serialization system.

      \param name
        unique name to use for string searching type T meta info.

      \param member
        Class member to register as a reference.
  */
  template <class T, typename M>
  static constexpr void Register(const char * name, M T::* member)
  {
    using namespace Reflection;
    const_cast<MetaData &>(Get<T>()).AddMember(
      new Member(name, 
                 uintptr_t(&((T*)nullptr->*member)),
                 Meta::Get<M>()));
  }

  template <typename T>
  static constexpr void SetSerializer(SerializeFN fn) 
  { 
    const_cast<Reflection::MetaData &>(Get<T>())._serialize = fn;
  }

  template <typename T>
  static constexpr void SetDeserializer(DeserializeFN fn) 
  { 
    const_cast<Reflection::MetaData &>(Get<T>())._deserialize = fn; 
  }
};

namespace Reflection
{
  /** POD struct
  Used to auto Register POD types for the user at compile time.
  */
  template <typename T>
  struct POD { POD(const char * name) { Meta::Register<T>(name); } };

  /***** Auto Registered Types *****/
  Reflection::POD<char> c("char");
  Reflection::POD<unsigned char> uc("unsigned char");
  Reflection::POD<int> i("int");
  Reflection::POD<unsigned int> ui("unsigned int");
  Reflection::POD<short int> si("short");
  Reflection::POD<unsigned short int> usi("unsigned short");
  Reflection::POD<long int> li("long");
  Reflection::POD<unsigned long int> uli("unsigned long");
  Reflection::POD<float> fp("float");
  Reflection::POD<double> df("double");
  Reflection::POD<long double> ld("long double");
  Reflection::POD<bool> b("bool");
  Reflection::POD<std::string> str("string");
}

/** Generic variable class.
    Used to store data generically.
*/
class Variable
{
  friend void Serialization::GenericSerialize(std::ostream &, Variable);
  friend void Serialization::GenericDeserialize(std::istream &, Variable);

  /** Manual setup constructor.
      Used internally by serialization.

      \param data
        pointer to data variable represents.

      \param meta
        pointer to the meta type information representing the data.
  */
  Variable(void *data, const Reflection::MetaData *meta) : _data(data), _meta(meta) {}

  void *_data;
  const Reflection::MetaData *_meta;
public:

  /** Default constructor
  Creates an invalid Variable
  */
  Variable() : _data(nullptr), _meta(nullptr) {}
  
  /** Templatized copy constructor
      Stores val of type T generically with its associated meta.

      \param val
        Reference to data to store.
  */
  template <typename T>
  Variable(const T &val) : _data(const_cast<T*>(&val)), _meta(&Meta::Get<T>()) { assert(_meta->Valid()); } // meta must be registered!

  /** Variable copy constructor.
      Shallow copies data and meta data addresses.

      \param val
        variable to copy.
  */
  Variable (const Variable &var) : _data(var._data), _meta(var._meta) {}

  /** Serialization
      Attempts to serialize the object

      \param os
        ostream to output into.
  */
  void Serialize(std::ostream &os) { if (_meta->_serialize) _meta->_serialize(os, *this); }

  /** Deserialization
      Attempts to deserialize the given file data into the object.

  \param is
    istream to input from.
  */
  void Deserialize(std::istream &is) { if (_meta->_deserialize) _meta->_deserialize(is, *this); }

  /** Validity Check
      Checks if the variable contains valid data & meta information

      \return
        True if valid, false if not.
  */
  bool Valid() const { return _data && _meta && _meta->Valid(); }

  /** Data cast.
      Cast data into type T, asserts if T and stored meta don't match.
  */
  MetaInfo Type() const { return *_meta; }
  template <typename T>
  T & Value()
  {
    assert(Meta::Get<T>() == *_meta); // run time type-checking.
    return (*reinterpret_cast<T *>(_data));
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
    os << var._meta->name << endl;
    Padding::Add(os) << "{" << endl;
    Padding::Increase();
    const Reflection::Member * mem = var._meta->_members;
    while (mem)
    {
      Padding::Add(os) << mem->name << " = " 
        << Variable(MemberPtr(var._data, mem->offset), &mem->meta) << endl;
      mem = mem->_next;
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
        Variable(MemberPtr(var._data, mem.offset), &mem.meta).Deserialize(is);
      }
      name = ReadToken(is); // next member
    }
  }

  template <typename T>
  void Serialize(std::ostream &os, Variable var) { os << var.Value<T>(); }

  template <>
  void Serialize<bool>(std::ostream &os, Variable var) { os << (var.Value<bool>() ? "true" : "false"); }

  template <>
  void Serialize<char *>(std::ostream &os, Variable var) { os << '"' << var.Value<char *>() << '"'; }

  template <> 
  void Serialize<std::string>(std::ostream &os, Variable var) { os << '"' << var.Value<std::string>().c_str() << '"'; }

  template <typename T>
  void Deserialize(std::istream &is, Variable var) { is >> var.Value<T>(); }

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