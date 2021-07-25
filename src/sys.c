#include <stdio.h>
#include <sys/stat.h>

#include "sys.h"

#define F_Link 0
#define F_Load 1
#define F_UnLink 2
#define F_Fork 3
#define F_Wait 4
#define F_Chain 5
#define F_Exit 6
#define F_Mem 7
#define F_Send 8
#define F_Icpt 9
#define F_Sleep 10
#define F_SSpd 11
#define F_ID 12
#define F_SPrior 13
#define F_STrap 14
#define F_PErr 15
#define F_PrsNam 16
#define F_CmpNam 17
#define F_SchBit 18
#define F_AllBit 19
#define F_DelBit 20
#define F_Time 21
#define F_STime 22
#define F_CRC 23
#define F_GPrDsc 24
#define F_GBlkMp 25
#define F_GModDr 26
#define F_CpyMem 27
#define F_SUser 28
#define F_UnLoad 29
#define F_RTE 30
#define F_GPrDBT 31
#define F_Julian 32
#define F_TLink 33
#define F_DFork 34
#define F_DExec 35
#define F_DExit 36
#define F_DatMod 37
#define F_SetCRC 38
#define F_SetSys 39
#define F_SRqMem 40
#define F_SRtMem 41
#define F_IRQ 42
#define F_IOQu 43
#define F_AProc 44
#define F_NProc 45
#define F_VModul 46
#define F_FindPD 47
#define F_AllPD 48
#define F_RetPD 49
#define F_SSvc 50
#define F_IODel 51
#define F_GProcP 55
#define F_Move 56
#define F_AllRAM 57
#define F_Permit 58
#define F_Protect 59
#define F_AllTsk 63
#define F_DelTsk 64
#define F_AllPrc 75
#define F_DelPrc 76
#define F_FModul 78
#define F_SysDbg 82
#define F_Event 83
#define F_Gregor 84
#define F_SysID 85
#define F_Alarm 86
#define F_SigMask 87
#define F_ChkMem 88
#define F_UAcct 89
#define F_CCtl 90
#define F_GSPUMp 91
#define F_SRqCMem 92
#define F_POSK 93
#define F_Panic 94
#define F_MBuf 95
#define F_Trans 96
#define I_Attach 128
#define I_Org 128
#define I_Detach 129
#define I_Dup 130
#define I_Create 131
#define I_Open 132
#define I_MakDir 133
#define I_ChgDir 134
#define I_Delete 135
#define I_Seek 136
#define I_Read 137
#define I_Write 138
#define I_ReadLn 139
#define I_WritLn 140
#define I_GetStt 141
#define I_SetStt 142
#define I_Close 143
#define I_SGetSt 146
#define I_Last 147

#define SS_Opt 0
#define SS_Ready 1
#define SS_Size 2
#define SS_Reset 3
#define SS_WTrk 4
#define SS_Pos 5
#define SS_EOF 6
#define SS_Link 7
#define SS_ULink 8
#define SS_Feed 9
#define SS_Frz 10
#define SS_SPT 11
#define SS_SQD 12
#define SS_DCmd 13
#define SS_DevNm 14
#define SS_FD 15
#define SS_Ticks 16
#define SS_Lock 17
#define SS_DStat 18
#define SS_Joy 19
#define SS_BlkRd 20
#define SS_BlkWr 21
#define SS_Reten 22
#define SS_WFM 23
#define SS_RFM 24
#define SS_ELog 25
#define SS_SSig 26
#define SS_Relea 27
#define SS_Attr 28
#define SS_Break 29
#define SS_RsBit 30
#define SS_RMS 31
#define SS_FDInf 32
#define SS_ACRTC 33
#define SS_IFC 34
#define SS_OFC 35
#define SS_EnRTS 36
#define SS_DsRTS 37
#define SS_DCOn 38
#define SS_DCOff 39
#define SS_Skip 40
#define SS_Mode 41
#define SS_Open 42
#define SS_Close 43
#define SS_Path 44
#define SS_Play 45
#define SS_HEADER 46
#define SS_Raw 47
#define SS_Seek 48
#define SS_Abort 49
#define SS_CDDA 50
#define SS_Pause 51
#define SS_Eject 52
#define SS_Mount 53
#define SS_Stop 54
#define SS_Cont 55
#define SS_Disable 56
#define SS_Enable 57
#define SS_ReadToc 58
#define SS_SM 59
#define SS_SD 60
#define SS_SC 61
#define SS_SEvent 62
#define SS_Sound 63
#define SS_DSize 64
#define SS_Net 65
#define SS_Rename 66
#define SS_Free 67
#define SS_VarSect 68
#define SS_UCM 76
#define SS_DM 81
#define SS_GC 82
#define SS_RG 83
#define SS_DP 84
#define SS_DR 85
#define SS_DC 86
#define SS_CO 87
#define SS_VIQ 88
#define SS_PT 89
#define SS_SLink 90
#define SS_KB 91
#define SS_Bind 108
#define SS_Listen 109
#define SS_Connect 110
#define SS_Resv 111
#define SS_Accept 112
#define SS_Recv 113
#define SS_Send 114
#define SS_GNam 115
#define SS_SOpt 116
#define SS_GOpt 117
#define SS_Shut 118
#define SS_SendTo 119
#define SS_RecvFr 120
#define SS_Install 121
#define SS_PCmd 122
#define SS_SN 140
#define SS_AR 141
#define SS_MS 142
#define SS_AC 143
#define SS_CDFD 144
#define SS_CChan 145
#define SS_FG 146

