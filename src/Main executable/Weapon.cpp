/*				  Оружие дальнего действия
 *			Организация перемещающихся анимаций.*/
#include "ddini.h"
#include "ResFile.h"
#include "FastDraw.h"
#include "mgraph.h"
#include "mouse.h"
#include "menu.h"
#include "MapDiscr.h"
#include "walls.h"
#include "Nucl.h"
#include <math.h>
#include "GSound.h"
#include "3DGraph.h"
#include "3DMapEd.h"
#include "MapSprites.h"
#include "NewMon.h"
#include "assert.h"
#include "GP_Draw.h"
#include "ZBuffer.h"
#include "3DBars.h"
#define MaxExpl 8192
#define ExMask (MaxExpl-1)
#define WEPSH 14
void PlayAnimation( NewAnimation* NA, int Frame, int x, int y );
extern byte AlphaR[65536];
extern byte AlphaW[65536];
void ShowRLCItemGrad( int x, int y, lpRLCTable lprt, int n, byte* Pal );
extern word FlyMops[256][256];
int NUCLUSE[4];
bool EUsage[MaxExpl];
word LastAnmIndex;
AnmObject* GAnm[MaxExpl];
short TSin[257];
short TCos[257];
short TAtg[257];
word ID_FIRE;
word ID_FIREX;
word ID_FIRE1;
word ID_MAGW;
word ID_EXPL1;
word ID_EXPL2;
word ID_GLASSFLY;
word ID_GLASSBROKEN;
word ID_FLYFIRE;
word ID_MAGEXP;
word ID_FOG;
word ID_FOG1;
NewAnimation** FiresAnm[2];
NewAnimation** PreFires[2];
NewAnimation** PostFires[2];
int            NFiresAnm[2];

int  NWeaponIDS;
char* WeaponIDS[32];
int GetWeaponType( char* Name )
{
	for ( int i = 0; i < NWeaponIDS; i++ )
	{
		if ( !strcmp( WeaponIDS[i], Name ) )
		{
			return i;
		}
	}
	return -1;
}

int mul3( int );
int nEused;
word LastReq;
short randoma[8192];
word rpos;

//Precalculates Sin/Cos, loads random.lst
void InitExplosions()
{
	for ( int i = 0; i < MaxExpl; i++ )
	{
		GAnm[i] = new AnmObject;
	}

	for ( int i = 0; i < 257; i++ )
	{
		TSin[i] = short( 256 * sin( 2 * 3.1415927*i / 256 ) );
		TCos[i] = short( 256 * cos( 2 * 3.1415927*i / 256 ) );
		TAtg[i] = short( 128 * atan2( i, 256 ) / 3.1415927 );
	}

	memset( &EUsage, 0, MaxExpl );
	LastReq = 0;
	nEused = 0;
	ResFile rf = RReset( "random.lst" );
	RBlockRead( rf, randoma, 16384 );
	RClose( rf );

	rpos = 0;
}

void CloseExu( int i )
{
	if ( EUsage[i] )
	{
		EUsage[i] = false;
		nEused--;
	}
}

void CloseExplosions()
{
	for ( int i = 0; i < MaxExpl; i++ )
	{
		if ( GAnm[i] )
		{
			free( GAnm[i] );
			GAnm[i] = nullptr;
		}
	}
}

bool Create3DAnmObject( Weapon* Weap, int xs, int ys, int zs,
	int xd, int yd, int zd,
	OneObject* OB );
extern int ExplMedia;
extern byte TexMedia[256];
extern byte* WaterDeep;
char WpnChar[228];

bool Create3DAnmObjectEX( Weapon* Weap, int xs, int ys, int zs1,
	int xd, int yd, int zd,
	OneObject* OB, byte AttType, word DestObj,
	int vx, int vy, int vz );
