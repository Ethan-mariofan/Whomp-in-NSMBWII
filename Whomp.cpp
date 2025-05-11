#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>
#include <stage.h>
#include "boss.h"
#include <profile.h> 

void whompCollisionCallback(ActivePhysics *apThis, ActivePhysics *apOther);

void whompCollisionCallback(ActivePhysics *apThis, ActivePhysics *apOther) {
	if (apOther->owner->profileId == ProfileId::PLAYER) { dEn_c::collisionCallback(apThis, apOther); }
	else { return; }
} 
 
 


const char* WhompNsmbwANL [] = { "whomp", NULL };

class daWhomp_c : public daEnBlockMain_c {
public:

	int onCreate();
	int onExecute();
	int onDelete();
	int onDraw();

	void updateModelMatrices();
	void bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate);

	static dActor_c* build();

	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	//void spriteCollision(ActivePhysics *apThis, ActivePhysics *apOther);
	//void yoshiCollision(ActivePhysics *apThis, ActivePhysics *apOther);

	//bool collisionCat3_StarPower(ActivePhysics *apThis, ActivePhysics *apOther); 
	//bool collisionCat5_Mario(ActivePhysics *apThis, ActivePhysics *apOther); 
	//bool collisionCatD_Drill(ActivePhysics *apThis, ActivePhysics *apOther); 
	//bool collisionCat8_FencePunch(ActivePhysics *apThis, ActivePhysics *apOther); 
	//bool collisionCat7_GroundPound(ActivePhysics *apThis, ActivePhysics *apOther); 
	//bool collisionCat7_GroundPoundYoshi(ActivePhysics *apThis, ActivePhysics *apOther); 
	//bool collisionCatA_PenguinMario(ActivePhysics *apThis, ActivePhysics *apOther); 
	//bool collisionCat11_PipeCannon(ActivePhysics *apThis, ActivePhysics *apOther); 
	//bool collisionCat9_RollingObject(ActivePhysics *apThis, ActivePhysics *apOther); 
	//bool collisionCat1_Fireball_E_Explosion(ActivePhysics *apThis, ActivePhysics *apOther); 
	//bool collisionCat2_IceBall_15_YoshiIce(ActivePhysics *apThis, ActivePhysics *apOther); 
	//bool collisionCat13_Hammer(ActivePhysics *apThis, ActivePhysics *apOther); 
	//bool collisionCat14_YoshiFire(ActivePhysics *apThis, ActivePhysics *apOther); 
	
	mHeapAllocator_c allocator;
	m3d::mdl_c bodyModel;
	m3d::anmChr_c animationChr;
	nw4r::g3d::ResFile resFile;

	int stomp_timer; 
	int down_timer; 
	int recover_timer;

	int dead;      

	Physics::Info physicsInfo;
	void calledWhenUpMoveExecutes();
	void calledWhenDownMoveExecutes();

	void blockWasHit(bool isDown);   

	USING_STATES(daWhomp_c);
	DECLARE_STATE(wait);
	DECLARE_STATE(stomp);
	DECLARE_STATE(down);
	DECLARE_STATE(recover);
	DECLARE_STATE(die);
};

CREATE_STATE(daWhomp_c, wait);
CREATE_STATE(daWhomp_c, stomp);
CREATE_STATE(daWhomp_c, down);
CREATE_STATE(daWhomp_c, recover);
CREATE_STATE(daWhomp_c, die);

void daWhomp_c::updateModelMatrices() {
	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);
}


void daWhomp_c::bindAnimChr_and_setUpdateRate(const char* name, int unk, float unk2, float rate) {
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr(name);
	this->animationChr.bind(&this->bodyModel, anmChr, unk);
	this->bodyModel.bindAnim(&this->animationChr, unk2);
	this->animationChr.setUpdateRate(rate);
}

dActor_c* daWhomp_c::build() {
	void *buffer = AllocFromGameHeap1(sizeof(daWhomp_c));
	return new(buffer) daWhomp_c;
}

const SpriteData WhompSpriteData = 
{ ProfileId::EN_WHOMP, 0, 0, 0, 0, 0x100, 0x100, 0, 0, 0, 0, 0 };

Profile WhompProfile(&daWhomp_c::build, SpriteId::EN_WHOMP, &WhompSpriteData, ProfileId::EN_WHOMP, ProfileId::EN_WHOMP, "Whomp", WhompNsmbwANL);


void daWhomp_c::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
	if (acState.getCurrentState() == &StateID_wait) {
		doStateChange(&StateID_stomp);
	}
	if (acState.getCurrentState() == &StateID_down) { 
		char hitType;
		hitType = usedForDeterminingStatePress_or_playerCollision(this, apThis, apOther, 2);


		if(hitType == 1 || hitType == 3) {	// regular jump
			

			PlaySound(this, SE_EMY_DOSSUN_DEAD);   

			doStateChange(&StateID_die);  
		} else {
			DamagePlayer(this, apThis, apOther); 
		}
	}
}