#define DC_CrFCT 0
#define DC_RdFCT 1
#define DC_WrFCT 2
#define DC_RdFI 3
#define DC_WrFI 4
#define DC_DlFCT 5
#define DC_CrLCT 6
#define DC_RdLCT 7
#define DC_WrLCT 8
#define DC_RdLI 9
#define DC_WrLI 10
#define DC_DlLCT 11
#define DC_FLnk 12
#define DC_LLnk 13
#define DC_Exec 14
#define DC_Intl 15
#define DC_NOP 16
#define DC_SSig 17
#define DC_Relea 18
#define DC_SetCmp 19
#define DC_DsplSiz 20
#define DC_GetClut 21
#define DC_GetCluts 22
#define DC_SetClut 23
#define DC_SetCluts 24
#define DC_MapDM 25
#define DC_Off 26
#define DC_PRdLCT 27
#define DC_PWrLCT 28

int dc_setcmp(short path, short mode) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	int result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[mode], %%d3\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.l %%d0, %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: moveq #-1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DC),
			[fncode]"irm"(DC_SetCmp),
			[mode]"irm"(mode)
			: "cc", "d0", "d1", "d2", "d3", "memory");
	return result;
#endif
}
short dc_intl(short path, short mode) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[mode], %%d3\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d0, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DC),
			[fncode]"irm"(DC_Intl),
			[mode]"irm"(mode)
			: "cc", "d0", "d1", "d2", "d3", "memory");
	return result;
#endif
}
// NOTE: char is actually short in OS9C abi func-arg, and ROM:video may treat it as short!!
short dc_crfct(short path, short planeno, short fctlen, /*char*/short res) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[planeno], %%d3\n"
			"move.w %[fctlen], %%d4\n"
			"move.w %[res], %%d5\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.w %%d0, %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w #-1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DC),
			[fncode]"irm"(DC_CrFCT),
			[planeno]"irm"(planeno),
			[fctlen]"irm"(fctlen),
			[res]"irm"(res)
			: "cc", "d0", "d1", "d2", "d3", "d4", "d5", "memory");
	return result;
#endif
}
short dc_wrfct(short path, short fctid, short startins, short count, unsigned int *buff) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[fctid], %%d3\n"
			"move.w %[startins], %%d4\n"
			"move.w %[count], %%d5\n"
			"movea.l %[buff], %%a0\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DC),
			[fncode]"irm"(DC_WrFCT),
			[fctid]"irm"(fctid),
			[startins]"irm"(startins),
			[count]"irm"(count),
			[buff]"irm"(buff)
			: "cc", "a0", "d0", "d1", "d2", "d3", "d4", "d5", "memory");
	return result;
#endif
}
short dc_exec(short path, short fctid1, short fctid2) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[fctid1], %%d3\n"
			"move.w %[fctid2], %%d4\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DC),
			[fncode]"irm"(DC_Exec),
			[fctid1]"irm"(fctid1),
			[fctid2]"irm"(fctid2)
			: "cc", "d0", "d1", "d2", "d3", "d4", "memory");
	return result;
#endif
}
short dc_ssig(short path, short sigcode, short count) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[sigcode], %%d3\n"
			"move.w %[count], %%d4\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DC),
			[fncode]"irm"(DC_SSig),
			[sigcode]"irm"(sigcode),
			[count]"irm"(count)
			: "cc", "d0", "d1", "d2", "d3", "d4", "memory");
	return result;
#endif
}
short dc_wrfi(short path, short fctid, short number, int instruct) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[fctid], %%d3\n"
			"move.w %[number], %%d4\n"
			"move.l %[instruct], %%d5\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DC),
			[fncode]"irm"(DC_WrFI),
			[fctid]"irm"(fctid),
			[number]"irm"(number),
			[instruct]"irm"(instruct)
			: "cc", "d0", "d1", "d2", "d3", "d4", "d5", "memory");
	return result;
#endif
}
short dc_dlfct(short path, short fctid) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[fctid], %%d3\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DC),
			[fncode]"irm"(DC_DlFCT),
			[fctid]"irm"(fctid)
			: "cc", "d0", "d1", "d2", "d3", "memory");
	return result;
#endif
}
// NOTE: char is actually short in OS9C abi func-arg, and ROM:video may treat it as short!!
short dc_crlct(short path, short planeno, short lctlen, /*char*/short res) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[planeno], %%d3\n"
			"move.w %[lctlen], %%d4\n"
			"move.w %[res], %%d5\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.w %%d0, %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w #-1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DC),
			[fncode]"irm"(DC_CrLCT),
			[planeno]"irm"(planeno),
			[lctlen]"irm"(lctlen),
			[res]"irm"(res)
			: "cc", "d0", "d1", "d2", "d3", "d4", "d5", "memory");
	return result;
#endif
}
int dc_rdli(short path, short lctid, short lineno, short insnum) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	int result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[lctid], %%d3\n"
			"move.w %[lineno], %%d4\n"
			"move.w %[insnum], %%d5\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.l %%d0, %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: moveq.l #-1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DC),
			[fncode]"irm"(DC_RdLI),
			[lctid]"irm"(lctid),
			[lineno]"irm"(lineno),
			[insnum]"irm"(insnum)
			: "cc", "d0", "d1", "d2", "d3", "d4", "d5", "memory");
	return result;