void ExplodeAnmObject( AnmObject* AO )
{



	Weapon* Weap = AO->Weap;
	if ( AO->Damage )
	{
		int Damage = AO->Damage;
		if ( Weap->FullParent )Damage = Weap->Damage;
		//if(AO->Weap->Radius) 
		MakeRoundDamage( AO->x >> ( WEPSH - 4 ), AO->y >> ( WEPSH - 4 ),
			AO->Weap->Radius, AO->Damage, AO->Sender, AO->AttType );
		if ( AO->DestObj != 0xFFFF )
		{
			OneObject* OB = Group[AO->DestObj];
			if ( OB&&OB->Serial == AO->DestSN )
			{
				ExplMedia = OB->newMons->ExplosionMedia;





				OB->MakeDamage( 0, AO->Damage, AO->Sender, AO->AttType );
			};
		};
	};
	if ( ExplMedia == -1 )
	{
		//water analysing
		int wx = AO->x >> ( WEPSH + 5 );
		int wy = ( ( AO->y >> ( WEPSH + 1 ) ) - GetHeight( AO->x >> WEPSH, AO->y >> WEPSH ) ) >> 4;
		if ( wx >= 1 && wy >= 1 && wx < (MaxWX) -1 && wy < ( MAPSY >> 1 ) - 1 )
		{
			int ofsw = wx + wy*( MaxWX );
			int z1 = ( int( WaterDeep[ofsw] ) + int( WaterDeep[ofsw + 1] ) + int( WaterDeep[ofsw + ( MaxWX )] ) + int( WaterDeep[ofsw + (MaxWX) +1] ) ) >> 2;
			if ( z1 > 128 )ExplMedia = 1;
		};
		if ( ExplMedia == -1 )
		{
			//texture analysing
			int tx = AO->x >> WEPSH;
			tx = div( tx + TriUnit, TriUnit + TriUnit ).quot;
			int ty = AO->y >> WEPSH;
			if ( tx & 1 )ty = div( ty, TriUnit + TriUnit ).quot;
			else ty = div( ty + TriUnit, TriUnit + TriUnit ).quot;
			if ( tx < 0 )tx = 0;
			if ( ty < 0 )ty = 0;
			if ( tx >= msx )tx = msx - 1;
			if ( ty >= msy )ty = msy - 1;
			ExplMedia = TexMedia[TexMap[tx + ty*( MaxTH + 1 )]];
		};
	};
	ChildWeapon* CWP = NULL;
	int cwtp = -1;
	if ( ExplMedia != -1 )
	{
		for ( int j = 0; j < Weap->NCustomEx; j++ )
		{
			ChildWeapon* CW = Weap->CustomEx + j;
			if ( CW->Type == ExplMedia )
			{
				CWP = CW;
				cwtp = j;
			};
		};
		if ( !CWP )CWP = &Weap->Default;
	};

	if ( CWP )
	{
		int rnd = rando();
		int NChild = CWP->MinChild + ( ( ( CWP->MaxChild - CWP->MinChild )*rnd ) >> 15 );
		//if(Weap->FullParent){
		for ( int i = 0; i < NChild; i++ )
		{
			Weapon* BWpn = CWP->Child[( rando()*CWP->NChildWeapon ) >> 15];

			Create3DAnmObjectEX( BWpn, AO->x >> WEPSH, AO->y >> WEPSH, AO->z >> WEPSH,
				AO->xd >> WEPSH, AO->yd >> WEPSH, AO->zd >> WEPSH, AO->Sender, AO->AttType, AO->DestObj, AO->vx, AO->vy, AO->vz );
		};
	};

	if ( Weap->NSyncWeapon && !( ExplMedia != -1 && CWP != &Weap->Default ) )
	{
		for ( int i = 0; i < Weap->NSyncWeapon; i++ )
		{
			Weapon* BWpn = Weap->SyncWeapon[i];

			Create3DAnmObjectEX( BWpn, AO->x >> WEPSH, AO->y >> WEPSH, AO->z >> WEPSH,
				AO->xd >> WEPSH, AO->yd >> WEPSH, AO->zd >> WEPSH, AO->Sender, AO->AttType, AO->DestObj, AO->vx, AO->vy, AO->vz );
		};
	};
};
int ConvScrY( int x, int y );
void ShowRLCItemDark( int x, int y, lpRLCTable lprt, int n );
extern bool NOPAUSE;
void ShowExpl()
{
	int ScrX = mapx << 5;
	int ScrY = mul3( mapy << 5 ) >> 2;
	int ScrX1 = ( mapx + smaplx ) << 5;
	int ScrY1 = mul3( ( mapy + smaply ) << 5 ) >> 2;
	for ( int i = 0; i < MaxExpl; i++ )if ( EUsage[i] )
	{
		if ( EUsage[i] )
		{
			AnmObject* AO = GAnm[i];
			Weapon* Weap = AO->Weap;
			int xs = ( AO->x >> WEPSH );
			int ys0 = mul3( AO->y >> 4 ) >> ( WEPSH - 2 );
			int ys = ys0 - ( AO->z >> WEPSH );
			int xs1 = xs;
			xs = xs1;
			NewAnimation* nan = Weap->NewAnm;
			//Visualization
			if ( xs >= ScrX&&ys >= ScrY&&xs <= ScrX1&&ys <= ScrY1 )
			{
				//determining the direction
				if ( AO->Frame > nan->NFrames - 1 )AO->Frame = nan->NFrames - 1;
				NewFrame* NF = &nan->Frames[AO->Frame];
				PlayAnimation( nan, AO->Frame, xs, ys );
				int NDir = ( ( nan->Rotations - 1 ) << 1 );
				int spr;
				xs -= ScrX - smapx;
				ys -= ScrY - smapy;
				ys0 -= ScrY - smapy;
				int oxs = xs;
				//int oys=ys;
				if ( NDir )
				{
					double angl;
					bool bdir;
					if ( !( AO->vx || AO->vy ) )
					{
						angl = atan2( AO->xd - AO->x, AO->yd - AO->y );
						bdir = AO->xd - AO->x > 0;
					}
					else
					{
						angl = atan2( -AO->vx, AO->vy - Prop43( AO->vz ) );
						bdir = AO->vx > 0;
					};
					//angl+=(3.14152927/NDir);
					if ( angl >= 3.1415297 )angl -= 3.14152927 * 2;
					if ( angl < 0 )angl = -angl;
					spr = angl*double( NDir ) / 3.14152927;
					if ( spr >= NDir )spr = NDir - 1;
					spr = ( spr + 1 ) >> 1;
					if ( bdir )
					{
						spr += 4096;
						xs -= NF->dx;
					}
					else
					{
						xs += NF->dx;
					};
				}
				else
				{
					spr = 0;
					xs += NF->dx;
				};
				int zz = GetHeight( AO->x >> WEPSH, AO->y >> WEPSH );
				int zz1 = ( AO->z >> WEPSH ) - zz;
				ys += NF->dy;
				spr += NF->SpriteID*nan->Rotations;
				if ( Weap->HiLayer )ys0 += 300;
				//it is visible!
				switch ( Weap->Transparency )
				{
				case 1://DARK
					//GPS.ShowGPDark(smapx+xs-ScrX,smapy+ys-ScrY,NF->FileID,spr,0);
					AddOptPoint( ZBF_NORMAL, oxs, ys0, xs, ys, NULL, NF->FileID, spr, AV_DARK | AV_GRADIENT );
					break;
				case 2://WHITE
					//GPS.ShowGPMutno(smapx+xs-ScrX,smapy+ys-ScrY,NF->FileID,spr,0);
					AddOptPoint( ZBF_NORMAL, oxs, ys0, xs, ys, NULL, NF->FileID, spr, AV_WHITE | AV_GRADIENT );
					break;
				case 3://RED
					//GPS.ShowGPFired(smapx+xs-ScrX,smapy+ys-ScrY,NF->FileID,spr,0);
					//break;
				case 4://BRIGHT
				case 5://YELLOW
				case 6://ALPHAR
					//GPS.ShowGPGrad(smapx+xs-ScrX,smapy+ys-ScrY,NF->FileID,spr,0,AlphaR);
					//break;
				case 7://ALPHAW
					//GPS.ShowGPGrad(smapx+xs-ScrX,smapy+ys-ScrY,NF->FileID,spr,0,AlphaW);
					//break;
				default:
					//GPS.ShowGP(smapx+xs-ScrX,smapy+ys-ScrY,NF->FileID,spr,0);
					AddOptPoint( ZBF_NORMAL, oxs, ys0, xs, ys, NULL, NF->FileID, spr, 0 );
					break;
				};
			};
		};
	};
};

