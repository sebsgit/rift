#pragma once

#include "rift/lightweight/function_traits.hpp"
#include "rift/lightweight/utils.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace rift {
    //TODO: fix with reference types
    class GenericArgument {
    public:
        template <typename T = int>
        GenericArgument(const T& t = T{})
            : _ref(&t)
        {
        }
        template <typename T>
        operator const T() const noexcept
        {
            return _ref ? *reinterpret_cast<const T*>(_ref) : T{};
        }

    private:
        const void* _ref;
    };

    struct IgnoreThis {
    };

    class ReturnArgument {
    public:
        template <typename T>
        explicit ReturnArgument(T& t)
            : _ref(&t)
        {
        }
        template <typename T>
        auto operator=(const T& value) const
        {
            if (this->_ref)
                *reinterpret_cast<T*>(const_cast<ReturnArgument*>(this)->_ref) = value;
            return *this;
        }
        template <>
        auto operator=<IgnoreThis>(const IgnoreThis&) const
        {
            return *this;
        }

    protected:
        ReturnArgument()
            : _ref(nullptr)
        {
        }

    private:
        void* _ref;
    };

    class IgnoreReturnValue : public ReturnArgument {
    };

    template <bool isResultVoid, size_t argumentCount>
    struct InvokeHelper {
    public:
        template <typename Obj, typename Func, typename... Args>
        static decltype(auto) invokeIt(Obj&& obj, Func f, Args... args)
        {
            return rift::apply<argumentCount>([&](auto&&... p) { return (obj.*f)(p...); }, std::forward<Args>(args)...);
        }
    };

    template <size_t argumentCount>
    struct InvokeHelper<true, argumentCount> {
    public:
        template <typename Obj, typename Func, typename... Args>
        static auto invokeIt(Obj&& obj, Func f, Args&&... args)
        {
            rift::apply<argumentCount>([&](auto&&... p) { return (obj.*f)(p...); }, std::forward<Args>(args)...);
            return IgnoreThis();
        }
    };

    template <int argumentCount, typename RetType>
    struct Invoker2 {
        template <typename Obj, typename Func, typename... Args>
        static decltype(auto) invokeIt(Obj&& obj, Func f, Args&&... args)
        {
            return InvokeHelper<std::is_same<RetType, void>::value, argumentCount>::invokeIt(std::forward<Obj>(obj), std::forward<Func>(f), std::forward<Args>(args)...);
        }
    };

    template <typename RetType>
    struct Invoker2<0, RetType> {
        template <typename Obj, typename Func, typename... Args>
        static decltype(auto) invokeIt(Obj&& obj, Func f, Args&&...)
        {
            return InvokeHelper<std::is_same<RetType, void>::value, 0>::invokeIt(std::forward<Obj>(obj), std::forward<Func>(f));
        }
    };

    template <typename T>
    class AbstractFunctionInfo {
    public:
        explicit AbstractFunctionInfo(const std::string& name)
            : _name(name)
        {
        }
        const std::string& name() const noexcept { return this->_name; }
        virtual AbstractFunctionInfo<T>* clone() const = 0;
        virtual void invoke(T& object, const rift::ReturnArgument& ret = rift::IgnoreReturnValue(),
            rift::GenericArgument arg0 = rift::GenericArgument(),
            rift::GenericArgument arg1 = rift::GenericArgument(),
            rift::GenericArgument arg2 = rift::GenericArgument())
            = 0;

        virtual void invoke(T& object,
            rift::GenericArgument arg0,
            rift::GenericArgument arg1 = rift::GenericArgument(),
            rift::GenericArgument arg2 = rift::GenericArgument())
            = 0;

    private:
        const std::string _name;
    };

    template <typename T, typename Func>
    class FunctionInfo : public AbstractFunctionInfo<T> {
    public:
        template <typename F>
        FunctionInfo(const std::string& name, F&& f)
            : AbstractFunctionInfo<T>(name)
            , _func(std::forward<F>(f))
        {
        }
        AbstractFunctionInfo<T>* clone() const override
        {
            return new FunctionInfo<T, Func>(this->name(), _func);
        }
        static constexpr int argumentCount() noexcept
        {
            return rift::FunctionTraits<Func>::argumentCount;
        }
        void invoke(T& object, const rift::ReturnArgument& ret = rift::IgnoreReturnValue(),
            rift::GenericArgument arg0 = rift::GenericArgument(),
            rift::GenericArgument arg1 = rift::GenericArgument(),
            rift::GenericArgument arg2 = rift::GenericArgument()) override
        {
            ret = rift::Invoker2<argumentCount(), typename rift::FunctionTraits<Func>::ResultType>::invokeIt(object, _func, arg0, arg1, arg2);
        }

        void invoke(T& object,
            rift::GenericArgument arg0,
            rift::GenericArgument arg1 = rift::GenericArgument(),
            rift::GenericArgument arg2 = rift::GenericArgument()) override
        {
            rift::Invoker2<argumentCount(), typename rift::FunctionTraits<Func>::ResultType>::invokeIt(object, _func, arg0, arg1, arg2);
        }

    private:
        Func _func;
    };

    class NoSuchMethod : public std::runtime_error {
    public:
        explicit NoSuchMethod(const std::string& name)
            : std::runtime_error("ERROR: Cannot locate reflection info about this function: " + name)
        {
        }
    };

    template <typename T>
    class Reflect {
    public:
        template <typename... FuncList>
        explicit Reflect(const FunctionInfo<T, FuncList>&... rest)
        {
            this->insert(rest...);
        }

        //TODO: support static functions, no return parameter, const objects!
        void invoke(T& object, const std::string& riftFnName,
            const rift::ReturnArgument& ret = rift::IgnoreReturnValue(),
            rift::GenericArgument arg0 = rift::GenericArgument(),
            rift::GenericArgument arg1 = rift::GenericArgument(),
            rift::GenericArgument arg2 = rift::GenericArgument())
        {
            auto it = this->_functions.find(riftFnName);
            if (it == this->_functions.end())
                throw NoSuchMethod(riftFnName);
            (*it).second->invoke(object, ret, arg0, arg1, arg2);
        }

        void invoke(T& object, const std::string& riftFnName,
            rift::GenericArgument arg0,
            rift::GenericArgument arg1 = rift::GenericArgument(),
            rift::GenericArgument arg2 = rift::GenericArgument())
        {
            auto it = this->_functions.find(riftFnName);
            if (it == this->_functions.end())
                throw NoSuchMethod(riftFnName);
            (*it).second->invoke(object, arg0, arg1, arg2);
        }

    private:
        template <typename Func>
        void insert(const FunctionInfo<T, Func>& info)
        {
            this->_functions[info.name()].reset(info.clone());
        }
        template <typename Func, typename... FuncList>
        void insert(const FunctionInfo<T, Func>& info, const FunctionInfo<T, FuncList>&... rest)
        {
            this->insert(info);
            this->insert(rest...);
        }

    private:
        std::unordered_map<std::string, std::unique_ptr<AbstractFunctionInfo<T>>> _functions;
    };
}


#define RIFT_IGNORE_RETURN() rift::IgnoreReturnValue()

#define RIFT_REFLECT(...)

#define RIFT_DECLARE_INVOKABLE(klassName, declarations) \
    static rift::Reflect<klassName>& reflection() \
    {\
        static rift::Reflect<klassName> data{\
            declarations\
        };\
        return data;\
    }
#define RIFT_INVOKABLE(func) rift::FunctionInfo<typename rift::FunctionTraits<decltype(&func)>::ClassType, decltype(&func)> { #func, &func },