#endif
}
short dc_wrli(short path, short lctid, short lineno, short insnum, int instruct) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[lctid], %%d3\n"
			"move.w %[lineno], %%d4\n"
			"move.w %[insnum], %%d5\n"
			"move.l %[instruct], %%d6\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DC),
			[fncode]"irm"(DC_WrLI),
			[lctid]"irm"(lctid),
			[lineno]"irm"(lineno),
			[insnum]"irm"(insnum),
			[instruct]"irm"(instruct)
			: "cc", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "memory");
	return result;
#endif
}
short dc_dllct(short path, short lctid) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[lctid], %%d3\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DC),
			[fncode]"irm"(DC_DlLCT),
			[lctid]"irm"(lctid)
			: "cc", "d0", "d1", "d2", "d3", "memory");
	return result;
#endif
}
short dc_nop(short path, short lctid, short startline, short startcol, short numlines, short numcols) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[lctid], %%d3\n"
			"move.w %[startline], %%d4\n"
			"move.w %[startcol], %%d5\n"
			"move.w %[numlines], %%d6\n"
			"move.w %[numcols], %%d7\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=m"(result) // "d" -> cannot find a register in class 'DATA_REGS' while reloading 'asm' (...ignore please, gcc)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DC),
			[fncode]"irm"(DC_NOP),
			[lctid]"irm"(lctid),
			[startline]"irm"(startline),
			[startcol]"irm"(startcol),
			[numlines]"irm"(numlines),
			[numcols]"irm"(numcols)
			: "cc", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "memory");
	return result;
#endif
}
short dc_flnk(short path, short fctid, short lctid, short lctline) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[fctid], %%d3\n"
			"move.w %[lctid], %%d4\n"
			"move.w %[lctline], %%d5\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DC),
			[fncode]"irm"(DC_FLnk),
			[fctid]"irm"(fctid),
			[lctid]"irm"(lctid),
			[lctline]"irm"(lctline)
			: "cc", "d0", "d1", "d2", "d3", "d4", "d5", "memory");
	return result;
#endif
}
short dc_llnk(short path, short lctid1, short line1, short lctid2, short line2) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[lctid1], %%d3\n"
			"move.w %[line1], %%d4\n"
			"move.w %[lctid2], %%d5\n"
			"move.w %[line2], %%d6\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DC),
			[fncode]"irm"(DC_LLnk),
			[lctid1]"irm"(lctid1),
			[line1]"irm"(line1),
			[lctid2]"irm"(lctid2),
			[line2]"irm"(line2)
			: "cc", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "memory");
	return result;
#endif
}

#define DP_Ptn 0
#define DP_PAln 1
#define DP_SCMM 2
#define DP_SCR 3
#define DP_GFnt 4
#define DP_AFnt 5
#define DP_DFnt 6
#define DP_RFnt 7
#define DP_Clip 8
#define DP_PnSz 9
#define DP_PStyl 10
#define DP_TCol 11

short dp_gfnt(short path, char *fntname) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"movea.l %[fntname], %%a0\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.w %%d0, %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w #-1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DP),
			[fncode]"irm"(DP_GFnt),
			[fntname]"irm"(fntname)
			: "cc", "a0", "d0", "d1", "d2", "memory");
	return result;
#endif
}
short dp_afnt(short path, short dmid, short fntident, short afn) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[dmid], %%d3\n"
			"move.w %[fntident], %%d4\n"
			"move.w %[afn], %%d5\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DP),
			[fncode]"irm"(DP_AFnt),
			[dmid]"irm"(dmid),
			[fntident]"irm"(fntident),
			[afn]"irm"(afn)
			: "cc", "d0", "d1", "d2", "d3", "d4", "d5", "memory");
	return result;
#endif
}
short dp_scmm(short path, short dmid, short mapmethod) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[dmid], %%d3\n"
			"move.w %[mapmethod], %%d4\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DP),
			[fncode]"irm"(DP_SCMM),
			[dmid]"irm"(dmid),
			[mapmethod]"irm"(mapmethod)
			: "cc", "d0", "d1", "d2", "d3", "d4", "memory");
	return result;
#endif
}
short dp_scr(short path, short dmid, short regno, short colordata) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[dmid], %%d3\n"
			"move.w %[regno], %%d4\n"
			"move.w %[colordata], %%d5\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DP),
			[fncode]"irm"(DP_SCR),
			[dmid]"irm"(dmid),
			[regno]"irm"(regno),
			[colordata]"irm"(colordata)
			: "cc", "d0", "d1", "d2", "d3", "d4", "d5", "memory");
	return result;
#endif
}

#define DM_Creat 0
#define DM_Org 1
#define DM_Copy 2
#define DM_Exch 3
#define DM_TCpy 4
#define DM_TExc 5
#define DM_Write 6
#define DM_IrWr 7
#define DM_Read 8
#define DM_WrPix 9
#define DM_RdPix 10
#define DM_Cncl 11
#define DM_Close 12
#define DM_DMDup 13