//Process explosion animations
void ProcessExpl()
{
	if ( !NOPAUSE )
		return;

	for ( int i = 0; i < MaxExpl; i++ )
	{
		if ( EUsage[i] )
		{
			AnmObject* AO = GAnm[i];
			Weapon* Weap = AO->Weap;
			NewAnimation* nan = Weap->NewAnm;

			if ( Weap->NTileWeapon )
			{
				int tpp = Weap->TileProbability;
				int vdx = div( AO->vx, tpp ).quot;
				int vdy = div( AO->vy, tpp ).quot;
				int vdz = div( AO->vz, tpp ).quot;
				int xxx = AO->x;
				int yyy = AO->y;
				int zzz = AO->z;
				for ( int j = 0; j < tpp; j++ )
				{
					Weapon* WP = Weap->TileWeapon[( rando()*Weap->NTileWeapon ) >> 15];

					Create3DAnmObject( WP, xxx >> WEPSH, yyy >> WEPSH, zzz >> WEPSH,
						AO->xd >> WEPSH, AO->yd >> WEPSH, AO->zd >> WEPSH, NULL );
					xxx -= vdx;
					yyy -= vdy;
					zzz -= vdz;
				}
			}
			AO->vz += AO->az;
			AO->x += AO->vx;
			AO->y += AO->vy;
			AO->z += AO->vz;

			int dis = abs( AO->x - AO->xd ) + abs( AO->y - AO->yd ) + abs( AO->z - AO->zd );
			int wprp = Weap->Propagation;
			if ( ( wprp == 3 || wprp == 5 ) && dis < 65536 * 4 )
			{
				ExplodeAnmObject( AO );
				CloseExu( i );
			}
			else
			{
				int ssx = AO->x >> WEPSH;
				int ssy = AO->y >> WEPSH;
				int zz = GetHeight( ssx, ssy );
				int zz0 = zz;
				int wpt = Weap->Propagation;
				int BHi = 0;
				if ( wpt >= 2 && wpt <= 5 )
				{
					BHi = GetBar3DHeight( ssx, ssy );
					zz += BHi;
				}

				if ( zz > ( AO->z >> WEPSH ) )
				{
					//Collision with surface
					if ( BHi )
					{
						int IDI = GetBar3DOwner( ssx, ssy );
						if ( AO->Sender && IDI != -1 && IDI == AO->Sender->Index )
						{
							zz = zz0;
							BHi = 0;
							goto UUU1;
						}
						else
						{
							OneObject* OB = Group[IDI];
							if ( OB )
							{
								AO->DestObj = IDI;
								AO->DestSN = OB->Serial;
							}
							ExplodeAnmObject( AO );
							CloseExu( i );
						}
					}
					else
					{
						ExplodeAnmObject( AO );
						CloseExu( i );
					}
				}
				else
				{
				UUU1:
					if ( AO->NTimes == 1 && AO->Frame == Weap->HotFrame )
					{
						ExplodeAnmObject( AO );
					}
					if ( AO->Frame >= nan->NFrames - FrmDec )
					{
						if ( AO->NTimes == 1 )
						{
							CloseExu( i );
						}
						else
						{
							if ( AO->NTimes > 0 )
								AO->NTimes--;
							AO->Frame = -1;
						}
					}
				};
			}
			AO->Frame++;
		}
	}
}

