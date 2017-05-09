bool ProcessMessages();

bool ProcessNewInternetLogin()
{
	DialogsSystem DSS( 0, 0 );

	SQPicture Back( "Interface\\Background_Wizard.bmp" );

	LocalGP WBT( "Interface\\LogWizard" );
	LocalGP BTNS( "Interface\\Login" );

	DSS.addPicture( nullptr, 0, 0, &Back, &Back, &Back );

	char REQ[1024];
	char REQ1[1024];
	char DATA[512] = "";

	ResFile F = RReset( "Internet\\Cash\\login.cmd" );
	if (F != INVALID_HANDLE_VALUE)
	{
		int sz = RFileSize( F );
		if (sz > 511)
		{
			sz = 511;
		}
		RBlockRead( F, REQ1, sz );
		REQ1[sz] = 0;
		RClose( F );
	}
	else
	{
		return false;
	}

	F = RReset( "Internet\\Cash\\LGDTA.LOG" );

	if (F != INVALID_HANDLE_VALUE)
	{
		int sz = RFileSize( F );
		if (sz > 511)
		{
			sz = 511;
		}
		RBlockRead( F, DATA, sz );
		DATA[sz] = 0;
		RClose( F );
	}

	sprintf( REQ, REQ1, DATA );

	RunExplorer( 2, REQ, 0, 0, 1024, 768 );

	ItemChoose = -1;
	if (SetAccessKey)
	{
		for (int i = 0; i < 8; i++)
		{
			SetAccessKey( i, "" );
		}
	}

	char* AKEY = "";
	ACCESS[0] = 0;
	bool pal = 0;
	int t0 = GetTickCount();

	do
	{
		ProcessMessages();

		DSS.ProcessDialogs();

		ProcessExplorer( 2 );

		DSS.RefreshView();

		if (GetAccessKey)
		{
			AKEY = GetAccessKey( 2 );
		}

		if (!pal)
		{
			SlowLoadPalette( "2\\agew_1.pal" );
			pal = 1;
		}
	} while (ItemChoose == -1 && AKEY[0] == 0);

	if (!strcmp( AKEY, "#CANCEL" ))
	{
		AKEY[0] = 0;
	}

	if (SetAccessKey)
	{
		for (int i = 0; i < 8; i++)
		{
			SetAccessKey( i, AKEY );
		}
	}

	strcpy( ACCESS, AKEY );

	DSS.ProcessDialogs();

	ProcessExplorer( 2 );

	DSS.RefreshView();

	return AKEY[0] != 0;
}