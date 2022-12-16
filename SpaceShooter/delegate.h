#pragma once
#include <vector>
#include <functional>

template<typename T, typename... U>
size_t getAddress(std::function<T(U...)> f) {
    typedef T(fnType)(U...);
    fnType** fnPointer = f.template target<fnType*>();
    return (size_t)*fnPointer;
}

/// <summary>
/// An attempt of recreating delegates as in C#
/// there are probably better ways of doing this in c++
/// </summary>
template<class T>
struct Delegate {
    std::vector<T> funcs;
    Delegate& operator+=(T mFunc) { funcs.push_back(mFunc); return *this; }
    Delegate& operator-=(T mFunc) 
    {
        // couldn't get == overloading to work, so this is a workaround
        for (int i = 0; i < funcs.size(); i++)
        {
            if (getAddress(funcs[i]) == getAddress(mFunc))
            {
                funcs.erase(funcs.begin() + i);
            }
        }
        return *this;
    }
    void operator()() { for (auto& f : funcs) f(); }
};


// couldn't get this to work
//bool operator==(std::function<void()>& lhs, std::function<void()>& rhs) { return getAddress(lhs) == getAddress(rhs); }
//bool operator==(const std::function<void()>& lhs, const std::function<void()>& rhs) { return getAddress(lhs) == getAddress(rhs); }