void MoveAway( int x, int y );
bool TraceObjectsInLine( int xs, int ys, int zs, int* xD, int* yD, int* zD, int damage, OneObject* Sender, byte AttType );
bool Create3DAnmObjectEX( Weapon* Weap, int xs, int ys, int zs1,
	int xd, int yd, int zd,
	OneObject* OB, byte AttType, word DestObj,
	int vx, int vy, int vz )
{
	if ( !Weap )return false;
	if ( AttType >= NAttTypes )AttType = 0;
	if ( Weap->SoundID != -1 )
	{
		AddEffect( xs, ( ys >> 1 ) - zs1, Weap->SoundID );
	};
	int zs;
	int hig = GetHeight( xs, ys );
	if ( zs1 < hig )zs = hig + 1;
	else zs = zs1;
	short i = LastReq;
	while ( EUsage[i] && i < MaxExpl )i++;
	if ( i >= MaxExpl )
	{
		i = 0;
		while ( EUsage[i] && i < LastReq )i++;
	};
	LastAnmIndex = (word) -1;
	if ( EUsage[i] )return 0;
	LastAnmIndex = i;
	LastReq = ( i + 1 )&ExMask;
	EUsage[i] = true;
	nEused++;
	AnmObject* AO = GAnm[i];
	AO->ASerial = rando();
	AO->AttType = AttType;

	if ( OB&&Weap->FullParent )
	{
		int nn = OB->newMons->MaxInside;
		if ( OB->newMons->ShotPtZ&&nn )
		{
			int N = OB->NInside;
			int ns = 0;
			for ( int i = 0; i < N; i++ )
			{
				word MID = OB->Inside[i];
				if ( MID != 0xFFFF )
				{
					OneObject* OBX = Group[MID];
					if ( OBX && !OBX->Sdoxlo )
					{
						byte use = OBX->newMons->Usage;
						if ( use == StrelokID )ns++;
					};
				};
			};
			int min = OB->newMons->MinDamage[AttType];
			int max = OB->Ref.General->MoreCharacter->MaxDamage[AttType];
			AO->Damage = min + ( ( ( max - min )*ns ) / nn );
		}
		else AO->Damage = OB->Ref.General->MoreCharacter->MaxDamage[AttType];
	}
	else AO->Damage = Weap->Damage;
	AO->Weap = Weap;
	AO->Sender = OB;
	AO->DestObj = 0xFFFF;
	AO->DestSN = 0xFFFF;
	if ( DestObj != 0xFFFF && Weap->FullParent )
	{
		OneObject* ob = Group[DestObj];
		if ( ob )
		{
			AO->DestObj = DestObj;
			AO->DestSN = ob->Serial;
		};
	};
	AO->az = -Weap->Gravity * 2000 * FrmDec*FrmDec;
	int SPEED = Weap->Speed << SpeedSh;
	int time, dist;
	switch ( Weap->Propagation )
	{
	case 0://STAND
		AO->x = xs << WEPSH;
		AO->y = ys << WEPSH;
		AO->z = zs << WEPSH;
		AO->vx = 0;
		AO->vy = 0;
		AO->vz = 0;
		AO->NTimes = Weap->Times;
		if ( AO->Weap->HotFrame == 0xFF )
		{
			AO->xd = xd << WEPSH;
			AO->yd = yd << WEPSH;
			AO->zd = zd << WEPSH;
			AO->Frame = 0;
			ExplodeAnmObject( AO );
		};
		break;
	case 1://SLIGHTUP
		AO->x = xs << WEPSH;
		AO->y = ys << WEPSH;
		AO->z = zs << WEPSH;
		AO->vx = 0;
		AO->vy = 0;
		AO->vz = SPEED << WEPSH;
		AO->NTimes = Weap->Times;
		if ( AO->Weap->HotFrame == 0xFF )
		{
			AO->xd = xd << WEPSH;
			AO->yd = yd << WEPSH;
			AO->zd = zd << WEPSH;
			AO->Frame = 0;
			ExplodeAnmObject( AO );
		};
		break;
	case 7://REFLECT
	{
		vx >>= 16;
		vy >>= 16;
		vz >>= 16;
		if ( vx || vy || vz )
		{
			int zL = GetHeight( xs - 32, ys );
			int zR = GetHeight( xs + 32, ys );
			int zU = GetHeight( xs, ys - 32 );
			int zD = GetHeight( xs, ys + 32 );
			int nx = zR - zL;
			int ny = zD - zU;
			int nz = 32;
			int norma = sqrt( nx*nx + ny*ny + nz*nz );
			int vn = 2 * ( vx*nx + vy*ny + vz*nz ) / norma;
			int vv = sqrt( vx*vx + vy*vy + vz*vz );
			vx -= ( vn*nx ) / norma;
			vy -= ( vn*ny ) / norma;
			vz -= ( vn*nz ) / norma;
		};
		vx = ( vx*Weap->Speed ) << 10;
		vy = ( vy*Weap->Speed ) << 10;
		vz = ( vz*Weap->Speed ) << 10;
		AO->NTimes = Weap->Times;
	};
	break;
	case 6:
	case 2://RANDOM
		AO->x = xs << WEPSH;
		AO->y = ys << WEPSH;
		AO->z = zs << WEPSH;
		AO->vx = ( ( rando() - 16384 )*SPEED );
		AO->vy = ( ( rando() - 16384 )*SPEED );
		AO->vz = ( ( rando() )*SPEED ) >> 1;
		AO->NTimes = Weap->Times;
		break;
	case 3://FLY
		AO->x = xs << WEPSH;
		AO->y = ys << WEPSH;
		AO->z = zs << WEPSH;
		dist = sqrt( ( xs - xd )*( xs - xd ) + ( ys - yd )*( ys - yd ) );
		time = div( dist, SPEED ).quot;
		AO->NTimes = Weap->Times;
		if ( time )
		{
			AO->vx = div( ( xd - xs ) << WEPSH, time + 1 ).quot;
			AO->vy = div( ( yd - ys ) << WEPSH, time + 1 ).quot;
			AO->vz = div( ( zd - zs ) << WEPSH, time + 1 ).quot - ( ( AO->az*( time + 2 ) ) >> 1 );
		}
		else
		{
#ifdef CONQUEST
			TraceObjectsInLine( xs, ys, zs, &xd, &yd, &zd, AO->Damage, AO->Sender, AO->AttType );
			//ExplodeAnmObject(AO);
			AO->vx = 0;
			AO->vy = 0;
			AO->vz = 0;
			AO->x = xd << WEPSH;
			AO->y = yd << WEPSH;
			AO->z = zd << WEPSH;
			AO->xd = xd << WEPSH;
			AO->yd = yd << WEPSH;
			AO->zd = zd << WEPSH;
			AO->Frame = 0;
			CloseExu( i );
#else
			AO->vx = 0;
			AO->vy = 0;
			AO->vz = 0;
			AO->x = xd << WEPSH;
			AO->y = yd << WEPSH;
			AO->z = zd << WEPSH;
			AO->xd = xd << WEPSH;
			AO->yd = yd << WEPSH;
			AO->zd = zd << WEPSH;
			AO->Frame = 0;
			ExplodeAnmObject( AO );
			CloseExu( i );
#endif
			return true;
		};
		break;
	case 4://IMMEDIATE
		AO->x = xd << WEPSH;
		AO->y = yd << WEPSH;
		AO->z = zd << WEPSH;
		AO->vx = 0;
		AO->vy = 0;
		AO->vz = 0;
		AO->NTimes = Weap->Times;
		break;
	case 5://ANGLE
	{
		int rxy = int( sqrt( ( xd - xs )*( xd - xs ) + ( yd - ys )*( yd - ys ) ) );
		int t = zd - zs - int( double( rxy )*tan( Weap->Speed*3.1415 / 180 ) );
		if ( t >= 0 || !AO->az )
		{
			EUsage[i] = false;
			return false;
		}
		/*
		Division by zero. Probably because of high AO->az (acceleration) value.
		In this case -40000: t = -1, 4*(-2048/-40000)=0,2048 -> int -> =0
		div(..., t) <- bummer
		Happened while playing grenadier attack explosion animation on port building
		Fixed by overloading div() in NewCode/SecureDivision.cpp
		*/
		t = 4 * sqrt( double( t << ( WEPSH - 3 ) ) / AO->az );
		AO->vx = div( ( xd - xs ) << WEPSH, t ).quot;
		AO->vy = div( ( yd - ys ) << WEPSH, t ).quot;
		int vxy = sqrt( double( AO->vx )*double( AO->vx ) + double( AO->vy )*double( AO->vy ) );
		AO->vz = int( double( vxy )*tan( Weap->Speed*3.1415 / 180 ) );
		AO->x = xs << WEPSH;
		AO->y = ys << WEPSH;
		AO->z = zs << WEPSH;
		AO->NTimes = Weap->Times;
	}
	break;
	default:
		assert( 0 );
	};
	AO->xd = xd << WEPSH;
	AO->yd = yd << WEPSH;
	AO->zd = zd << WEPSH;
	AO->Frame = 0;
	return true;
}

