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
#include <array>         // array

/***** Reflection Backend Forward Declarations *****/
namespace Reflection { class MetaData;  class Member; }

/***** Reflection Interface Forward Declarations *****/
namespace Trs { class Meta;  class VarPtr; }

/***** Serialization Forward Declarations *****/
namespace Serialization
{
  using namespace Trs;

  /****** Function Prototypes *****/
  void GenericSerialize(std::ostream &, VarPtr);
  void GenericDeserialize(std::istream &, VarPtr);
  template <typename T> void Serialize(std::ostream &, VarPtr);
  template <typename T> void Deserialize(std::istream &, VarPtr);
  template <> void Serialize<std::string>(std::ostream &, VarPtr);
  template <> void Deserialize<std::string>(std::istream &, VarPtr);
}

/***** Serialization Declarations *****/
typedef void(*SerializeFN)(std::ostream &, Trs::VarPtr);
typedef void(*DeserializeFN)(std::istream &, Trs::VarPtr);

/***** Info Types for User *****/
typedef const Reflection::MetaData & MetaInfo;
typedef const Reflection::Member & MemberInfo;

/*****Function typedefs *****/
typedef void(* const CallFn)(Trs::VarPtr ret, void * self, void *fn, Trs::VarPtr *args, size_t num_args);
typedef MetaInfo ( * const GetArgFn)(size_t index);

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
    friend class Trs::Meta;
    friend class MetaData;
    friend void Serialization::GenericSerialize(std::ostream &, Trs::VarPtr);

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
    friend class Trs::Meta;
    friend void Serialization::GenericSerialize(std::ostream &, Trs::VarPtr);
    friend class Trs::VarPtr;

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

  template <class T, std::enable_if_t<!std::is_class<T>::value || std::is_same<std::string, T>::value>* = nullptr>
  constexpr void Serializers()
  {
    Meta::SetSerializer<T>(Serialization::Serialize<T>);
    Meta::SetDeserializer<T>(Serialization::Deserialize<T>);
  }
  
  template <class T, std::enable_if_t<std::is_class<T>::value && !std::is_same<std::string, T>::value>* = nullptr>
  constexpr void Serializers()
  {
    Meta::SetSerializer<T>(Serialization::GenericSerialize);
    Meta::SetDeserializer<T>(Serialization::GenericDeserialize);
  }
}

/***** Reflection Interface *****/
namespace Trs
{
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
      Serializers<T>();
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
}

namespace Reflection
{
  /** POD struct
  Used to auto Register POD types for the user at compile time.
  */
  template <typename T>
  struct POD { POD(const char * name) { Meta::Register<T>(name); } };

  /***** Auto Registered Types *****/
  Reflection::POD<char> c("char");
  Reflection::POD<const char *> cc("const char *");
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

namespace Trs
{
  /** Generic VarPtr class.
      Used to store data generically.
  */
  class VarPtr
  {
    friend void Serialization::GenericSerialize(std::ostream &, VarPtr);
    friend void Serialization::GenericDeserialize(std::istream &, VarPtr);

    /** Manual setup constructor.
        Used internally by serialization.

        \param data
          pointer to data variable represents.

        \param meta
          pointer to the meta type information representing the data.
    */
    VarPtr(void *data, const Reflection::MetaData *meta) : _data(data), _meta(meta) {}

    void *_data;
    const Reflection::MetaData *_meta;
  public:

    /** Default constructor
    Creates an invalid VarPtr
    */
    VarPtr() : _data(nullptr), _meta(nullptr) {}

    /** Templatized copy constructor
        Stores val of type T generically with its associated meta.

        \param val
          Reference to data to store.
    */
    template <typename T, std::enable_if_t<!std::is_pointer<T>::value>* = nullptr>
    VarPtr(const T &val) 
      : _data(const_cast<T*>(&val)), 
        _meta(&Meta::Get<T>()) 
    { assert(_meta->Valid()); } // meta must be registered!

    /** Templatized copy constructor
        Stores val of type T generically with its associated meta.

        \param val
          Reference to data to store.
    */
    template <typename T>
    VarPtr(const T*val) : _data(const_cast<T *>(val)), _meta(&Meta::Get<T>()) { assert(_meta->Valid()); }

