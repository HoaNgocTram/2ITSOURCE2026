#include "stdafx.h"

#include "ZGame.h"
#include "ZScreenEffectManager.h"
#include "RealSpace2.h"
#include "MDebug.h"
#include "Mint.h"
#include "ZApplication.h"
#include "ZSoundEngine.h"
#include "ZMyInfo.h"
#include "ZQuest.h"
#include "ZRuleDuel.h"
#include "ZRuleDeathMatch.h"
#include "ZRuleQuestChallenge.h"
#include "ZActorWithFSM.h"
#include "RGMain.h"
void DrawGauge(float x,float y,float fWidth,float fHeight,float fLeanDir,DWORD color);
void DrawCircleEXP(float x, float y, float radius, float fPercent);

ZScreenEffect::ZScreenEffect(RMesh *pMesh,rvector offset)
{
	m_nDrawMode = ZEDM_NONE ;
	m_fDist = 0.f;

	m_Offset=offset;

	//	m_VMesh.m_bRenderInstantly = true;
	m_VMesh.Create(pMesh);
	m_VMesh.SetAnimation("play");
	m_VMesh.SetCheckViewFrustum(false);

}

bool ZScreenEffect::Draw(u64 nTime)
{

	return DrawCustom(nTime, m_Offset);
}

void ZScreenEffect::Update()
{
	m_VMesh.Frame();
}

bool ZScreenEffect::IsDeleteTime()
{
	if(m_VMesh.isOncePlayDone())
		return true;
	return false;
}

bool ZScreenEffect::DrawCustom(u64 nTime, const rvector& vOffset, float fAngle)
{
	RGetDevice()->SetRenderState(D3DRS_ZENABLE, FALSE);
	RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	rmatrix World;
	D3DXMatrixIdentity(&World);

	if (fAngle != 0.0f)
	{
		D3DXMatrixRotationZ(&World, fAngle);
	}

	rmatrix View,Offset,Scale;
	D3DXMatrixIdentity(&Scale);
	if( RGetIsWidthScreen() )
	{
		const rvector eye(0,0,-780),at(0,0,0),up(0,1,0);
		D3DXMatrixLookAtLH(&View,&eye,&at,&up);
	}
	else if (RGetWidthScreen() != 0.75f)
	{
		const rvector eye(0,0, -(975.0f/RGetWidthScreen()/2.0f)),at(0,0,0),up(0,1,0);
		D3DXMatrixLookAtLH(&View,&eye,&at,&up);
		D3DXMatrixScaling(&Scale, (float)RGetScreenWidth() / ((float)RGetScreenHeight() * (4.0f/3.0f)), 1, 1);
		//D3DXMatrixScaling(&Scale, (float)RGetScreenWidth() / ((float)RGetScreenHeight() * (4.0f/3.0f)), 1, 1);
	}
	//if( RGetIsWidthScreen() )
	//{
	//	const rvector eye(0,0,-780),at(0,0,0),up(0,1,0);
	//	D3DXMatrixLookAtLH(&View,&eye,&at,&up);
	//	m_VMesh.SetScale(rvector(1,1,1));
	//}

	//else if( RGetIsLongScreen() )
	//{
	//	const rvector eye(0,0,-865),at(0,0,0),up(0,1,0); // debug
	//	//const rvector eye(0,0,-865),at(0,0,0),up(0,1,0); // eye(0,0,-865) recommended
	//	D3DXMatrixLookAtLH(&View,&eye,&at,&up);
	//	m_VMesh.SetScale(rvector(1.1f,1,1)); // debug
	//	//m_VMesh.SetScale(rvector(1.07f,1,1));
	//}
	else
	{
		const rvector eye(0,0,-650),at(0,0,0),up(0,1,0);
		D3DXMatrixLookAtLH(&View,&eye,&at,&up);
		//m_VMesh.SetScale(rvector(1,1,1));
	}
	D3DXMatrixTranslation(&Offset,vOffset.x,vOffset.y,vOffset.z);

	View=Offset*View;
	View=Scale*View;

	RGetDevice()->SetTransform( D3DTS_VIEW, &View );

	m_VMesh.SetWorldMatrix(World);
	//m_VMesh.Frame();
	m_VMesh.Render();

	if(m_VMesh.isOncePlayDone()) {
		return false;
	}

	return true;
}





///////////////////////////////////////////////////////////////////////////////////////
// ZScreenEffectLetterBox ///////////////////////////////////////////////////////////////////////
bool ZScreenEffectLetterBox::Draw(u64 nTime)
{
	RGetDevice()->SetRenderState(D3DRS_ZENABLE, FALSE);
	RGetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	rmatrix World;
	D3DXMatrixIdentity(&World);

	rmatrix View, Offset, Scale;
	D3DXMatrixIdentity(&Scale);
	if (RGetIsWidthScreen())
	{
		const rvector eye(0, 0, -780), at(0, 0, 0), up(0, 1, 0);
		D3DXMatrixLookAtLH(&View, &eye, &at, &up);
		m_VMesh.SetScale(rvector(5, 1, 1));
	}
	else if (RGetWidthScreen() != 0.75f)
	{
		const rvector eye(0, 0, -(975.0f / RGetWidthScreen() / 2.0f)), at(0, 0, 0), up(0, 1, 0);
		D3DXMatrixLookAtLH(&View, &eye, &at, &up);
		D3DXMatrixScaling(&Scale, (float)RGetScreenWidth() / ((float)RGetScreenHeight() * (4.0f / 3.0f)), 1, 1);
		View = Scale * View;
	}
	//else if( RGetIsLongScreen() )
	//{
	//	const rvector eye(0,0,-865),at(0,0,0),up(0,1,0); // debug
	//	//const rvector eye(0,0,-865),at(0,0,0),up(0,1,0); // eye(0,0,-865) recommended
	//	D3DXMatrixLookAtLH(&View,&eye,&at,&up);
	//	m_VMesh.SetScale(rvector(5,1,1));
	//}
	else
	{
		const rvector eye(0, 0, -650), at(0, 0, 0), up(0, 1, 0);
		D3DXMatrixLookAtLH(&View, &eye, &at, &up);
	}

	RGetDevice()->SetTransform(D3DTS_VIEW, &View);

	m_VMesh.SetWorldMatrix(World);
	//	m_VMesh.Frame();
	m_VMesh.Render();

	if (m_VMesh.isOncePlayDone()) {
		return false;
	}

	return true;
}



///////////////////////////////////////////////////////////////////////////////////////
// ZComboEffect ///////////////////////////////////////////////////////////////////////

ZComboEffect::ZComboEffect(RMesh *pMesh,rvector offset)
:ZScreenEffect(pMesh,offset)
{
	bDelete=false;
	// ÃÖ¼ÒÇÑ ÀÌÆåÆ® ±æÀÌ´Â 10ÃÊ´Â ³ÑÁö¾Ê´Â´Ù
	fDeleteTime=ZGetGame()->GetTime()+10.f;
}

void ZComboEffect::SetFrame(int nFrame)
{
	AniFrameInfo* pInfo = GetVMesh()->GetFrameInfo(ani_mode_lower);

	pInfo->m_nFrame =
		//		max(min(GetVMesh()->m_nFrame[0],2400),nFrame);
		max(min(pInfo->m_nFrame,2400),nFrame);
	fDeleteTime=ZGetGame()->GetTime()+10.f;
}

bool ZComboEffect::Draw(u64 nTime)
{
	if (bDelete && ZGetGame()->GetTime() >= fDeleteTime)
		return false;

	ZScreenEffect::Draw(nTime);
	return true;
}

void ZComboEffect::DeleteAfter(float fTime)
{
	bDelete=true;
	fDeleteTime=ZGetGame()->GetTime()+fTime;
}

///////////////////////////////////////////////////////////////////////////////////////
// ZBossGaugeEffect ///////////////////////////////////////////////////////////////////


ZBossGaugeEffect::ZBossGaugeEffect(RMesh *pMesh,rvector offset)
:ZScreenEffect(pMesh,offset), m_bShocked(false), m_fShockStartTime(0.0f),
m_fShockPower(0.0f), m_fLastTime(0.0f), m_ShockOffset(0.0f, 0.0f, 0.0f), m_ShockVelocity(0.0f, 0.0f, 0.0f),
m_nVisualValue(-1)
{


}


void ZBossGaugeEffect::Shock(float fPower)
{
	m_bShocked = true;

	m_fShockStartTime=ZGetGame()->GetTime();
	m_fLastTime = ZGetGame()->GetTime();
	m_fShockPower = max((min(20.0f, 20.0f + fPower)), 70.0f);
}

