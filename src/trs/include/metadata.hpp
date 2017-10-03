#ifndef _TRS_METADATA_HPP
#define _TRS_METADATA_HPP

namespace _trs_backend
{
  class MetaData
  {
    template <typename T> friend MetaInfo trs::get_type<T>();
    template <typename T> friend class Builder;

    /** Private default constructor
    Defaults to unregistered type.
    */
    MetaData() : properties(), functions(), serialize(nullptr),
      deserialize(nullptr), create(nullptr), shallowCopy(nullptr),
      deepCopy(nullptr), destroy(nullptr), name("Unregistered"), size(0) {}

    MetaData(const MetaData &) = delete;
    MetaData & operator=(const MetaData &) = delete;

    /** Adds a Property to the class's Property list.
    Used internally.

    \param Property
    Property node to add to the class's Property list.

    */
    void add_property(const char *name, Property * property) { properties.add(name, *property); }

    /** Adds a Function to the class's Function list.
    Used internally.

    \param function
    function node to add to the class's function list.

    */
    void add_function(const char * name, Function * function) { functions.add(name, *function); }

  public:

    /** Destructor
    */
    ~MetaData()
    {
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
    bool valid() const { return this != nullptr && size != 0; }

    const char * name;
    size_t size;

    NewFn create;
    CopyFn shallowCopy;
    CopyFn deepCopy;
    DeleteFn destroy;

    Map<PropertyInfo> properties;
    Map<FunctionInfo> functions;

    SerializeFN serialize;
    DeserializeFN deserialize;
  };
}

#endif