    /** Templatized copy constructor
    Stores val of type T generically with its associated meta.

    \param val
    Reference to data to store.
    */
    VarPtr(const std::nullptr_t& val) : VarPtr() {}

    /** Templatized copy constructor
    Stores val of type T generically with its associated meta.

    \param val
    Reference to data to store.
    */
    VarPtr(const char *val) : _data(const_cast<char *>(val)), _meta(&Meta::Get<const char *>()) { assert(_meta->Valid()); }

    /** VarPtr copy constructor.
        Shallow copies data and meta data addresses.

        \param val
          VarPtr to copy.
    */
    VarPtr(const VarPtr &var) : _data(var._data), _meta(var._meta) {}

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
        Checks if the VarPtr contains valid data & meta information

        \return
          True if valid, false if not.
    */
    bool Valid() const { return _data && _meta && _meta->Valid(); }

    /** Data cast.
        Cast data into type T, asserts if T and stored meta don't match.
    */
    MetaInfo Type() const { return *_meta; }

    template < typename T, std::enable_if_t<!std::is_pointer<T>::value>* = nullptr>
    std::remove_reference_t<T> & Value()
    {
      assert(Meta::Get<std::remove_const_t<std::remove_reference_t<T>>>() == *_meta); // run time type-checking.
      return (*reinterpret_cast<std::remove_reference_t<T> *>(_data));
    }

    template <typename T, std::enable_if_t<std::is_pointer<T>::value>* = nullptr>
    T Value()
    {
      assert(Meta::Get<std::remove_const_t<std::remove_pointer_t<T>>>() == *_meta ||
             Meta::Get<T>() == *_meta); // run time type-checking, ignores const and pointer types.
      return reinterpret_cast<T>(_data);
    }

  };

  std::ostream & operator<<(std::ostream &os, VarPtr var)
  {
    var.Serialize(os);
    return os;
  }

  std::istream & operator >> (std::istream &is, VarPtr var)
  {
    var.Deserialize(is);
    return is;
  }
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

  void GenericSerialize(std::ostream &os, VarPtr var)
  {
    using namespace std;
    os << var._meta->name << endl;
    Padding::Add(os) << "{" << endl;
    Padding::Increase();
    const Reflection::Member * mem = var._meta->_members;
    while (mem)
    {
      Padding::Add(os) << mem->name << " = " 
        << VarPtr(MemberPtr(var._data, mem->offset), &mem->meta) << endl;
      mem = mem->_next;
    }
    Padding::Decrease();
    Padding::Add(os);
    os << "}" << endl;
  }

  void GenericDeserialize(std::istream &is, VarPtr var)
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
        VarPtr(MemberPtr(var._data, mem.offset), &mem.meta).Deserialize(is);
      }
      name = ReadToken(is); // next member
    }
  }

  template <typename T>
  void Serialize(std::ostream &os, VarPtr var) { os << var.Value<T>(); }

  template <>
  void Serialize<bool>(std::ostream &os, VarPtr var) { os << (var.Value<bool>() ? "true" : "false"); }

  template <>
  void Serialize<char *>(std::ostream &os, VarPtr var) { os << '"' << var.Value<char *>() << '"'; }

  template <>
  void Serialize<const char *>(std::ostream &os, VarPtr var) { os << '"' << var.Value<const char *>() << '"'; }

  template <> 
  void Serialize<std::string>(std::ostream &os, VarPtr var) { os << '"' << var.Value<std::string>().c_str() << '"'; }

  template <typename T>
  void Deserialize(std::istream &is, VarPtr var) { is >> var.Value<T>(); }

  template <>
  void Deserialize<bool>(std::istream &is, VarPtr var) { var.Value<bool>() = (ReadToken(is) == "true" ? true : false); }

  template <>
  void Deserialize<char *>(std::istream &is, VarPtr var) { /* Don't deserialize char pointer's. */ }

  template <>
  void Deserialize<const char *>(std::istream &is, VarPtr var) { /* Don't deserialize char pointer's. */}

  template <>
  void Deserialize<std::string>(std::istream &is, VarPtr var) 
  {
    is.get(); // first "
    var.Value<std::string>() = ReadToken(is, '"');
  }
}

/***** Function Reflection Backend *****/
namespace Reflection
{
  using namespace Trs;