bool ZBossGaugeEffect::Draw(u64 nTime)
{
	MUID uidBoss;
	if(ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
		uidBoss = ZGetQuest()->GetGameInfo()->GetBoss();
	else
	{
		ZRuleQuestChallenge* pCqRule = NULL;
		if(ZGetGame()->GetMatch()->GetRule())
			pCqRule = (ZRuleQuestChallenge*)ZGetGame()->GetMatch()->GetRule();

		if(pCqRule)
		{
			uidBoss = pCqRule->GetBoss();
		}
	}

	if (uidBoss == MUID(0,0)) return true;

	const float fShockDuration=	0.5f;
	const rvector ShockOffset=rvector(0,0,0);
	const rvector ShockVelocity=rvector(0,0,0);

	rvector offset = rvector(0.0f, 0.0f, 0.0f);

	float fElapsed = ZGetGame()->GetTime() - m_fLastTime;

	if (m_bShocked)
	{
		float fA=RandomNumber(0.0f, 1.0f)*2*pi;
		float fB=RandomNumber(0.0f, 1.0f)*2*pi;
		rvector velocity=rvector(cos(fA)*cos(fB), sin(fA)*sin(fB), 0.0f);

		float fPower=(ZGetGame()->GetTime() - m_fShockStartTime) / fShockDuration;
		if(fPower>1.f)
		{
			m_bShocked=false;
		}
		else
		{
			fPower=1.f-fPower;
			fPower=pow(fPower,1.5f);
			m_ShockVelocity = (RandomNumber(0.0f, 1.0f) * m_fShockPower * velocity);
			m_ShockOffset += fElapsed * m_ShockVelocity;
			offset = fPower * m_ShockOffset;
			/*
			char text[256];
			sprintf(text, "%.3f, %.3f\n", offset.x, offset.y);
			OutputDebugString(text);
			*/
		}
	}

	m_fLastTime = ZGetGame()->GetTime();
	offset.z = 0.0f;


	bool ret = ZScreenEffect::DrawCustom(0, offset);

	// HP°ÔÀÌÁE
	ZObject* pBoss = ZGetObjectManager()->GetObject(uidBoss);
	if ((pBoss) && (pBoss->IsNPC()))
	{
		int nMax = 0;
		int nCurr = 0;
		if(ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
		{
			ZActor* pBossActor = (ZActor*)pBoss;
			nMax = pBossActor->GetActualMaxHP()/* + pBossActor->GetActualMaxAP()*/;
			nCurr = min(pBossActor->GetHP() /*+ pBossActor->GetAP()*/, nMax);
		}
		else
		{
			ZActorWithFSM* pBossActor = (ZActorWithFSM*)pBoss;
			nMax = pBossActor->GetActualMaxHP()/* + pBossActor->GetActualMaxAP()*/;
			nCurr = min(pBossActor->GetActualHP() /*+ pBossActor->GetAP()*/, nMax);
		}

		if ((m_nVisualValue < 0) || (m_nVisualValue > nCurr) || (nCurr - m_nVisualValue > 100))
		{
			m_nVisualValue = nCurr;
		}

		if (m_nVisualValue > 0)
		{
			const int width = 433+1;
			const int height = 12;

			int x = (800 - width) * 0.5f;
			int y = 600 * 0.028f;

			float fGaugeWidth = width * (m_nVisualValue / (float)nMax);

			DWORD color = D3DCOLOR_ARGB(255, 0xBB, 0, 0);

			RGetDevice()->SetFVF( D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE );
			RGetDevice()->SetTexture(0,NULL);

			float fx = 183.0f/800.0f + offset.x/980.0f;
			float fy = 574.0f/600.0f - offset.y/720.0f;

			DrawGauge(fx, fy, fGaugeWidth/800.0f, 7.0f/600.0f, 0.0f, color);
		}
	}


	return ret;
}


///////////////////////////////////////////////////////////////////////////////////////
// ZKOEffect //////////////////////////////////////////////////////////////////////////

ZKOEffect::ZKOEffect(RMesh* pMesh,rvector offset) : ZScreenEffect(pMesh, offset)
{

}

void ZKOEffect::InitFrame()
{
	m_VMesh.Stop(ani_mode_lower);
	m_VMesh.GetMesh()->SetFrame(0 , 0);
	m_VMesh.Play(ani_mode_lower);
}

void ZKOEffect::SetFrame(int nFrame)
{
	m_VMesh.GetMesh()->SetFrame(nFrame , 0);
}

int ZKOEffect::GetFrame()
{
	AniFrameInfo* pInfo = GetVMesh()->GetFrameInfo(ani_mode_lower);
	return pInfo->m_nFrame;
}

///////////////////////////////////////////////////////////////////////////////////////
// ZTDMBlinkEffect ////////////////////////////////////////////////////////////////////

ZTDMBlinkEffect::ZTDMBlinkEffect(RMesh* pMesh,rvector offset) : ZScreenEffect(pMesh, offset)
{
}

void ZTDMBlinkEffect::SetAnimationSpeed(int nKillsDiff)
{
	float speed = 4.8f;
	if (nKillsDiff > 5)			// Á¨ÀE_-; ÇÏµåÄÚµE
		speed = 9.6f;

	m_VMesh.SetSpeed(speed);
}

///////////////////////////////////////////////////////////////////////////////////////
// ZScreenEffectManager ///////////////////////////////////////////////////////////////
ZScreenEffectManager::ZScreenEffectManager()
{

	m_WeaponType = MWT_NONE;
	m_SelectItemDesc = NULL;

	m_pGaugeTexture = NULL;

	m_pHPPanelNew = NULL; //Custom: Toggle change bar HP
	m_pHPPanel = NULL;
	m_pHPPanelEffect = NULL;
	m_pScorePanel = NULL;
	m_pBuffPanel = NULL;


	m_pScorePanelTeam = NULL;
	m_pScorePanelSolo = NULL;

	m_pReload = NULL;
	m_pEmpty = NULL;

	for(int i=0;i<MWT_END;i++)
		m_pWeaponIcons[i]=NULL;

	m_pEffectMeshMgr=NULL;

	m_pSpectator = NULL;

	m_bGameStart = false;
	m_nHpReset = 0;

	m_bShowReload = false;
	m_bShowEmpty = false;

	m_pQuestEffectMeshMgr = NULL;
	m_pBossHPPanel = NULL;
	m_pArrow = NULL;
	for (int i = 0; i < 10; i++) m_pKONumberEffect[i] = NULL;
	m_pKO = NULL;
	m_nKO = 0;
}

ZScreenEffectManager::~ZScreenEffectManager()
{
	DestroyQuestRes();
	Destroy();
}

void ZScreenEffectManager::Destroy()
{
	Clear();

	if(m_pGaugeTexture)
	{
		RDestroyBaseTexture(m_pGaugeTexture);
		m_pGaugeTexture=NULL;
	}

	SAFE_DELETE(m_pSpectator);

	SAFE_DELETE(m_pHPPanelNew); //Custom: Toggle change bar HP
	SAFE_DELETE(m_pHPPanel);

	SAFE_DELETE(m_pHPPanelEffect);
	SAFE_DELETE(m_pScorePanel);
	SAFE_DELETE(m_pBuffPanel);

	SAFE_DELETE(m_pScorePanelTeam);
	SAFE_DELETE(m_pScorePanelSolo);

	SAFE_DELETE(m_pReload);
	SAFE_DELETE(m_pEmpty);

	SAFE_DELETE(m_pWeaponIcons[MWT_DAGGER]);
	SAFE_DELETE(m_pWeaponIcons[MWT_DUAL_DAGGER]);
	SAFE_DELETE(m_pWeaponIcons[MWT_KATANA]);
	SAFE_DELETE(m_pWeaponIcons[MWT_GREAT_SWORD]);
	SAFE_DELETE(m_pWeaponIcons[MWT_DOUBLE_KATANA]);
	SAFE_DELETE(m_pWeaponIcons[MWT_SPYCASE]);

	SAFE_DELETE(m_pWeaponIcons[MWT_PISTOL]);
	SAFE_DELETE(m_pWeaponIcons[MWT_PISTOLx2]);
	SAFE_DELETE(m_pWeaponIcons[MWT_REVOLVER]);
	SAFE_DELETE(m_pWeaponIcons[MWT_REVOLVERx2]);
	SAFE_DELETE(m_pWeaponIcons[MWT_SMG]);

	SAFE_DELETE(m_pWeaponIcons[MWT_SMGx2]);
	SAFE_DELETE(m_pWeaponIcons[MWT_SHOTGUN]);
	SAFE_DELETE(m_pWeaponIcons[MWT_SAWED_SHOTGUN]);

	SAFE_DELETE(m_pWeaponIcons[MWT_RIFLE]);
	SAFE_DELETE(m_pWeaponIcons[MWT_MACHINEGUN]);
	SAFE_DELETE(m_pWeaponIcons[MWT_ROCKET]);
	SAFE_DELETE(m_pWeaponIcons[MWT_SNIFER]);

	SAFE_DELETE(m_pWeaponIcons[MWT_MED_KIT]);
	SAFE_DELETE(m_pWeaponIcons[MWT_REPAIR_KIT]);
	SAFE_DELETE(m_pWeaponIcons[MWT_BULLET_KIT]);
	SAFE_DELETE(m_pWeaponIcons[MWT_FLASH_BANG]);
	SAFE_DELETE(m_pWeaponIcons[MWT_FRAGMENTATION]);
	SAFE_DELETE(m_pWeaponIcons[MWT_SMOKE_GRENADE]);
	SAFE_DELETE(m_pWeaponIcons[MWT_FOOD]);
	SAFE_DELETE(m_pWeaponIcons[MWT_POTION]);
	SAFE_DELETE(m_pWeaponIcons[MWT_TRAP]);
	SAFE_DELETE(m_pWeaponIcons[MWT_DYNAMITYE]);
	SAFE_DELETE(m_pWeaponIcons[MWT_STUNGRENADE]);
	SAFE_DELETE(m_pWeaponIcons[MWT_LANDMINE]);

	SAFE_DELETE(m_pWeaponIcons[MWT_TRAP_SPY]);
	SAFE_DELETE(m_pWeaponIcons[MWT_STUN_GRENADE_SPY]);
	SAFE_DELETE(m_pWeaponIcons[MWT_LANDMINE_SPY]);
	SAFE_DELETE(m_pWeaponIcons[MWT_FLASH_BANG_SPY]);
	SAFE_DELETE(m_pWeaponIcons[MWT_SMOKE_GRENADE_SPY]);

#ifdef _PORTALGUN 1
	SAFE_DELETE(m_pWeaponIcons[MWT_PORTAL_GUN]);
#endif

	for (ItorWeaponIconPotion it=m_mapWeaponIconPotion.begin(); it!=m_mapWeaponIconPotion.end(); ++it)
		delete it->second;
	m_mapWeaponIconPotion.clear();

	for (ItorWeaponIconTrap it=m_mapWeaponIconTrap.begin(); it!=m_mapWeaponIconTrap.end(); ++it)
		delete it->second;
	m_mapWeaponIconTrap.clear();

	SAFE_DELETE(m_pKO);

	for (int i = 0; i < 10; i++) 
	{
		SAFE_DELETE(m_pKONumberEffect[i]);
	}

	SAFE_DELETE(m_pTDScoreBoard);
	SAFE_DELETE(m_pTDScoreBlink_R);
	SAFE_DELETE(m_pTDScoreBlink_B);


	SAFE_DELETE(m_pEffectMeshMgr);	
	SAFE_DELETE(m_pQuestEffectMeshMgr);

	
}

void ZScreenEffectManager::Clear()
{
	m_eraseQueue.clear();
	while(!empty())
	{
		delete *begin();
		pop_front();
	}

	for(int i=0;i<COMBOEFFECTS_COUNT;i++)
		m_pComboEffects[i]=NULL;
}


bool ZScreenEffectManager::Create()
{

	DWORD _begin_time,_end_time;
#define BEGIN_ { _begin_time = timeGetTime(); }
#define END_(x) { _end_time = timeGetTime(); float f_time = (_end_time - _begin_time) / 1000.f; mlog("%s : %f \n", x,f_time ); }

	BEGIN_;

	m_pQuestEffectMeshMgr = new RMeshMgr();
	if(m_pQuestEffectMeshMgr->LoadXmlList("interface/default/combat/screeneffects_quest.xml")==-1) {
		mlog("quest combat list loding error\n");
		SAFE_DELETE(m_pQuestEffectMeshMgr);
		return false;
	}

	m_pEffectMeshMgr = new RMeshMgr;
	if(m_pEffectMeshMgr->LoadXmlList("interface/default/combat/screeneffects.xml")==-1) {
		mlog("combat list loding error\n");
		SAFE_DELETE(m_pEffectMeshMgr);
		return false;
	}

	m_pHPPanel = new ZScreenEffect(m_pEffectMeshMgr->Get("panelold"));
	m_pHPPanelNew = new ZScreenEffect(m_pEffectMeshMgr->Get("panelnew")); //Custom: Toggle change bar HP

	m_pHPPanelEffect = new ZScreenEffect(m_pEffectMeshMgr->Get("hppaneleffect"));
	m_pScorePanel = new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_tab.elu"));
	m_pBuffPanel = new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_tab.elu"));

	m_pScorePanelTeam = new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_tab_tdm.elu"));
	m_pScorePanelSolo = new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_tab_new.elu"));

	m_pSpectator = new ZScreenEffectLetterBox(m_pEffectMeshMgr->Get("spectator"));

	m_pEmpty	= new ZScreenEffect(m_pEffectMeshMgr->Get("empty"));
	m_pReload	= new ZScreenEffect(m_pEffectMeshMgr->Get("reload"));

	m_pWeaponIcons[MWT_DAGGER]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_dagger.elu"));
	m_pWeaponIcons[MWT_DUAL_DAGGER]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_d_dagger.elu"));
	m_pWeaponIcons[MWT_KATANA]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_katana.elu"));
	m_pWeaponIcons[MWT_GREAT_SWORD]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_sword.elu"));//±×³É ¿E± °°Àº°Í »ç¿E
	m_pWeaponIcons[MWT_DOUBLE_KATANA]	= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_blade.elu"));//±×³É ¿E± °°Àº°Í »ç¿E
	m_pWeaponIcons[MWT_SPYCASE] = new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_spy_spycase.elu"));

	m_pWeaponIcons[MWT_PISTOL]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_pistol.elu"));
	m_pWeaponIcons[MWT_PISTOLx2]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_d_pistol.elu"));
	m_pWeaponIcons[MWT_REVOLVER]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_pistol.elu"));
	m_pWeaponIcons[MWT_REVOLVERx2]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_d_pistol.elu"));
	m_pWeaponIcons[MWT_SMG]				= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_smg.elu"));

	m_pWeaponIcons[MWT_SMGx2]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_d_smg.elu"));
	m_pWeaponIcons[MWT_SHOTGUN]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_shotgun.elu"));
	m_pWeaponIcons[MWT_SAWED_SHOTGUN]	= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_shotgun.elu"));

	m_pWeaponIcons[MWT_RIFLE]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_rifle.elu"));
	m_pWeaponIcons[MWT_MACHINEGUN]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_machinegun.elu"));
	m_pWeaponIcons[MWT_ROCKET]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_rocket.elu"));
	m_pWeaponIcons[MWT_SNIFER]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_rifle.elu"));

#ifdef _PORTALGUN 1
	m_pWeaponIcons[MWT_PORTAL_GUN] = new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_rocket.elu"));
#endif

	m_pWeaponIcons[MWT_MED_KIT]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_medikit.elu"));
	m_pWeaponIcons[MWT_REPAIR_KIT]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_repairkit.elu"));
	m_pWeaponIcons[MWT_FLASH_BANG]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_flashbang.elu"));
	m_pWeaponIcons[MWT_FRAGMENTATION]	= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_grenade.elu"));
	m_pWeaponIcons[MWT_SMOKE_GRENADE]	= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_flashbang.elu"));
	m_pWeaponIcons[MWT_FOOD]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_food.elu"));
	m_pWeaponIcons[MWT_BULLET_KIT]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_magazine.elu"));

	// Æ÷¼Ç°EÆ®·¦Àº ´Ù¸¥ ¹è¿­¿¡ °E®ÇÑ´Ù
	m_pWeaponIcons[MWT_POTION]			= NULL;
	m_pWeaponIcons[MWT_TRAP]			= NULL;

	m_pWeaponIcons[MWT_DYNAMITYE]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_exgrenade.elu"));
	m_pWeaponIcons[MWT_LANDMINE] = new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_spy_landmine.elu"));

	//SpyItems 
	m_pWeaponIcons[MWT_STUNGRENADE] = new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_spy_stungrenade.elu"));
	m_pWeaponIcons[MWT_FLASH_BANG_SPY] = new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_flashbang.elu"));
	m_pWeaponIcons[MWT_SMOKE_GRENADE_SPY] = new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_flashbang.elu"));
	m_pWeaponIcons[MWT_TRAP_SPY] = new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_frozenfield.elu"));
	m_pWeaponIcons[MWT_STUN_GRENADE_SPY] = new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_spy_stungrenade.elu"));
	m_pWeaponIcons[MWT_LANDMINE_SPY] = new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_spy_landmine.elu"));


	// Æ÷¼Ç ÃÊ±âÈ­
	m_mapWeaponIconPotion[MMIEI_POTION_HEAL_INSTANT]	= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_hp_capsule.elu"));
	m_mapWeaponIconPotion[MMIEI_POTION_REPAIR_INSTANT]	= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_ap_capsule.elu"));
	m_mapWeaponIconPotion[MMIEI_POTION_HEAL_OVERTIME]	= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_hp_ampulla.elu"));
	m_mapWeaponIconPotion[MMIEI_POTION_REPAIR_OVERTIME]	= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_ap_ampulla.elu"));
	m_mapWeaponIconPotion[MMIEI_POTION_HASTE]			= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_haste_ampulla.elu"));

	// Æ®·¦ ÃÊ±âÈ­
	m_mapWeaponIconTrap[MMDT_FIRE]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_firefield.elu"));
	m_mapWeaponIconTrap[MMDT_COLD]		= new ZScreenEffect(m_pEffectMeshMgr->Get("ef_in_icon_frozenfield.elu"));


	m_pHit = m_pEffectMeshMgr->Get("hit");
	m_pHit = m_pEffectMeshMgr->Get("hitmaker");
	m_pComboBeginEffect = m_pEffectMeshMgr->Get("combo_begin");
	m_pComboEndEffect = m_pEffectMeshMgr->Get("combo_end");
	for(int i=0;i<10;i++)
	{
		char meshname[256];
		sprintf(meshname,"combo%d",i);
		m_pComboNumberEffect[i] = m_pEffectMeshMgr->Get(meshname);

		sprintf(meshname,"exp%d",i);
		m_pExpNumberEffect[i] = m_pEffectMeshMgr->Get(meshname);
#ifdef _FLOATDMG
		sprintf(meshname, "dmg%d", i);
		m_pDmgNumberEffect[i] = m_pEffectMeshMgr->Get(meshname);
#endif
	}
#ifdef _FLOATDMG
	m_pDmgPlusEffect = m_pEffectMeshMgr->Get("dmg+");
	m_pDmgMinusEffect = m_pEffectMeshMgr->Get("dmg-");
#endif
	m_pExpPlusEffect = m_pEffectMeshMgr->Get("exp+");
	m_pExpMinusEffect = m_pEffectMeshMgr->Get("exp-");

	for(int i=0;i<COMBOEFFECTS_COUNT;i++)
		m_pComboEffects[i]=NULL;

	m_pPraiseEffect[0] = m_pEffectMeshMgr->Get("allkill");
	m_pPraiseEffect[1] = m_pEffectMeshMgr->Get("unbelievable");
	m_pPraiseEffect[2] = m_pEffectMeshMgr->Get("excellent");
	m_pPraiseEffect[3] = m_pEffectMeshMgr->Get("fantastic");
	m_pPraiseEffect[4] = m_pEffectMeshMgr->Get("headshot");
	m_pPraiseEffect[5] = m_pEffectMeshMgr->Get("firstblood");

	m_pGoodEffect = m_pEffectMeshMgr->Get("good");
	m_pNiceEffect = m_pEffectMeshMgr->Get("nice");
	m_pGreatEffect = m_pEffectMeshMgr->Get("great");
#ifdef _PERFECT
	m_pPerfect = m_pEffectMeshMgr->Get("perfect");
#endif
	m_pWonderfullEffect = m_pEffectMeshMgr->Get("wonderful");

	m_pCoolEffect = m_pEffectMeshMgr->Get("cool");

	m_pAlertEffect[0] = m_pEffectMeshMgr->Get("alert_front");
	m_pAlertEffect[1] = m_pEffectMeshMgr->Get("alert_right");
	m_pAlertEffect[2] = m_pEffectMeshMgr->Get("alert_back");
	m_pAlertEffect[3] = m_pEffectMeshMgr->Get("alert_left");

	m_CurrentComboLevel=ZCL_NONE;

	m_pGaugeTexture=RCreateBaseTexture("Interface/Default/COMBAT/gauge.bmp");

	m_fGaugeHP=m_fGaugeAP=m_fGaugeEXP =0.f;
	m_fCurGaugeHP=m_fCurGaugeAP=-1.f;

	m_pKO = new ZKOEffect(m_pQuestEffectMeshMgr->Get("ko"));
	for (int i = 0; i < 10; i++)
	{
		char name[64];
		sprintf(name, "ko%d", i);
		m_pKONumberEffect[i] = new ZKOEffect(m_pQuestEffectMeshMgr->Get(name));
	}

	m_pTDScoreBoard = new ZScreenEffect(m_pEffectMeshMgr->Get("td_scoreboard"));
	m_pTDScoreBlink_B = new ZTDMBlinkEffect(m_pEffectMeshMgr->Get("td_scoreblink_b"));
	m_pTDScoreBlink_R = new ZTDMBlinkEffect(m_pEffectMeshMgr->Get("td_scoreblink_r"));


	END_("Screen Effect Manager Create");
	return true;
}

#ifdef _FLOATDMG
void ZScreenEffectManager::AddDamegeEffect(int nDmg)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bFloatdmg)
	{
		if (nDmg == 0)
			return;

		ZEffect* pNew = NULL;

		char buffer[32];
		sprintf(buffer, "%d", abs(nDmg));
		int nCount = (int)strlen(buffer);


		for (int i = 0; i < nCount; i++)
		{
			float fOffset = 30.f * (float)(i - nCount + 2);
			Add(new ZScreenEffect(m_pDmgNumberEffect[buffer[i] - '0'], rvector(fOffset + 425.f, -230.f, 295.f)));
		}
		float fOffset = 30.f * (float)(3 - nCount);
		Add(new ZScreenEffect(nDmg > 0 ? m_pDmgPlusEffect : m_pDmgMinusEffect, rvector(fOffset + 425.f, -230.f, 295.f)));
	}
}
#endif

