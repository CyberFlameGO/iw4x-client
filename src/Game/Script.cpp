#include <STDInclude.hpp>

namespace Game
{
	AddRefToObject_t AddRefToObject = AddRefToObject_t(0x61C360);
	RemoveRefToObject_t RemoveRefToObject = RemoveRefToObject_t(0x437190);
	AllocObject_t AllocObject = AllocObject_t(0x434320);
	AddRefToValue_t AddRefToValue = AddRefToValue_t(0x482740);
	AllocThread_t AllocThread = AllocThread_t(0x4F78C0);

	VM_Execute_0_t VM_Execute_0 = VM_Execute_0_t(0x6222A0);

	Scr_GetGameTypeNameForScript_t Scr_GetGameTypeNameForScript = Scr_GetGameTypeNameForScript_t(0x462460);
	Scr_IsValidGameType_t Scr_IsValidGameType = Scr_IsValidGameType_t(0x4F1B60);
	Scr_LoadGameType_t Scr_LoadGameType = Scr_LoadGameType_t(0x4D9520);
	Scr_StartupGameType_t Scr_StartupGameType = Scr_StartupGameType_t(0x438720);

	Scr_LoadScript_t Scr_LoadScript = Scr_LoadScript_t(0x45D940);
	Scr_GetFunctionHandle_t Scr_GetFunctionHandle = Scr_GetFunctionHandle_t(0x4234F0);

	Scr_GetString_t Scr_GetString = Scr_GetString_t(0x425900);
	Scr_GetConstString_t Scr_GetConstString = Scr_GetConstString_t(0x494830);
	Scr_GetDebugString_t Scr_GetDebugString = Scr_GetDebugString_t(0x4EBF50);
	Scr_GetFloat_t Scr_GetFloat = Scr_GetFloat_t(0x443140);
	Scr_GetInt_t Scr_GetInt = Scr_GetInt_t(0x4F31D0);
	Scr_GetObject_t Scr_GetObject = Scr_GetObject_t(0x462100);
	Scr_GetTypeName_t Scr_GetTypeName = Scr_GetTypeName_t(0x4EFF10);
	Scr_GetNumParam_t Scr_GetNumParam = Scr_GetNumParam_t(0x4B0E90);
	Scr_GetEntityId_t Scr_GetEntityId = Scr_GetEntityId_t(0x4165E0);

	Scr_ExecThread_t Scr_ExecThread = Scr_ExecThread_t(0x4AD0B0);
	Scr_FreeThread_t Scr_FreeThread = Scr_FreeThread_t(0x4BD320);

	Scr_AddEntity_t Scr_AddEntity = Scr_AddEntity_t(0x4BFB40);
	Scr_AddString_t Scr_AddString = Scr_AddString_t(0x412310);
	Scr_AddConstString_t Scr_AddConstString = Scr_AddConstString_t(0x488860);
	Scr_AddIString_t Scr_AddIString = Scr_AddIString_t(0x455F20);
	Scr_AddInt_t Scr_AddInt = Scr_AddInt_t(0x41D7D0);
	Scr_AddFloat_t Scr_AddFloat = Scr_AddFloat_t(0x61E860);
	Scr_AddObject_t Scr_AddObject = Scr_AddObject_t(0x430F40);
	Scr_Notify_t Scr_Notify = Scr_Notify_t(0x4A4750);
	Scr_NotifyLevel_t Scr_NotifyLevel = Scr_NotifyLevel_t(0x4D9C30);

	Scr_Error_t Scr_Error = Scr_Error_t(0x61E8B0);
	Scr_ObjectError_t Scr_ObjectError = Scr_ObjectError_t(0x42EF40);
	Scr_ParamError_t Scr_ParamError = Scr_ParamError_t(0x4FBC70);

	Scr_GetType_t Scr_GetType = Scr_GetType_t(0x422900);
	Scr_GetPointerType_t Scr_GetPointerType = Scr_GetPointerType_t(0x4828E0);

	Scr_ClearOutParams_t Scr_ClearOutParams = Scr_ClearOutParams_t(0x4386E0);

	Scr_GetObjectField_t Scr_GetObjectField = Scr_GetObjectField_t(0x4FF3D0);
	Scr_SetObjectField_t Scr_SetObjectField = Scr_SetObjectField_t(0x4F20F0);
	Scr_GetEntityField_t Scr_GetEntityField = Scr_GetEntityField_t(0x4E8390);
	Scr_SetClientField_t Scr_SetClientField = Scr_SetClientField_t(0x4A6DF0);
	Scr_AddClassField_t Scr_AddClassField = Scr_AddClassField_t(0x4C0E70);

	GetEntity_t GetEntity = GetEntity_t(0x4BC270);
	GetPlayerEntity_t GetPlayerEntity = GetPlayerEntity_t(0x49C4A0);

	Scr_RegisterFunction_t Scr_RegisterFunction = Scr_RegisterFunction_t(0x492D50);
	Scr_ShutdownAllocNode_t Scr_ShutdownAllocNode = Scr_ShutdownAllocNode_t(0x441650);
	Scr_IsSystemActive_t Scr_IsSystemActive = Scr_IsSystemActive_t(0x4B24E0);

	scr_const_t* scr_const = reinterpret_cast<scr_const_t*>(0x1AA2E00);

	scrVmPub_t* scrVmPub = reinterpret_cast<scrVmPub_t*>(0x2040CF0);
	scrVarPub_t* scrVarPub = reinterpret_cast<scrVarPub_t*>(0x201A408);

	void IncInParam()
	{
		Scr_ClearOutParams();

		if (scrVmPub->top == scrVmPub->maxStack)
		{
			Sys_Error("Internal script stack overflow");
		}

		scrVmPub->top++;
		scrVmPub->inparamcount++;
	}

	void Scr_AddBool(int value)
	{
		assert(value == 0 || value == 1);

		IncInParam();
		scrVmPub->top->type = VAR_INTEGER;
		scrVmPub->top->u.intValue = value;
	}

	void RuntimeErrorInternal(int channel, const char* codePos, unsigned int index, const char* msg)
	{
		static DWORD RuntimeErrorInternal_t = 0x61ABE0;

		__asm
		{
			pushad
			mov eax, msg
			mov edi, channel
			push index
			push codePos
			call RuntimeErrorInternal_t
			add esp, 0x8
			popad
		}
	}

	void Scr_NotifyId(unsigned int id, unsigned __int16 stringValue, unsigned int paramcount)
	{
		static DWORD Scr_NotifyId_t = 0x61E670;

		__asm
		{
			pushad
			mov eax, paramcount
			push stringValue
			push id
			call Scr_NotifyId_t
			add esp, 8h
			popad
		}
	}
}
