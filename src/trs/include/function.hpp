#ifndef _TRS_FUNCTION_HPP
#define _TRS_FUNCTION_HPP

namespace _trs_backend
{
  template <class R, class ... ArgTypes>
  void StaticCall(VarPtr &ret, void *self, void *fn, VarPtr *args, size_t num_args)
  {
    assert(num_args-- == sizeof ...(ArgTypes));
    ret.value<R>() = reinterpret_cast<R(*)(ArgTypes...)>(fn)(args[num_args--].value<ArgTypes>()...);
  }

  template <class ... ArgTypes>
  void VoidStaticCall(VarPtr &ret, void *self, void *fn, VarPtr *args, size_t num_args)
  {
    assert(num_args-- == sizeof ...(ArgTypes));
    reinterpret_cast<void(*)(ArgTypes...)>(fn)(args[num_args--].value<ArgTypes>()...);
  }

  template <class R, class C, class ... ArgTypes>
  void MemberCall(VarPtr &ret, void *self, void *fn, VarPtr *args, size_t num_args)
  {
    assert(num_args-- == sizeof ...(ArgTypes));
    assert(self != nullptr);
    ret.value<R>() = (((C*)self)->*reinterpret_cast<R(C::*&)(ArgTypes...)>(fn))(args[num_args--].value<ArgTypes>()...);
  }

  template <class C, class ... ArgTypes>
  void VoidMemberCall(VarPtr &ret, void *self, void *fn, VarPtr *args, size_t num_args)
  {
    assert(num_args-- == sizeof ...(ArgTypes));
    assert(self != nullptr);
    (((C*)self)->*reinterpret_cast<void(C::*&)(ArgTypes...)>(fn))(args[num_args--].value<ArgTypes>()...);
  }

  template <class ... ArgTypes>
  constexpr MetaInfo GetArg(size_t index)
  {
    using namespace _trs_backend;
    return *(std::array<const MetaData *, sizeof ... (ArgTypes)+1> { &get_type<ArgTypes>()..., nullptr })[index];
  }

  class Function
  {
    template <typename T> friend class Builder;
    template <class R, class ... ArgTypes>
    friend constexpr void trs::reflect(const char *name, R(*fn)(ArgTypes ... args));
    template <class ... ArgTypes>
    friend constexpr void trs::reflect(const char *name, void(*fn)(ArgTypes ... args));

    Function(const char * name, void *fn, CallFn call, GetArgFn arg, MetaInfo context, MetaInfo ret, size_t size)
      : name(name), _fnptr(fn), call(call), arg(arg), returnType(ret), context(context), numArgs(size) {}

    void * _fnptr;

  public:

    // Wrapper call
    void operator()(VarPtr &ret, void * context, std::vector<VarPtr> args) const
    {
      call(ret, context, _fnptr, args.data(), args.size());
    }

    // Explict call
    void operator()(VarPtr &ret, void * context, VarPtr *arg_array, size_t size) const
    {
      call(ret, context, _fnptr, arg_array, size);
    }

    // Arg call
    template <typename ... ArgTypes>
    void operator()(VarPtr &ret, void * context, ArgTypes ... args) const
    {
      VarPtr arg[] = { args ... };
      call(ret, context, _fnptr, arg, sizeof ... (ArgTypes));
    }

    //0 argument call
    void operator()(VarPtr &ret, void *context = nullptr) const
    {
      call(ret, context, _fnptr, nullptr, 0);
    }

    bool valid() const { return this != nullptr && _fnptr != nullptr; }

    const char * name;
    CallFn call;
    GetArgFn arg;
    MetaInfo context;
    MetaInfo returnType;
    const size_t numArgs;
  };
}

#endif