#ifdef _PERFECT
void ZScreenEffectManager::Perfect(int Perf)
{
	ZEffect* pNew = new ZScreenEffect(Perf > 0 ? m_pPerfect : m_pPerfect, rvector(-210.f, -330.f, 450.f));
	Add(pNew);
	ZGetGameInterface()->PlayVoiceSound(VOICE_PERFECT, 1200);

}
#endif

void ZScreenEffectManager::Add(ZEffect *pEffect)
{	
	push_back(pEffect); 
}

void DrawGauge(float x,float y,float fWidth,float fHeight,float fLeanDir,DWORD color)
{
	if( RGetIsWidthScreen() )
	{
		x = (x*800 + 80)/960;
		fWidth = fWidth*800/960;
	}
	//else if( RGetIsLongScreen() )
	//{
	//	x= (x*800+83)/960;// debug
	//	//x = (x*800 + 90)/960;
	//	fWidth = fWidth*800/960;
	//}

	struct TLVERTEX {
		float x, y, z, w;
		DWORD color;
		float u,v;
	} ;

	TLVERTEX ver[4];

#define SETVERTEX(_a,_x,_y,_z,_u,_v,_color) { ver[_a].x=_x;ver[_a].y=_y;ver[_a].z=_z;ver[_a].u=_u;ver[_a].v=_v;ver[_a].color=_color;  ver[_a].w=.1f; }

	float fLean=fHeight*(float)MGetWorkspaceHeight()*fLeanDir;

	int x1,y1,x2,y2;
	x1=x*(float)MGetWorkspaceWidth();
	y1=y*(float)MGetWorkspaceHeight();
	x2=(x+fWidth)*(float)MGetWorkspaceWidth();
	y2=(y+fHeight)*(float)MGetWorkspaceHeight();

	SETVERTEX(0,x1		,y1,0,	0,0,color);
	SETVERTEX(1,x2		,y1,0,	1,0,color);
	SETVERTEX(2,x1+fLean,y2,0,	0,1,color);
	SETVERTEX(3,x2+fLean,y2,0,	1,1,color);

	HRESULT hr=RGetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,ver,sizeof(TLVERTEX));
}

void DrawCircleEXP(float centerX, float centerY, float radius, float fPercent)
{
	if (fPercent <= 0.0f) return;

	struct VERTEX { float x, y, z, rhw; DWORD color; };
	const int segments = 64;
	VERTEX v[segments + 1];

	// Màu xanh Neon chuẩn EXP Masang
	DWORD colorEXP = D3DCOLOR_ARGB(255, 30, 247, 255);

	int nActiveSegments = (int)(segments * fPercent);
	if (nActiveSegments < 1) nActiveSegments = 1;

	for (int i = 0; i <= nActiveSegments; i++) {
		// Vẽ từ góc -90 độ (đỉnh 12 giờ)
		float angle = ((float)i / segments) * (D3DX_PI * 2.0f) - (D3DX_PI / 2.0f);
		v[i].x = centerX + cos(angle) * radius;
		v[i].y = centerY + sin(angle) * radius;
		v[i].z = 0.0f;
		v[i].rhw = 1.0f;
		v[i].color = colorEXP;
	}

	RGetDevice()->SetTexture(0, NULL);
	RGetDevice()->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	RGetDevice()->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);
	RGetDevice()->DrawPrimitiveUP(D3DPT_LINESTRIP, nActiveSegments, v, sizeof(VERTEX));
}

void ZScreenEffectManager::ReSetHpPanel()
{
	if (m_pHPPanel->GetVMesh())
	{

		AniFrameInfo* pAni = m_pHPPanel->GetVMesh()->GetFrameInfo(ani_mode_lower);

		pAni->m_nFrame = 0;

		if (pAni->m_isPlayDone) {
			pAni->m_isPlayDone = false;
		}

		pAni->m_pAniSet = NULL;

		m_pHPPanel->GetVMesh()->SetAnimation("play");
	}

	if(m_pHPPanelEffect->GetVMesh()) 
	{

		AniFrameInfo* pAni = m_pHPPanelEffect->GetVMesh()->GetFrameInfo( ani_mode_lower );

		pAni->m_nFrame = 0;

		if( pAni->m_isPlayDone ) {
			pAni->m_isPlayDone = false;
		}

		pAni->m_pAniSet = NULL;

		m_pHPPanel->GetVMesh()->SetAnimation("play");
	}

	m_nHpReset = 1;
}
//Custom: Toggle change bar HP
void ZScreenEffectManager::ReSetHpPanelNew()
{
	if (m_pHPPanelNew->GetVMesh())
	{

		AniFrameInfo* pAni = m_pHPPanelNew->GetVMesh()->GetFrameInfo(ani_mode_lower);

		pAni->m_nFrame = 0;

		if (pAni->m_isPlayDone) {
			pAni->m_isPlayDone = false;
		}

		pAni->m_pAniSet = NULL;

		m_pHPPanelNew->GetVMesh()->SetAnimation("play");
	}

	if (m_pHPPanelEffectNew->GetVMesh())
	{

		AniFrameInfo* pAni = m_pHPPanelEffectNew->GetVMesh()->GetFrameInfo(ani_mode_lower);

		pAni->m_nFrame = 0;

		if (pAni->m_isPlayDone) {
			pAni->m_isPlayDone = false;
		}

		pAni->m_pAniSet = NULL;

		m_pHPPanelNew->GetVMesh()->SetAnimation("play");
	}

	m_nHpResetNew = 1;
}

void ZScreenEffectManager::SetGauge_HP(float fHP) 
{
	if(m_fCurGaugeHP==-1)
		m_fCurGaugeHP = fHP;

	m_fGaugeHP=fHP; 

	if( m_fCurGaugeHP < m_fGaugeHP) {
		m_fCurGaugeHP = fHP;
	}
}

void ZScreenEffectManager::SetGauge_AP(float fAP)	
{
	if(m_fCurGaugeAP==-1)
		m_fCurGaugeAP = fAP;

	m_fGaugeAP=fAP; 

	if( m_fCurGaugeAP < m_fGaugeAP) {
		m_fCurGaugeAP = fAP;
	}
}

