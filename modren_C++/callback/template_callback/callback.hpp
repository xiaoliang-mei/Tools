/**
 * @file callback.hpp
 * @author xiaoliang.mei
 * @date 2023/6/16
 * 
 * @brief 实现统一回调入口
 * 
*/
#pragma once
#include <tuple>
#include <memory>
#include <functional>

/**
 * @name Callback
 * @brief 统一回调抽象基类
 * 
*/
struct Callback {
    using Ptr = std::shared_ptr<Callback>;
    virtual void call() = 0;
};

/**
 * @name FunctionCallback(N)
 * @brief 普通函数回调类，带N后缀的表示无参函数版本
 * @param Ret 返回值类型
 * @param Args 函数参数类型列表
 * 
*/
template <typename Ret, typename... Args>
class FunctionCallback : public Callback {
public:
    using Function = std::function<Ret(Args...)>;
    
    explicit FunctionCallback(Function fn, Args... args)
        : m_fn(std::move(fn)), m_args(std::forward<Args>(args)...) {}
    
    void call() override { std::apply(m_fn, m_args); }
    
private:
    Function m_fn;
    std::tuple<Args...> m_args;
};
template <typename Ret, typename... Args>
class FunctionCallbackN : public Callback {
public:
    using Function = std::function<Ret()>;
    
    explicit FunctionCallbackN(Function fn)
        : m_fn(std::move(fn)) {}
    
    void call() override { m_fn(); }
    
private:
    Function m_fn;
};

/**
 * @name MemberCallback(N)
 * @brief 成员函数回调类
 * @param Class 类名
 * @param Ret 返回值类型
 * @param Args 函数参数类型列表
 * 
*/
template <typename Class, typename Ret, typename... Args>
class MemberCallback : public Callback {
public:
    using Function = Ret(Class::*)(Args...);
    
    explicit MemberCallback(std::shared_ptr<Class> obj, Function fn, Args... args)
        : m_obj(obj), m_fn(fn), m_args(std::forward<Args>(args)...) {}
    
    void call() override { std::apply(std::bind(m_fn, m_obj.get(), std::placeholders::_1), m_args); }
    
private:
    std::shared_ptr<Class> m_obj;
    Function m_fn;
    std::tuple<Args...> m_args;
};
template <typename Class, typename Ret>
class MemberCallbackN : public Callback {
public:
    using Function = Ret(Class::*)();
    
    explicit MemberCallbackN(std::shared_ptr<Class> obj, Function fn)
        : m_obj(obj), m_fn(fn) {}
    
    void call() override { ((*m_obj).*m_fn)(); }
    
private:
    std::shared_ptr<Class> m_obj;
    Function m_fn;
};

/**
 * @name ConstMemberCallback(N)
 * @brief 静态成员函数回调类
 * @param Class 类名
 * @param Ret 返回值类型
 * @param Args 函数参数类型列表
 * 
*/
template <typename Class, typename Ret, typename... Args>
class ConstMemberCallback : public Callback {
public:
    using Function = Ret(Class::*)(Args...) const;
    
    explicit ConstMemberCallback(Function fn, Args... args)
        : m_fn(fn), m_args(std::forward<Args>(args)...) {}
    
    void call() override { std::apply(std::bind(m_fn, nullptr, std::placeholders::_1), m_args); }
    
private:
    Function m_fn;
    std::tuple<Args...> m_args;
};
template <typename Class, typename Ret>
class ConstMemberCallbackN : public Callback {
public:
    using Function = Ret(Class::*)() const;
    
    explicit ConstMemberCallbackN(Function fn)
        : m_fn(fn) {}
    
    void call() override { m_fn(); }
    
private:
    Function m_fn;
};

/*******************************************
 * 统一回调Make函数入口区，根据后续的各类创建需求，在此处追加各种重载模板
*/

/* 普通函数版本的Make */
template<typename Ret, typename... Args>
Callback::Ptr MakeCallback(Ret(*fn)(Args...), Args... args) {
    return std::make_shared<FunctionCallback<Ret, Args...>>(fn, std::forward<Args>(args)...);
}

/* 类成员函数版本的Make */
template<typename Class, typename Ret, typename... Args>
Callback::Ptr MakeCallback(std::shared_ptr<Class> obj, Ret(Class::*fn)(Args...), Args... args) {
    return std::make_shared<MemberCallback<Class, Ret, Args...>>(obj, fn, std::forward<Args>(args)...);
}

/* 类静态成员函数版本的Make */
template<typename Class, typename Ret, typename... Args>
Callback::Ptr MakeCallback(Ret(Class::*fn)(Args...), Args... args) {
    return std::make_shared<ConstMemberCallback<Class, Ret, Args...>>(fn, std::forward<Args>(args)...);
}

/* 普通函数无参版本的Make */
template<typename Ret>
Callback::Ptr MakeCallback(Ret(*fn)()) {
    return std::make_shared<FunctionCallbackN<Ret>>(fn);
}

/* 类成员函数无参版本的Make */
template<typename Class, typename Ret>
Callback::Ptr MakeCallback(std::shared_ptr<Class> obj, Ret(Class::*fn)()) {
    return std::make_shared<MemberCallbackN<Class, Ret>>(obj, fn);
}

/* 类静态成员函数无参版本的Make */
template<typename Class, typename Ret>
Callback::Ptr MakeCallback(Ret(Class::*fn)()) {
    return std::make_shared<ConstMemberCallbackN<Class, Ret>>(fn);
}