bool Create3DAnmObject( Weapon* Weap, int xs, int ys, int zs1,
	int xd, int yd, int zd,
	OneObject* OB, byte AttType, word DestObj )
{
	return Create3DAnmObjectEX( Weap, xs, ys, zs1, xd, yd, zd, OB, AttType, DestObj, 0, 0, 0 );
}

bool Create3DAnmObject( Weapon* Weap, int xs, int ys, int zs1,
	int xd, int yd, int zd,
	OneObject* OB )
{
	return Create3DAnmObjectEX( Weap, xs, ys, zs1, xd, yd, zd, OB, 0, 0xFFFF, 0, 0, 0 );
}

void ChildWeapon::InitChild()
{
	MinChild = 0;
	MaxChild = 0;
	NChildWeapon = 0;
	for ( int i = 0; i < 4; i++ )
	{
		Child[i] = nullptr;
	}
}

bool CheckWpn( Weapon* WP )
{
	int wpt = WP->Propagation;
	if ( wpt >= 3 && wpt <= 5 )return true;
	return false;
}

Weapon* ReturnFlyChild( Weapon* Weap )
{
	if ( CheckWpn( Weap ) )
	{
		return Weap;
	}

	ChildWeapon* CWP = &Weap->Default;
	for ( int i = 0; i < CWP->NChildWeapon; i++ )
	{
		Weapon* CW = CWP->Child[i];
		if ( CheckWpn( CW ) )
		{
			return CW;
		}
	}

	for ( int i = 0; i < Weap->NSyncWeapon; i++ )
	{
		Weapon* SW = Weap->SyncWeapon[i];
		if ( CheckWpn( SW ) )
		{
			return SW;
		}
	}

	return nullptr;
}