ZScreenEffect* ZScreenEffectManager::GetCurrWeaponImage()
{
	if(m_WeaponType == MWT_NONE) return NULL;

	if(m_WeaponType == MWT_POTION && m_SelectItemDesc)
	{
		ItorWeaponIconPotion it = m_mapWeaponIconPotion.find(m_SelectItemDesc->m_nEffectId);
		if (it != m_mapWeaponIconPotion.end())
			return it->second;
	}

	if(m_WeaponType == MWT_TRAP && m_SelectItemDesc)
	{
		ItorWeaponIconTrap it = m_mapWeaponIconTrap.find(m_SelectItemDesc->m_nDamageType.Ref());
		if (it != m_mapWeaponIconTrap.end())
			return it->second;
	}

	return m_pWeaponIcons[m_WeaponType];
}

//Custom: Toggle change bar HP
int ZScreenEffectManager::DrawResetGauges()
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bScore)
	{
		RGetDevice()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE);
		DWORD color = 0xffffffff;
		static float _hp = 0.f;
		static float _ap = 0.f;
		static DWORD _backtime = timeGetTime();
		DWORD newtime = timeGetTime();
		if (_backtime == 0)
			_backtime = newtime;
		DWORD delta = newtime - _backtime;
		_hp += 0.002f * delta;
		_ap += 0.002f * delta;
		_backtime = newtime;

		RGetDevice()->SetTexture(0, NULL);

		color = D3DCOLOR_ARGB(255, 0, 128, 255);
		DrawGauge(80.f / 800.f, 25.f / 600.f, min(1.f, m_fGaugeHP) * 150.f / 800.f, 15.f / 600.f, 0.f, color);
		// ap
		color = D3DCOLOR_ARGB(255, 68, 193, 62);
		DrawGauge(80.f / 800.f, 47.f / 600.f, min(1.f, m_fGaugeAP) * 150.f / 800.f, 15.f / 600.f, 0.f, color);
		// exp
		color = D3DCOLOR_ARGB(255, 200, 200, 200);
		DrawGauge(80.f / 800.f, 65.f / 600.f, min(1.f, m_fGaugeEXP) * 130.f / 800.f, 4.f / 600.f, 0.f, color);

		if (_hp > 1.0f)
		{
			_hp = 0.f;
			_ap = 0.f;
			_backtime = 0;
			return 0;
		}
	}
	else
	{
		RGetDevice()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE);

		DWORD color = 0xffffffff;
		static float _hp = 0.f;
		static float _ap = 0.f;
		static DWORD _backtime = timeGetTime();
		DWORD newtime = timeGetTime();

		if (_backtime == 0)
			_backtime = newtime;

		DWORD delta = newtime - _backtime;

		_hp += 0.002f * delta;
		_ap += 0.002f * delta;

		_backtime = newtime;

		if (m_pGaugeTexture)
			RGetDevice()->SetTexture(0, m_pGaugeTexture->GetTexture());
		else
			RGetDevice()->SetTexture(0, NULL);

		float adjustWidth = 800.f;
		color = D3DCOLOR_ARGB(255, 0, 128, 255);
		DrawGauge(70.f / adjustWidth, 23.f / 600.f, min(1.f, _hp) * 138.f / adjustWidth, 13.f / 600.f, 1.f, color);

		// ap
		color = D3DCOLOR_ARGB(255, 68, 193, 62);
		DrawGauge(84.f / adjustWidth, 50.f / 600.f, min(1.f, _ap) * 138.f / adjustWidth, 13.f / 600.f, -1.f, color);

		// exp
		color = D3DCOLOR_ARGB(255, 200, 200, 200);
		DrawGauge(66.f / adjustWidth, 70.f / 600.f, min(1.f, m_fGaugeEXP) * 138.f / adjustWidth, 4.f / 600.f, -1.f, color);

		if (_hp > 1.0f)
		{
			_hp = 0.f;
			_ap = 0.f;
			_backtime = 0;

			return 0;
		}
	}

	return 1;
}

//Custom: Toggle change bar HP
void ZScreenEffectManager::DrawGauges(MDrawContext* pDC)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bScore)
	{
		RGetDevice()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE);

		DWORD color = 0xffffffff;

		bool render_cur_hp = false;
		bool render_cur_ap = false;

		////////////////////////////////////////////

		if (m_fCurGaugeHP > 1.0f)
			m_fCurGaugeHP = 1.0f;

		if (m_fCurGaugeAP > 1.0f)
			m_fCurGaugeAP = 1.0f;

		if (m_fCurGaugeHP > m_fGaugeHP) {
			render_cur_hp = true;
			m_fCurGaugeHP -= 0.01f;// * (m_fCurGaugeHP / m_fGaugeHP);
		}
		else {
			m_fCurGaugeHP = m_fGaugeHP;
		}

		////////////////////////////////////////////

		if (m_fCurGaugeAP > m_fGaugeAP) {
			render_cur_ap = true;
			m_fCurGaugeAP -= 0.01f;// * (m_fCurGaugeAP / m_fGaugeAP);
		}
		else {
			m_fCurGaugeAP = m_fGaugeAP;
		}


		// ÇÏµåÄÚµå HP °ÔÀÌÁö !
		if (m_pGaugeTexture)
			RGetDevice()->SetTexture(0, m_pGaugeTexture->GetTexture());
		else
			RGetDevice()->SetTexture(0, NULL);

		// hp

		if (m_fGaugeHP == 1.0f)		color = D3DCOLOR_ARGB(255, 0, 128, 255);
		else if (m_fGaugeHP > 0.7f)	color = D3DCOLOR_ARGB(255, 69, 177, 186);
		else if (m_fGaugeHP > 0.3f)	color = D3DCOLOR_ARGB(255, 231, 220, 24);
		else						color = D3DCOLOR_ARGB(255, 233, 44, 22);

		DrawGauge(70.f / 800.f, 23.f / 600.f, min(1.f, m_fGaugeHP) * 138.f / 800.f, 13.f / 600.f, 1.f, color);

		// ap

		color = D3DCOLOR_ARGB(255, 68, 193, 62);

		DrawGauge(84.f / 800.f, 50.f / 600.f, min(1.f, m_fGaugeAP) * 138.f / 800.f, 13.f / 600.f, -1.f, color);

		// exp

		color = D3DCOLOR_ARGB(255, 200, 200, 200);

		DrawGauge(66.f / 800.f, 70.f / 600.f, min(1.f, m_fGaugeEXP) * 138.f / 800.f, 4.f / 600.f, -1.f, color);

		// alpha

		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		if (render_cur_hp) {

			color = 0x60ef0000;

			float x = min(1.f, m_fGaugeHP) * 138.f / 800.f;
			float w = (m_fCurGaugeHP - m_fGaugeHP) * 138.f / 800.f;

			DrawGauge(70.f / 800.f + x, 23.f / 600.f, w, 13.f / 600.f, 1.f, color);
		}

		if (render_cur_ap) {

			color = 0x60ef0000;

			float x = min(1.f, m_fGaugeAP) * 138.f / 800.f;
			float w = (m_fCurGaugeAP - m_fGaugeAP) * 138.f / 800.f;

			DrawGauge(84.f / 800.f + x, 50.f / 600.f, w, 13.f / 600.f, -1.f, color);
		}

		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}
	else // Bố cục Masang - Thu nhỏ tỉ lệ 2.4/1.8
	{
		float screenW = (float)RGetScreenWidth();
		float screenH = (float)RGetScreenHeight();

		// Tỉ lệ thu nhỏ
		float shrinkX = 2.4f;
		float shrinkY = 1.8f;

		// Gốc tọa độ mới (Dịch chuyển cả cụm UI)
		float virtualBX = 39.f;
		float virtualBY = 41.f;

		// Tỉ lệ scale tổng hợp
		float scaleX = (screenW / 800.0f) / shrinkX;
		float scaleY = (screenH / 600.0f) / shrinkY;
		float commonScale = scaleX; // Dùng chung scaleX để chống móp méo hình tròn

		pDC->SetBitmapColor(255, 255, 255, 255);

		// 1. Level Background (Hạ xuống thêm 5px ảo)
		MBitmap* pLevelBG = MBitmapManager::Get("MainTop_LevelBG.png");
		if (pLevelBG) {
			pDC->SetBitmap(pLevelBG);
			// Cộng thêm (5 * scaleY) để hạ thấp xuống cho cân với thanh HP
			pDC->Draw(virtualBX * scaleX, (virtualBY + 8.0f) * scaleY, 68 * scaleX, 68 * scaleY);
		}

		// 2. Status Boundary (Hạ xuống thêm 5px ảo để khớp với LevelBG)
		MBitmap* pBoundaryBtn = MBitmapManager::Get("Ingame_StatusBoundary.png");
		if (pBoundaryBtn) {
			pDC->SetBitmap(pBoundaryBtn);
			pDC->Draw((virtualBX + 76.f) * scaleX, (virtualBY + 8.0f) * scaleY, 4 * scaleX, 72 * scaleY);
		}

		// 3. HP/AP Background (Nền sọc)
		MBitmap* pBarBG = MBitmapManager::Get("Ingame_HPBar.png");
		if (pBarBG) {
			pDC->SetBitmap(pBarBG);
			pDC->Draw((virtualBX + 87.f) * scaleX, (virtualBY + 10.f) * scaleY, 366 * scaleX, 26 * scaleY);
			pDC->Draw((virtualBX + 87.f) * scaleX, (virtualBY + 56.f) * scaleY, 366 * scaleX, 26 * scaleY);
		}

		// --- HÀM VẼ THANH MÀU (Đã fix tọa độ khớp với tỉ lệ thu nhỏ) ---
		RGetDevice()->SetTexture(0, NULL);

		// Tọa độ lọt lòng chuẩn
		float barStartX = (virtualBX + 90.f) * scaleX;
		float barY_HP = (virtualBY + 13.f) * scaleY;
		float barY_AP = (virtualBY + 59.f) * scaleY;
		float barW = 360.f * scaleX;
		float barH = 20.f * scaleY;

		// Vẽ HP
		DWORD colHP;
		if (m_fGaugeHP > 0.8f)      colHP = D3DCOLOR_XRGB(220, 220, 220); // Trắng bạc (Máu đầy - Index 0)
		else if (m_fGaugeHP > 0.6f) colHP = D3DCOLOR_XRGB(255, 235, 60);  // Vàng (Index 1)
		else if (m_fGaugeHP > 0.3f) colHP = D3DCOLOR_XRGB(255, 179, 60);  // Cam (Index 2)
		else                        colHP = D3DCOLOR_XRGB(216, 81, 29);   // Đỏ gạch (Index 3)
		// Lưu ý: Nếu DrawGauge nhận 0.0-1.0, hãy chia cho screenW/screenH
		DrawGauge(barStartX / screenW, barY_HP / screenH, (m_fGaugeHP * barW) / screenW, barH / screenH, 0.f, colHP);

		// Vẽ AP
		float segW = 118.f * scaleX;
		float gap = 3.f * scaleX;
		for (int i = 0; i < 3; i++) {
			float v = min(1.0f, max(0.0f, (m_fGaugeAP * 3.0f) - (float)i));
			float segX = barStartX + (i * (segW + gap));
			DrawGauge(segX / screenW, barY_AP / screenH, (v * segW) / screenW, barH / screenH, 0.f, D3DCOLOR_XRGB(47, 119, 175));
		}

		// --- VÒNG TRÒN EXP (Dùng Pixel thực tế) ---
		// Tính toán tâm dựa trên ảnh MainTop_LevelBG đã hạ xuống 5px
		float expCenterX = (virtualBX * scaleX) + (34.0f * scaleX);
		float expCenterY = ((virtualBY + 8.0f) * scaleY) + (34.0f * scaleY);
		float expRadius = 33.0f * scaleX; // Tăng nhẹ bán kính lên 33 để ôm khít viền ngoài

		// GỌI HÀM: Không chia cho screenW/screenH nữa vì hàm DrawCircleEXP nhận pixel
		DrawCircleEXP(expCenterX, expCenterY, expRadius, m_fGaugeEXP);
	}

}
void ZScreenEffectManager::DrawReloadStatus(MDrawContext* pDC)
{
	if (!m_bShowReload && !m_bShowEmpty || pDC == NULL) return;

	float fTime = (float)GetTickCount() * 0.001f;

	// Công thức "Chớp tắt tử thần" - Nháy 2 lần mỗi giây (0.5s/chu kỳ)
	int nAlpha = 127 + (int)(128.0f * sin(fTime * 12.56f));

	// Giới hạn Alpha trong tầm an toàn 0-255
	if (nAlpha < 0) nAlpha = 0;
	if (nAlpha > 255) nAlpha = 255;

	MBitmap* pImg = (m_bShowReload) ? MBitmapManager::Get("Ingame_Reload.png") : MBitmapManager::Get("Ingame_Empty.png");

	if (pImg)
	{
		float fScaleX = (float)MGetWorkspaceWidth() / 1920.0f;
		float fScaleY = (float)MGetWorkspaceHeight() / 1080.0f;

		int nDrawX = MGetWorkspaceWidth() - (int)(914.0f * fScaleX);
		int nDrawY = MGetWorkspaceHeight() - (int)(555.0f * fScaleY);
		int nDrawW = (int)(112.0f * fScaleX);
		int nDrawH = (int)(34.0f * fScaleY);

		pDC->SetColor(255, 255, 255, nAlpha); // Áp dụng Alpha nhấp nháy
		pDC->SetBitmap(pImg);
		pDC->Draw(nDrawX, nDrawY, nDrawW, nDrawH);
		pDC->SetColor(255, 255, 255, 255); // Trả lại màu trắng chuẩn
	}
}
void ZScreenEffectManager::Draw(MDrawContext* pDC)
{
	ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if (!pTargetCharacter || !pTargetCharacter->GetInitialized()) return;

	// 1. Kiểm tra chế độ quan sát và ẩn UI
	if (ZGetCombatInterface()->GetObserverMode() || ZGetCombatInterface()->IsSkupUIDraw()) return;

	// 2. Logic kiểm tra đạn (Update trạng thái)
	ZItem* pSelectedItem = pTargetCharacter->GetItems()->GetSelectedWeapon();
	m_bShowReload = false;
	m_bShowEmpty = false;

	if (pSelectedItem && pSelectedItem->GetItemType() != MMIT_MELEE)
	{
		if (pSelectedItem->GetBulletCurrMagazine() <= 0)
		{
			// Nếu không còn đạn để thay -> EMPTY, ngược lại -> RELOAD
			if (pSelectedItem->GetBulletSpare() <= 0) {
				m_bShowEmpty = true;
			}
			else {
				m_bShowReload = true;
			}
		}
	}

	// 3. Vẽ các thành phần Screen Effect
	// A. Vẽ Reload/Empty báo động
	DrawReloadStatus(pDC);

	// B. Vẽ Combo (HIT, HEADSHOT...)
	DrawCombo(pDC);

	// C. Reset render state nếu cần (cho chắc ăn)
	RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	LPDIRECT3DDEVICE9 pd3dDevice = RGetDevice();
	pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	if (!ZGetGame()->IsReplay() || ZGetGame()->IsShowReplayInfo())
	{
		if (ZGetCombatInterface()->IsShowUI())
			DrawEffects(); // ÄÞº¸ ¿¡´Ï¸ÞÀÌ¼Ç µå·Î¿E

		// ÄÞº¸ÀÌÆåÆ®´Â Á÷Á¢°E®ÇØÁà¾ßÇÑ´Ù
		//DrawCombo(pDC);
	}

	if (ZGetCombatInterface()->IsShowUI())
	{
		DrawQuestEffects(); // Äù½ºÆ®½Ã K.O ÀÌ¹ÌÁE
		DrawDuelEffects();
		DrawTDMEffects();
		DrawCTFEffects();
	}
}
//void ZScreenEffectManager::Draw()
//{
//	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
//	if(!pTargetCharacter || !pTargetCharacter->GetInitialized()) return;
//
//	if(!ZGetCombatInterface()->GetObserverMode() && !ZGetCombatInterface()->IsSkupUIDraw())
//	{
//		RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE,	FALSE);
//		RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
//
//		if(pTargetCharacter) 
//		{
//			ZItem* pSelectedItem = pTargetCharacter->GetItems()->GetSelectedWeapon();
//
//			if(pSelectedItem){
//				if( pSelectedItem->GetItemType() != MMIT_MELEE ) {
//					if (pSelectedItem->GetBulletCurrMagazine() <= 0) {
//						if(pSelectedItem->isReloadable()==false) {
//							m_bShowReload = false;
//							m_bShowEmpty = true;
//						}
//						else {
//							m_bShowReload = true;
//							m_bShowEmpty = false;
//						}
//					}
//					else {
//						m_bShowReload = false;
//						m_bShowEmpty = false;
//					}
//				}
//				else {
//					m_bShowReload = false;
//					m_bShowEmpty = false;
//				}
//			}
//		}
//
//		if( m_bShowReload ) {
//			if(m_pReload)
//			{
//				m_pReload->Update();
//				m_pReload->Draw(0);
//			}
//		}
//		else if(m_bShowEmpty) {
//			if(m_pEmpty)
//			{
//				m_pEmpty->Update();
//				m_pEmpty->Draw(0);
//			}
//		}
//	}
//
//	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();
//	pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
//	pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
//
//	if ( !ZGetGame()->IsReplay() || ZGetGame()->IsShowReplayInfo())
//	{
//		if(ZGetCombatInterface()->IsShowUI())
//			DrawEffects(); // ÄÞº¸ ¿¡´Ï¸ÞÀÌ¼Ç µå·Î¿E
//
//		// ÄÞº¸ÀÌÆåÆ®´Â Á÷Á¢°E®ÇØÁà¾ßÇÑ´Ù
//		//DrawCombo(pDC);
//	}
//
//	if(ZGetCombatInterface()->IsShowUI())
//	{
//		DrawQuestEffects(); // Äù½ºÆ®½Ã K.O ÀÌ¹ÌÁE
//		DrawDuelEffects();
//		DrawTDMEffects();
//		DrawCTFEffects();
//	}
//}

