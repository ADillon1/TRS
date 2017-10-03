#ifndef _TRS_PROPERTY_HPP
#define _TRS_PROPERTY_HPP

namespace _trs_backend
{
  /** Meta class for class Propertys.
  Stores information on abstract type elements.
  */
  class Property
  {
    friend class MetaData;
    template <typename T> friend class Builder;

    /** Constructor

    \param name
    Identifier of the class Property.

    \param  offset
    Offset in bytes from the class pointer.

    \param meta
    Meta information of the Property's type.
    */
    Property(const char * name, size_t offset, MetaInfo meta)
      : name(name), offset(offset), meta(meta) {}

    Property() = delete;
    Property(const MetaData &) = delete;
    Property & operator=(const MetaData &) = delete;

  public:

    /** Validity Check
    Tests whether the meta has been registered.

    \return
    True if registered, false if not registered.
    */
    bool valid() const { return this != nullptr; }

    const char *name;
    size_t offset;
    MetaInfo meta;
  };
}

#endif