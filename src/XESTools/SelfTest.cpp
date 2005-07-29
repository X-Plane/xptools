#if DEV
void TEST_CompGeomDefs2(void);
void TEST_MapDefs(void);
#endif

void SelfTestAll(void)
{
#if DEV
	TEST_CompGeomDefs2();
	TEST_MapDefs();
	printf("Self-tests completed.\n");
#endif	
}