void ZScreenEffectManager::DrawMyWeaponImage()
{
	if(ZGetGameInterface()->GetCombatInterface()->GetObserverMode()) return;
	if(!ZGetCombatInterface()->IsShowUI()) return;

	// ¹«±EÀÌ¹ÌÁE
	ZScreenEffect* pWeaponIcon = GetCurrWeaponImage();
	if( pWeaponIcon )
	{
		pWeaponIcon->Update();
		pWeaponIcon->Draw(0);
	}
}

void ZScreenEffectManager::DrawMyBuffImage()
{
	if(ZGetGameInterface()->GetCombatInterface()->GetObserverMode()) return;

	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if(!pTargetCharacter || !pTargetCharacter->GetInitialized()) return;

	// TodoH(»E - ÀÚ½ÅÀÇ ¹öÇÁ È¿°E¾ÆÀÌÄÜÀ» ±×·Á¾ß ÇÕ´Ï´Ù. ¸®¼Ò½ºµµ ÇÊ¿äÇÕ´Ï´Ù.
}

//Custom: Toggle change bar HP
void ZScreenEffectManager::DrawMyHPAPHud(MDrawContext* pDC)
{
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bScore)
	{
		if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode()) return;

		ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
		if (!pTargetCharacter || !pTargetCharacter->GetInitialized()) return;

		if (ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
		{
			if ((ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL) || (!pTargetCharacter->IsObserverTarget()))
			{
				m_pHPPanel->Update();
				m_pHPPanelEffect->Update();

				if (!ZGetCombatInterface()->IsShowUI())
					return;

				m_pHPPanel->Draw(0);
				m_pHPPanelEffect->Draw(0);

				bool bDrawGauges = false;
				if (m_pHPPanel->GetVMesh())
					if (m_pHPPanel->GetVMesh()->GetFrameInfo(ani_mode_lower)->m_isPlayDone)
						bDrawGauges = true;

				if (bDrawGauges)
				{
					if (m_nHpReset) 	m_nHpReset = DrawResetGauges();
					else  			DrawGauges(pDC);
				}
			}
		}
	}
	else
	{
		if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode()) return;

		ZCharacter* pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
		if (!pTargetCharacter || !pTargetCharacter->GetInitialized()) return;

		if (ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
		{
			if ((ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL) || (!pTargetCharacter->IsObserverTarget()))
			{
				m_pHPPanelNew->Update();
				m_pHPPanelEffect->Update();

				if (!ZGetCombatInterface()->IsShowUI())
					return;

				//m_pHPPanelNew->Draw(0);
				//m_pHPPanelEffect->Draw(0);

				bool bNewDrawGauges = false;
				if (m_pHPPanelNew->GetVMesh())
					if (m_pHPPanelNew->GetVMesh()->GetFrameInfo(ani_mode_lower)->m_isPlayDone)
						bNewDrawGauges = true;

				if (bNewDrawGauges)
				{
					if (m_nHpResetNew) 	m_nHpResetNew = DrawResetGauges();
					else  			DrawGauges(pDC);
				}
			}
		}
	}
	//Custom: change else to MMATCH_GAMETYPE_DUELTOURNAMENT
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
	{
		ZObserver* pObserver = ZGetGameInterface()->GetCombatInterface()->GetObserver();
		pObserver->DrawPlayerDuelHPAPBar(pDC);
	}
}

void ZScreenEffectManager::DrawSpectator()
{
	m_pSpectator->Update();
	m_pSpectator->Draw(timeGetTime());

	ZScreenEffect* pWeaponIcon = GetCurrWeaponImage();
	if( pWeaponIcon )
		pWeaponIcon->DrawCustom(0, rvector(0.0f, 80.0f, 0.0f));

	DrawEffects();
}

void ZScreenEffectManager::ResetSpectator()
{
	m_pSpectator->GetVMesh()->ClearFrame();
}

void ZScreenEffectManager::DrawEffects()
{
	ZEffect* pEffect = NULL;

	for( iterator i=begin(); i!=end();i++)
	{
		pEffect = *i;
		pEffect->Draw(0);
	}

	/*
	for( iterator i=begin(); i!=end();)
	{
	pEffect = *i;

	if(pEffect->Draw(0)==false) {

	delete pEffect;
	i = erase(i);
	} else {
	i++;
	}
	}
	*/
}

void ZScreenEffectManager::UpdateEffects()
{
	for (list<ZEffectList::iterator>::iterator i = m_eraseQueue.begin(); i != m_eraseQueue.end(); i++)
	{
		ZEffectList::iterator ieffect = *i;
		delete* ieffect;
		erase(ieffect);
	}
	m_eraseQueue.clear();

	ZEffect* pEffect = NULL;

	for (iterator i = begin(); i != end(); ++i)
	{
		pEffect = *i;

		pEffect->Update();
		if (pEffect->IsDeleteTime())
			m_eraseQueue.push_back(i);
	}

	if (ZGetGameInterface()->GetState() != GUNZ_GAME || (!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) && ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE))
		return;

	if (m_pBossHPPanel)
	{
		m_pBossHPPanel->Update();
	}

	// ½ºÆÑÀÌ¸EKO¶EÈ­»E¥´Â ±×¸®ÁE¾Ê´Â´Ù.
	if (ZGetGameInterface()->GetCombatInterface())
	{ // ZGetGameInterface()->GetCombatInterface() <- NULLÀÏ °æ¿E¡ µé¾ûÛÃ¶§°¡ ÀÖÀ½
		if (!ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
		{
			for (int i = 0; i < 10; i++)
			{
				m_pKONumberEffect[i]->Update();
			}
			m_pKO->Update();
		}
	}
}


void ZScreenEffectManager::AddRoundStart(int nRound)
{
#define ROUND_NUMBER_SPACE	60.f

	if(nRound<0) return;

	ZEffect* pNew = NULL;

	char buffer[32];
	sprintf(buffer,"%d",nRound);
	int nCount=(int)strlen(buffer);

	int nOver=max(nCount-2,0);

	for(int i=0;i<nCount;i++)
	{
		char meshname[256];
		sprintf(meshname,"round%d",buffer[i]-'0');
		RMesh *pMesh = m_pEffectMeshMgr->Get(meshname);
		if(pMesh)
			Add(new ZScreenEffect(pMesh , rvector(ROUND_NUMBER_SPACE*(float)(i-nCount+1+nOver),0,0)));
	}

	RMesh *pMesh = m_pEffectMeshMgr->Get("round");
	if(pMesh)
		Add(new ZScreenEffect(pMesh));

	ZGetGameInterface()->PlayVoiceSound( VOICE_GET_READY, 1300);
}

