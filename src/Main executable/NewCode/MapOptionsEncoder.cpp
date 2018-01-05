//Encodes 10 given option values into a 7-digit number for map name
//Reason: fixed network buffer size with limited mapname space
int EncodeOptionsInNumber(int* selected_opt_values)
{
	int result = 0;
	int temp_digit = 0;

	int start_opts = selected_opt_values[0];
	int balloon_opts = selected_opt_values[1];
	int cannons_opts = selected_opt_values[2];
	int peace_opts = selected_opt_values[3];
	int century_opts = selected_opt_values[4];
	int capture_opts = selected_opt_values[5];
	int save_opts = selected_opt_values[6];
	int dip_opts = selected_opt_values[7];
	int shipyard_opts = selected_opt_values[8];
	int market_opts = selected_opt_values[9];

	bool no_artillery = false;
	if (3 == cannons_opts)
	{
		//Will be encoded together with shipyard options
		no_artillery = true;
		cannons_opts = 1; //'No Cannons...'
	}

	result += start_opts;
	result += 10 * (3 * balloon_opts + cannons_opts);
	result += 100 * peace_opts;
	result += 1000 * (3 * century_opts + market_opts);
	result += 10000 * (4 * dip_opts + capture_opts);
	result += 100000 * save_opts;
	result += 1000000 * shipyard_opts;
	if (no_artillery)
	{
		result += 4000000;
	}

	return result;
}

//Decodes 7-digit number into game settings
//Saves settings values in result[10]
//Returns 'Start options' value
int DecodeOptionsFromNumber(const int number, int* result)
{
	//Separate digits in number (starting with last)
	int digits[7] = {0};
	digits[0] = number % 10;
	for (int i = 1, divisor = 10; i < 7; i++)
	{
		digits[i] = (number / divisor) % 10;
		divisor *= 10;
	}

	int start_opts = digits[0];
	int balloon_opts = digits[1] / 3;
	int cannons_opts = digits[1] % 3;
	int peace_opts = digits[2];
	int century_opts = digits[3] / 3;
	int market_opts = digits[3] % 3;
	int dip_opts = digits[4] / 4;
	int capture_opts = digits[4] % 4;
	int save_opts = digits[5];
	int no_artillery_opt = digits[6] / 4;
	int shipyard_opts = digits[6] % 4;

	result[0] = balloon_opts;
	result[1] = cannons_opts;
	result[2] = no_artillery_opt;
	result[3] = peace_opts;
	result[4] = century_opts;
	result[5] = capture_opts;
	result[6] = save_opts;
	result[7] = dip_opts;
	result[8] = shipyard_opts;
	result[9] = market_opts;

	return start_opts;
}
