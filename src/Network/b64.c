// Ben says: for compilers that think EVERYTHING is C++!
#ifdef __cplusplus
extern "C" {
#endif


static char encodingTable [64] = {

    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',

    'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',

    'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',

    'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'
	};

void	decode (const char * startP, const char * endP, char * destP, char ** outP)
{
	unsigned long ixtext;
	unsigned long lentext;
	unsigned long origsize;
	unsigned long ctremaining;
	unsigned char ch;
	unsigned char inbuf [4], outbuf [3];
	short i, ixinbuf;
	int flignore;
	int flendtext = 0;

	ixtext = 0;

	lentext = endP - startP;

	ixinbuf = 0;

	while (1) {

		if (ixtext >= lentext)
			break;

		ch = startP[ixtext++];

		flignore = 0;

		if ((ch >= 'A') && (ch <= 'Z'))
			ch = ch - 'A';

		else if ((ch >= 'a') && (ch <= 'z'))
			ch = ch - 'a' + 26;

		else if ((ch >= '0') && (ch <= '9'))
			ch = ch - '0' + 52;

		else if (ch == '+')
			ch = 62;

		else if (ch == '=') /*no op -- can't ignore this one*/
			flendtext = 1;

		else if (ch == '/')
			ch = 63;

		else
			flignore = 1;

		if (!flignore) {

			short ctcharsinbuf = 3;
			int flbreak = 0;

			if (flendtext) {

				if (ixinbuf == 0)
					break;

				if ((ixinbuf == 1) || (ixinbuf == 2))
					ctcharsinbuf = 1;
				else
					ctcharsinbuf = 2;

				ixinbuf = 3;

				flbreak = 1;
				}

			inbuf [ixinbuf++] = ch;

			if (ixinbuf == 4) {

				ixinbuf = 0;

				outbuf [0] = (inbuf [0] << 2) | ((inbuf [1] & 0x30) >> 4);

				outbuf [1] = ((inbuf [1] & 0x0F) << 4) | ((inbuf [2] & 0x3C) >> 2);

				outbuf [2] = ((inbuf [2] & 0x03) << 6) | (inbuf [3] & 0x3F);


				for (i = 0; i < ctcharsinbuf; i++)
					*destP++ = outbuf [i];
				}

			if (flbreak)
				break;
			}
		} /*while*/

	exit:

 	if (outP) *outP = destP;

} /*decodeHandle*/


#ifdef __cplusplus
}
#endif
