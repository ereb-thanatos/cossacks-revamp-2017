#include <Windows.h>

/*
	Catch division by zero by "overloading" div() via #define
	(in case we will be able to link dynamically sometime, this causes no linking warnings)
*/
div_t __cdecl SecureDivision(int const numerator, int const denominator)
{
	div_t result;

	if (0 == denominator)
	{
		result.quot = numerator;
		result.rem = 0;
	}
	else
	{
		result.quot = numerator / denominator;
		result.rem = numerator % denominator;
	}

	return result;
}