//void ZScreenEffectManager::DrawCombo()
//{
//	int nFrame,nLastDigit=0;
//	for(int i=0;i<COMBOEFFECTS_COUNT;i++)
//	{
//		if(m_pComboEffects[i]) {
//
//			nLastDigit=i;
//
//			//			nFrame = m_pComboEffects[i]->GetVMesh()->m_nFrame[0];
//
//			// Custom: VMesh NULL and GetFrameInfo fix
//			if (m_pComboEffects[i]->GetVMesh() == NULL) continue;
//			if (m_pComboEffects[i]->GetVMesh()->GetFrameInfo(ani_mode_lower) == NULL) continue;
//
//			nFrame = m_pComboEffects[i]->GetVMesh()->GetFrameInfo(ani_mode_lower)->m_nFrame;
//
//			if(m_pComboEffects[i]->GetVMesh()->isOncePlayDone()) {
//
//				m_pComboEffects[i]->DeleteAfter();
//				m_pComboEffects[i]=NULL;
//				if(i==0)
//				{
//					if(m_pComboEffects[1])
//						m_pComboEffects[1]->DeleteAfter();
//					m_pComboEffects[1]=new ZComboEffect(m_pComboEndEffect);
//					Add(m_pComboEffects[1]);
//				}
//			}
//		}
//	}
//
//	// °°Àº Å¸ÀÌ¹Ö¿¡ »ç¶óÁöµµ·Ï ÇÏ±EÀ§ÇØ ¾ÕÀÇ ¼ýÀÚµéµµ 
//	// ¸¶Áö¸· ÀÚ¸®¼öÀÇ ÇÁ·¹ÀÓÀÌ¶E°°Àº ÇÁ·¹ÀÓÀ¸·Î µ¹·Á³õ´Â´Ù
//
//	for(int i=2;i<nLastDigit;i++) {
//		if(m_pComboEffects[i]) {
//			m_pComboEffects[i]->SetFrame(nFrame);
//		}
//	}
//}
// Biến lưu trạng thái Combo Masang
static int   g_nComboDisplay = 0;
static float g_fComboTime = 0.0f;
static int   g_nComboLevel = 0;
int   g_nKillLevel = -1;
float g_fKillTime = 0.0f;

// Hàm hỗ trợ vẽ từng thành phần Masang
void DrawMasangElement(MDrawContext* pDC, MBitmap* pBitmap, int x, int y, int w, int h, int srcX = 0, int srcY = 0, int srcW = 0, int srcH = 0) {
	if (!pBitmap || !pDC) return;
	if (srcW == 0) srcW = pBitmap->GetWidth();
	if (srcH == 0) srcH = pBitmap->GetHeight();
	pDC->SetBitmap(pBitmap);
	pDC->Draw(x, y, w, h, srcX, srcY, srcW, srcH);
}
// ===================================================================================
// ZScreenEffectManager.cpp - BẢN TÁI CẤU TRÚC LOGIC
// ===================================================================================

void ZScreenEffectManager::AddHit()
{
	// 1. Dẹp bỏ ELU cũ (đã comment)
	// AddScreenEffect("hit"); 

	// 2. Ép cập nhật dữ liệu vẽ PNG cho viên đầu tiên
	// Vì viên 1 thường nCombo chưa kịp tăng hoặc SetCombo chưa chạy, 
	// mình chủ động gán tại đây để DrawCombo quét thấy ngay.
	extern int g_nComboDisplay;
	extern float g_fComboTime;

	g_nComboDisplay = 1; // Mặc định là HIT (Combo 1)
	g_fComboTime = (float)GetTickCount() * 0.001f;
}

void ZScreenEffectManager::DrawCombo(MDrawContext* pDC)
{
	extern int g_nComboDisplay;
	extern float g_fComboTime;
	extern int g_nComboLevel;
	extern int g_nKillLevel;
	extern float g_fKillTime;

	if (g_nComboDisplay < 1 || pDC == NULL) return;

	float fCurrentTime = (float)GetTickCount() * 0.001f;
	float fElapsed = fCurrentTime - g_fComboTime;
	if (fElapsed > 2.0f) return;

	MBitmap* pImgCount = MBitmapManager::Get("Ingame_ComboCount.png");
	MBitmap* pImgHit = MBitmapManager::Get("Ingame_Hit.png");
	MBitmap* pImgWord = MBitmapManager::Get("Ingame_Combo.png");
	MBitmap* pImgBoundary = MBitmapManager::Get("Ingame_ComboBoundary.png");
	MBitmap* pImgDirection = MBitmapManager::Get("Ingame_ComboDirection.png");

	if (!pImgHit || !pImgBoundary) return;

	float fScale = (float)MGetWorkspaceWidth() / 1920.0f;

	// Tọa độ Gốc - Masang lấy vạch Boundary làm chuẩn
	// Vị trí X này là mép PHẢI của vạch Boundary (Chuẩn 1920x1080)
	int nMaxRightX = (int)(1870.0f * fScale);
	int nBaseY = (int)(359.0f * fScale);
	int nVachW = (int)(438.0f * fScale);
	int nVachX = nMaxRightX - nVachW; // Vẽ ngược từ phải sang trái

	float fAni = (fElapsed < 0.1f) ? (1.2f - (fElapsed * 2.0f)) : 1.0f;
	if (fAni < 1.0f) fAni = 1.0f;

	// 1. VẼ VẠCH BOUNDARY TRƯỚC (Nền tảng)
	int nBoundY = nBaseY + (int)(81.0f * fScale);
	pDC->SetBitmap(pImgBoundary);
	pDC->Draw(nVachX, nBoundY, nVachW, (int)(5.0f * fScale));

	if (g_nComboDisplay == 1) {
		// --- CASE 1: CHỮ HIT (Căn lề phải bám theo mép phải vạch) ---
		int nHitW = (int)(74.0f * fScale * fAni);
		int nHitH = (int)(54.0f * fScale * fAni);
		// Vẽ HIT cách mép phải một khoảng nhỏ (giống Combo Word)
		int nHitX = nMaxRightX - nHitW - (int)(10.0f * fScale);
		int nHitY = nBaseY + (int)(20.0f * fScale);

		pDC->SetBitmap(pImgHit);
		pDC->Draw(nHitX, nHitY, nHitW, nHitH);
	}
	else {
		// --- CASE 2: COMBO >= 2 (Căn lề phải) ---
		if (!pImgCount || !pImgWord) return;

		char szBuf[10]; sprintf_s(szBuf, "%d", g_nComboDisplay);
		int nLen = (int)strlen(szBuf);
		int nDigitW = pImgCount->GetWidth() / 10;
		int nDigitH = pImgCount->GetHeight();

		// A. Vẽ chữ "Combo" ở bên phải nhất
		int nComboWordW = (int)(160.0f * fScale);
		int nComboWordH = (int)(56.0f * fScale);
		int nComboWordX = nMaxRightX - nComboWordW - (int)(5.0f * fScale);
		int nComboWordY = nBaseY + (int)(24.0f * fScale);

		pDC->SetBitmap(pImgWord);
		pDC->Draw(nComboWordX, nComboWordY, nComboWordW, nComboWordH);

		// B. Vẽ Số Combo (Vẽ ngược từ phải sang trái, bám vào chữ "Combo")
		int nNumberGap = (int)(10.0f * fScale); // Khoảng cách với chữ Combo
		int nCurrentDigitX = nComboWordX - nNumberGap;

		for (int i = nLen - 1; i >= 0; i--) { // Chạy ngược loop để căn phải
			int nDigit = szBuf[i] - '0';
			int nDrawW = (int)(nDigitW * fScale * fAni);
			int nDrawH = (int)(nDigitH * fScale * fAni);

			nCurrentDigitX -= nDrawW; // Dịch trái dần

			pDC->SetBitmap(pImgCount);
			pDC->Draw(nCurrentDigitX, nBaseY, nDrawW, nDrawH,
				nDigit * nDigitW, 0, nDigitW, nDigitH);
		}

		// C. Vẽ GOOD/NICE (Căn giữa theo vạch Boundary)
		if (pImgDirection && g_nComboLevel > 0) {
			int srcX = 0, srcW = 0;
			int nImgFullH = pImgDirection->GetHeight();
			switch (g_nComboLevel) {
			case 1: srcX = 190; srcW = 347 - 190; break;
			case 2: srcX = 560; srcW = 694 - 560; break;
			case 3: srcX = 855; srcW = 1039 - 855; break;
			case 4: srcX = 1039; srcW = 1384 - 1039; break;
			}
			if (srcW > 0) {
				int nDisplayW = (int)(srcW * fScale);
				int nLvlX = nVachX + (nVachW / 2) - (nDisplayW / 2);
				int nLvlY = nBaseY + (int)(100.0f * fScale);
				pDC->SetBitmap(pImgDirection);
				pDC->Draw(nLvlX, nLvlY, nDisplayW, (int)(nImgFullH * fScale), srcX, 0, srcW, nImgFullH);
			}
		}
	}
	// ===================================================================================
	// VẼ KILL DIRECTION (HEAD SHOT, FANTASTIC...) - CHUẨN MASANG
	// ===================================================================================
	MBitmap* pImgKill = MBitmapManager::Get("Ingame_KillDirection.png");
	float fKillElapsed = fCurrentTime - g_fKillTime;

	if (pImgKill && g_nKillLevel >= 0 && fKillElapsed < 2.0f)
	{
		int kSrcX = 0;
		int kSrcW = 0;
		int kImgH = pImgKill->GetHeight();

		// Tọa độ cắt pixel dựa trên Ingame_KillDirection.png
		switch (g_nKillLevel) {
		case ZCI_FANTASTIC:     kSrcX = 0;  kSrcW = 411 - 0; break; // FANTASTIC
		case ZCI_ALLKILL:		kSrcX = 412;  kSrcW = 824 - 412; break;
		case ZCI_EXCELLENT:     kSrcX = 1649; kSrcW = 2060 - 1649; break; // EXCELLENT
		case ZCI_UNBELIEVABLE:  kSrcX = 2062; kSrcW = 2473 - 2062; break; // UNBELIEVABLE
		case ZCI_HEADSHOT:      kSrcX = 2475;  kSrcW = 2886 - 2475; break; // HEAD SHOT
		// Có thể thêm các case khác nếu có ảnh tương ứng
		}

		// 1. Xác định lại vị trí vạch Boundary (nVachX) dựa trên lề phải nMaxRightX
		int nVachW = (int)(438.0f * fScale);
		int nVachX = nMaxRightX - nVachW; // Vạch bám lề phải

		if (kSrcW > 0) {
			int nKDisplayW = (int)(kSrcW * fScale);
			int nKDisplayH = (int)(kImgH * fScale);

			// 2. CÔNG THỨC CĂN GIỮA CHUẨN:
			// Tọa độ X = (Vị trí đầu vạch) + (Nửa chiều dài vạch) - (Nửa chiều dài chữ Kill)
			int nMidVach = nVachW / 2;
			int nKX = nVachX + nMidVach - (nKDisplayW / 2);

			// Tọa độ Y: Nằm dưới GOOD/NICE một chút (khoảng 165px tính từ BaseY là đẹp)
			int nKY = nBaseY + (int)(165.0f * fScale);

			pDC->SetBitmap(pImgKill);
			pDC->Draw(nKX, nKY, nKDisplayW, nKDisplayH, kSrcX, 0, kSrcW, kImgH);
		}
	}
}