int daWhomp_c::onCreate() {
	allocator.link(-1, GameHeaps[0], 0, 0x20);

	resFile.data = getResource("whomp", "g3d/t00.brres");
	nw4r::g3d::ResMdl mdl = this->resFile.GetResMdl("whomp");
	bodyModel.setup(mdl, &allocator, 0x224, 1, 0);
	SetupTextures_Enemy(&bodyModel, 0);
	nw4r::g3d::ResAnmChr anmChr = this->resFile.GetResAnmChr("wait");
	this->animationChr.setup(mdl, anmChr, &this->allocator, 0);

	allocator.unlink(); 

	this->scale.x = 1.0; 
	this->scale.y = 1.0; 
	this->scale.z = 1.0;
	this->stomp_timer = 0; 
	this->down_timer = 0;     
	this->recover_timer = 0;     


	ActivePhysics::Info HitMeBaby; 
	HitMeBaby.xDistToCenter = 0.0; 
	HitMeBaby.yDistToCenter = 8.0; 
	HitMeBaby.xDistToEdge = 24.0; 
	HitMeBaby.yDistToEdge = 8.0; 
	HitMeBaby.category1 = 0x3; 
	HitMeBaby.category2 = 0x0; 
	HitMeBaby.bitfield1 = 0x4F; 
	HitMeBaby.bitfield2 = 0xFFC00000; 
	HitMeBaby.unkShort1C = 0; 
	HitMeBaby.callback = &whompCollisionCallback;  
	this->aPhysics.initWithStruct(this, &HitMeBaby); 
	//this->aPhysics.addToList();


	//blockPhyscs      
	blockInit(pos.y);

	physicsInfo.x1 = -24;
	physicsInfo.y1 = 0;
	physicsInfo.x2 = 30;
	physicsInfo.y2 = 16;              

	physicsInfo.otherCallback1 = &daEnBlockMain_c::OPhysicsCallback1;
	physicsInfo.otherCallback2 = &daEnBlockMain_c::OPhysicsCallback2;
	physicsInfo.otherCallback3 = &daEnBlockMain_c::OPhysicsCallback3;

	physics.setup(this, &physicsInfo, 3, currentLayerID);
	physics.flagsMaybe = 0x260;
	physics.callback1 = &daEnBlockMain_c::PhysicsCallback1;
	physics.callback2 = &daEnBlockMain_c::PhysicsCallback2;
	physics.callback3 = &daEnBlockMain_c::PhysicsCallback3;
	//physics.addToList();
          




	bindAnimChr_and_setUpdateRate("wait", 1, 0.0, 1.0);

	doStateChange(&StateID_wait);



	this->onExecute();

	return true;
}

int daWhomp_c::onExecute() {
	acState.execute();

	updateModelMatrices();
	bodyModel._vf1C();

	if (acState.getCurrentState() != &StateID_stomp && acState.getCurrentState() != &StateID_recover) {
		if (this->animationChr.isAnimationDone()) {
			this->animationChr.setCurrentFrame(0.0);
		}
	}

	

	return true;
}

int daWhomp_c::onDelete() {
	return true;
}

int daWhomp_c::onDraw() {
	bodyModel.scheduleForDrawing();

	return true;
}

//wait state  
void daWhomp_c::beginState_wait() {
	bindAnimChr_and_setUpdateRate("wait", 1, 0.0, 1.0);
	this->aPhysics.addToList();    
}
void daWhomp_c::executeState_wait() {

}
void daWhomp_c::endState_wait() {

}





void daWhomp_c::beginState_stomp() {
	bindAnimChr_and_setUpdateRate("stomp", 1, 0.0, 1.0);
	this->stomp_timer = 0;   
}
void daWhomp_c::executeState_stomp() {
	
	if (this->stomp_timer >= 55) {
		doStateChange(&StateID_down);     
	}
	this->stomp_timer += 1;    
}
void daWhomp_c::endState_stomp() {

}



  

void daWhomp_c::blockWasHit(bool isDown) {
	if (isDown == true && acState.getCurrentState() == &StateID_down) {
		doStateChange(&StateID_die);
	}
}

 

void daWhomp_c::calledWhenUpMoveExecutes() {
	
}

void daWhomp_c::calledWhenDownMoveExecutes() {

		
}







void daWhomp_c::beginState_down() {
	bindAnimChr_and_setUpdateRate("down", 1, 0.0, 1.0);
	this->down_timer = 0;   
}
void daWhomp_c::executeState_down() {
	this->down_timer += 1;  
	  

	if (this->down_timer >= 300) {
		doStateChange(&StateID_recover);
	}
}
void daWhomp_c::endState_down() {
	 
}

void daWhomp_c::beginState_recover() {
	this->recover_timer = 0;   
	bindAnimChr_and_setUpdateRate("moveup", 1, 0.0, 1.0);
}
void daWhomp_c::executeState_recover() {
	
	if (this->recover_timer >= 32) {
		doStateChange(&StateID_wait);     
	}
	this->recover_timer += 1;      
}
void daWhomp_c::endState_recover() {

}

void daWhomp_c::beginState_die() {
	this->dead = 0;
	aPhysics.removeFromList();
	bindAnimChr_and_setUpdateRate("die", 1, 0.0, 1.0);
}
void daWhomp_c::executeState_die() {
	this->dead += 1;
	if (this->dead >= 60) {
		this->Delete(1);  
	}
}
void daWhomp_c::endState_die() {

}