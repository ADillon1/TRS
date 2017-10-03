#ifndef _TRS_UTILITIES_HPP
#define _TRS_UTILITIES_HPP

namespace trs 
{ 
  class VarPtr; 
}

/***** Reflection Backend Forward Declarations *****/
namespace _trs_backend
{
  class MetaData;
  class Property;
  class Function;
  template <typename T> struct PointerProxy;
  template <typename T> class Builder;
}

/***** Info Types for User *****/
typedef const _trs_backend::MetaData & MetaInfo;
typedef const _trs_backend::Property & PropertyInfo;
typedef const _trs_backend::Function & FunctionInfo;

/***** interface forward declaration *****/
namespace trs
{
  template <typename T> MetaInfo get_type();
  template <class R, class ... ArgTypes>
  constexpr void trs::reflect(const char *name, R(*fn)(ArgTypes ... args));
  template <class ... ArgTypes>
  constexpr void reflect(const char *name, void(*fn)(ArgTypes ... args));
}

namespace _trs_backend
{
  using namespace trs;

  /****** Function Prototypes *****/
  void GenericSerialize(std::ostream &, VarPtr);
  void GenericDeserialize(std::istream &, VarPtr);
  template <typename T> void Serialize(std::ostream &, VarPtr);
  template <typename T> void Deserialize(std::istream &, VarPtr);
  template <> void Serialize<std::string>(std::ostream &, VarPtr);
  template <> void Deserialize<std::string>(std::istream &, VarPtr);

  /***** Reflection Operation Definitions *****/
  typedef void *(*NewFn)(void);
  typedef void *(*CopyFn)(void *, void *);
  typedef void(*DeleteFn)(void *);

  /***** Serialization Definitions *****/
  typedef void(*SerializeFN)(std::ostream &, VarPtr);
  typedef void(*DeserializeFN)(std::istream &, VarPtr);

  /*****Function typedefs *****/
  typedef void(*const CallFn)(VarPtr &ret, void * self, void *fn, VarPtr *args, size_t num_args);
  typedef MetaInfo(*const GetArgFn)(size_t index);

  /***** Function Helper forward declares *****/
  template <class R, class ... ArgTypes>
  void StaticCall(VarPtr &ret, void *self, void *fn, VarPtr *args, size_t num_args);
  template <class ... ArgTypes>
  void VoidStaticCall(VarPtr &ret, void *self, void *fn, VarPtr *args, size_t num_args);
  template <class R, class C, class ... ArgTypes>
  void MemberCall(VarPtr &ret, void *self, void *fn, VarPtr *args, size_t num_args);
  template <class C, class ... ArgTypes>
  void VoidMemberCall(VarPtr &ret, void *self, void *fn, VarPtr *args, size_t num_args);
  template <class ... ArgTypes>
  constexpr MetaInfo GetArg(size_t index);

  /***** Interface forward declares *****/

  /** unordered_map wrapper
  */
  template <typename T>
  class Map
  {
    template <typename t> friend class Builder;
    template <class R, class ... ArgTypes>
    friend constexpr void trs::reflect(const char *name, R(*fn)(ArgTypes ... args));
    template <class ... ArgTypes>
    friend constexpr void trs::reflect(const char *name, void(*fn)(ArgTypes ... args));
    friend MetaData;
    std::unordered_map<std::string, T> _map;

    void add(const char * key, T val) { _map.emplace(std::pair<std::string, T>(key, val)); }

  public:
    T find(std::string &key) const
    {
      auto it = _map.find(key);
      if (it == _map.cend())
        return *(std::remove_reference_t<T>*)nullptr;
      return it->second;
    }

    T find(const char *key) const { return find(std::string(key)); }

    auto begin() const { return _map.begin(); }
    auto end() const { return _map.end(); }
  };

  template <typename T>
  struct PointerProxy
  {
    void *&_data;

    PointerProxy(void *&data) : _data(data) {}

    PointerProxy(const PointerProxy &r) : _data(r._data) {}
    void operator=(T rhs) { _data = rhs; }
    operator T() const { return (T)(_data); }
  };

  /** Auto POD serializer setup for registration.
  */
  template <class T, std::enable_if_t<!std::is_class<T>::value || std::is_same<std::string, T>::value>* = nullptr>
  constexpr void Serializers()
  {
    set_serializer<T>(_trs_backend::Serialize<T>);
    set_deserializer<T>(_trs_backend::Deserialize<T>);
  }

  /** Auto abstract serializer setup for registration.
  */
  template <class T, std::enable_if_t<std::is_class<T>::value && !std::is_same<std::string, T>::value>* = nullptr>
  constexpr void Serializers()
  {
    set_serializer<T>(_trs_backend::GenericSerialize);
    set_deserializer<T>(_trs_backend::GenericDeserialize);
  }

  /***** Map Type definition *****/
  typedef Map<MetaInfo> TypeMap;
  typedef Map<FunctionInfo> FuncMap;
  typedef Map<PropertyInfo> PropertyMap;
}

#endif