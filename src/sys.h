typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;

int dc_setcmp(short path, short mode);
short dc_intl(short path, short mode);
short dc_crfct(short path, short planeno, short fctlen, /*char*/short res);
short dc_wrfct(short path, short fctid, short startins, short count, unsigned int *buff);
short dc_exec(short path, short fctid1, short fctid2);
short dc_ssig(short path, short sigcode, short count);
short dc_wrfi(short path, short fctid, short number, int instruct);
short dc_dlfct(short path, short fctid);
short dc_crlct(short path, short planeno, short lctlen, short res);
int dc_rdli(short path, short lctid, short lineno, short insnum);
short dc_wrli(short path, short lctid, short lineno, short insnum, int instruct);
short dc_dllct(short path, short lctid);
short dc_nop(short path, short lctid, short startline, short startcol, short numlines, short numcols);
short dc_flnk(short path, short fctid, short lctid, short lctline);
short dc_llnk(short path, short lctid1, short line1, short lctid2, short line2);

short dp_gfnt(short path, char *fntname);
short dp_afnt(short path, short dmid, short fntident, short afn);
short dp_scmm(short path, short dmid, short mapmethod);
short dp_scr(short path, short dmid, short regno, short colordata);

typedef struct DrawmapDesc_t {
	short dm_dnum;
	unsigned short dm_dtype;
	char *dm_map1;
	int *dm_lnatbl1;
	int dm_mapsz1;
	short dm_plane;
	short dm_pxlsz;
	short dm_pmapw;
	short dm_pmaph;
	short dm_xorg;
	short dm_yorg;
	short dm_cminx;
	short dm_cmaxx;
	short dm_cminy;
	short dm_cmaxy;
	short dm_penw;
	short dm_penh;
	int dm_psstr;
	short dm_pscnt;
	unsigned short dm_tcol1;
	char *dm_patt;
	short dm_pmode;
	short dm_poffx;
	short dm_poffy;
	unsigned short dm_crtbl[16];
	short dm_mpmthd;
	short dm_afbeg0;
	short dm_afend0;
	char *dm_afdata0;
	short dm_afbeg1;
	short dm_afend1;
	char *dm_afdata1;
	short dm_afbeg2;
	short dm_afend2;
	char *dm_afdata2;
	short dm_afbeg3;
	short dm_afend3;
	char *dm_afdata3;
	short dm_rgnid;
	char *dm_map2;
	int *dm_lnatbl2;
	int dm_mapsz2;
	unsigned short dm_tcol2;
	char dm_rsrvd[116];
} DrawmapDesc;

// dtype
#define D_TYPE 0x000F
#define D_OFF 0
#define D_CLUT4 3
#define D_CLUT7 4
#define D_CLUT8 5
#define D_RL3 6
#define D_RL7 7
#define D_DYUV 8
#define D_RGB 9
#define D_QHY 11
#define RES_MASK 0x0030
#define RES_HIGH 0x0030
#define RES_DOUBLE 0x0010
#define RES_NORMAL 0x0000
#define LN_SEPARATE 0x8000

// cp_dprm
#define RMS_NORMAL 0x00
#define RMS_RL 0x02
#define RMS_MOSAIC 0x03
#define PRF_X2 0x00
#define PRF_X4 0x01
#define PRF_X8 0x02
#define PRF_X16 0x03
#define BP_NORMAL 0x00
#define BP_DOUBLE 0x01
#define BP_HIGH 0x02

// cp_icm
#define ICM_OFF 0x00
#define ICM_RGB 0x01
#define ICM_CLUT8 0x01
#define ICM_CLUT7 0x03
#define ICM_CLUT77 0x04
#define ICM_DYUV 0x05
#define ICM_CLUT4 0x0b
#define CS_A 0x00
#define CS_B 0x01
#define NM_1 0x00
#define NM_2 0x01
#define EV_OFF 0x00
#define EV_ON 0x01

// cp_tci
#define MIX_ON 0x00
#define MIX_OFF 0x01
#define TR_ON 0x00
#define TR_CKEY_T 0x01
#define TR_TBIT_1 0x02
#define TR_MAT0_T 0x03
#define TR_MAT1_T 0x04
#define TR_M0CK_T 0x05
#define TR_M1CK_T 0x06
#define TR_OFF 0x08
#define TR_CKEY_F 0x09
#define TR_TBIT_F 0x0a
#define TR_MAT0_F 0x0b
#define TR_MAT1_F 0x0c
#define TR_M0CK_F 0x0d
#define TR_M1CK_F 0x0e

// cp_po
#define PR_AB 0x00
#define PR_BA 0x01

// cp_matte
#define MF_MF0 0x00
#define MF_MF1 0x01
#define MO_END 0x00
#define MO_ICF_A 0x04
#define MO_ICF_B 0x06
#define MO_RES 0x08
#define MO_SET 0x09
#define MO_RES_ICFA 0x0c
#define MO_SET_ICFA 0x0d
#define MO_RES_ICFB 0x0e
#define MO_SET_ICFB 0x0f