//void ZScreenEffectManager::SetCombo(int nCombo)
//{
//	static int combonumbers[COMBOEFFECTS_COUNT]={0,};
//
//	if (nCombo > MAX_COMBO) nCombo = MAX_COMBO;		// 99 ³ÑÀ¸¸E¿¡·¯³­´Ù
//
//	ZCOMBOLEVEL thislevel;
//	if(nCombo<5) thislevel=ZCL_NONE;else
//		if(nCombo<10) thislevel=ZCL_GOOD;else
//			if(nCombo<15) thislevel=ZCL_NICE;else
//				if(nCombo<20) thislevel=ZCL_GREAT;else
//					thislevel=ZCL_WONDERFUL;
//
//	if(thislevel>m_CurrentComboLevel)
//	{
//		switch(thislevel) {
//		case ZCL_GOOD	: AddGood();	
//			break;
//		case ZCL_NICE	: AddNice();
//			break;
//		case ZCL_GREAT	: AddGreat();
//			break;
//		case ZCL_WONDERFUL: AddWonderful();
//			break;
//		}
//		m_CurrentComboLevel=thislevel;
//	}
//
//	if(thislevel==ZCL_NONE)
//		m_CurrentComboLevel=ZCL_NONE;
//
//	if(nCombo<3) return;
//
//	// 0¹øÀº ¾Õ¿¡ "combo" ±ÛÀÚ°¡ ³ªÅ¸³ª´Â ÀÌÆåÆ®
//	// 1¹øÀº »ç¶óÁö´Â ÀÌÆåÆ® 2ºÎÅÍ Ã¹¹øÂ° ÀÚ¸® ¼ýÀÚ
//	if(m_pComboEffects[0]==NULL)
//	{
//		m_pComboEffects[0]=new ZComboEffect(m_pComboBeginEffect);
//		Add(m_pComboEffects[0]);
//	}
//
//	char buffer[32];
//	// Custom: sprintf_s prevent buffer overrun
//	sprintf_s(buffer,"%d",nCombo);
//	int nCount=(int)strlen(buffer);
//
//	for(int i=0;i<nCount;i++)
//	{
//		int ncurrent=buffer[i]-'0';
//		if(combonumbers[i]!=ncurrent || m_pComboEffects[i+2]==NULL)
//		{
//			combonumbers[i]=ncurrent;
//
//			// ±× ÀÚ¸®¿¡ ¼ýÀÚ°¡ ÀÌ¹Ì ÀÖÀ¸¸EÇÁ·¹ÀÓÀ» µÚÂÊÀ¸·Î µ¹·Á °E»ç¶óÁöµµ·Ï ¸¸µç´Ù
//			if(m_pComboEffects[i+2]!=NULL)
//			{
//				RVisualMesh *pMesh=m_pComboEffects[i+2]->GetVMesh();
//
//				if (pMesh)
//				{
//					AniFrameInfo* pInfo = pMesh->GetFrameInfo(ani_mode_lower);
//
//					if(pInfo != NULL && pInfo->m_pAniSet)
//						pInfo->m_nFrame = pInfo->m_pAniSet->GetMaxFrame() - 4800.f*.2f;
//				}
//
//				m_pComboEffects[i+2]->DeleteAfter(1.f);
//			}
//
//			// ¼ýÀÚ¸¦ Ãß°¡ÇÑ´Ù
//			m_pComboEffects[i+2]=new ZComboEffect(m_pComboNumberEffect[ncurrent],rvector(-10.f+40.f*float(i-1),0,0));
//			Add(m_pComboEffects[i+2]);
//		}
//	}
//}
void ZScreenEffectManager::SetCombo(int nCombo)
{
	if (nCombo > 100) nCombo = 100;

	// 1. Tính toán Level
	ZCOMBOLEVEL thislevel;
	if (nCombo < 5) thislevel = ZCL_NONE;
	else if (nCombo < 10) thislevel = ZCL_GOOD;
	else if (nCombo < 15) thislevel = ZCL_NICE;
	else if (nCombo < 20) thislevel = ZCL_GREAT;
	else thislevel = ZCL_WONDERFUL;

	// 2. Âm thanh Praise (Giữ nguyên)
	if (thislevel > m_CurrentComboLevel)
	{
		switch (thislevel) {
		case ZCL_GOOD: AddGood(); break;
		case ZCL_NICE: AddNice(); break;
		case ZCL_GREAT: AddGreat(); break;
		case ZCL_WONDERFUL: AddWonderful(); break;
		}
		m_CurrentComboLevel = thislevel;
	}
	if (thislevel == ZCL_NONE) m_CurrentComboLevel = ZCL_NONE;

	// 3. LƯU DỮ LIỆU CHO PNG (Sửa nCombo >= 1)
	if (nCombo >= 1) {
		g_nComboDisplay = nCombo;
		g_fComboTime = (float)GetTickCount() * 0.001f;
		g_nComboLevel = (int)thislevel;
	}

	// --- ĐÃ COMMENT TOÀN BỘ PHẦN ELU CŨ Ở ĐÂY ---
	/*
	if(m_pComboEffects[0]==NULL) { ... }
	...
	*/
}

void ZScreenEffectManager::AddExpEffect(int nExp)
{
#define EXP_NUMBER_SPACE	30.f

	if(nExp==0) return;
	if (nExp > 0) PlaySoundScoreGet();

	ZEffect* pNew = NULL;

	char buffer[32];
	sprintf(buffer,"%d",abs(nExp));
	int nCount=(int)strlen(buffer);


	for(int i=0;i<nCount;i++)
	{
		float fOffset=EXP_NUMBER_SPACE*(float)(i-nCount+2);
		Add(new ZScreenEffect(m_pExpNumberEffect[buffer[i]-'0'], rvector(fOffset,0,0)));
	}

	float fOffset=EXP_NUMBER_SPACE*(float)(3-nCount);
	Add(new ZScreenEffect(nExp>0 ? m_pExpPlusEffect : m_pExpMinusEffect , rvector(fOffset,0,0)));
}

void ZScreenEffectManager::DrawScoreBoard()
{
	m_pScorePanel->Draw(0);
}

void ZScreenEffectManager::DrawScoreBoardTeam()
{
	m_pScorePanelTeam->Draw(0);
}

void ZScreenEffectManager::DrawScoreBoardSolo()
{
	m_pScorePanelSolo->Draw(0);
}

void ZScreenEffectManager::AddAlert(const rvector& vVictimPos, rvector& vVictimDir, rvector& vAttackerPos)
{
	rvector my_dir = vVictimDir;
	rvector my_pos = vVictimPos;
	rvector attackPos = vAttackerPos;

	my_pos.z = 0.0f;
	attackPos.z = 0.0f;

	Normalize(my_dir);

	rvector dir = attackPos - my_pos;
	Normalize(dir);


	rvector vector1 = my_dir, vector2 = dir;
	vector1.y = -vector1.y;
	vector2.y = -vector2.y;
	float cosAng1 = DotProduct(vector1, vector2); 

	float r;
	if (-vector1.y*vector2.x + vector1.x*vector2.y > 0.0f)
	{
		r = (float)(acos(cosAng1));
	}
	else
	{
		r = -(float)(acos(cosAng1)); 
	}

	float t = (pi / 4.0f);

	int nIndex = -1;
	if (((r > 0) && (r < t)) || ((r <= 0) && (r > -t))) nIndex = 0;
	else if ((r <= -t) && (r > -t*3)) nIndex = 1;
	else if (((r >= t*3) && (r <= t*4)) || ((r <= -t*3) && (r > -t*4))) nIndex = 2;
	else if ((r >= t) && (r < t*3)) nIndex = 3;

	if ((nIndex >= 0) && (nIndex < 4))
	{
		// ¸¸¾EÀÌ¹Ì ±×¸®°EÀÖÀ¸¸E±×¸®ÁE¾Ê´Â´Ù.
		for (iterator itor = begin(); itor != end(); ++itor)
		{
			ZScreenEffect* pEffect = (ZScreenEffect*)(*itor);
			if (pEffect->GetVMesh()->GetMesh() == m_pAlertEffect[nIndex]) return;
		}

		Add(new ZScreenEffect(m_pAlertEffect[nIndex]));
	}

}

void ZScreenEffectManager::PlaySoundScoreFlyby()
{
#ifdef _BIRDSOUND
	ZGetSoundEngine()->PlaySound("if_score_flyby");		// ¾ÆÁEµô·¹ÀÌ Àû¿EÈµÇ¾EÀÖÀ½ 
#else
	ZGetSoundEngine()->PlaySound("if_score_flyby",false, 2000);
#endif
}

void ZScreenEffectManager::PlaySoundScoreGet()
{
	ZGetSoundEngine()->PlaySound("if_score_get");
}

void ZScreenEffectManager::AddPraise(int nPraise)
{
	if(nPraise<0 || nPraise>=ZCI_END) return;

	// Gán để DrawCombo biết mà vẽ PNG
	g_nKillLevel = nPraise;
	g_fKillTime = (float)GetTickCount() * 0.001f;

	PlaySoundScoreFlyby(); 

	//AddScreenEffect(m_pPraiseEffect[nPraise]);

	switch (nPraise)
	{
	case ZCI_ALLKILL:
		ZGetGameInterface()->PlayVoiceSound( VOICE_KILLEDALL, 2000);
		break;
	case ZCI_UNBELIEVABLE:
		ZGetGameInterface()->PlayVoiceSound( VOICE_UNBELIEVABLE, 1300);
		break;
	case ZCI_EXCELLENT:
		ZGetGameInterface()->PlayVoiceSound( VOICE_EXCELLENT, 1000);
		break;
	case ZCI_FANTASTIC:
		ZGetGameInterface()->PlayVoiceSound( VOICE_FANTASTIC, 1500);
		break;
	case ZCI_HEADSHOT:
		ZGetGameInterface()->PlayVoiceSound( VOICE_HEADSHOT, 700);
		break;
	case ZCI_FIRSTBLOOD:
		ZGetGameInterface()->PlayVoiceSound( VOICE_FIRST_KILL, 2000);
		break;
	};

}

void ZScreenEffectManager::SetGaugeExpFromMyInfo()
{
	int nExpPercent = ZGetMyInfo()->GetLevelPercent();
	float fRatio = (float)(nExpPercent) / 100.0f;
	SetGauge_EXP(fRatio);
}
void ZScreenEffectManager::AddGood()
{	
	//AddScreenEffect(m_pGoodEffect); 
}
void ZScreenEffectManager::AddNice()
{	
	//AddScreenEffect(m_pNiceEffect); 
	ZGetGameInterface()->PlayVoiceSound( VOICE_NICE, 1000);

}
void ZScreenEffectManager::AddGreat()
{	
	//AddScreenEffect(m_pGreatEffect); 
	ZGetGameInterface()->PlayVoiceSound( VOICE_GREAT, 800);
}

void ZScreenEffectManager::AddWonderful()
{	
	//AddScreenEffect(m_pWonderfullEffect); 
	ZGetGameInterface()->PlayVoiceSound( VOICE_WONDERFUL, 1200);
}

void ZScreenEffectManager::AddCool()
{	
	//AddScreenEffect(m_pCoolEffect); 
	ZGetGameInterface()->PlayVoiceSound( VOICE_COOL, 700);
}

void ZScreenEffectManager::AddRock()
{	
	AddScreenEffect("rock"); 
	ZGetGameInterface()->PlayVoiceSound( VOICE_LETS_ROCK, 1100);
}