DrawmapDesc *dm_create(short path, short plane, short type, unsigned short width, unsigned short height, int length, int qlength) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	DrawmapDesc *result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[plane], %%d3\n"
			"move.w %[type], %%d4\n"
			"move.l %[wh], %%d5\n"
			"move.l %[length], %%d6\n"
			"move.l %[qlength], %%d7\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.l %%a0, %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: clr.l %[result]\n"
			".ok%=:"
			: [result]"=m"(result) // "d" -> cannot find a register in class 'DATA_REGS' while reloading 'asm' (...ignore please, gcc)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DM),
			[fncode]"irm"(DM_Creat),
			[plane]"irm"(plane),
			[type]"irm"(type),
			[wh]"irm"(((unsigned int)width << 16) | height),
			[length]"irm"(length),
			[qlength]"irm"(qlength)
			: "cc", "a0", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "memory");
	return result;
#endif
}
short dm_wrpix(short path, short dmid, short x, short y, short pixdata) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[dmid], %%d3\n"
			"move.l %[hv], %%d4\n"
			"move.w %[data], %%d5\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.w #1, %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: clr.w %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DM),
			[fncode]"irm"(DM_WrPix),
			[dmid]"irm"(dmid),
			[hv]"irm"(((unsigned int)(unsigned short)x << 16) | (unsigned short)y),
			[data]"irm"(pixdata)
			: "cc", "d0", "d1", "d2", "d3", "d4", "d5", "memory");
	return result;
#endif
}

#define DR_Dot 0
#define DR_Line 1
#define DR_PLin 2
#define DR_CArc 3
#define DR_EArc 4
#define DR_Rect 5
#define DR_ERect 6
#define DR_PGon 7
#define DR_Circ 8
#define DR_CWdg 9
#define DR_Elps 10
#define DR_EWdg 11
#define DR_DRgn 12
#define DR_BFil 13
#define DR_FFil 14
#define DR_Copy 15
#define DR_Text 16
#define DR_JTxt 17

short dr_rect(short path, short dmid, short opcode, short sx, short sy, short ex, short ey) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.l %[dmid_op], %%d3\n"
			"move.l %[shv], %%d4\n"
			"move.l %[ehv], %%d5\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DR),
			[fncode]"irm"(DR_Rect),
			[dmid_op]"irm"(((unsigned int)(unsigned short)dmid << 16) | (unsigned short)opcode),
			[shv]"irm"(((unsigned int)(unsigned short)sx << 16) | (unsigned short)sy),
			[ehv]"irm"(((unsigned int)(unsigned short)ex << 16) | (unsigned short)ey)
			: "cc", "d0", "d1", "d2", "d3", "d4", "d5", "memory");
	return result;
#endif
}
short dr_text(short path, int dmid, int opcode, short x, short y, char *str, short maxchrs) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.l %[dmid_op], %%d3\n"
			"move.l %[hv], %%d4\n"
			"movea.l %[text], %%a0\n"
			"move.w %[maxchrs], %%d5\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_DR),
			[fncode]"irm"(DR_Text),
			[dmid_op]"irm"(((unsigned int)(unsigned short)dmid << 16) | (unsigned short)opcode),
			[hv]"irm"(((unsigned int)(unsigned short)x << 16) | (unsigned short)y),
			[text]"irm"(str),
			[maxchrs]"irm"(maxchrs)
			: "cc", "a0", "d0", "d1", "d2", "d3", "d4", "d5", "memory");
	return result;
#endif
}

#define GC_Pos 0
#define GC_Show 1
#define GC_Hide 2
#define GC_Ptn 3
#define GC_Col 4
#define GC_Blnk 5
#define GC_Org 6

int gc_org(short path, short x, short y) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.l %[hv], %%d3\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_GC),
			[fncode]"irm"(GC_Org),
			[hv]"irm"(((unsigned int)(unsigned short)x << 16) | (unsigned short)y)
			: "cc", "d0", "d1", "d2", "d3", "memory");
	return result;
#endif
}
short gc_ptn(short path, short hitx, short hity, short width, short height, int res, void *pattern) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.l %[hv], %%d3\n"
			"move.l %[wh], %%d4\n"
			"move.w %[res], %%d5\n"
			"movea.l %[pattern], %%a0\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_GC),
			[fncode]"irm"(GC_Ptn),
			[hv]"irm"(((unsigned int)(unsigned short)hitx << 16) | (unsigned short)hity),
			[wh]"irm"(((unsigned int)(unsigned short)width << 16) | (unsigned short)height),
			[res]"irm"(res),
			[pattern]"irm"(pattern)
			: "cc", "a0", "d0", "d1", "d2", "d3", "d4", "d5", "memory");
	return result;
#endif
}
short gc_col(short path, int color) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.l %[color], %%d3\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_GC),
			[fncode]"irm"(GC_Col),
			[color]"irm"(color)
			: "cc", "d0", "d1", "d2", "d3", "memory");
	return result;
#endif
}
short gc_hide(short path) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_GC),
			[fncode]"irm"(GC_Hide)
			: "cc", "d0", "d1", "d2", "memory");
	return result;
#endif
}
short gc_show(short path) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_GC),
			[fncode]"irm"(GC_Show)
			: "cc", "d0", "d1", "d2", "memory");
	return result;
#endif
}
short gc_pos(short path, short x, short y) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.l %[hv], %%d3\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_GC),
			[fncode]"irm"(GC_Pos),
			[hv]"irm"(((unsigned int)(unsigned short)x << 16) | (unsigned short)y)
			: "cc", "d0", "d1", "d2", "d3", "memory");
	return result;
#endif
}

#define SC_Atten 0