int CheckWpPoint( int x, int y, int z, word Index )
{
	int zg = GetBar3DHeight( x, y );
	int zL = GetUnitHeight( x, y );
	if ( zg < 20 )
	{
		zL -= 45;
	}

	if ( zg + zL <= z )
	{
		return -1;
	}

	if ( zg )
	{
		word Own = GetBar3DOwner( x, y );
		if ( Own != Index )
		{
			return Own;
		}
		else
		{
			if ( zL < z )return -1;
		}
	}
	return 0xFFFF;
}

int CheckWpLine( int xs, int ys, int zs, int xd, int yd, int zd, word Index )
{
	int v = Norma( xd - xs, yd - ys );

	if ( !v )
	{
		return -1;
	}

	int N = ( v >> 4 ) + 1;
	int vx = div( ( xd - xs ) << WEPSH, N ).quot;
	int vy = div( ( yd - ys ) << WEPSH, N ).quot;
	int vz = div( ( zd - zs ) << WEPSH, N ).quot;
	xs <<= WEPSH;
	ys <<= WEPSH;
	zs <<= WEPSH;

	for ( int i = 0; i < N; i++ )
	{
		int res = CheckWpPoint( xs >> WEPSH, ys >> WEPSH, zs >> WEPSH, Index );

		if ( res != -1 )
		{
			return res;
		}

		xs += vx;
		ys += vy;
		zs += vz;
	}
	return -1;
}

