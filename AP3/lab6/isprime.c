/*
 * * is_prime() - returns 1 if argument is prime, 0 if not
 * */
int is_prime(unsigned long number) {
	unsigned long tNum;
	unsigned long tLim;
	if (number == 1 || number == 2)
		return 1;
	if ((number % 2) == 0)
		return 0;
	for (tNum=3, tLim=number; tLim > tNum; tLim = number / tNum, tNum += 2) {
		if ((number % tNum) == 0)
			return 0;
	}
	return 1;
}