short sc_atten(short path, int attenvals) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.l %[attenvals], %%d3\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_SC),
			[fncode]"irm"(SC_Atten),
			[attenvals]"irm"(attenvals)
			: "cc", "d0", "d1", "d2", "d3", "memory");
	return result;
#endif
}

#define PT_Coord 0
#define PT_SSig 1
#define PT_Relea 2
#define PT_Pos 3
#define PT_Org 4

short pt_org(short path, short x, short y) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.l %[hv], %%d3\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_PT),
			[fncode]"irm"(PT_Org),
			[hv]"irm"(((unsigned int)(unsigned short)x << 16) | (unsigned short)y)
			: "cc", "d0", "d1", "d2", "d3", "memory");
	return result;
#endif
}
short pt_pos(short path, short x, short y) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.l %[hv], %%d3\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_PT),
			[fncode]"irm"(PT_Pos),
			[hv]"irm"(((unsigned int)(unsigned short)x << 16) | (unsigned short)y)
			: "cc", "d0", "d1", "d2", "d3", "memory");
	return result;
#endif
}
short pt_ssig(short path, short sigcode) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[sigcode], %%d3\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_PT),
			[fncode]"irm"(PT_SSig),
			[sigcode]"irm"(sigcode)
			: "cc", "d0", "d1", "d2", "d3", "memory");
	return result;
#endif
}
// note: short is enough, int for compat.
short pt_coord(short path, int *btnstate, int *x, int *y) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"ext.l %%d0\n"
			"move.l %%d0, %[btnstate]\n"
			"move.l %%d1, %%d0\n"
			"ext.l %%d1\n"
			"move.l %%d1, %[y]\n"
			"swap %%d0\n"
			"ext.l %%d0\n"
			"move.l %%d0, %[x]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result),
			[btnstate]"=m"(*btnstate),
			[x]"=m"(*x),
			[y]"=m"(*y)
			: [syscallno]"i"(I_GetStt), // NOTE: GetStt, not SetStt.
			[path]"irm"(path),
			[sscode]"irm"(SS_PT),
			[fncode]"irm"(PT_Coord)
			: "cc", "d0", "d1", "d2", "memory");
	return result;
#endif
}

#define SM_Creat 0
#define SM_Out 1
#define SM_Off 2
#define SM_Cncl 3
#define SM_Close 4
//#define SM_Loop 5
//#define SM_In 6
#define SM_Info 0
#define SM_Stat 1

short sm_creat(short path, short datatype, short numgroups, /*out*/ char **mapaddr) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[datatype], %%d3\n"
			"move.w %[numgroups], %%d4\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.w %%d0, %[result]\n"
			"move.l %%a0, %[mapaddr]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w #-1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result),
			[mapaddr]"=m"(*mapaddr)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_SM),
			[fncode]"irm"(SM_Creat),
			[datatype]"irm"(datatype),
			[numgroups]"irm"(numgroups)
			: "cc", "a0", "d0", "d1", "d2", "d3", "d4", "memory");
	return result;
#endif
}
short sm_out(short path, short smid, STAT_BLK *statusblk) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[smid], %%d3\n"
			"movea.l %[statusblk], %%a0\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_SM),
			[fncode]"irm"(SM_Out),
			[smid]"irm"(smid),
			[statusblk]"irm"(statusblk)
			: "cc", "a0", "d0", "d1", "d2", "d3", "memory");
	return result;
#endif
}

#define SD_MMix 0
#define SD_SMix 1
#define SD_Loop 2

short sd_smix(short path, short smids, short smidt, short smidd, short stsct, short chansel) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"move.w %[fncode], %%d2\n"
			"move.w %[smids], %%d3\n"
			"move.w %[smidt], %%d4\n"
			"move.w %[smidd], %%d5\n"
			"move.w %[stsct], %%d6\n"
			"move.w %[chansel], %%d7\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=m"(result) // "d" -> cannot find a register in class 'DATA_REGS' while reloading 'asm' (...ignore please, gcc)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_SD),
			[fncode]"irm"(SD_SMix),
			[smids]"irm"(smids),
			[smidt]"irm"(smidt),
			[smidd]"irm"(smidd),
			[stsct]"irm"(stsct),
			[chansel]"irm"(chansel)
			: "cc", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "memory");
	return result;
#endif
}

short ss_play(short path, PCB *pcb) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[sscode], %%d1\n"
			"movea.l %[pcb], %%a0\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(path),
			[sscode]"irm"(SS_Play),
			[pcb]"irm"(pcb)
			: "cc", "a0", "d0", "d1", "memory");
	return result;
#endif
}

int alm_cycle(int sigcode, int timeinterval) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	int result;
	__asm__ volatile(
			"clr.l %%d0\n"
			"move.w %[acode], %%d1\n"
			"move.l %[sigcode], %%d2\n"
			"move.l %[timeinterval], %%d3\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.l %%d0, %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: ext.l %%d1\n"
			"neg.l %%d1\n"
			"move.l %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(F_Alarm),
			[acode]"irm"(A_Cycle),
			[sigcode]"irm"(sigcode),
			[timeinterval]"irm"(timeinterval)
			: "cc", "d0", "d1", "d2", "d3", "memory");
	return result;
#endif
}

// note: short is enough for sigcode, int for compat.
void (*intercept_callback)(int);
void intercept_thunk(void); // no naked support; written in another object
void intercept(void (*callback)(int)) {
	intercept_callback = callback;
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	// pass a6 as is.
	__asm__ volatile(
			"movea.l %[callback], %%a0\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			:
			: [syscallno]"i"(F_Icpt),
			[callback]"irm"(callback ? intercept_thunk : 0)
			: "cc", "a0", "memory");
#endif
}

