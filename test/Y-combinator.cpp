#include <iostream>
#include <functional>

/**
 * typename F: the type of origin recursive function.
 * typename T: the type of principle argument in the recursive function.
 * typename R: the type of the final result.
 */

template<typename F, typename T, typename R>
auto Y(auto recurse) -> F {
	auto helper = [=](auto self) -> F {
		return recurse([=](T x) -> R {
			return self(self)(x);
		});
	};
	return helper(helper);
}

int main()
{
   	auto fact = Y<std::function<int(int)>, int, int>([](auto recurse) {
		return [=](auto n) {
			return n == 0 ? 1 : n * recurse(n - 1);
		};
	});
	
	auto fibonacci = Y<std::function<int(int)>, int, int>([](auto recurse) {
		return [=](auto n) {
			return n == 0 ? 0 : n == 1 ? 1 : recurse(n - 1) + recurse(n - 2);
		};
	});
	
	std::cout << fact(10) << ' ' << fibonacci(10) << std::endl;
   	return 0;
}
