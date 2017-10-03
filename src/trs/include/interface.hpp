#ifndef _TRS_INTERFACE_HPP
#define _TRS_INTERFACE_HPP

namespace _trs_backend
{
  template <typename T>
  class Builder
  {
  public:
    _trs_backend::MetaData & meta;
    Builder(const char *name) : meta(const_cast<_trs_backend::MetaData &>(get_type<T>()))
    {
      using namespace _trs_backend;
      meta.name = name;
      meta.size = sizeof(T);
      Serializers<T>();
      get_type_map().add(name, meta);
    }
  public:
    template <class ... Args>
    _trs_backend::Builder<T> &constructor() { return *this; }

    template <typename C, typename M, std::enable_if_t<std::is_same<T, C>::value && !std::is_function<M>::value>* = nullptr>
    _trs_backend::Builder<T> &property(const char *name, M C::*property)
    {
      using namespace _trs_backend;
      meta.add_property(name, new Property(name, uintptr_t(&((C*)nullptr->*property)), get_type<M>()));
      return *this;
    }

    template <class R, class C, class ... ArgTypes, std::enable_if_t<std::is_same<C, T>::value>* = nullptr>
    _trs_backend::Builder<T> &function(const char *name, R(C::*fn)(ArgTypes ... args))
    {
      using namespace _trs_backend;
      meta.add_function(name, new Function(name, (void*&)fn, MemberCall<R, C, ArgTypes ...>, GetArg<ArgTypes ...>, get_type<C>(), get_type<R>(), sizeof ... (ArgTypes)));
      return *this;
    }

    template <class C, class ... ArgTypes, std::enable_if_t<std::is_same<C, T>::value>* = nullptr>
    _trs_backend::Builder<T> &function(const char *name, void(C::*fn)(ArgTypes ... args))
    {
      using namespace _trs_backend;
      meta.add_function(name, new Function(name, (void*&)fn, VoidMemberCall<C, ArgTypes ...>, GetArg<ArgTypes ...>, get_type<C>(), (*(MetaData *)nullptr), sizeof ...(ArgTypes)));
      return *this;
    }

  };
}

/***** Reflection Interface *****/

/** Meta interface class.
Used to register, get, and modify metainfo instances
*/
namespace trs
{
  /** Private map singleton.
  Stores type name and metainfo instance as a key value pair.

  \return
  Reference to the map.
  */
  static _trs_backend::TypeMap &get_type_map() // hidden map for string hashing.
  {
    static _trs_backend::TypeMap map;
    return map;
  }

  /** Private map singleton.
  Stores function name and meta info as a keyvalue pair.

  \return
  Reference to the map.
  */
  static _trs_backend::FuncMap &get_func_map()
  {
    static _trs_backend::FuncMap map;
    return map;
  }

  /** Gets MetaData info for type T
  Templatized Getter for type information.

  \return
  Returns a meta instance of type T, only valid if registered.
  */
  template <typename T>
  static MetaInfo get_type()
  {
    static _trs_backend::MetaData meta;
    return meta;
  }

  /** Gets meta information for a function
  
  \return
  Returns a FunctionInfo of the specified name

  */
  FunctionInfo get_func(const char *name) { return get_func_map().find(name); }

  FunctionInfo get_func(std::string &name) { return get_func_map().find(name); }

  /** Gets MetaData info by string
  Hashs map by registered string identifier for type.

  \param name
  type name identifier to search for

  \return
  A valid MetaInfo if registered, invalid MetaInfo if not.
  */
  MetaInfo get_type(std::string &name) { return get_type_map().find(name); }

  /** Gets MetaData info by c-string.
  Hashs map by registered string identifier for type.

  \param name
  type name identifier to search for

  \return
  A valid MetaInfo if registered, invalid MetaInfo if not.
  */
  MetaInfo get_type(const char *name) { return get_type_map().find(name); }

  /** Registration for type T.
  Registers Type T to the reflection & serialization systems.

  \param name
  unique name to use for string searching type T meta info.

  */
  template <typename T>
  constexpr _trs_backend::Builder<T> reflect(const char * name) { return _trs_backend::Builder<T>(name); }