void *srqmem(unsigned long/*=uint32_t*/ *len) {
#ifdef ASMOUTGOTOSUPPORT
	register int _len __asm__("d0") = *len;
	register short _err __asm__("d1");
	register void *_ptr __asm__("a2");
	__asm__ goto ("trap #0\n"
	    ".word %c[syscallno]\n"
	    "bcs %[onerror]"
	    : "=a"(_ptr), "+d"(_len), "=d"(_err)
	    : [syscallno]"i"(F_SRqMem)
	    : "cc", "memory"
	    : onerror);
	*len = _len;
	return _ptr;

onerror:
	return 0;
#else
	void *ptr;
	__asm__ volatile(
			"move.l %[len], %%d0\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.l %%d0, %[len]\n"
			"move.l %%a2, %[ptr]\n"
			"bra.s .ok%=\n"
			".onerror%=: clr.l %[ptr]\n"
			".ok%=:"
			: [ptr]"=dm"(ptr),
			[len]"+rm"(*len)
			: [syscallno]"i"(F_SRqMem)
			: "cc", "a2", "d0", "d1", "memory");
	return ptr;
#endif
}
short srtmem(void *ptr, unsigned long len) {
#ifdef ASMOUTGOTOSUPPORT
	register int _len __asm__("d0") = len;
	register short _err __asm__("d1");
	register void *_ptr __asm__("a2") = ptr;
	__asm__ goto("trap #0\n"
	    ".word %c[syscallno]\n"
	    "bcs %[onerror]"
	    : "=d"(_err)
	    : [syscallno]"i"(F_SRtMem), "d"(_len), "a"(_ptr)
	    : "cc", "d1", "memory"
	    : onerror);
	return 0;

onerror:
	return _err;
#else
	short result;
	__asm__ volatile(
			"move.l %[len], %%d0\n"
			"move.l %[ptr], %%a2\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.w %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(F_SRtMem),
			[ptr]"irm"(ptr),
			[len]"irm"(len)
			: "cc", "a2", "d0", "d1", "memory");
	return result;
#endif
}
void *srqcmem(unsigned long len, int memtype) {
	// TODO: save granted size to _srqcsiz
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	void *result;
	unsigned long granted;
	__asm__ volatile(
			"move.l %[len], %%d0\n"
			"move.l %[memtype], %%d1\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.l %%a2, %[result]\n"
			"move.l %%d0, %[granted]\n"
			"bra.s .ok%=\n"
			".onerror%=: clr.l %[result]\n"
			".ok%=:"
			: [result]"=dm"(result),
			[granted]"=dm"(granted)
			: [syscallno]"i"(F_SRqCMem),
			[len]"irm"(len),
			[memtype]"irm"(memtype)
			: "cc", "a2", "d0", "d1", "memory");
	return result;
#endif
}

// TODO: act like original libc (there are 32 segments limit, can't use floodly)
void *malloc(unsigned long/*=uint32_t*/ len) {
	if(!len) {
		return 0;
	}

	len += 4;

	void *ptr = srqmem(&len);

	if(ptr) {
		*(int*)ptr = len;
		return ((char*)ptr) + 4;
	} else {
		return 0;
	}
}
void free(void *ptr) {
	if(!ptr) {
		return;
	}
	ptr = (char*)ptr - 4;
	int len = *(int*)ptr;
	srtmem(ptr, len);
}