bool ShotRecommended;
int PredictShot( Weapon* Weap, int xs, int ys, int zs, int xd, int yd, int zd, word Index )
{
	Weapon* WP = ReturnFlyChild( Weap );
	if ( WP )
	{
		int x, y, z, xf, yf, zf, g;
		int vx = 0;
		int vy = 0;
		int vz = 0;
		g = -WP->Gravity * 2000 * FrmDec*FrmDec;
		int SPEED = WP->Speed << SpeedSh;
		x = xs << WEPSH;
		y = ys << WEPSH;
		z = zs << WEPSH;
		xf = xd << WEPSH;
		yf = yd << WEPSH;
		zf = zd << WEPSH;
		int N = 0;
		int dist, time;
		bool NoCheck = 1;
		switch ( WP->Propagation )
		{
		case 3://FLY
			dist = sqrt( ( xs - xd )*( xs - xd ) + ( ys - yd )*( ys - yd ) );
			time = div( dist, SPEED ).quot;
			vx = div( ( xd - xs ) << WEPSH, time + 1 ).quot;
			vy = div( ( yd - ys ) << WEPSH, time + 1 ).quot;
			vz = div( ( zd - zs ) << WEPSH, time + 1 ).quot - ( ( g*( time + 2 ) ) >> 1 );
			if ( !time )NoCheck = 0;
			break;
		case 4://IMMEDIATE
			vx = xf - x;
			vy = yf - y;
			vz = zf - z;
			NoCheck = 0;
			break;
		case 5://ANGLE
		{
			int rxy = int( sqrt( ( xd - xs )*( xd - xs ) + ( yd - ys )*( yd - ys ) ) );
			int t1 = zd - zs - int( double( rxy )*tan( WP->Speed*3.1415 / 180 ) );
			if ( t1 >= 0 )return 0xFFFE;
			int t = 4 * sqrt( double( t1 << ( WEPSH - 3 ) ) / g );
			vx = div( ( xd - xs ) << WEPSH, t ).quot;
			vy = div( ( yd - ys ) << WEPSH, t ).quot;
			int vxy = sqrt( double( vx )*double( vx ) + double( vy )*double( vy ) );
			vz = int( double( vxy )*tan( WP->Speed*3.1415 / 180 ) );
		}
		break;
		}

		if ( NoCheck )
		{
			return -1;
		}

		//process checking
		if ( abs( vx >> WEPSH ) + abs( vy >> WEPSH ) + abs( vz >> WEPSH ) < 200 )
		{
			ShotRecommended = true;
		}
		else
		{
			ShotRecommended = false;
		}

		int x0 = x;
		int y0 = y;
		int z0 = z;
		int dst = 0;
		do
		{
			vz += g;
			int x1 = x0 + vx;
			int y1 = y0 + vy;
			int z1 = z0 + vz;
			int LRes = CheckWpLine( x0 >> WEPSH, y0 >> WEPSH, z0 >> WEPSH, x1 >> WEPSH, y1 >> WEPSH, z1 >> WEPSH, Index );

			if ( LRes != -1 )
			{
				return LRes;
			}

			x0 = x1; y0 = y1; z0 = z1;
			dst = Norma( x0 - xf, y0 - yf );
		} while ( dst > ( 4 << WEPSH ) );
	}
	return -1;
}