void ZScreenEffectManager::AddCTFEffect(MMatchTeam nTeam, int nArg)
{
	if (!IsGameRuleCTF(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) return;

	ZRuleTeamCTF* pRule = (ZRuleTeamCTF*)ZGetGame()->GetMatch()->GetRule();

	if (!pRule) return;

	// scored
	if (nArg == 2)
	{
		ZGetSoundEngine()->PlaySound( "scored" );
		ZGetGameInterface()->PlayVoiceSound( nTeam == MMT_RED ? VOICE_RED_TEAM_SCORE : VOICE_BLUE_TEAM_SCORE, 2000);
		ZGetScreenEffectManager()->AddScreenEffect( nTeam == MMT_RED ? "ctf_score_r" : "ctf_score_b" );

		// who is the carrier
		ZCharacter* pCarrier = (ZCharacter*)ZGetGame()->GetCharacterMgr()->Find(nTeam == MMT_RED ? pRule->GetRedCarrier() : pRule->GetBlueCarrier());
		char szMsg[128];

		if (ZGetGame()->m_pMyCharacter->GetTeamID() == nTeam)
		{
			if (pCarrier)
			{
				sprintf_s (szMsg, "Your team (%s) has captured the enemy flag!", pCarrier->GetUserName() );
				ZGetGameInterface()->GetCombatInterface()->UpdateCTFMsg( szMsg );
			}
			else
			{
				sprintf_s (szMsg, "Your team has captured the enemy flag!" );
				ZGetGameInterface()->GetCombatInterface()->UpdateCTFMsg( szMsg );
			}
		}
		else
		{
			sprintf_s (szMsg, "Your team flag was captured!" );
			ZGetGameInterface()->GetCombatInterface()->UpdateCTFMsg( szMsg );
		}

		return;
	}

	// flag taken
	if (nArg == 1)
	{
		if(nTeam == MMT_BLUE)
		{
			ZGetScreenEffectManager()->AddScreenEffect("ctf_taken_r");
			ZGetSoundEngine()->PlaySound( "ctf_flagtaken" );
			ZGetGameInterface()->PlayVoiceSound( VOICE_BLUE_HAS_FLAG, 1600);
		}
		else if (nTeam == MMT_RED)
		{
			ZGetScreenEffectManager()->AddScreenEffect("ctf_taken_b");
			ZGetSoundEngine()->PlaySound( "ctf_flagtaken" );
			ZGetGameInterface()->PlayVoiceSound( VOICE_RED_HAS_FLAG, 1600);
		}

		// who is the carrier
		ZCharacter* pCarrier = (ZCharacter*)ZGetGame()->GetCharacterMgr()->Find(nTeam == MMT_RED ? pRule->GetRedCarrier() : pRule->GetBlueCarrier());
		char szMsg[128];

		if (ZGetGame()->m_pMyCharacter->GetTeamID() == nTeam)
		{
			if (pCarrier)
			{
				sprintf_s (szMsg, "Your team (%s) has taken the enemy flag!", pCarrier->GetUserName() );
				ZGetGameInterface()->GetCombatInterface()->UpdateCTFMsg( szMsg );
			}
		}
		else
		{
			sprintf_s (szMsg, "Your team flag has been taken!" );
			ZGetGameInterface()->GetCombatInterface()->UpdateCTFMsg( szMsg );
		}

		return;
	}

	// flag dropped
	if (nArg == 0)
	{
		if(nTeam == MMT_BLUE)
		{
			ZGetScreenEffectManager()->AddScreenEffect("ctf_flagdrop_r");
			ZGetSoundEngine()->PlaySound( "ctf_flagreturn" );
			ZGetGameInterface()->PlayVoiceSound( VOICE_RED_FLAG_RETURN, 1600);
		}
		else if (nTeam == MMT_RED)
		{
			ZGetScreenEffectManager()->AddScreenEffect("ctf_flagdrop_b");
			ZGetSoundEngine()->PlaySound( "ctf_flagreturn" );
			ZGetGameInterface()->PlayVoiceSound( VOICE_BLUE_FLAG_RETURN, 1600);
		}

		// who is the carrier
		ZCharacter* pCarrier = (ZCharacter*)ZGetGame()->GetCharacterMgr()->Find(nTeam == MMT_RED ? pRule->GetRedCarrier() : pRule->GetBlueCarrier());
		char szMsg[128];

		if (ZGetGame()->m_pMyCharacter->GetTeamID() == nTeam)
		{
			if (pCarrier)
			{
				sprintf_s (szMsg, "Your team (%s) has dropped the enemy flag..", pCarrier->GetUserName() );
				ZGetGameInterface()->GetCombatInterface()->UpdateCTFMsg( szMsg );
			}
		}
		else
		{
			sprintf_s (szMsg, "Your team flag was returned." );
			ZGetGameInterface()->GetCombatInterface()->UpdateCTFMsg( szMsg );
		}
	}
}


bool ZScreenEffectManager::CreateQuestRes()
{
	m_nKO = 0;

	m_pBossHPPanel = new ZBossGaugeEffect(m_pQuestEffectMeshMgr->Get("boss_hppanel"));
	m_pArrow = new ZScreenEffect(m_pQuestEffectMeshMgr->Get("arrow"));

	return true;
}

void ZScreenEffectManager::DestroyQuestRes()
{
	if (m_pBossHPPanel)
	{
		SAFE_DELETE(m_pBossHPPanel);
	}
	if (m_pArrow)
	{
		SAFE_DELETE(m_pArrow);
	}
}


void ZScreenEffectManager::DrawQuestEffects()
{
	if (!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) 
		&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE) return;
	// Custom: Check if boss in CQ is alive, if so draw the hppanel
	MUID uidBoss;
	if (m_pBossHPPanel)
	{
		if (ZGetCombatInterface()->IsShowUI())
		{
			if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
			{
				ZRuleQuestChallenge* pCqRule = NULL;
				if (ZGetGame()->GetMatch()->GetRule())
				{
					pCqRule = (ZRuleQuestChallenge*)ZGetGame()->GetMatch()->GetRule();
				}
				uidBoss = pCqRule->GetBoss();
				ZObject* pBoss = ZGetObjectManager()->GetObject(uidBoss);
				if (pBoss && !pBoss->IsDie())
				{
					m_pBossHPPanel->Draw(0);
				}
			}
			else
			{
				m_pBossHPPanel->Draw(0);
			}
		}
	}

	// ½ºÆÑÀÌ¸EKO¶EÈ­»E¥´Â ±×¸®ÁE¾Ê´Â´Ù.
	if (!ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		DrawKO();

		if (ZGetQuest()->IsRoundClear() && ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
		{
			rvector to = ZGetQuest()->GetGameInfo()->GetPortalPos();
			DrawArrow(to);
		}
	}
}

void ZScreenEffectManager::DrawCTFEffects()
{
	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_CTF) return;
	if (ZGetGame()->GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PLAY) return;

	if (!ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		ZRuleTeamCTF* pTeamCTF = (ZRuleTeamCTF*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();

		if (pTeamCTF == NULL)
			return;

		rvector to = ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_RED ? 
						pTeamCTF->GetBlueFlagPos() : pTeamCTF->GetRedFlagPos();
		ZCharacter* pTarget = NULL;

		if (ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_RED)
		{
			if (ZGetGame()->m_pMyCharacter->GetUID() == pTeamCTF->GetRedCarrier() && pTeamCTF->GetBlueCarrier().IsInvalid())
			{
				// draw arrow to red base
				to = pTeamCTF->GetRedFlagPos();
			}
			else if (pTeamCTF->GetRedCarrier().IsValid() && pTeamCTF->GetBlueCarrier().IsInvalid())
			{
				// draw arrow to red carrier (team)
				pTarget = (ZCharacter*)ZGetGame()->GetCharacterMgr()->Find( pTeamCTF->GetRedCarrier() );

				if (pTarget)
					to = pTarget->GetPosition();
			}
			else if (pTeamCTF->GetBlueCarrier().IsValid())
			{
				// draw arrow to blue carrier
				pTarget = (ZCharacter*)ZGetGame()->GetCharacterMgr()->Find( pTeamCTF->GetBlueCarrier() );

				if (pTarget)
					to = pTarget->GetPosition();
			}
		}
		else if (ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_BLUE)
		{
			if (ZGetGame()->m_pMyCharacter->GetUID() == pTeamCTF->GetBlueCarrier() && pTeamCTF->GetRedCarrier().IsInvalid())
			{
				// draw arrow to blue base
				to = pTeamCTF->GetBlueFlagPos();
			}
			else if (pTeamCTF->GetBlueCarrier().IsValid() && pTeamCTF->GetRedCarrier().IsInvalid())
			{
				// draw arrow to blue carrier (team)
				pTarget = (ZCharacter*)ZGetGame()->GetCharacterMgr()->Find( pTeamCTF->GetBlueCarrier() );

				if (pTarget)
					to = pTarget->GetPosition();
			}
			else if (pTeamCTF->GetRedCarrier().IsValid())
			{
				// draw arrow to red carrier
				pTarget = (ZCharacter*)ZGetGame()->GetCharacterMgr()->Find( pTeamCTF->GetRedCarrier() );

				if (pTarget)
					to = pTarget->GetPosition();
			}
		}

		DrawArrow(to);
	}	
}

void ZScreenEffectManager::AddKO(int nKills)
{
	m_nKO += nKills;

	for (int i = 0; i < 10; i++)
	{
		m_pKONumberEffect[i]->InitFrame();
	}
}

void ZScreenEffectManager::SetKO(int nKills)
{
	m_nKO = nKills;
}

void ZScreenEffectManager::DrawKO()
{
	if ((m_pKO == NULL) || (m_nKO <= 0)) return;

	char buffer[32];
	sprintf(buffer,"%d", m_nKO);
	int nCount=(int)strlen(buffer);


	unsigned int nNowTime = timeGetTime();

	int nFirstNumberFrame = -1;

	for(int i=0;i<nCount;i++)
	{
		int nIndex = buffer[i]-'0';


		if (i > 0)
		{
			// ÇÁ·¹ÀÓÀ» ¸ÂÃá´Ù.
			AniFrameInfo* pInfo = m_pKONumberEffect[nIndex]->GetVMesh()->GetFrameInfo(ani_mode_lower);
			pInfo->m_nFrame = nFirstNumberFrame;

		}

		float fOffset= 40 * (float)(i-nCount+2) - 40;
		m_pKONumberEffect[nIndex]->DrawCustom(nNowTime, rvector(fOffset, 0.0f, 0.0f));

		if (i == 0)
		{
			nFirstNumberFrame = m_pKONumberEffect[nIndex]->GetFrame();
		}

		m_pKONumberEffect[nIndex]->InitFrame();
	}

	AniFrameInfo* pInfo = m_pKONumberEffect[buffer[0]-'0']->GetVMesh()->GetFrameInfo(ani_mode_lower);
	pInfo->m_nFrame = nFirstNumberFrame;

	m_pKO->Draw(nNowTime);
}

void ZScreenEffectManager::DrawArrow(rvector& vTargetPos)
{
	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if(!pTargetCharacter || !pTargetCharacter->GetInitialized()) return;

	rvector at = pTargetCharacter->GetPosition();
	rvector to = vTargetPos;

	rvector dir1, dir2;
	dir1 = pTargetCharacter->GetDirection();
	dir2 = to - at;

	float fAng = GetAngleOfVectors(dir2, dir1);

	// Custom: Removed const
	/*const*/ float fOffsetY = 285.0f;

	if (IsGameRuleCTF(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		fOffsetY = 230.f;
	}

	if (m_pArrow)
	{
		m_pArrow->DrawCustom(0, rvector(0.0f, fOffsetY, 0.0f), fAng);
	}
}

void ZScreenEffectManager::ShockBossGauge(float fPower)
{
	if (m_pBossHPPanel) m_pBossHPPanel->Shock(fPower);
}


void ZScreenEffectManager::DrawDuelEffects()
{
	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DUEL) return;
	if (ZGetCombatInterface()->GetObserver()->IsVisible()) return;
	else
	{
		if (ZGetGame()->m_pMyCharacter->GetKils() > 0)
		{
			char buffer[32];
			sprintf(buffer, "%d", ZGetGame()->m_pMyCharacter->GetKils());
			int nCount = (int)strlen(buffer);

			unsigned int nNowTime = timeGetTime();

			int nFirstNumberFrame = -1;

			for (int i = 0; i < nCount; i++)
			{
				int nIndex = buffer[i] - '0';

				if (i > 0)
				{
					// ÇÁ·¹ÀÓÀ» ¸ÂÃá´Ù.
					AniFrameInfo* pInfo = m_pKONumberEffect[nIndex]->GetVMesh()->GetFrameInfo(ani_mode_lower);
					pInfo->m_nFrame = nFirstNumberFrame;

				}

				float fOffset = 40 * (float)(i - nCount + 2) - 40;
				m_pKONumberEffect[nIndex]->Update();
				m_pKONumberEffect[nIndex]->DrawCustom(nNowTime, rvector(fOffset, 0.0f, 0.0f));


				if (i == 0)
				{
					nFirstNumberFrame = m_pKONumberEffect[nIndex]->GetFrame();
				}

				m_pKONumberEffect[nIndex]->InitFrame();
			}

			AniFrameInfo* pInfo = m_pKONumberEffect[buffer[0] - '0']->GetVMesh()->GetFrameInfo(ani_mode_lower);
			pInfo->m_nFrame = nFirstNumberFrame;

			m_pKO->Update();
			m_pKO->Draw(nNowTime);
		}
	}
}

void ZScreenEffectManager::UpdateDuelEffects()
{
	for (int i = 0; i < 10; i++)
	{
		m_pKONumberEffect[i]->InitFrame();
	}

	ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGame()->GetMatch()->GetRule();

	//	if (pDuel->QInfo.m_nVictory + 1 >= 3)
	{
		char buffer[32];
		sprintf(buffer,"%d", pDuel->QInfo.m_nVictory);
		int nCount=(int)strlen(buffer);

		for(int i=0;i<nCount;i++)
		{
			char meshname[256];
			sprintf(meshname,"duel%d",buffer[i]-'0');
			RMesh *pMesh = m_pEffectMeshMgr->Get(meshname);
			if(pMesh)
				Add(new ZScreenEffect(pMesh , rvector(60*(float)(i-nCount+2),0,0)));
		}
	}
}


void ZScreenEffectManager::DrawTDMEffects()
{
	if (ZGetGameTypeManager()->IsTeamExtremeGame(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) return;

	unsigned int nNowTime = timeGetTime();

	m_pTDScoreBoard->Update();
	m_pTDScoreBoard->Draw(nNowTime);

	int nBlueKills = ZGetGame()->GetMatch()->GetTeamKills(MMT_BLUE);
	int nRedKills = ZGetGame()->GetMatch()->GetTeamKills(MMT_RED);
	int diff = abs(nRedKills - nBlueKills);


	if (nBlueKills > nRedKills)
	{
		m_pTDScoreBlink_B->SetAnimationSpeed(diff);
		m_pTDScoreBlink_B->Update();
		m_pTDScoreBlink_B->Draw(nNowTime);
	}
	else if (nRedKills > nBlueKills)
	{
		m_pTDScoreBlink_R->SetAnimationSpeed(diff);
		m_pTDScoreBlink_R->Update();
		m_pTDScoreBlink_R->Draw(nNowTime);
	}

}