// cp_bkcol
#define BK_HIGH 0x01
#define BK_LOW 0x00
#define BK_BLACK 0x00
#define BK_BLUE 0x01
#define BK_GREEN 0x02
#define BK_CYAN 0x03
#define BK_RED 0x04
#define BK_MAGENTA 0x05
#define BK_YELLOW 0x06
#define BK_WHITE 0x07

// cp_phld
#define PH_OFF 0x00
#define PH_ON 0x01

// cp_icf
#define ICF_MIN 0x00
#define ICF_MAX 0x3f

// plane index symbol
#define PA 0x00
#define PB 0x01
#define DPA 0x00
#define DPB 0x01

// DCP opcodes
#define _OP_ICM 0xc0001010
#define _OP_TCI 0xc1000000
#define _OP_PO 0xc2000000
#define _OP_TCOL_0 0xc4000000
#define _OP_TCOL_1 0xc6000000
#define _OP_MCOL_0 0xc7000000
#define _OP_MCOL_1 0xc9000000
#define _OP_YUV_0 0xca000000
#define _OP_YUV_1 0xcb000000
#define _OP_BKCOL 0xd8000000
#define _OP_MPH_0 0xd9000000
#define _OP_MPH_1 0xda000000
#define _OP_ICF_0 0xdb000000
#define _OP_ICF_1 0xdc000000
#define _OP_END 0x00000000
#define _OP_NOP 0x10000000
#define _OP_DADR 0x40000000
#define _OP_SIG 0x60000000
#define _OP_DPRM 0x78000400
#define _OP_CLUT 0x80000000
#define _OP_CBNK 0xc3000000
#define _OP_MATTE 0xd0000000

// DCP opcode macros
#define cp_(v,w,s) (((v) & ((1 << w) - 1)) << s)
#define cp_nop() (_OP_NOP)
#define cp_icm(cm0,cm1,nm,ev,cs) (_OP_ICM | cp_(cs,1,22) | cp_(nm,1,19) | cp_(nm,1,18) | cp_(cm1,4,8) | cp_(cm0,4,0))
#define cp_dadr(addr) (_OP_DADR | cp_(addr,24,0))
#define cp_sig() (_OP_SIG)
#define cp_dprm(rms,prf,bp) (_OP_DPRM | cp_(bp,2,8) | cp_(prf,2,2) | cp_(rms,2,0))
#define cp_clut(clut,r,g,b) (_OP_CLUT | cp_(clut,6,24) | cp_(r,8,16) | cp_(g,8,8) | cp_(b,8,0))
#define cp_tci(mix,ta,tb) (_OP_TCI | cp_(mix,1,23) | cp_(tb,4,8) | cp_(ta,4,0))
#define cp_po(order) (_OP_PO | cp_(order,3,0))
#define cp_cbnk(bank) (_OP_CBNK | cp_(bank,2,0))
#define cp_tcol(plane,r,g,b) (((plane)==PA ? _OP_TCOL_0 : _OP_TCOL_1) | cp_(r,8,16) | cp_(g,8,8) | cp_(b,8,0))
#define cp_mcol(plane,r,g,b) (((plane)==PA ? _OP_MCOL_0 : _OP_MCOL_1) | cp_(r,8,16) | cp_(g,8,8) | cp_(b,8,0))
#define cp_yuv(plane,y,u,v) (((plane)==PA ? _OP_YUV_0 : _OP_YUV_1) | cp_(y,8,16) | cp_(u,8,8) | cp_(v,8,0))
#define cp_matte(reg,op,mf,icf,x) (_OP_MATTE | cp_(reg,3,24) | cp_(op,4,20) | cp_(mf,1,16) | cp_(icf,6,10) | cp_(x,10,0))
#define cp_bkcol(intensity,color) (_OP_BKCOL | cp_(intensity,1,3) | cp_(color,3,0))
#define cp_phld(plane,en,factor) (((plane)==PA ? _OP_MPH_0 : _OP_MPH_1) | cp_(en,1,23) | cp_(factor,8,0))
#define cp_icf(plane,icf) (((plane)==PA ? _OP_ICF_0: _OP_ICF_1) | cp_(icf,6,0))

DrawmapDesc *dm_create(short path, short plane, short type, unsigned short width, unsigned short height, int length, int qlength);
short dm_wrpix(short path, short dmid, short x, short y, short pixdata);

// draw opcodes
#define OP_CLPOFF 0x0000
#define OP_CLPON 0x8000

#define OP_FMOUT 0x0000
#define OP_FMFIL 0x4000

#define OP_PMSLD 0x0000
#define OP_PMPAT 0x2000

#define OP_DSSLD 0x0000
#define OP_DSDSH 0x1000

#define OP_DTVIS 0x0000
#define OP_DTTRA 0x0800

#define OP_TMVIS 0x0000
#define OP_TMTRA 0x0200

