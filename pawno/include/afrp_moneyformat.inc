/*
	afrp_moneyformat.inc - Formatage des montants avec separateurs de milliers

	FormatMoney(1500000, buf, sizeof(buf)) -> buf = "1.500.000"
	Utilise le point comme separateur (convention francaise), pour
	distinguer clairement milliers/millions/milliards sur les gros prix
	(ex: armes Ammunation, achat de biz).
*/

#if defined AFRP_MONEYFORMAT_INCLUDED
	#endinput
#endif
#define AFRP_MONEYFORMAT_INCLUDED

stock FormatMoney(amount, output[], size)
{
	new numStr[16];
	new neg = (amount < 0);
	if(neg) amount = -amount;
	format(numStr, sizeof(numStr), "%d", amount);
	new len = strlen(numStr);

	new outPos = 0;
	if(neg && outPos < size - 1) output[outPos++] = '-';
	for(new i = 0; i < len; i++)
	{
		if(i > 0 && (len - i) % 3 == 0 && outPos < size - 1)
			output[outPos++] = '.';
		if(outPos < size - 1)
			output[outPos++] = numStr[i];
	}
	output[outPos] = EOS;
	return outPos;
}