  template <class R, class ... ArgTypes>
  constexpr void reflect(const char *name, R(*fn)(ArgTypes ... args))
  {
    using namespace _trs_backend;
    get_func_map().add(
      name,
      *new Function(
        name,
        fn,
        StaticCall<R, ArgTypes ...>,
        GetArg<ArgTypes ...>,
        (*(MetaData *)nullptr),
        get_type<R>(),
        sizeof ... (ArgTypes)
      )
    );
  }

  template <class ... ArgTypes>
  constexpr void reflect(const char *name, void(*fn)(ArgTypes ... args))
  {
    using namespace _trs_backend;
    get_func_map().add(
      name,
      *new Function(
        name,
        fn,
        VoidStaticCall<ArgTypes ...>,
        GetArg<ArgTypes ...>,
        (*(MetaData *)nullptr),
        (*(MetaData *)nullptr),
        sizeof ...(ArgTypes)
      )
    );
  }

  template <typename T>
  constexpr void set_serializer(_trs_backend::SerializeFN fn)
  {
    const_cast<_trs_backend::MetaData &> (get_type<T>()).serialize = fn;
  }

  template <typename T>
  constexpr void set_deserializer(_trs_backend::DeserializeFN fn)
  {
    const_cast<_trs_backend::MetaData &>(get_type<T>()).deserialize = fn;
  }

  /** Generic VarPtr class.
  Used to store data generically.
  */
  class VarPtr
  {
    void *_data;
    const _trs_backend::MetaData *_meta;
  public:

    /** Default constructor
    Creates an invalid VarPtr
    */
    VarPtr() : _data(nullptr), _meta(nullptr) {}

    /** Manual setup constructor.
    Used internally by serialization.

    \param data
    pointer to data variable represents.

    \param meta
    pointer to the meta type information representing the data.
    */
    VarPtr(void *data, const _trs_backend::MetaData *meta) : _data(data), _meta(meta) {}

    /** Templatized copy constructor
    Stores val of type T generically with its associated meta.

    \param val
    Reference to data to store.
    */
    template <typename T, std::enable_if_t<!std::is_pointer<T>::value>* = nullptr>
    VarPtr(const T &val)
      : _data(const_cast<T*>(&val)),
      _meta(&trs::get_type<T>())
    {
      assert(_meta->valid());
    } // meta must be registered!

      /** Templatized copy constructor
      Stores val of type T generically with its associated meta.

      \param val
      Reference to data to store.
      */
    template <typename T>
    VarPtr(const T*val) : _data(const_cast<T *>(val)), _meta(&get_type<T>()) { assert(_meta->valid()); }

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
    VarPtr(const char *val) : _data(const_cast<char *>(val)), _meta(&get_type<const char *>()) { assert(_meta->valid()); }

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
    std::ostream & serialize(std::ostream &os) { if (_meta->serialize) _meta->serialize(os, *this); return os; }

    /** Deserialization
    Attempts to deserialize the given file data into the object.

    \param is
    istream to input from.
    */
    void deserialize(std::istream &is) { if (_meta->deserialize) _meta->deserialize(is, *this); }

    /** Validity Check
    Checks if the VarPtr contains valid data & meta information

    \return
    True if valid, false if not.
    */
    bool valid() const { return _data && _meta && _meta->valid(); }

    /** Data cast.
    Cast data into type T, asserts if T and stored meta don't match.
    */
    MetaInfo type() const { return *_meta; }
    void * data() const { return _data; }

    template < typename T, std::enable_if_t<!std::is_pointer<T>::value && !std::is_same<VarPtr, T>::value>* = nullptr>
    std::remove_reference_t<T> & value()
    {
      assert(get_type<std::remove_const_t<std::remove_reference_t<T>>>() == *_meta); // run time type-checking.
      return (*reinterpret_cast<std::remove_reference_t<T> *>(_data));
    }
    template <typename T, std::enable_if_t<std::is_pointer<T>::value && !std::is_same<VarPtr, T>::value>* = nullptr>
    _trs_backend::PointerProxy<T> value()
    {
      assert(get_type<std::remove_const_t<std::remove_pointer_t<T>>>() == *_meta ||
        get_type<T>() == *_meta); // run time type-checking, ignores const and pointer types.
      return _data;
    }

    template <typename T, std::enable_if_t<std::is_same<VarPtr, T>::value>* = nullptr>
    T value() { return *this; }
  };
}
#endif