#define OP_CR0 0x0000
#define OP_CR1 0x0020
#define OP_CR2 0x0040
#define OP_CR3 0x0060
#define OP_CR4 0x0080
#define OP_CR5 0x00A0
#define OP_CR6 0x00C0
#define OP_CR7 0x00E0
#define OP_CR8 0x0100
#define OP_CR9 0x0120
#define OP_CR10 0x0140
#define OP_CR11 0x0160
#define OP_CR12 0x0180
#define OP_CR13 0x01A0
#define OP_CR14 0x01C0
#define OP_CR15 0x01E0

#define OP_ZERO 0x0000
#define OP_SAD 0x0001
#define OP_SAND 0x0002
#define OP_RPLC 0x0003
#define OP_NSAD 0x0004
#define OP_NMOD 0x0005
#define OP_SXD 0x0006
#define OP_SOD 0x0007
#define OP_N_SOD 0x0008
#define OP_N_SXD 0x0009
#define OP_ND 0x000A
#define OP_SOND 0x000B
#define OP_NS 0x000C
#define OP_NSOD 0x000D
#define OP_N_SAD 0x000E
#define OP_ONES 0x000F
#define OP_SPD 0x0010
#define OP_SMD 0x0011
#define OP_DMS 0x0012

short dr_rect(short path, short dmid, short opcode, short sx, short sy, short ex, short ey);
short dr_text(short path, int dmid, int opcode, short x, short y, char *str, short maxchrs);

int gc_org(short path, short x, short y);
short gc_ptn(short path, short hitx, short hity, short width, short height, int res, void *pattern);
short gc_col(short path, int color);
short gc_hide(short path);
short gc_show(short path);
short gc_pos(short path, short x, short y);

short sc_atten(short path, int attenvals);

short pt_org(short path, short x, short y);
short pt_pos(short path, short x, short y);
short pt_ssig(short path, short sigcode);
short pt_coord(short path, int *btnstate, int *x, int *y);

typedef struct _stat_blk {
	short asy_stat;
	short asy_sig;
} STAT_BLK;
typedef struct _pl_err {
	int Err_BufSiz;
	char Err_Res;
	char Err_Rsv1;
	short Err_Cnt;
	short Err_Offset;
	char Err_Sys[8];
	char Err_App[8];
	int Err_Blocks;
} PL_ERR;
typedef struct _pcl {
	char PCL_Ctrl;
	char PCL_Rsv1;
	char PCL_Smode;
	char PCL_Type;
	short PCL_Sig;
	struct _pcl *PCL_Nxt;
	char *PCL_Buf;
	int PCL_BufSz;
	PL_ERR *PCL_Err;
	short PCL_Rsv2;
	int PCL_Cnt;
} PCL;
typedef struct _pcb {
	short PCB_Stat;
	short PCB_Sig;
	int PCB_Rec;
	int PCB_Chan;
	short PCB_AChan;
	PCL **PCB_Video;
	PCL **PCB_Audio;
	PCL **PCB_Data;
} PCB;

#define D_AMONO 0x00
#define D_ASTRO 0x01
#define D_BMONO 0x02
#define D_BSTRO 0x03
#define D_CMONO 0x04
#define D_CSTRO 0x05
#define D_EMPH  0x08

short sm_creat(short path, short datatype, short numgroups, /*out*/ char **mapaddr);
short sm_out(short path, short smid, STAT_BLK *statusblk);

#define MIX_L1L2 0x00
#define MIX_L1R2 0x01
#define MIX_R1L2 0x02
#define MIX_R1R2 0x03

short sd_smix(short path, short smids, short smidt, short smidd, short stsct, short chansel);

short ss_play(short path, PCB *pcb);

#define A_Delete 0
#define A_Set 1
#define A_Cycle 2
#define A_AtDate 3
#define A_AtJul 4

#define SYSRAM 0x01
#define VIDEO1 0x80
#define VIDEO2 0x81

int alm_cycle(int sigcode, int timeinterval);
void intercept(void (*callback)(int));
void *srqmem(unsigned long/*=uint32_t*/ *len);
short srtmem(void *ptr, unsigned long len);
void *srqcmem(unsigned long len, int memtype);

void *malloc(unsigned long len);
void free(void *ptr);

#define READ_ 1
#define WRITE_ 2
#define UPDAT_ (READ_|WRITE_)

short open(const char *pathname, char mode);
int close(short pathnumber);
int read(short pathnumber, void *ptr, int len);
int readln(short pathnumber, void *buffer, int buflen);
int write(short pathnumber, const char *ptr, int len);
short seek(short pathnumber, int pos);
int getstat_pos(short pathnumber);
int getstat_size(short pathnumber);

#define DT_SYSTEM 0
#define DT_CDC 1
#define DT_AUDIO 2
#define DT_VIDEO 3
#define DT_NVRAM 4 // but "/nvr" is clearly defined
#define DT_PTR 5
#define DT_CDPLAY 6
#define DT_AUDSET 7
#define DT_DISPLY 8
#define DT_PIPEDEV 9
#define DT_KEYBRD 10

char *csd_devname(int type, int index);
char *csd_devparam(char *pathlist);

#define printf(...) (void)0