short open(const char *pathname, char mode) {
#ifdef ASMOUTGOTOSUPPORT
	register char _mode __asm__("d0") = mode;
	register const void *_pname __asm__("a0") = pathname;
	register short _pnumber __asm__("d0");
	__asm__ goto("trap #0\n"
	    ".word %c[syscallno]\n"
	    "bcs %[onerror]"
	    : "=d"(_pnumber)
	    : [syscallno]"i"(I_Open), "a"(_pname), "d"(_mode)
	    : "cc", "d1", "memory"
	    : onerror);
	return _pnumber; // pathname -> pathnumber

onerror:
	return -1;
#else
	short pathnumber;
	__asm__ volatile(
			"move.l %[pathname], %%a0\n"
			"move.b %[mode], %%d0\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.w %%d0, %[pathnumber]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.l #-1, %[pathnumber]\n"
			".ok%=:"
			: [pathnumber]"=dm"(pathnumber)
			: [syscallno]"i"(I_Open),
			[pathname]"irm"(pathname),
			[mode]"irm"(mode)
			: "cc", "a0", "d0", "d1", "memory");
	return pathnumber;
#endif
}
int close(short pathnumber) {
#ifdef ASMOUTGOTOSUPPORT
	register short _pn __asm__("d0") = pathnumber;
	__asm__ goto("trap #0\n"
	    ".word %c[syscallno]\n"
	    "bcs %[onerror]"
	    :
	    : [syscallno]"i"(I_Close), "a"(_pn)
	    : "cc", "d1", "memory"
	    : onerror);
	return 0;

onerror:
	return -1;
#else
	int result;
	__asm__ volatile(
			"move.w %[pathnumber], %%d0\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.l %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.l #-1, %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_Close),
			[pathnumber]"irm"(pathnumber)
			: "cc", "d0", "d1", "memory");
	return result;
#endif
}
int read(short pathnumber, void *ptr, int len) {
#ifdef ASMOUTGOTOSUPPORT
	register int _pn __asm__("d0") = pathnumber;
	register int _len __asm__("d1") = len;
	register void *_ptr __asm__("a0") = ptr;
	__asm__ goto("trap #0\n"
	    ".word %c[syscallno]\n"
	    "bcs %[onerror]"
	    : "+d"(_len)
	    : [syscallno]"i"(I_Read), "d"(_pn), "a"(_ptr)
	    : "cc", "memory"
	    : onerror);
	return _len; // actual read

onerror:
	return -1;
#else
	int read;
	__asm__ volatile(
			"move.w %[pathnumber], %%d0\n"
			"move.l %[len], %%d1\n"
			"move.l %[ptr], %%a0\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.l %%d1, %[read]\n"
			"bra.s .ok%=\n"
			".onerror%=: clr.l %[read]\n"
			".ok%=:"
			: [read]"=dm"(read)
			: [syscallno]"i"(I_Read),
			[pathnumber]"irm"(pathnumber),
			[len]"irm"(len),
			[ptr]"irm"(ptr)
			: "cc", "a0", "d0", "d1", "memory");
	return read;
#endif
}
int readln(short pathnumber, void *buffer, int buflen) {
#ifdef ASMOUTGOTOSUPPORT
	register int _pn __asm__("d0") = pathnumber;
	register int _buflen __asm__("d1") = buflen;
	register void *_ptr __asm__("a0") = buffer;
	__asm__ goto("trap #0\n"
	    ".word %c[syscallno]\n"
	    "bcs %[onerror]"
	    : "=d"(_buflen)
	    : [syscallno]"i"(I_ReadLn), "d"(_pn), "a"(_ptr)
	    : "cc", "memory"
	    : onerror);
	return _buflen; // actual read

onerror:
	return -1;
#else
	int read;
	__asm__ volatile(
			"move.w %[pathnumber], %%d0\n"
			"move.l %[buflen], %%d1\n"
			"move.l %[buffer], %%a0\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.l %%d1, %[read]\n"
			"bra.s .ok%=\n"
			".onerror%=: clr.l %[read]\n"
			".ok%=:"
			: [read]"=dm"(read)
			: [syscallno]"i"(I_ReadLn),
			[pathnumber]"irm"(pathnumber),
			[buflen]"irm"(buflen),
			[buffer]"irm"(buffer)
			: "cc", "a0", "d0", "d1", "memory");
	return read;
#endif
}
int write(short pathnumber, const char *ptr, int len) {
#ifdef ASMOUTGOTOSUPPORT
	register int _pn __asm__("d0") = pathnumber;
	register int _len __asm__("d1") = len;
	register const char *_ptr __asm__("a0") = ptr;
	__asm__ goto("trap #0\n"
	    ".word %c[syscallno]\n"
	    "bcs %[onerror]"
	    : "+d"(_len)
	    : [syscallno]"i"(I_Write), "d"(_pn), "a"(_ptr)
	    : "cc", "memory"
	    : onerror);
	return _len; // actual wrote

onerror:
	return -1;
#else
	int wrote;
	__asm__ volatile(
			"move.w %[pathnumber], %%d0\n"
			"move.l %[len], %%d1\n"
			"move.l %[ptr], %%a0\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.l %%d1, %[wrote]\n"
			"bra.s .ok%=\n"
			".onerror%=: clr.l %[wrote]\n"
			".ok%=:"
			: [wrote]"=dm"(wrote)
			: [syscallno]"i"(I_Write),
			[pathnumber]"irm"(pathnumber),
			[len]"irm"(len),
			[ptr]"irm"(ptr)
			: "cc", "a0", "d0", "d1", "memory");
	return wrote;
#endif
}
short seek(short pathnumber, int pos) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	short result;
	__asm__ volatile(
			"move.w %[pathnumber], %%d0\n"
			"move.l %[pos], %%d1\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"clr.l %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			"neg.w %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_Seek),
			[pathnumber]"irm"(pathnumber),
			[pos]"irm"(pos)
			: "cc", "d0", "d1", "memory");
	return result;
#endif
}
#if 0
int setstat(int code, short pathnumber, ...) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	int result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[code], %%d1\n"
			//"move.l %[buffer], %%a0\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"moveq.l #1, %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: clr.l %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_SetStt),
			[path]"irm"(pathnumber),
			[code]"irm"(code)
			//[buffer]"irm"(buffer)
			: "cc", /*"a0",*/ "d0", "d1", "memory");
	return result;
#endif
}
#endif
int getstat_pos(short pathnumber) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	int result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[code], %%d1\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.l %%d2, %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			"neg.w %[result]\n"
			"ext.l %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_GetStt),
			[path]"irm"(pathnumber),
			[code]"irm"(SS_Pos)
			: "cc", "d0", "d1", "d2", "memory");
	return result;
#endif
}
int getstat_size(short pathnumber) {
#ifdef ASMOUTGOTOSUPPORT
#error to be implemented
#else
	int result;
	__asm__ volatile(
			"move.w %[path], %%d0\n"
			"move.w %[code], %%d1\n"
			"trap #0\n"
			".word %c[syscallno]\n"
			"bcs.s .onerror%=\n"
			"move.l %%d2, %[result]\n"
			"bra.s .ok%=\n"
			".onerror%=: move.w %%d1, %[result]\n"
			"neg.w %[result]\n"
			"ext.l %[result]\n"
			".ok%=:"
			: [result]"=dm"(result)
			: [syscallno]"i"(I_GetStt),
			[path]"irm"(pathnumber),
			[code]"irm"(SS_Size)
			: "cc", "d0", "d1", "d2", "memory");
	return result;
#endif
}