bool TraceObjectsInLine( int xs, int ys, int zs,
	int* xD, int* yD, int* zD, int damage,
	OneObject* Sender, byte AttType )
{
	int cx = -1;
	int cy = -1;

	int dx = *xD - xs;
	int dy = *yD - ys;
	int dz = *zD - zs;

	int Len = sqrt( dx*dx + dy*dy + dz*dz );
	int N = ( Len >> 4 ) + 1;
	int N2 = N + N;
	int MinR = 10000;
	word BestID = 0xFFFF;
	int SMID = 0xFFFF;
	byte MASK = 0;
	if ( Sender )
	{
		SMID = Sender->Index;
		MASK = Sender->NMask;
	}

	for ( int i = 0; i < N2; i++ )
	{
		int xx = ( dx*i ) / N;
		int yy = ( dy*i ) / N;
		int zz = zs + ( dz*i ) / N;
		int ccx = ( xs + xx ) >> 7;
		int ccy = ( ys + yy ) >> 7;
		if ( cx != ccx || cy != ccy )
		{
			cx = ccx;
			cy = ccy;
			if ( cx >= 0 && cy >= 0 && cx < VAL_MAXCX - 1 && cy < VAL_MAXCX - 1 )
			{
				int cell = 1 + VAL_MAXCX + ( cy << VAL_SHFCX ) + cx;
				int NMon = MCount[cell];
				if ( NMon )
				{
					int ofs1 = cell << SHFCELL;
					word MID;
					for ( int i = 0; i < NMon; i++ )
					{
						MID = GetNMSL( ofs1 + i );
						if ( MID != 0xFFFF && MID != SMID )
						{
							OneObject* OB = Group[MID];
							if ( OB && !OB->Sdoxlo )
							{
								int ux = OB->RealX >> 4;
								int uy = OB->RealY >> 4;
								int R = Norma( ux - xs, uy - ys );
								int dz = zz - OB->RZ;
								int minR = 80;

								if ( OB->NMask&MASK )
								{
									minR = 250;
								}

								if ( dz > 0 && dz<90 && R>minR&&R < MinR )
								{
									int r = abs( ( ux - xs )*yy - ( uy - ys )*xx ) / Len;
									if ( r < OB->newMons->UnitRadius )
									{
										BestID = MID;
										MinR = R;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if ( BestID != 0xFFFF )
	{
		OneObject* OB = Group[BestID];
		*xD = ( OB->RealX >> 4 );
		*yD = ( OB->RealY >> 4 );
		if ( Sender&&AttType < NAttTypes )
		{
			int RR = Norma( *xD - xs, *yD - ys );
			damage = int( float( damage )*exp( -0.693147*float( RR ) / float( Sender->newMons->DamageDecr[AttType] ) ) );
		}
		OB->MakeDamage( damage, damage, Sender, AttType );
		return true;
	}
	return false;
}