  template <class R, class ... ArgTypes>
  void StaticCall(VarPtr ret, void *self, void *fn, VarPtr *args, size_t num_args)
  {
    assert(num_args-- == sizeof ...(ArgTypes));
    ret.Value<R>() = reinterpret_cast<R(*)(ArgTypes...)>(fn)(args[num_args--].Value<ArgTypes>()...);
  }

  template <class ... ArgTypes>
  void VoidStaticCall(VarPtr ret, void *self, void *fn, VarPtr *args, size_t num_args)
  {
    assert(num_args-- == sizeof ...(ArgTypes));
    reinterpret_cast<void(*)(ArgTypes...)>(fn)(args[num_args--].Value<ArgTypes>()...);
  }

  template <class R, class C, class ... ArgTypes>
  void MemberCall(VarPtr ret, void *self, void *fn, VarPtr *args, size_t num_args)
  {
    assert(num_args-- == sizeof ...(ArgTypes));
    assert(self != nullptr);
    ret.Value<R>() = (((C*)self)->*reinterpret_cast<R(C::*&)(ArgTypes...)>(fn))(args[num_args--].Value<ArgTypes>()...);
  }

  template <class C, class ... ArgTypes>
  void VoidMemberCall(VarPtr ret, void *self, void *fn, VarPtr *args, size_t num_args)
  {
    assert(num_args-- == sizeof ...(ArgTypes));
    assert(self != nullptr);
    (((C*)self)->*reinterpret_cast<void(C::*&)(ArgTypes...)>(fn))(args[num_args--].Value<ArgTypes>()...);
  }

  template <class ... ArgTypes>
  constexpr MetaInfo GetArg(size_t index)
  {
    return *(std::array<const Reflection::MetaData *, sizeof ... (ArgTypes)+1> { &Meta::Get<ArgTypes>()..., nullptr })[index];
  }
}

/***** Function Reflection Interface *****/
namespace Trs
{
  class Function
  {
    void * _fnptr;

  public:
    template <class R, class ... ArgTypes>
    Function(R(*fn)(ArgTypes ... args))
      : _fnptr(fn),
      Call(Reflection::StaticCall<R, ArgTypes...>),
      Arg(Reflection::GetArg<ArgTypes ...>),
      ReturnType(Meta::Get<R>()),
      NumArgs(sizeof ... (ArgTypes))
    {
    }

    template <class ... ArgTypes>
    Function(void(*fn)(ArgTypes ... args))
      : _fnptr(fn),
      Call(Reflection::VoidStaticCall<ArgTypes...>),
      Arg(Reflection::GetArg<ArgTypes ...>),
      ReturnType(nullptr),
      NumArgs(sizeof ... (ArgTypes))
    {
    }

    template <class R, class C, class ... ArgTypes>
    Function(R(C::*fn)(ArgTypes ... args))
      : _fnptr((void*&)fn),
      Call(Reflection::MemberCall<R, C, ArgTypes...>),
      Arg(Reflection::GetArg<ArgTypes ...>),
      ReturnType(Meta::Get<R>()),
      NumArgs(sizeof ... (ArgTypes))
    {
    }

    template <class C, class ... ArgTypes>
    Function(void(C::*fn)(ArgTypes ... args))
      : _fnptr((void*&)fn),
      Call(Reflection::VoidMemberCall<C, ArgTypes...>),
      Arg(Reflection::GetArg<ArgTypes ...>),
      ReturnType(nullptr),
      NumArgs(sizeof ... (ArgTypes))
    {
    }

    // Wrapper call
    void operator()(VarPtr ret, void * context, std::vector<VarPtr> args)
    {
      Call(ret, context, _fnptr, args.data(), args.size());
    }

    // Explict call
    void operator()(VarPtr ret, void * context, VarPtr *arg_array, size_t size)
    {
      Call(ret, context, _fnptr, arg_array, size);
    }

    // Arg call
    template <typename ... ArgTypes>
    void operator()(VarPtr ret, void * context, ArgTypes ... args)
    {
      VarPtr arg[] = { args ... };
      Call(ret, context, _fnptr, arg, sizeof ... (ArgTypes));
    }

    //0 argument call
    void operator()(VarPtr ret, void *context = nullptr)
    {
      Call(ret, context, _fnptr, nullptr, 0);
    }

    CallFn Call;
    GetArgFn Arg;
    MetaInfo ReturnType;
    const size_t NumArgs;
  };
}