char *csd_devname(int type, int index) {
	char buffer[256];

	int pn = open("/nvr/csd", UPDAT_);
	if(pn == -1) {
		return 0;
	}

	int i = 0;
	for(;;) {
		int s = readln(pn, buffer, sizeof(buffer) - 1);
		if(s <= 0) {
			close(pn);
			return 0;
		}
		buffer[s] = '\0'; // ensure
		// chomp
		if(buffer[s - 1] == '\r') {
			buffer[s - 1] = '\0';
		}

		// atoi
		char *p = buffer;
		int t = 0;
		for(; '0' <= *p && *p <= '9'; p++) {
			t = t * 10 + *p - '0';
		}
		if(t != type) {
			continue;
		}

		// instance index
		i++;
		if(i != index) {
			continue;
		}

		// found
		if(*p != ':') {
			// corrupted dsd...
			close(pn);
			return 0;
		}
		p++;
		// strdup with colon-terminate
		char *q = p;
		for(; *q && *q != ':'; q++) { /* nothing */ }
		if(*q != ':') {
			// corrupted dsd...
			close(pn);
			return 0;
		}
		char *d = malloc(q - p + 1);
		for(q = d; *p != ':'; p++, q++) {
			*q = *p;
		}
		*q = '\0';
		close(pn);
		return d;
	}
}
char *csd_devparam(char *pathlist) {
	char buffer[256];

	int pn = open("/nvr/csd", UPDAT_);
	if(pn == -1) {
		return 0;
	}

	for(;;) {
		int s = readln(pn, buffer, sizeof(buffer) - 1);
		if(s <= 0) {
			close(pn);
			return 0;
		}
		buffer[s] = '\0'; // ensure
		// chomp
		if(buffer[s - 1] == '\r') {
			buffer[s - 1] = '\0';
		}

		// skip type number
		char *p = buffer;
		for(; *p && *p != ':'; p++) { /*nothing*/ }
		if(!*p) {
			// corrupt
			continue;
		}
		p++;

		// test pathlist
		char *q = pathlist;
		for(; *p && *q && *p != ':'; p++, q++) {
			char lp = *p;
			char lq = *q;
			if(lp != lq) {
				break;
			}
		}
		if(*q) {
			// mismatch
			continue;
		}
		if(*p != ':') {
			// not exact (or corrupt)
			continue;
		}

		// found
		p++;
		// strdup following
		q = p;
		for(; *q; q++) { /* nothing */ }
		char *d = malloc(q - p + 1);
		for(q = d; *p; p++, q++) {
			*q = *p;
		}
		*q = '\0';
		close(pn);
		return d;
	}
}

// newlib fileops
// TODO: set errno and return -1 on error
int _open(const char *name, int flags /* , int mode */ ) {
	return open(name, flags + 1); // O_RDONLY,O_WRONLY,O_RDWR=0..2 -> READ_,WRITE_,UPDAT_=1..3
}
int _read(int file, void *ptr, int len) {
	return read(file, ptr, len);
}
int _write(int file, void *ptr, int len) {
	return write(file, ptr, len);
}
int _close(int file) {
	return close(file);
}

// override abort() to avoid bloating... no more required
/*void abort(void) {
	for(;;);
}*/

// my sprintf that don't call malloc_r (and smaller, with enough format for Nobelia)
int sprintf(char *str, const char *format, ...) {
	__builtin_va_list va;
	__builtin_va_start(va, format);

	const char *p;
	char *q;
	for(p = format, q = str; *p; p++) {
		if(*p == '%') {
			p++;
			int zeropad = 0;
			int width = 0;
			for(; *p; p++) {
				if(width == 0 && *p == '0') {
					zeropad = 1;
				}
				if(*p < '0' || '9' < *p) {
					break;
				}
				width = width * 10 + *p - '0';
			}
			switch(*p) { // incr by for()
			case 'd': {
				int v = __builtin_va_arg(va, int);
				int minus = 0;
				if(v < 0) {
					minus = 1;
					v = -v;
				}
				char *l = q;
				do {
					*q++ = '0' + (v % 10);
					v /= 10;
				} while(v);
				if(minus) {
					*q++ = '-';
				}
				while(q < l + width) {
					*q++ = zeropad ? '0' : ' ';
				}
				char *r = q;
				while(l < r) {
					char t = *l;
					*l++ = *--r;
					*r = t;
				}
				break;
			}
			case '\0':
				p--; // don't let over
				break;
			default:
				// not supported
				break;
			}
		} else {
			*q++ = *p;
		}
	}
	*q = '\0';

	__builtin_va_end(va);

	return q - str;
}

int _fstat(int file, struct stat *st) {
	return -1;
}
int _isatty(int file) {
	return 1;
}
int _lseek(int file, int offset, int whence) {
	switch(whence) {
	case SEEK_SET:
		return seek(file, offset);
	case SEEK_CUR:
		return seek(file, getstat_pos(file) + offset);
	case SEEK_END:
		return seek(file, getstat_size(file) + offset);
	default:
		return -1;
	}
}
