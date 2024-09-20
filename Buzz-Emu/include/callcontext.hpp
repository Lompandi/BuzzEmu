#pragma once

#include <type_traits>
#include <utility> // For std::forward

namespace bzmu::detail {
    template <typename FuncType, typename Func2Type>
    struct context {
        template <typename OperandType, typename Operand2Type, typename... ExtraArgs>
        void operator() (OperandType op1, Operand2Type op2, ExtraArgs&&... extra_args) {
            if constexpr (std::is_void_v<FuncType>) {
                return func2(std::forward<ExtraArgs>(extra_args)...); // Fixed closing parenthesis
            }
            else {
                return func(func2(std::forward<ExtraArgs>(extra_args)...));
            }
        }

    private:
        FuncType func;
        Func2Type func2;
        // template <typename F, typename... Args>
        // auto call_function(F&& func, Args&&... args) {
        //     return std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
        // }
    };
}