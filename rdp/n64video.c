/*

MAME Legal Information
License

Redistribution and use of the MAME code or any derivative works are permitted provided that the following conditions are met:

    * Redistributions may not be sold, nor may they be used in a commercial product or activity.
    * Redistributions that are modified from the original source must include the complete source code, including the source code for all components used by a binary built from the modified sources. However, as a special exception, the source code distributed need not include anything that is normally distributed (in either source or binary form) with the major components (compiler, kernel, and so on) of the operating system on which the executable runs, unless that component itself accompanies the executable.
    * Redistributions must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
Trademark

MAME® is a registered trademark of Nicola Salmoria. The "MAME" name and MAME logo may not be used without first obtaining permission of the trademark holder.
Copyright

The code in MAME is the work of hundreds of developers, each of whom owns the copyright to the code they wrote. There is no central copyright authority you can license the code from. The proper way to use the MAME source code is to examine it, using it to understand how the games worked, and then write your own emulation code. Sorry, there is no free lunch here.
Common Questions

Q. Can I include MAME with my product?
A. No. MAME is not licensed for commercial use. Using MAME as a "freebie" or including it at "no cost" with your product still constitutes commerical usage and is forbidden by the license.

Q. Can I sell my product with the MAME logo on it?
A. No. Putting the logo on your product makes it appear that the product is something officially endorsed by Nicola Salmoria, and constitutes trademark infringement.

Q. Can I use the MAME logo to advertise my product?
A. No. Using the logo in your advertising makes it appear that the product is something officially endorsed by Nicola Salmoria, and constitutes trademark infringement.

Q. Can I use the term "MAME" in the name of my software?
A. Generally, no, especially if it is something that is sold. However, if you are producing a free MAME-related piece of software, it is common that permission is granted. Send a query to double- check first, please.

Q. Can I put an arcade cabinet running MAME in a public location?
A. No. This this a commercial use of MAME and is prohibited by the license. Even if you don't charge money, putting a machine in a public location is "operating" an arcade machine and falls under commercial rules in most locations.

Q. Can my non-profit use MAME or an arcade cabinet running MAME to help raise money?
A. No, sorry. Even for the most worthwhile cause, this still is a commercial use of MAME and is prohibited by the license.

Q. How do I obtain a license to the MAME source code?
A. You can't. See the Copyright section above.

Q. Is it legal to download ROMs for a game when I own the PCB?
A. This is unclear and depends on where you live. In most cases you would need to obtain permission from the original manufacturer to do so.

Q. What about the free ROMs on the MAME site? Can I use those with my product?
A. Almost all of the free ROMs on the MAME site are licensed only for non- commercial use, and only for distribution from the MAME site. Just because they are available for "free" here does not grant further redistribution rights, nor does it allow you to treat them as "freebies" for commercial use.

Q. If I obtain a license from an original manufacturer to distribute the ROMs can I use MAME to run them?
A. Generally, no, because it constitutes a commercial use of MAME. However, we have in the past made a couple of exceptions for this particular case. We will not consider making any further exceptions without proof that such a license has already been obtained.

Q. Can I use a PC running MAME to replace a real arcade PCB?
A. In order to do this you would have to use a copy of the original ROMs, which would require obtaining permission from the original manufacturer. Once you had permission from them, if it was used for non-commercial purposes, then you would not technically be violating the MAME license. However we still do not explicitly give permission to use MAME in this way because of the possibility of the game being sold sometime later, which would constitute commercial use of MAME. If you sell your game later you must sell it without MAME included.

Q. Can I ask for donations for the work I did on my port of MAME to platform X?
A. No. You would be earning money from the MAME trademark and copyrights, and that would be a commercial use, which is prohibited by the license. It is our wish that MAME remain free.
*/

/*
This little RDP plugin was initially based on MESS 0.128 source code.
Many thanks to Ville Linde, MooglyGuy and other people who wrote the RDP implementation in MESS 0.128. The rest of the code is by me, angrylion.
Many thanks to people who helped me in various ways: olivieryuyu, marshallh, LaC, oman, pinchy, ziggy, FatCat and other folks I forgot.
The code comes under MAME license.
Sorry for my terrible English.

angrylion
*/

/*
I tried to keep angrylion's plugin as unmodified as possible while making it compatible with CEN64.
This version of n64video was forked from angrylion's googlecode repository (r83) and aligned to r96.

MarathonMan
*/

#include "common.h"
#include "bus/controller.h"
#include "device/device.h"
#include "ri/controller.h"
#include "tctables.h"
#include "vr4300/interface.h"
#include <stdint.h>
#include <string.h>

#define byteswap_16(x) ((uint16_t) (((uint8_t) (x >> 8)) | ((uint16_t) (x << 8))))

#define SP_INTERRUPT	0x1
#define SI_INTERRUPT	0x2
#define AI_INTERRUPT	0x4
#define VI_INTERRUPT	0x8
#define PI_INTERRUPT	0x10
#define DP_INTERRUPT	0x20

#define SP_STATUS_HALT			0x0001
#define SP_STATUS_BROKE			0x0002
#define SP_STATUS_DMABUSY		0x0004
#define SP_STATUS_DMAFULL		0x0008
#define SP_STATUS_IOFULL		0x0010
#define SP_STATUS_SSTEP			0x0020
#define SP_STATUS_INTR_BREAK	0x0040
#define SP_STATUS_SIGNAL0		0x0080
#define SP_STATUS_SIGNAL1		0x0100
#define SP_STATUS_SIGNAL2		0x0200
#define SP_STATUS_SIGNAL3		0x0400
#define SP_STATUS_SIGNAL4		0x0800
#define SP_STATUS_SIGNAL5		0x1000
#define SP_STATUS_SIGNAL6		0x2000
#define SP_STATUS_SIGNAL7		0x4000

#define DP_STATUS_XBUS_DMA		0x01
#define DP_STATUS_FREEZE		0x02
#define DP_STATUS_FLUSH			0x04
#define DP_STATUS_START_GCLK		0x008
#define DP_STATUS_TMEM_BUSY		0x010
#define DP_STATUS_PIPE_BUSY		0x020
#define DP_STATUS_CMD_BUSY			0x040
#define DP_STATUS_CBUF_READY		0x080
#define DP_STATUS_DMA_BUSY			0x100
#define DP_STATUS_END_VALID		0x200
#define DP_STATUS_START_VALID		0x400

#define R4300i_SP_Intr 1

#undef WORD_ADDR_XOR
#define LSB_FIRST 1
#ifdef LSB_FIRST
	#define BYTE_ADDR_XOR		3
	#define WORD_ADDR_XOR		1
	#define BYTE4_XOR_BE(a) 	((a) ^ 3)				
#else
	#define BYTE_ADDR_XOR		0
	#define WORD_ADDR_XOR		0
	#define BYTE4_XOR_BE(a) 	(a)
#endif

#ifdef LSB_FIRST
#define BYTE_XOR_DWORD_SWAP 7
#define WORD_XOR_DWORD_SWAP 3
#else
#define BYTE_XOR_DWORD_SWAP 4
#define WORD_XOR_DWORD_SWAP 2
#endif
#define DWORD_XOR_DWORD_SWAP 1

#define PRESCALE_WIDTH 640
#define PRESCALE_HEIGHT 625
extern const int screen_width, screen_height;

typedef unsigned int offs_t;

static struct cen64_device *cen64;

#define rsp_imem ((uint32_t*)(cen64->rsp.mem+0x1000))
#define rsp_dmem ((uint32_t*)cen64->rsp.mem)

#define rdram ((uint32_t*)cen64->ri.ram)
#define rdram16 ((uint16_t*)cen64->ri.ram)
#define rdram8 (cen64->ri.ram)

#define vi_width (cen64->vi.regs[VI_WIDTH_REG])

#define dp_start (cen64->rdp.regs[DPC_START_REG])
#define dp_end (cen64->rdp.regs[DPC_END_REG])
#define dp_current (cen64->rdp.regs[DPC_CURRENT_REG])
#define dp_status (cen64->rdp.regs[DPC_STATUS_REG])

#define SIGN16(x)	((int16_t)(x))
#define SIGN8(x)	((int8_t)(x))


#define SIGN(x, numb)	(((x) & ((1 << numb) - 1)) | -((x) & (1 << (numb - 1))))
#define SIGNF(x, numb)	((x) | -((x) & (1 << (numb - 1))))




#define GET_LOW(x)	(((x) & 0x3e) << 2)
#define GET_MED(x)	(((x) & 0x7c0) >> 3)
#define GET_HI(x)	(((x) >> 8) & 0xf8)


#define GET_LOW_RGBA16_TMEM(x)	(replicated_rgba[((x) >> 1) & 0x1f])
#define GET_MED_RGBA16_TMEM(x)	(replicated_rgba[((x) >> 6) & 0x1f])
#define GET_HI_RGBA16_TMEM(x)	(replicated_rgba[(x) >> 11])

#define LOG_RDP_EXECUTION 0
#define	DETAILED_LOGGING 0

FILE *rdp_exec;

uint32_t rdp_cmd_data[0x10000];
uint32_t rdp_cmd_ptr = 0;
uint32_t rdp_cmd_cur = 0;
uint32_t ptr_onstart = 0;

extern FILE* zeldainfo;

int32_t oldvstart = 1337;
uint32_t oldhstart = 0;
uint32_t oldsomething = 0;
uint32_t prevwasblank = 0;
uint32_t double_stretch = 0;
int blshifta = 0, blshiftb = 0, pastblshifta = 0, pastblshiftb = 0;
int32_t pastrawdzmem = 0;
uint32_t plim = 0x7fffff;
uint32_t idxlim16 = 0x3fffff;
uint32_t idxlim32 = 0x1fffff;
uint8_t* rdram_8;
uint16_t* rdram_16;
uint32_t brightness = 0;
int32_t iseed = 1;

typedef struct
{
	int lx, rx;
	int unscrx;
	int validline;
  int rgbastwz[8];
	int32_t majorx[4];
	int32_t minorx[4];
	int32_t invalyscan[4];
} SPAN __attribute__((aligned(16)));

cen64_align(static SPAN span[1024], 16);
uint8_t cvgbuf[1024];

static int spans_drgba[4] __attribute__((aligned(16)));
static int spans_dstwz[4] __attribute__((aligned(16)));
static int spans_dzpix;

int16_t spans_cdrgba_drgbady[8] __attribute__((aligned(16)));
int spans_cdz;

static int spans_dstwzdy[4] __attribute__((aligned(16)));



typedef int32_t COLOR[4] __attribute__((aligned(16)));
typedef int8_t FBCOLOR[3];
typedef int8_t CCVG[4];

typedef struct
{
	uint16_t xl, yl, xh, yh;		
} RECTANGLE;

typedef struct
{
	int tilenum;
	uint16_t xl, yl, xh, yh;		
	int16_t s, t;					
	int16_t dsdx, dtdy;			
	uint32_t flip;	
} TEX_RECTANGLE;

typedef struct
{
	int clampdiffs, clampdifft;
	int clampens, clampent;
	int masksclamped, masktclamped;
	int notlutswitch, tlutswitch;
} FAKETILE;

typedef struct
{
	int format;
	int size;
	int line;
	int tmem;
	int palette;
	int ct, mt, cs, ms;
	int mask_t, shift_t, mask_s, shift_s;
	
	uint16_t sl, tl, sh, th;		
	
	FAKETILE f;
} TILE;

typedef struct
{
	int sub_a_rgb0;
	int sub_b_rgb0;
	int mul_rgb0;
	int add_rgb0;
	int sub_a_a0;
	int sub_b_a0;
	int mul_a0;
	int add_a0;

	int sub_a_rgb1;
	int sub_b_rgb1;
	int mul_rgb1;
	int add_rgb1;
	int sub_a_a1;
	int sub_b_a1;
	int mul_a1;
	int add_a1;
} COMBINE_MODES;

typedef struct 
{
	int stalederivs;
	int dolod;
	int partialreject_1cycle; 
	int partialreject_2cycle;
	int special_bsel0;
	int special_bsel1;
	int rgb_alpha_dither;
	int realblendershiftersneeded;
	int interpixelblendershiftersneeded;
} MODEDERIVS;

typedef struct
{
	int cycle_type;
	int persp_tex_en;
	int detail_tex_en;
	int sharpen_tex_en;
	int tex_lod_en;
	int en_tlut;
	int tlut_type;
	int sample_type;
	int mid_texel;
	int bi_lerp0;
	int bi_lerp1;
	int convert_one;
	int key_en;
	int rgb_dither_sel;
	int alpha_dither_sel;
	int blend_m1a_0;
	int blend_m1a_1;
	int blend_m1b_0;
	int blend_m1b_1;
	int blend_m2a_0;
	int blend_m2a_1;
	int blend_m2b_0;
	int blend_m2b_1;
	int force_blend;
	int alpha_cvg_select;
	int cvg_times_alpha;
	int z_mode;
	int cvg_dest;
	int color_on_cvg;
	int image_read_en;
	int z_update_en;
	int z_compare_en;
	int antialias_en;
	int z_source_sel;
	int dither_alpha_en;
	int alpha_compare_en;
	MODEDERIVS f;
} OTHER_MODES;



#define PIXEL_SIZE_4BIT			0
#define PIXEL_SIZE_8BIT			1
#define PIXEL_SIZE_16BIT		2
#define PIXEL_SIZE_32BIT		3

#define CYCLE_TYPE_1			0
#define CYCLE_TYPE_2			1
#define CYCLE_TYPE_COPY			2
#define CYCLE_TYPE_FILL			3


#define FORMAT_RGBA				0
#define FORMAT_YUV				1
#define FORMAT_CI				2
#define FORMAT_IA				3
#define FORMAT_I				4


#define TEXEL_RGBA4				0
#define TEXEL_RGBA8				1
#define TEXEL_RGBA16			2
#define TEXEL_RGBA32			3
#define TEXEL_YUV4				4
#define TEXEL_YUV8				5
#define TEXEL_YUV16				6
#define TEXEL_YUV32				7
#define TEXEL_CI4				8
#define TEXEL_CI8				9
#define TEXEL_CI16				0xa
#define TEXEL_CI32				0xb
#define TEXEL_IA4				0xc
#define TEXEL_IA8				0xd
#define TEXEL_IA16				0xe
#define TEXEL_IA32				0xf
#define TEXEL_I4				0x10
#define TEXEL_I8				0x11
#define TEXEL_I16				0x12
#define TEXEL_I32				0x13


#define CVG_CLAMP				0
#define CVG_WRAP				1
#define CVG_ZAP					2
#define CVG_SAVE				3


#define ZMODE_OPAQUE			0
#define ZMODE_INTERPENETRATING	1
#define ZMODE_TRANSPARENT		2
#define ZMODE_DECAL				3

COMBINE_MODES combine;
OTHER_MODES other_modes;

COLOR blend_color;
COLOR prim_color;
COLOR env_color;
COLOR fog_color;
COLOR combined_color;
COLOR texel0_color;
COLOR texel1_color;
COLOR nexttexel_color;
COLOR shade_color;
COLOR key_scale;
COLOR key_center;
COLOR key_width;
static int32_t noise = 0;
static int32_t primitive_lod_frac = 0;
static int32_t one_color = 0x100;
static int32_t zero_color = 0x00;

int32_t keyalpha;

static int32_t blenderone	= 0xff;


static int32_t *combiner_rgbsub_a_r[2];
static int32_t *combiner_rgbsub_a_g[2];
static int32_t *combiner_rgbsub_a_b[2];
static int32_t *combiner_rgbsub_b_r[2];
static int32_t *combiner_rgbsub_b_g[2];
static int32_t *combiner_rgbsub_b_b[2];
static int32_t *combiner_rgbmul_r[2];
static int32_t *combiner_rgbmul_g[2];
static int32_t *combiner_rgbmul_b[2];
static int32_t *combiner_rgbadd_r[2];
static int32_t *combiner_rgbadd_g[2];
static int32_t *combiner_rgbadd_b[2];

static int32_t *combiner_alphasub_a[2];
static int32_t *combiner_alphasub_b[2];
static int32_t *combiner_alphamul[2];
static int32_t *combiner_alphaadd[2];


static int32_t *blender1a_r[2];
static int32_t *blender1a_g[2];
static int32_t *blender1a_b[2];
static int32_t *blender1b_a[2];
static int32_t *blender2a_r[2];
static int32_t *blender2a_g[2];
static int32_t *blender2a_b[2];
static int32_t *blender2b_a[2];

COLOR pixel_color;
COLOR inv_pixel_color;
COLOR blended_pixel_color;
COLOR memory_color;
COLOR pre_memory_color;

uint32_t fill_color;		

uint32_t primitive_z;
uint16_t primitive_delta_z;

static int fb_format = FORMAT_RGBA;
static int fb_size = PIXEL_SIZE_4BIT;
static int fb_width = 0;
static uint32_t fb_address = 0;

static int ti_format = FORMAT_RGBA;
static int ti_size = PIXEL_SIZE_4BIT;
static int ti_width = 0;
static uint32_t ti_address = 0;

static uint32_t zb_address = 0;

static TILE tile[8];

static RECTANGLE clip = {0,0,0x2000,0x2000};
static int scfield = 0;
static int sckeepodd = 0;
int oldscyl = 0;

uint8_t TMEM[0x1000]; 

#define tlut ((uint16_t*)(&TMEM[0x800]))

#define PIXELS_TO_BYTES(pix, siz) (((pix) << (siz)) >> 1)

typedef struct{
	int startspan;
	int endspan;
	int preendspan;
	int nextspan;
	int midspan;
	int longspan;
	int onelessthanmid;
}SPANSIGS;


static void rdp_set_other_modes(uint32_t w1, uint32_t w2);
static void fetch_texel(COLOR color, int s, int t, uint32_t tilenum);
static void fetch_texel_entlut(COLOR color, int s, int t, uint32_t tilenum);
static void fetch_texel_quadro_rgba16(COLOR color0, COLOR color1, COLOR color2, COLOR color3, const int16_t colors[4]);
static void fetch_texel_quadro(COLOR color0, COLOR color1, COLOR color2, COLOR color3, int s0, int s1, int t0, int t1, uint32_t tilenum);
static void fetch_texel_entlut_quadro(COLOR color0, COLOR color1, COLOR color2, COLOR color3, int s0, int s1, int t0, int t1, uint32_t tilenum);
static void tile_tlut_common_cs_decoder(uint32_t w1, uint32_t w2);
static void loading_pipeline(int start, int end, int tilenum, int coord_quad, int ltlut);
static void get_tmem_idx(int s, int t, uint32_t tilenum, uint32_t* idx0, uint32_t* idx1, uint32_t* idx2, uint32_t* idx3, uint32_t* bit3flipped, uint32_t* hibit);
static void sort_tmem_idx(uint32_t *idx, uint32_t idxa, uint32_t idxb, uint32_t idxc, uint32_t idxd, uint32_t bankno);
static void sort_tmem_shorts_lowhalf(uint32_t* bindshort, uint32_t short0, uint32_t short1, uint32_t short2, uint32_t short3, uint32_t bankno);
static void compute_color_index(uint32_t* cidx, uint32_t readshort, uint32_t nybbleoffset, uint32_t tilenum);
static void read_tmem_copy(int s, int s1, int s2, int s3, int t, uint32_t tilenum, uint32_t* sortshort, int* hibits, int* lowbits);
static void replicate_for_copy(uint32_t* outbyte, uint32_t inshort, uint32_t nybbleoffset, uint32_t tilenum, uint32_t tformat, uint32_t tsize);
static void fetch_qword_copy(uint32_t* hidword, uint32_t* lowdword, int32_t ssss, int32_t ssst, uint32_t tilenum);
static void render_spans_1cycle_complete(int start, int end, int tilenum, int flip, __m128i spans_drgba_v, __m128i spans_dstwz_v, __m128i spans_cdrgba_drgbady_v);
static void render_spans_1cycle_notexel1(int start, int end, int tilenum, int flip, __m128i spans_drgba_v, __m128i spans_dstwz_v, __m128i spans_cdrgba_drgbady_v);
static void render_spans_1cycle_notex(int start, int end, int tilenum, int flip, __m128i spans_drgba_v, __m128i spans_dstwz_v, __m128i spans_cdrgba_drgbady_v);
static void render_spans_2cycle_complete(int start, int end, int tilenum, int flip, __m128i spans_drgba_v, __m128i spans_dstwz_v, __m128i spans_cdrgba_drgbady_v);
static void render_spans_2cycle_notexelnext(int start, int end, int tilenum, int flip, __m128i spans_drgba_v, __m128i spans_dstwz_v, __m128i spans_cdrgba_drgbady_v);
static void render_spans_2cycle_notexel1(int start, int end, int tilenum, int flip, __m128i spans_drgba_v, __m128i spans_dstwz_v, __m128i spans_cdrgba_drgbady_v);
static void render_spans_2cycle_notex(int start, int end, int tilenum, int flip, __m128i spans_drgba_v, __m128i spans_dstwz_v, __m128i spans_cdrgba_drgbady_v);
static void render_spans_fill(int start, int end, int flip);
static void render_spans_copy(int start, int end, int tilenum, int flip);
static inline void combiner_1cycle(int adseed, uint32_t* curpixel_cvg);
static inline void combiner_2cycle(int adseed, uint32_t* curpixel_cvg, int32_t* acalpha);
static inline int blender_1cycle(uint32_t* fr, uint32_t* fg, uint32_t* fb, int dith, uint32_t blend_en, uint32_t prewrap, uint32_t curpixel_cvg, uint32_t curpixel_cvbit);
static inline int blender_2cycle(uint32_t* fr, uint32_t* fg, uint32_t* fb, int dith, uint32_t blend_en, uint32_t prewrap, uint32_t curpixel_cvg, uint32_t curpixel_cvbit, int32_t acalpha);
static inline void texture_pipeline_cycle(COLOR TEX, COLOR prev, int32_t SSS, int32_t SST, uint32_t tilenum, uint32_t cycle);
static inline void tc_pipeline_copy(int32_t* sss0, int32_t* sss1, int32_t* sss2, int32_t* sss3, int32_t* sst, int tilenum);
static inline void tc_pipeline_load(int32_t* sss, int32_t* sst, int tilenum, int coord_quad);
static inline void tcclamp_generic(int32_t* S, int32_t* T, int32_t* SFRAC, int32_t* TFRAC, int32_t maxs, int32_t maxt, int32_t num);
static inline void tcclamp_cycle(int32_t* S, int32_t* T, int32_t* SFRAC, int32_t* TFRAC, int32_t maxs, int32_t maxt, int32_t num);
static inline void tcclamp_cycle_light(int32_t* S, int32_t* T, int32_t maxs, int32_t maxt, int32_t num);
static inline void tcshift_cycle(int32_t* S, int32_t* T, int32_t* maxs, int32_t* maxt, uint32_t num);
static inline void tcshift_copy(int32_t* S, int32_t* T, uint32_t num);
cen64_cold static void precalculate_everything(void);
static inline int alpha_compare(int32_t comb_alpha);
static inline int32_t color_combiner_equation(int32_t a, int32_t b, int32_t c, int32_t d);
static inline int32_t alpha_combiner_equation(int32_t a, int32_t b, int32_t c, int32_t d);
static inline void blender_equation_cycle0(int* r, int* g, int* b);
static inline void blender_equation_cycle0_2(int* r, int* g, int* b);
static inline void blender_equation_cycle1(int* r, int* g, int* b);
static inline uint32_t rightcvghex(uint32_t x, uint32_t fmask); 
static inline uint32_t leftcvghex(uint32_t x, uint32_t fmask);
static inline void compute_cvg_noflip(int32_t scanline);
static inline void compute_cvg_flip(int32_t scanline);
static void fbwrite_4(uint32_t curpixel, uint32_t r, uint32_t g, uint32_t b, uint32_t blend_en, uint32_t curpixel_cvg, uint32_t curpixel_memcvg);
static void fbwrite_8(uint32_t curpixel, uint32_t r, uint32_t g, uint32_t b, uint32_t blend_en, uint32_t curpixel_cvg, uint32_t curpixel_memcvg);
static void fbwrite_16(uint32_t curpixel, uint32_t r, uint32_t g, uint32_t b, uint32_t blend_en, uint32_t curpixel_cvg, uint32_t curpixel_memcvg);
static void fbwrite_32(uint32_t curpixel, uint32_t r, uint32_t g, uint32_t b, uint32_t blend_en, uint32_t curpixel_cvg, uint32_t curpixel_memcvg);
static void fbfill_4(uint32_t curpixel);
static void fbfill_8(uint32_t curpixel);
static void fbfill_16(uint32_t curpixel);
static void fbfill_32(uint32_t curpixel);
static void fbread_4(uint32_t num, uint32_t* curpixel_memcvg);
static void fbread_8(uint32_t num, uint32_t* curpixel_memcvg);
static void fbread_16(uint32_t num, uint32_t* curpixel_memcvg);
static void fbread_32(uint32_t num, uint32_t* curpixel_memcvg);
static void fbread2_4(uint32_t num, uint32_t* curpixel_memcvg);
static void fbread2_8(uint32_t num, uint32_t* curpixel_memcvg);
static void fbread2_16(uint32_t num, uint32_t* curpixel_memcvg);
static void fbread2_32(uint32_t num, uint32_t* curpixel_memcvg);
static inline uint32_t z_decompress(uint32_t rawz);
static inline uint32_t dz_decompress(uint32_t compresseddz);
static inline uint32_t dz_compress(uint32_t value);
cen64_cold static void z_build_com_table(void);
cen64_cold static void precalc_cvmask_derivatives(void);
static inline uint16_t decompress_cvmask_frombyte(uint8_t byte);
static inline void lookup_cvmask_derivatives(uint32_t mask, uint8_t* offx, uint8_t* offy, uint32_t* curpixel_cvg, uint32_t* curpixel_cvbit);
static inline void z_store(uint32_t zcurpixel, uint32_t z, int dzpixenc);
static inline uint32_t z_compare(uint32_t zcurpixel, uint32_t sz, uint16_t dzpix, int dzpixenc, uint32_t* blend_en, uint32_t* prewrap, uint32_t* curpixel_cvg, uint32_t curpixel_memcvg);
static inline int finalize_spanalpha(uint32_t blend_en, uint32_t curpixel_cvg, uint32_t curpixel_memcvg);
static inline int32_t normalize_dzpix(int32_t sum);
static inline int32_t CLIP(int32_t value,int32_t min,int32_t max);
static inline void video_filter16(int* r, int* g, int* b, uint32_t fboffset, uint32_t num, uint32_t hres, uint32_t centercvg, uint32_t fetchstate);
static inline void video_filter32(int* endr, int* endg, int* endb, uint32_t fboffset, uint32_t num, uint32_t hres, uint32_t centercvg, uint32_t fetchstate);
static inline void divot_filter(CCVG final, CCVG centercolor, CCVG leftcolor, CCVG rightcolor);
static inline void restore_filter16(int* r, int* g, int* b, uint32_t fboffset, uint32_t num, uint32_t hres, uint32_t fetchstate);
static inline void restore_filter32(int* r, int* g, int* b, uint32_t fboffset, uint32_t num, uint32_t hres, uint32_t fetchstate);
static inline void gamma_filters(int* r, int* g, int* b, int gamma_and_dither);
static inline void adjust_brightness(int* r, int* g, int* b, int brightcoeff);
static void clearfb16(uint16_t* fb, uint32_t width,uint32_t height);
static void tcdiv_persp(int32_t ss, int32_t st, int32_t sw, int32_t* sss, int32_t* sst);
static void tcdiv_persp_sse(__m128i stwz, int32_t ssst[2]);
static void tcdiv_nopersp(int32_t ss, int32_t st, int32_t sw, int32_t* sss, int32_t* sst);
static void tcdiv_nopersp_sse(__m128i stwz, int32_t ssst[2]);
static inline void tclod_4x17_to_15(int32_t scurr, int32_t snext, int32_t tcurr, int32_t tnext, int32_t previous, int32_t* lod);
static inline void tclod_tcclamp(int32_t* sss, int32_t* sst);
static inline void lodfrac_lodtile_signals(int lodclamp, int32_t lod, uint32_t* l_tile, uint32_t* magnify, uint32_t* distant);
static inline void tclod_1cycle_current(int32_t* sss, int32_t* sst, int32_t nexts, int32_t nextt, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t scanline, int32_t prim_tile, int32_t* t1, SPANSIGS* sigs);
static inline void tclod_1cycle_current_simple(int32_t* sss, int32_t* sst, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t scanline, int32_t prim_tile, int32_t* t1, SPANSIGS* sigs);
static inline void tclod_1cycle_next(int32_t* sss, int32_t* sst, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t scanline, int32_t prim_tile, int32_t* t1, SPANSIGS* sigs, int32_t* prelodfrac);
static inline void tclod_2cycle_current(int32_t* sss, int32_t* sst, int32_t nexts, int32_t nextt, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t prim_tile, int32_t* t1, int32_t* t2);
static inline void tclod_2cycle_current_simple_sse(int32_t ssst[2], __m128i stwz, __m128i dstwzinc, int32_t prim_tile, int32_t* t1, int32_t* t2);
static inline void tclod_2cycle_current_simple(int32_t* sss, int32_t* sst, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t prim_tile, int32_t* t1, int32_t* t2);
static inline void tclod_2cycle_current_notexel1(int32_t* sss, int32_t* sst, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t prim_tile, int32_t* t1);
static inline void tclod_2cycle_next(int32_t* sss, int32_t* sst, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t prim_tile, int32_t* t1, int32_t* t2, int32_t* prelodfrac);
static inline void tclod_copy(int32_t* sss, int32_t* sst, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t prim_tile, int32_t* t1);
static inline void get_texel1_1cycle(int32_t* s1, int32_t* t1, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t scanline, SPANSIGS* sigs);
static inline void get_nexttexel0_2cycle(int32_t* s1, int32_t* t1, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc);
static inline void video_max_optimized(uint32_t* Pixels, uint32_t* penumin, uint32_t* penumax, int numofels);
static void calculate_clamp_diffs(uint32_t tile);
static void calculate_tile_derivs(uint32_t tile);
static void rgb_dither_complete(int* r, int* g, int* b, int dith);
static void rgb_dither_nothing(int* r, int* g, int* b, int dith);
static void get_dither_noise_complete(int x, int y, int* cdith, int* adith);
static void get_dither_only(int x, int y, int* cdith, int* adith);
static void get_dither_nothing(int x, int y, int* cdith, int* adith);
static inline void vi_vl_lerp(CCVG up, CCVG down, uint32_t frac);
static inline void rgbaz_correct_clip_sse(int offx, int offy, __m128i rgba, int *z, uint32_t curpixel_cvg, __m128i spans_cdrgba_drgbady_v);
static inline void rgbaz_correct_clip(int offx, int offy, int r, int g, int b, int a, int* z, uint32_t curpixel_cvg);
static inline void vi_fetch_filter16(CCVG res, uint32_t fboffset, uint32_t cur_x, uint32_t fsaa, uint32_t dither_filter, uint32_t vres, uint32_t fetchstate);
static inline void vi_fetch_filter32(CCVG res, uint32_t fboffset, uint32_t cur_x, uint32_t fsaa, uint32_t dither_filter, uint32_t vres, uint32_t fetchstate);
cen64_cold static uint32_t vi_integer_sqrt(uint32_t a);
cen64_cold static void deduce_derivatives(void);
static inline int32_t irand();

static int32_t k0_tf = 0, k1_tf = 0, k2_tf = 0, k3_tf = 0;
static int32_t k4 = 0, k5 = 0;
static int32_t lod_frac = 0;
uint32_t DebugMode = 0, DebugMode2 = 0; int32_t DebugMode3 = 0;
int debugcolor = 0;
uint8_t hidden_bits[0x400000];
struct {uint32_t shift; uint32_t add;} z_dec_table[8] = {
     {6, 0x00000},
     {5, 0x20000},
     {4, 0x30000},
     {3, 0x38000},
     {2, 0x3c000},
     {1, 0x3e000},
     {0, 0x3f000},
     {0, 0x3f800},
};


static void (*vi_fetch_filter_func[2])(CCVG, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) =
{
	vi_fetch_filter16, vi_fetch_filter32
};

static void (*fbread_func[4])(uint32_t, uint32_t*) = 
{
	fbread_4, fbread_8, fbread_16, fbread_32
};

static void (*fbread2_func[4])(uint32_t, uint32_t*) =
{
	fbread2_4, fbread2_8, fbread2_16, fbread2_32
};

static void (*fbwrite_func[4])(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) = 
{
	fbwrite_4, fbwrite_8, fbwrite_16, fbwrite_32
};

static void (*fbfill_func[4])(uint32_t) =
{
	fbfill_4, fbfill_8, fbfill_16, fbfill_32
};

static void (*get_dither_noise_func[3])(int, int, int*, int*) = 
{
	get_dither_noise_complete, get_dither_only, get_dither_nothing
};

static void (*rgb_dither_func[2])(int*, int*, int*, int) =
{
	rgb_dither_complete, rgb_dither_nothing
};

static void (*tcdiv_func[2])(int32_t, int32_t, int32_t, int32_t*, int32_t*) =
{
	tcdiv_nopersp, tcdiv_persp
};

static void (*render_spans_1cycle_func[3])(int, int, int, int) =
{
	render_spans_1cycle_notex, render_spans_1cycle_notexel1, render_spans_1cycle_complete
};

static void (*render_spans_2cycle_func[4])(int, int, int, int) =
{
	render_spans_2cycle_notex, render_spans_2cycle_notexel1, render_spans_2cycle_notexelnext, render_spans_2cycle_complete
};

void (*fbread1_ptr)(uint32_t, uint32_t*) = fbread_4;
void (*fbread2_ptr)(uint32_t, uint32_t*) = fbread2_4;
void (*fbwrite_ptr)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) = fbwrite_4;
void (*fbfill_ptr)(uint32_t) = fbfill_4;
void (*get_dither_noise_ptr)(int, int, int*, int*) = get_dither_noise_complete;
void (*rgb_dither_ptr)(int*, int*, int*, int) = rgb_dither_complete;
void (*tcdiv_ptr)(int32_t, int32_t, int32_t, int32_t*, int32_t*) = tcdiv_nopersp;
void (*render_spans_1cycle_ptr)(int, int, int, int, __m128i, __m128i, __m128i) = render_spans_1cycle_complete;
void (*render_spans_2cycle_ptr)(int, int, int, int, __m128i, __m128i, __m128i) = render_spans_2cycle_notexel1;

typedef struct{
	uint8_t cvg;
	uint8_t cvbit;
	uint8_t xoff;
	uint8_t yoff;
}CVtcmaskDERIVATIVE;

uint32_t gamma_table[0x100];
uint32_t gamma_dither_table[0x4000];
uint16_t z_com_table[0x40000];
uint32_t z_complete_dec_table[0x4000];
uint8_t replicated_rgba[32];
int vi_restore_table[0x400];
int32_t maskbits_table[16];
uint32_t special_9bit_clamptable[512];
int32_t special_9bit_exttable[512];
int32_t ge_two_table[128];
int32_t log2table[256];
int32_t tcdiv_table[0x8000];
uint8_t bldiv_hwaccurate_table[0x8000];
uint16_t deltaz_comparator_lut[0x10000];
int32_t clamp_t_diff[8];
int32_t clamp_s_diff[8];
CVtcmaskDERIVATIVE cvarray[0x100];

#define RDRAM_MASK 0x007fffff


#define RREADADDR8(rdst, in) {(in) &= RDRAM_MASK; (rdst) = ((in) <= plim) ? (rdram_8[(in)]) : 0;}
#define RREADIDX16(rdst, in) {(in) &= (RDRAM_MASK >> 1); (rdst) = ((in) <= idxlim16) ? (byteswap_16(rdram_16[(in)])) : 0;}
#define RREADIDX32(rdst, in) {(in) &= (RDRAM_MASK >> 2); (rdst) = ((in) <= idxlim32) ? (byteswap_32(rdram[(in)])) : 0;}

#define RWRITEADDR8(in, val)	{(in) &= RDRAM_MASK; if ((in) <= plim) rdram_8[(in)] = (val);}
#define RWRITEIDX16(in, val)	{(in) &= (RDRAM_MASK >> 1); if ((in) <= idxlim16) rdram_16[(in)] = byteswap_16(val);}
#define RWRITEIDX32(in, val)	{(in) &= (RDRAM_MASK >> 2); if ((in) <= idxlim32) rdram[(in)] = byteswap_32(val);}



#define PAIRREAD16(rdst, hdst, in)		\
{										\
	(in) &= (RDRAM_MASK >> 1);			\
	if ((in) <= idxlim16) {(rdst) = byteswap_16(rdram_16[(in)]); (hdst) = hidden_bits[(in)];}	\
	else {(rdst) = (hdst) = 0;}			\
}

#define PAIRWRITE16(in, rval, hval)		\
{										\
	(in) &= (RDRAM_MASK >> 1);			\
	if ((in) <= idxlim16) {rdram_16[(in)] = byteswap_16(rval); hidden_bits[(in)] = (hval);}	\
}

#define PAIRWRITE32(in, rval, hval0, hval1)	\
{											\
	(in) &= (RDRAM_MASK >> 2);				\
	if ((in) <= idxlim32) {rdram[(in)] = byteswap_32(rval); hidden_bits[(in) << 1] = (hval0); hidden_bits[((in) << 1) + 1] = (hval1);}	\
}

#define PAIRWRITE8(in, rval, hval)	\
{									\
	(in) &= RDRAM_MASK;				\
	if ((in) <= plim) {rdram_8[(in)] = (rval); if ((in) & 1) hidden_bits[(in) >> 1] = (hval);}	\
}

struct onetime
{
       int nolerp, copymstrangecrashes, fillmcrashes, fillmbitcrashes, syncfullcrash, vbusclock;
} onetimewarnings;

uint32_t z64gl_command = 0;
uint32_t command_counter = 0;
int SaveLoaded = 0;
uint32_t max_level = 0;
int32_t min_level = 0;
int32_t* PreScale;
uint32_t tvfadeoutstate[625];
int rdp_pipeline_crashed = 0;


static inline void tcmask(int32_t* S, int32_t* T, int32_t num);
static inline void tcmask(int32_t* S, int32_t* T, int32_t num)
{
	int32_t wrap;
	
	

	if (tile[num].mask_s)
	{
		if (tile[num].ms)
		{
			wrap = *S >> tile[num].f.masksclamped;
			wrap &= 1;
			*S ^= (-wrap);
		}
		*S &= maskbits_table[tile[num].mask_s];
	}

	if (tile[num].mask_t)
	{
		if (tile[num].mt)
		{
			wrap = *T >> tile[num].f.masktclamped;
			wrap &= 1;
			*T ^= (-wrap);
		}
		
		*T &= maskbits_table[tile[num].mask_t];
	}
}


static inline void tcmask_coupled(int32_t* S, int32_t* S1, int32_t* T, int32_t* T1, int32_t num);
static inline void tcmask_coupled(int32_t* S, int32_t* S1, int32_t* T, int32_t* T1, int32_t num)
{
	int32_t wrap;
	int32_t maskbits; 
	int32_t wrapthreshold; 


	if (tile[num].mask_s)
	{
		if (tile[num].ms)
		{
			wrapthreshold = tile[num].f.masksclamped;

			wrap = (*S >> wrapthreshold) & 1;
			*S ^= (-wrap);

			wrap = (*S1 >> wrapthreshold) & 1;
			*S1 ^= (-wrap);
		}

		maskbits = maskbits_table[tile[num].mask_s];
		*S &= maskbits;
		*S1 &= maskbits;
	}

	if (tile[num].mask_t)
	{
		if (tile[num].mt)
		{
			wrapthreshold = tile[num].f.masktclamped;

			wrap = (*T >> wrapthreshold) & 1;
			*T ^= (-wrap);

			wrap = (*T1 >> wrapthreshold) & 1;
			*T1 ^= (-wrap);
		}
		maskbits = maskbits_table[tile[num].mask_t];
		*T &= maskbits;
		*T1 &= maskbits;
	}
}


static inline void tcmask_copy(int32_t* S, int32_t* S1, int32_t* S2, int32_t* S3, int32_t* T, int32_t num);
static inline void tcmask_copy(int32_t* S, int32_t* S1, int32_t* S2, int32_t* S3, int32_t* T, int32_t num)
{
	int32_t wrap;
	int32_t maskbits_s; 
	int32_t swrapthreshold; 

	if (tile[num].mask_s)
	{
		if (tile[num].ms)
		{
			swrapthreshold = tile[num].f.masksclamped;

			wrap = (*S >> swrapthreshold) & 1;
			*S ^= (-wrap);

			wrap = (*S1 >> swrapthreshold) & 1;
			*S1 ^= (-wrap);

			wrap = (*S2 >> swrapthreshold) & 1;
			*S2 ^= (-wrap);

			wrap = (*S3 >> swrapthreshold) & 1;
			*S3 ^= (-wrap);
		}

		maskbits_s = maskbits_table[tile[num].mask_s];
		*S &= maskbits_s;
		*S1 &= maskbits_s;
		*S2 &= maskbits_s;
		*S3 &= maskbits_s;
	}

	if (tile[num].mask_t)
	{
		if (tile[num].mt)
		{
			wrap = *T >> tile[num].f.masktclamped; 
			wrap &= 1;
			*T ^= (-wrap);
		}

		*T &= maskbits_table[tile[num].mask_t];
	}
}


static inline void tcshift_cycle(int32_t* S, int32_t* T, int32_t* maxs, int32_t* maxt, uint32_t num)
{



	int32_t coord = *S;
	int32_t shifter = tile[num].shift_s;

	if (shifter < 11)
	{
		coord = SIGN16(coord);
		coord >>= shifter;
	}
	else
	{
		coord <<= (16 - shifter);
		coord = SIGN16(coord);
	}
	*S = coord; 

	

	
	*maxs = ((coord >> 3) >= tile[num].sh);
	
	

	coord = *T;
	shifter = tile[num].shift_t;

	if (shifter < 11)
	{
		coord = SIGN16(coord);
		coord >>= shifter;
	}
	else
	{
		coord <<= (16 - shifter);
		coord = SIGN16(coord);
	}
	*T = coord; 
	*maxt = ((coord >> 3) >= tile[num].th);
}	


static inline void tcshift_copy(int32_t* S, int32_t* T, uint32_t num)
{
	int32_t coord = *S;
	int32_t shifter = tile[num].shift_s;

	if (shifter < 11)
	{
		coord = SIGN16(coord);
		coord >>= shifter;
	}
	else
	{
		coord <<= (16 - shifter);
		coord = SIGN16(coord);
	}
	*S = coord; 

	coord = *T;
	shifter = tile[num].shift_t;

	if (shifter < 11)
	{
		coord = SIGN16(coord);
		coord >>= shifter;
	}
	else
	{
		coord <<= (16 - shifter);
		coord = SIGN16(coord);
	}
	*T = coord; 
	
}


static inline void tcclamp_cycle(int32_t* S, int32_t* T, int32_t* SFRAC, int32_t* TFRAC, int32_t maxs, int32_t maxt, int32_t num)
{

	int32_t locs = *S, loct = *T;
	if (tile[num].f.clampens)
	{
		if (!(locs & 0x10000))
		{
			if (!maxs)
				*S = (locs >> 5);
			else
			{
				*S = tile[num].f.clampdiffs;
				*SFRAC = 0;
			}
		}
		else
		{
			*S = 0;
			*SFRAC = 0;
		}
	}
	else
		*S = (locs >> 5);

	if (tile[num].f.clampent)
	{
		if (!(loct & 0x10000))
		{
			if (!maxt)
				*T = (loct >> 5);
			else
			{
				*T = tile[num].f.clampdifft;
				*TFRAC = 0;
			}
		}
		else
		{
			*T = 0;
			*TFRAC = 0;
		}
	}
	else
		*T = (loct >> 5);
}


static inline void tcclamp_cycle_light(int32_t* S, int32_t* T, int32_t maxs, int32_t maxt, int32_t num)
{
	int32_t locs = *S, loct = *T;
	if (tile[num].f.clampens)
	{
		if (!(locs & 0x10000))
		{
			if (!maxs)
				*S = (locs >> 5);
			else
				*S = tile[num].f.clampdiffs;
		}
		else
			*S = 0;
	}
	else
		*S = (locs >> 5);

	if (tile[num].f.clampent)
	{
		if (!(loct & 0x10000))
		{
			if (!maxt)
				*T = (loct >> 5);
			else
				*T = tile[num].f.clampdifft;
		}
		else
			*T = 0;
	}
	else
		*T = (loct >> 5);
}


cen64_cold int angrylion_rdp_init(struct cen64_device *device)
{
  cen64 = device;

	if (LOG_RDP_EXECUTION)
		rdp_exec = fopen("rdp_execute.txt", "wt");

	combiner_rgbsub_a_r[0] = combiner_rgbsub_a_r[1] = &one_color;
	combiner_rgbsub_a_g[0] = combiner_rgbsub_a_g[1] = &one_color;
	combiner_rgbsub_a_b[0] = combiner_rgbsub_a_b[1] = &one_color;
	combiner_rgbsub_b_r[0] = combiner_rgbsub_b_r[1] = &one_color;
	combiner_rgbsub_b_g[0] = combiner_rgbsub_b_g[1] = &one_color;
	combiner_rgbsub_b_b[0] = combiner_rgbsub_b_b[1] = &one_color;
	combiner_rgbmul_r[0] = combiner_rgbmul_r[1] = &one_color;
	combiner_rgbmul_g[0] = combiner_rgbmul_g[1] = &one_color;
	combiner_rgbmul_b[0] = combiner_rgbmul_b[1] = &one_color;
	combiner_rgbadd_r[0] = combiner_rgbadd_r[1] = &one_color;
	combiner_rgbadd_g[0] = combiner_rgbadd_g[1] = &one_color;
	combiner_rgbadd_b[0] = combiner_rgbadd_b[1] = &one_color;

	combiner_alphasub_a[0] = combiner_alphasub_a[1] = &one_color;
	combiner_alphasub_b[0] = combiner_alphasub_b[1] = &one_color;
	combiner_alphamul[0] = combiner_alphamul[1] = &one_color;
	combiner_alphaadd[0] = combiner_alphaadd[1] = &one_color;

	rdp_set_other_modes(0, 0);
	other_modes.f.stalederivs = 1;
	
	memset(TMEM, 0, 0x1000);

	memset(hidden_bits, 3, sizeof(hidden_bits));
	
	

	memset(tile, 0, sizeof(tile));
	
	for (int i = 0; i < 8; i++)
	{
		calculate_tile_derivs(i);
		calculate_clamp_diffs(i);
	}

	memset(&combined_color, 0, sizeof(COLOR));
	memset(&prim_color, 0, sizeof(COLOR));
	memset(&env_color, 0, sizeof(COLOR));
	memset(&key_scale, 0, sizeof(COLOR));
	memset(&key_center, 0, sizeof(COLOR));

	rdp_pipeline_crashed = 0;
	memset(&onetimewarnings, 0, sizeof(onetimewarnings));

	precalculate_everything();

  // TODO: Set limits based on RDRAM size.
	plim = 0x7fffff;
	idxlim16 = 0x3fffff;
	idxlim32 = 0x1fffff;
	
	rdram_8 = (uint8_t*)rdram;
	rdram_16 = (uint16_t*)rdram;
	return 0;
}

static inline void vi_fetch_filter16(CCVG res, uint32_t fboffset, uint32_t cur_x, uint32_t fsaa, uint32_t dither_filter, uint32_t vres, uint32_t fetchstate)
{
	int r, g, b;
	uint32_t idx = (fboffset >> 1) + cur_x;
	uint32_t pix, hval;
	uint32_t cur_cvg;
	if (fsaa)
	{
		PAIRREAD16(pix, hval, idx);
		cur_cvg = ((pix & 1) << 2) | hval;
	}
	else
	{
		RREADIDX16(pix, idx);
		cur_cvg = 7;
	}
	r = GET_HI(pix);
	g = GET_MED(pix);
	b = GET_LOW(pix);

	uint32_t fbw = vi_width & 0xfff;

	if (cur_cvg == 7)
	{
		if (dither_filter)
			restore_filter16(&r, &g, &b, fboffset, cur_x, fbw, fetchstate);
	}
	else
	{
		video_filter16(&r, &g, &b, fboffset, cur_x, fbw, cur_cvg, fetchstate);
	}


	res[0] = r;
	res[1] = g;
	res[2] = b;
	res[3] = cur_cvg;
}

static inline void vi_fetch_filter32(CCVG res, uint32_t fboffset, uint32_t cur_x, uint32_t fsaa, uint32_t dither_filter, uint32_t vres, uint32_t fetchstate)
{
	int r, g, b;
	uint32_t pix, addr = (fboffset >> 2) + cur_x;
	RREADIDX32(pix, addr);
	uint32_t cur_cvg;
	if (fsaa)
		cur_cvg = (pix >> 5) & 7;
	else
		cur_cvg = 7;
	r = (pix >> 24) & 0xff;
	g = (pix >> 16) & 0xff;
	b = (pix >> 8) & 0xff;

	uint32_t fbw = vi_width & 0xfff;
	
	if (cur_cvg == 7)
	{
		if (dither_filter)
			restore_filter32(&r, &g, &b, fboffset, cur_x, fbw, fetchstate);
	}
	else
	{
		video_filter32(&r, &g, &b, fboffset, cur_x, fbw, cur_cvg, fetchstate);
	}

	res[0] = r;
	res[1] = g;
	res[2] = b;
	res[3] = cur_cvg;
}



static void SET_SUBA_RGB_INPUT(int32_t **input_r, int32_t **input_g, int32_t **input_b, int code)
{
	switch (code & 0xf)
	{
		case 0:		*input_r = &combined_color[0];	*input_g = &combined_color[1];	*input_b = &combined_color[2];	break;
		case 1:		*input_r = &texel0_color[0];		*input_g = &texel0_color[1];		*input_b = &texel0_color[2];		break;
		case 2:		*input_r = &texel1_color[0];		*input_g = &texel1_color[1];		*input_b = &texel1_color[2];		break;
		case 3:		*input_r = &prim_color[0];		*input_g = &prim_color[1];		*input_b = &prim_color[2];		break;
		case 4:		*input_r = &shade_color[0];		*input_g = &shade_color[1];		*input_b = &shade_color[2];		break;
		case 5:		*input_r = &env_color[0];		*input_g = &env_color[1];		*input_b = &env_color[2];		break;
		case 6:		*input_r = &one_color;			*input_g = &one_color;			*input_b = &one_color;		break;
		case 7:		*input_r = &noise;				*input_g = &noise;				*input_b = &noise;				break;
		case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		{
			*input_r = &zero_color;		*input_g = &zero_color;		*input_b = &zero_color;		break;
		}
	}
}

static void SET_SUBB_RGB_INPUT(int32_t **input_r, int32_t **input_g, int32_t **input_b, int code)
{
	switch (code & 0xf)
	{
		case 0:		*input_r = &combined_color[0];	*input_g = &combined_color[1];	*input_b = &combined_color[2];	break;
		case 1:		*input_r = &texel0_color[0];		*input_g = &texel0_color[1];		*input_b = &texel0_color[2];		break;
		case 2:		*input_r = &texel1_color[0];		*input_g = &texel1_color[1];		*input_b = &texel1_color[2];		break;
		case 3:		*input_r = &prim_color[0];		*input_g = &prim_color[1];		*input_b = &prim_color[2];		break;
		case 4:		*input_r = &shade_color[0];		*input_g = &shade_color[1];		*input_b = &shade_color[2];		break;
		case 5:		*input_r = &env_color[0];		*input_g = &env_color[1];		*input_b = &env_color[2];		break;
		case 6:		*input_r = &key_center[0];		*input_g = &key_center[1];		*input_b = &key_center[2];		break;
		case 7:		*input_r = &k4;					*input_g = &k4;					*input_b = &k4;					break;
		case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		{
			*input_r = &zero_color;		*input_g = &zero_color;		*input_b = &zero_color;		break;
		}
	}
}

static void SET_MUL_RGB_INPUT(int32_t **input_r, int32_t **input_g, int32_t **input_b, int code)
{
	switch (code & 0x1f)
	{
		case 0:		*input_r = &combined_color[0];	*input_g = &combined_color[1];	*input_b = &combined_color[2];	break;
		case 1:		*input_r = &texel0_color[0];		*input_g = &texel0_color[1];		*input_b = &texel0_color[2];		break;
		case 2:		*input_r = &texel1_color[0];		*input_g = &texel1_color[1];		*input_b = &texel1_color[2];		break;
		case 3:		*input_r = &prim_color[0];		*input_g = &prim_color[1];		*input_b = &prim_color[2];		break;
		case 4:		*input_r = &shade_color[0];		*input_g = &shade_color[1];		*input_b = &shade_color[2];		break;
		case 5:		*input_r = &env_color[0];		*input_g = &env_color[1];		*input_b = &env_color[2];		break;
		case 6:		*input_r = &key_scale[0];		*input_g = &key_scale[1];		*input_b = &key_scale[2];		break;
		case 7:		*input_r = &combined_color[3];	*input_g = &combined_color[3];	*input_b = &combined_color[3];	break;
		case 8:		*input_r = &texel0_color[3];		*input_g = &texel0_color[3];		*input_b = &texel0_color[3];		break;
		case 9:		*input_r = &texel1_color[3];		*input_g = &texel1_color[3];		*input_b = &texel1_color[3];		break;
		case 10:	*input_r = &prim_color[3];		*input_g = &prim_color[3];		*input_b = &prim_color[3];		break;
		case 11:	*input_r = &shade_color[3];		*input_g = &shade_color[3];		*input_b = &shade_color[3];		break;
		case 12:	*input_r = &env_color[3];		*input_g = &env_color[3];		*input_b = &env_color[3];		break;
		case 13:	*input_r = &lod_frac;			*input_g = &lod_frac;			*input_b = &lod_frac;			break;
		case 14:	*input_r = &primitive_lod_frac;	*input_g = &primitive_lod_frac;	*input_b = &primitive_lod_frac; break;
		case 15:	*input_r = &k5;					*input_g = &k5;					*input_b = &k5;					break;
		case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
		case 24: case 25: case 26: case 27: case 28: case 29: case 30: case 31:
		{
			*input_r = &zero_color;		*input_g = &zero_color;		*input_b = &zero_color;		break;
		}
	}
}

static void SET_ADD_RGB_INPUT(int32_t **input_r, int32_t **input_g, int32_t **input_b, int code)
{
	switch (code & 0x7)
	{
		case 0:		*input_r = &combined_color[0];	*input_g = &combined_color[1];	*input_b = &combined_color[2];	break;
		case 1:		*input_r = &texel0_color[0];		*input_g = &texel0_color[1];		*input_b = &texel0_color[2];		break;
		case 2:		*input_r = &texel1_color[0];		*input_g = &texel1_color[1];		*input_b = &texel1_color[2];		break;
		case 3:		*input_r = &prim_color[0];		*input_g = &prim_color[1];		*input_b = &prim_color[2];		break;
		case 4:		*input_r = &shade_color[0];		*input_g = &shade_color[1];		*input_b = &shade_color[2];		break;
		case 5:		*input_r = &env_color[0];		*input_g = &env_color[1];		*input_b = &env_color[2];		break;
		case 6:		*input_r = &one_color;			*input_g = &one_color;			*input_b = &one_color;			break;
		case 7:		*input_r = &zero_color;			*input_g = &zero_color;			*input_b = &zero_color;			break;
	}
}

static void SET_SUB_ALPHA_INPUT(int32_t **input, int code)
{
	switch (code & 0x7)
	{
		case 0:		*input = &combined_color[3]; break;
		case 1:		*input = &texel0_color[3]; break;
		case 2:		*input = &texel1_color[3]; break;
		case 3:		*input = &prim_color[3]; break;
		case 4:		*input = &shade_color[3]; break;
		case 5:		*input = &env_color[3]; break;
		case 6:		*input = &one_color; break;
		case 7:		*input = &zero_color; break;
	}
}

static void SET_MUL_ALPHA_INPUT(int32_t **input, int code)
{
	switch (code & 0x7)
	{
		case 0:		*input = &lod_frac; break;
		case 1:		*input = &texel0_color[3]; break;
		case 2:		*input = &texel1_color[3]; break;
		case 3:		*input = &prim_color[3]; break;
		case 4:		*input = &shade_color[3]; break;
		case 5:		*input = &env_color[3]; break;
		case 6:		*input = &primitive_lod_frac; break;
		case 7:		*input = &zero_color; break;
	}
}

static inline void combiner_1cycle(int adseed, uint32_t* curpixel_cvg)
{

	int32_t redkey, greenkey, bluekey, temp;
	COLOR chromabypass;

	if (other_modes.key_en)
	{
		chromabypass[0] = *combiner_rgbsub_a_r[1];
		chromabypass[1] = *combiner_rgbsub_a_g[1];
		chromabypass[2] = *combiner_rgbsub_a_b[1];
	}



	if (combiner_rgbmul_r[1] != &zero_color)
	{

		combined_color[0] = color_combiner_equation(*combiner_rgbsub_a_r[1],*combiner_rgbsub_b_r[1],*combiner_rgbmul_r[1],*combiner_rgbadd_r[1]);
		combined_color[1] = color_combiner_equation(*combiner_rgbsub_a_g[1],*combiner_rgbsub_b_g[1],*combiner_rgbmul_g[1],*combiner_rgbadd_g[1]);
		combined_color[2] = color_combiner_equation(*combiner_rgbsub_a_b[1],*combiner_rgbsub_b_b[1],*combiner_rgbmul_b[1],*combiner_rgbadd_b[1]);
	}
	else
	{
		combined_color[0] = ((special_9bit_exttable[*combiner_rgbadd_r[1]] << 8) + 0x80) & 0x1ffff;
		combined_color[1] = ((special_9bit_exttable[*combiner_rgbadd_g[1]] << 8) + 0x80) & 0x1ffff;
		combined_color[2] = ((special_9bit_exttable[*combiner_rgbadd_b[1]] << 8) + 0x80) & 0x1ffff;
	}

	if (combiner_alphamul[1] != &zero_color)
		combined_color[3] = alpha_combiner_equation(*combiner_alphasub_a[1],*combiner_alphasub_b[1],*combiner_alphamul[1],*combiner_alphaadd[1]);
	else
		combined_color[3] = special_9bit_exttable[*combiner_alphaadd[1]] & 0x1ff;

	pixel_color[3] = special_9bit_clamptable[combined_color[3]];
	if (pixel_color[3] == 0xff)
		pixel_color[3] = 0x100;

	if (!other_modes.key_en)
	{
		
		combined_color[0] >>= 8;
		combined_color[1] >>= 8;
		combined_color[2] >>= 8;
		pixel_color[0] = special_9bit_clamptable[combined_color[0]];
		pixel_color[1] = special_9bit_clamptable[combined_color[1]];
		pixel_color[2] = special_9bit_clamptable[combined_color[2]];
	}
	else
	{
		redkey = SIGN(combined_color[0], 17);
		if (redkey >= 0)
			redkey = (key_width[0] << 4) - redkey;
		else
			redkey = (key_width[0] << 4) + redkey;
		greenkey = SIGN(combined_color[1], 17);
		if (greenkey >= 0)
			greenkey = (key_width[1] << 4) - greenkey;
		else
			greenkey = (key_width[1] << 4) + greenkey;
		bluekey = SIGN(combined_color[2], 17);
		if (bluekey >= 0)
			bluekey = (key_width[2] << 4) - bluekey;
		else
			bluekey = (key_width[2] << 4) + bluekey;
		keyalpha = (redkey < greenkey) ? redkey : greenkey;
		keyalpha = (bluekey < keyalpha) ? bluekey : keyalpha;
		keyalpha = CLIP(keyalpha, 0, 0xff);


		pixel_color[0] = special_9bit_clamptable[chromabypass[0]];
		pixel_color[1] = special_9bit_clamptable[chromabypass[1]];
		pixel_color[2] = special_9bit_clamptable[chromabypass[2]];


		combined_color[0] >>= 8;
		combined_color[1] >>= 8;
		combined_color[2] >>= 8;
	}
	
	
	if (other_modes.cvg_times_alpha)
	{
		temp = (pixel_color[3] * (*curpixel_cvg) + 4) >> 3;
		*curpixel_cvg = (temp >> 5) & 0xf;
	}

	if (!other_modes.alpha_cvg_select)
	{	
		if (!other_modes.key_en)
		{
			pixel_color[3] += adseed;
			if (pixel_color[3] & 0x100)
				pixel_color[3] = 0xff;
		}
		else
			pixel_color[3] = keyalpha;
	}
	else
	{
		if (other_modes.cvg_times_alpha)
			pixel_color[3] = temp;
		else
			pixel_color[3] = (*curpixel_cvg) << 5;
		if (pixel_color[3] > 0xff)
			pixel_color[3] = 0xff;
	}
	

	shade_color[3] += adseed;
	if (shade_color[3] & 0x100)
		shade_color[3] = 0xff;
}

static inline void combiner_2cycle(int adseed, uint32_t* curpixel_cvg, int32_t* acalpha)
{
	int32_t redkey, greenkey, bluekey, temp;
	COLOR chromabypass;

	if (combiner_rgbmul_r[0] != &zero_color)
	{
		combined_color[0] = color_combiner_equation(*combiner_rgbsub_a_r[0],*combiner_rgbsub_b_r[0],*combiner_rgbmul_r[0],*combiner_rgbadd_r[0]);
		combined_color[1] = color_combiner_equation(*combiner_rgbsub_a_g[0],*combiner_rgbsub_b_g[0],*combiner_rgbmul_g[0],*combiner_rgbadd_g[0]);
		combined_color[2] = color_combiner_equation(*combiner_rgbsub_a_b[0],*combiner_rgbsub_b_b[0],*combiner_rgbmul_b[0],*combiner_rgbadd_b[0]);
	}
	else
	{
		combined_color[0] = ((special_9bit_exttable[*combiner_rgbadd_r[0]] << 8) + 0x80) & 0x1ffff;
		combined_color[1] = ((special_9bit_exttable[*combiner_rgbadd_g[0]] << 8) + 0x80) & 0x1ffff;
		combined_color[2] = ((special_9bit_exttable[*combiner_rgbadd_b[0]] << 8) + 0x80) & 0x1ffff;
	}

	if (combiner_alphamul[0] != &zero_color)
		combined_color[3] = alpha_combiner_equation(*combiner_alphasub_a[0],*combiner_alphasub_b[0],*combiner_alphamul[0],*combiner_alphaadd[0]);
	else
		combined_color[3] = special_9bit_exttable[*combiner_alphaadd[0]] & 0x1ff;


	if (other_modes.alpha_compare_en)
	{
		if (other_modes.key_en)
		{
			redkey = SIGN(combined_color[0], 17);
			if (redkey >= 0)
				redkey = (key_width[0] << 4) - redkey;
			else
				redkey = (key_width[0] << 4) + redkey;
			greenkey = SIGN(combined_color[1], 17);
			if (greenkey >= 0)
				greenkey = (key_width[1] << 4) - greenkey;
			else
				greenkey = (key_width[1] << 4) + greenkey;
			bluekey = SIGN(combined_color[2], 17);
			if (bluekey >= 0)
				bluekey = (key_width[2] << 4) - bluekey;
			else
				bluekey = (key_width[2] << 4) + bluekey;
			keyalpha = (redkey < greenkey) ? redkey : greenkey;
			keyalpha = (bluekey < keyalpha) ? bluekey : keyalpha;
			keyalpha = CLIP(keyalpha, 0, 0xff);
		}

		int32_t preacalpha = special_9bit_clamptable[combined_color[3]];
		if (preacalpha == 0xff)
			preacalpha = 0x100;

		if (other_modes.cvg_times_alpha)
			temp = (preacalpha * (*curpixel_cvg) + 4) >> 3;

		if (!other_modes.alpha_cvg_select)
		{
			if (!other_modes.key_en)
			{
				preacalpha += adseed;
				if (preacalpha & 0x100)
					preacalpha = 0xff;
			}
			else
				preacalpha = keyalpha;
		}
		else
		{
			if (other_modes.cvg_times_alpha)
				preacalpha = temp;
			else
				preacalpha = (*curpixel_cvg) << 5;
			if (preacalpha > 0xff)
				preacalpha = 0xff;
		}

		*acalpha = preacalpha;
	}



	combined_color[0] >>= 8;
	combined_color[1] >>= 8;
	combined_color[2] >>= 8;


  memcpy(texel0_color, texel1_color, sizeof(COLOR));
  memcpy(texel1_color, nexttexel_color, sizeof(COLOR));



	if (other_modes.key_en)
	{
		chromabypass[0] = *combiner_rgbsub_a_r[1];
		chromabypass[1] = *combiner_rgbsub_a_g[1];
		chromabypass[2] = *combiner_rgbsub_a_b[1];
	}


	if (combiner_rgbmul_r[1] != &zero_color)
	{
		combined_color[0] = color_combiner_equation(*combiner_rgbsub_a_r[1],*combiner_rgbsub_b_r[1],*combiner_rgbmul_r[1],*combiner_rgbadd_r[1]);
		combined_color[1] = color_combiner_equation(*combiner_rgbsub_a_g[1],*combiner_rgbsub_b_g[1],*combiner_rgbmul_g[1],*combiner_rgbadd_g[1]);
		combined_color[2] = color_combiner_equation(*combiner_rgbsub_a_b[1],*combiner_rgbsub_b_b[1],*combiner_rgbmul_b[1],*combiner_rgbadd_b[1]);
	}
	else
	{
		combined_color[0] = ((special_9bit_exttable[*combiner_rgbadd_r[1]] << 8) + 0x80) & 0x1ffff;
		combined_color[1] = ((special_9bit_exttable[*combiner_rgbadd_g[1]] << 8) + 0x80) & 0x1ffff;
		combined_color[2] = ((special_9bit_exttable[*combiner_rgbadd_b[1]] << 8) + 0x80) & 0x1ffff;
	}

	if (combiner_alphamul[1] != &zero_color)
		combined_color[3] = alpha_combiner_equation(*combiner_alphasub_a[1],*combiner_alphasub_b[1],*combiner_alphamul[1],*combiner_alphaadd[1]);
	else
		combined_color[3] = special_9bit_exttable[*combiner_alphaadd[1]] & 0x1ff;

	if (!other_modes.key_en)
	{
		
		combined_color[0] >>= 8;
		combined_color[1] >>= 8;
		combined_color[2] >>= 8;

		pixel_color[0] = special_9bit_clamptable[combined_color[0]];
		pixel_color[1] = special_9bit_clamptable[combined_color[1]];
		pixel_color[2] = special_9bit_clamptable[combined_color[2]];
	}
	else
	{
		redkey = SIGN(combined_color[0], 17);
		if (redkey >= 0)
			redkey = (key_width[0] << 4) - redkey;
		else
			redkey = (key_width[0] << 4) + redkey;
		greenkey = SIGN(combined_color[1], 17);
		if (greenkey >= 0)
			greenkey = (key_width[1] << 4) - greenkey;
		else
			greenkey = (key_width[1] << 4) + greenkey;
		bluekey = SIGN(combined_color[2], 17);
		if (bluekey >= 0)
			bluekey = (key_width[2] << 4) - bluekey;
		else
			bluekey = (key_width[2] << 4) + bluekey;
		keyalpha = (redkey < greenkey) ? redkey : greenkey;
		keyalpha = (bluekey < keyalpha) ? bluekey : keyalpha;
		keyalpha = CLIP(keyalpha, 0, 0xff);

		pixel_color[0] = special_9bit_clamptable[chromabypass[0]];
		pixel_color[1] = special_9bit_clamptable[chromabypass[1]];
		pixel_color[2] = special_9bit_clamptable[chromabypass[2]];


		combined_color[0] >>= 8;
		combined_color[1] >>= 8;
		combined_color[2] >>= 8;
	}
	
	pixel_color[3] = special_9bit_clamptable[combined_color[3]];
	if (pixel_color[3] == 0xff)
		pixel_color[3] = 0x100;

	
	if (other_modes.cvg_times_alpha)
	{
		temp = (pixel_color[3] * (*curpixel_cvg) + 4) >> 3;
		*curpixel_cvg = (temp >> 5) & 0xf;
	}

	if (!other_modes.alpha_cvg_select)
	{
		if (!other_modes.key_en)
		{
			pixel_color[3] += adseed;
			if (pixel_color[3] & 0x100)
				pixel_color[3] = 0xff;
		}
		else
			pixel_color[3] = keyalpha;
	}
	else
	{
		if (other_modes.cvg_times_alpha)
			pixel_color[3] = temp;
		else
			pixel_color[3] = (*curpixel_cvg) << 5;
		if (pixel_color[3] > 0xff)
			pixel_color[3] = 0xff;
	}
	

	shade_color[3] += adseed;
	if (shade_color[3] & 0x100)
		shade_color[3] = 0xff;
}

static void precalculate_everything(void)
{
	int i = 0, k = 0, j = 0;

	
	
	for (i = 0; i < 256; i++)
	{
		gamma_table[i] = vi_integer_sqrt(i << 6);
		gamma_table[i] <<= 1;
	}
	for (i = 0; i < 0x4000; i++)
	{
		gamma_dither_table[i] = vi_integer_sqrt(i);
		gamma_dither_table[i] <<= 1;
	}

	
	
	
	
	z_build_com_table();

	
	
	uint32_t exponent;
	uint32_t mantissa;
	for (i = 0; i < 0x4000; i++)
	{
		exponent = (i >> 11) & 7;
		mantissa = i & 0x7ff;
		z_complete_dec_table[i] = ((mantissa << z_dec_table[exponent].shift) + z_dec_table[exponent].add) & 0x3ffff;
	}

	
	
	precalc_cvmask_derivatives();

	
	
	i = 0;
	log2table[0] = log2table[1] = 0;
	for (i = 2; i < 256; i++)
	{
		for (k = 7; k > 0; k--)
		{
			if((i >> k) & 1)
			{
				log2table[i] = k;
				break;
			}
		}
	}

	
	
	
	for (i = 0; i < 0x400; i++)
	{
		if (((i >> 5) & 0x1f) < (i & 0x1f))
			vi_restore_table[i] = 1;
		else if (((i >> 5) & 0x1f) > (i & 0x1f))
			vi_restore_table[i] = -1;
		else
			vi_restore_table[i] = 0;
	}

	
	
	for (i = 0; i < 32; i++)
		replicated_rgba[i] = (i << 3) | ((i >> 2) & 7); 

	
	
	maskbits_table[0] = 0x3ff;
	for (i = 1; i < 16; i++)
		maskbits_table[i] = ((uint16_t)(0xffff) >> (16 - i)) & 0x3ff;
	

	
	
	for(i = 0; i < 0x200; i++)
	{
		switch((i >> 7) & 3)
		{
		case 0:
		case 1:
			special_9bit_clamptable[i] = i & 0xff;
			break;
		case 2:
			special_9bit_clamptable[i] = 0xff;
			break;
		case 3:
			special_9bit_clamptable[i] = 0;
			break;
		}
	}

	
	
	for(i = 0; i < 0x200; i++)
	{
		special_9bit_exttable[i] = ((i & 0x180) == 0x180) ? (i | ~0x1ff) : (i & 0x1ff);
	}

	
	
	
	
	
	
	int temppoint, tempslope; 
	int normout;
	int wnorm;
	int shift, tlu_rcp;

	for (i = 0; i < 0x8000; i++)
	{
		for (k = 1; k <= 14 && !((i << k) & 0x8000); k++) 
			;
		shift = k - 1;
		normout = (i << shift) & 0x3fff;
		wnorm = (normout & 0xff) << 2;
		normout >>= 8;

		
		
		temppoint = norm_point_table[normout];
		tempslope = norm_slope_table[normout];

		tempslope = (tempslope | ~0x3ff) + 1;
		
		tlu_rcp = (((tempslope * wnorm) >> 10) + temppoint) & 0x7fff;
		
		tcdiv_table[i] = shift | (tlu_rcp << 4);
	}

	
	int d = 0, n = 0, temp = 0, res = 0, invd = 0, nbit = 0;
	int ps[9];

	for (i = 0; i < 0x8000; i++)
	{
		res = 0;
		d = (i >> 11) & 0xf;
		n = i & 0x7ff;
		invd = (~d) & 0xf;
		

		temp = invd + (n >> 8) + 1;
		ps[0] = temp & 7;
		for (k = 0; k < 8; k++)
		{
			nbit = (n >> (7 - k)) & 1;
			if (res & (0x100 >> k))
				temp = invd + (ps[k] << 1) + nbit + 1;
			else
				temp = d + (ps[k] << 1) + nbit;
			ps[k + 1] = temp & 7;
			if (temp & 0x10)
				res |= (1 << (7 - k));
		}
		bldiv_hwaccurate_table[i] = res;
	}


	deltaz_comparator_lut[0] = 0;
	for (i = 1; i < 0x10000; i++)
	{
		for (k = 15; k >= 0; k--)
		{
			if (i & (1 << k))
			{
				deltaz_comparator_lut[i] = 1 << k;
				break;
			}
		}
	}

}

static void SET_BLENDER_INPUT(int cycle, int which, int32_t **input_r, int32_t **input_g, int32_t **input_b, int32_t **input_a, int a, int b)
{

	switch (a & 0x3)
	{
		case 0:
		{
			if (cycle == 0)
			{
				*input_r = &pixel_color[0];
				*input_g = &pixel_color[1];
				*input_b = &pixel_color[2];
			}
			else
			{
				*input_r = &blended_pixel_color[0];
				*input_g = &blended_pixel_color[1];
				*input_b = &blended_pixel_color[2];
			}
			break;
		}

		case 1:
		{
			*input_r = &memory_color[0];
			*input_g = &memory_color[1];
			*input_b = &memory_color[2];
			break;
		}

		case 2:
		{
			*input_r = &blend_color[0];		*input_g = &blend_color[1];		*input_b = &blend_color[2];
			break;
		}

		case 3:
		{
			*input_r = &fog_color[0];		*input_g = &fog_color[1];		*input_b = &fog_color[2];
			break;
		}
	}

	if (which == 0)
	{
		switch (b & 0x3)
		{
			case 0:		*input_a = &pixel_color[3]; break;
			case 1:		*input_a = &fog_color[3]; break;
			case 2:		*input_a = &shade_color[3]; break;
			case 3:		*input_a = &zero_color; break;
		}
	}
	else
	{
		switch (b & 0x3)
		{
			case 0:		*input_a = &inv_pixel_color[3]; break;
			case 1:		*input_a = &memory_color[3]; break;
			case 2:		*input_a = &blenderone; break;
			case 3:		*input_a = &zero_color; break;
		}
	}
}



static const uint8_t bayer_matrix[16] =
{
	 0,  4,  1, 5,
	 4,  0,  5, 1,
	 3,	 7,  2, 6,
	 7,  3,  6, 2
};


static const uint8_t magic_matrix[16] =
{
	 0,  6,  1, 7,
	 4,  2,  5, 3,
	 3,	 5,  2, 4,
	 7,  1,  6, 0
};

static inline int blender_1cycle(uint32_t* fr, uint32_t* fg, uint32_t* fb, int dith, uint32_t blend_en, uint32_t prewrap, uint32_t curpixel_cvg, uint32_t curpixel_cvbit)
{
	int r, g, b, dontblend;
	
	
	if (alpha_compare(pixel_color[3]))
	{

		

		
		
		
		if (other_modes.antialias_en ? (curpixel_cvg) : (curpixel_cvbit))
		{

			if (!other_modes.color_on_cvg || prewrap)
			{
				dontblend = (other_modes.f.partialreject_1cycle && pixel_color[3] >= 0xff);
				if (!blend_en || dontblend)
				{
					r = *blender1a_r[0];
					g = *blender1a_g[0];
					b = *blender1a_b[0];
				}
				else
				{
					inv_pixel_color[3] =  (~(*blender1b_a[0])) & 0xff;
					
					
					
					

					blender_equation_cycle0(&r, &g, &b);
				}
			}
			else
			{
				r = *blender2a_r[0];
				g = *blender2a_g[0];
				b = *blender2a_b[0];
			}

			rgb_dither_ptr(&r, &g, &b, dith);
			*fr = r;
			*fg = g;
			*fb = b;
			return 1;
		}
		else 
			return 0;
		}
	else 
		return 0;
}

static inline int blender_2cycle(uint32_t* fr, uint32_t* fg, uint32_t* fb, int dith, uint32_t blend_en, uint32_t prewrap, uint32_t curpixel_cvg, uint32_t curpixel_cvbit, int32_t acalpha)
{
	int r, g, b, dontblend;


	if (alpha_compare(acalpha))
	{
		if (other_modes.antialias_en ? (curpixel_cvg) : (curpixel_cvbit))
		{
			
			inv_pixel_color[3] =  (~(*blender1b_a[0])) & 0xff;

			blender_equation_cycle0_2(&r, &g, &b);

			
			memcpy(memory_color, pre_memory_color, sizeof(COLOR));

			blended_pixel_color[0] = r;
			blended_pixel_color[1] = g;
			blended_pixel_color[2] = b;
			blended_pixel_color[3] = pixel_color[3];

			if (!other_modes.color_on_cvg || prewrap)
			{
				dontblend = (other_modes.f.partialreject_2cycle && pixel_color[3] >= 0xff);
				if (!blend_en || dontblend)
				{
					r = *blender1a_r[1];
					g = *blender1a_g[1];
					b = *blender1a_b[1];
				}
				else
				{
					inv_pixel_color[3] =  (~(*blender1b_a[1])) & 0xff;
					blender_equation_cycle1(&r, &g, &b);
				}
			}
			else
			{
				r = *blender2a_r[1];
				g = *blender2a_g[1];
				b = *blender2a_b[1];
			}

			
			rgb_dither_ptr(&r, &g, &b, dith);
			*fr = r;
			*fg = g;
			*fb = b;
			return 1;
		}
		else
		{
			memcpy(memory_color, pre_memory_color, sizeof(COLOR));
			return 0;
                }
	}
	else
	{
		memcpy(memory_color, pre_memory_color, sizeof(COLOR));
		return 0;
	}
}



static void fetch_texel(COLOR color, int s, int t, uint32_t tilenum)
{
	uint32_t tbase = tile[tilenum].line * t + tile[tilenum].tmem;
	

	uint32_t tpal	= tile[tilenum].palette;

	
	
	
	
	
	
	
	uint16_t *tc16 = (uint16_t*)TMEM;
	uint32_t taddr = 0;

	

	

	switch (tile[tilenum].f.notlutswitch)
	{
	case TEXEL_RGBA4:
		{
			taddr = ((tbase << 4) + s) >> 1;
			taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
			uint8_t byteval, c; 

			byteval = TMEM[taddr & 0xfff];
			c = ((s & 1)) ? (byteval & 0xf) : (byteval >> 4);
			c |= (c << 4);
			color[0] = c;
			color[1] = c;
			color[2] = c;
			color[3] = c;
		}
		break;
	case TEXEL_RGBA8:
		{
			taddr = (tbase << 3) + s;
			taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
			
			uint8_t p;

			p = TMEM[taddr & 0xfff];
			color[0] = p;
			color[1] = p;
			color[2] = p;
			color[3] = p;
		}
		break;
	case TEXEL_RGBA16:
		{
			taddr = (tbase << 2) + s;
			taddr ^= ((t & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR);
			
								
			uint16_t c;
					
			c = tc16[taddr & 0x7ff];
			color[0] = GET_HI_RGBA16_TMEM(c);
			color[1] = GET_MED_RGBA16_TMEM(c);
			color[2] = GET_LOW_RGBA16_TMEM(c);
			color[3] = (c & 1) ? 0xff : 0;
		}
		break;
	case TEXEL_RGBA32:
		{
			
			
			
			taddr = (tbase << 2) + s;
			taddr ^= ((t & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR);
					
			uint16_t c;
					
			
			taddr &= 0x3ff;
			c = tc16[taddr];
			color[0] = c >> 8;
			color[1] = c & 0xff;
			c = tc16[taddr | 0x400];
			color[2] = c >> 8;
			color[3] = c & 0xff;
		}
		break;
	case TEXEL_YUV4:
	case TEXEL_YUV8:
		{
			taddr = (tbase << 3) + s;

			taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
					
			int32_t u, save;

			save = u = TMEM[taddr & 0x7ff];
			
			u = (u - 0x80) & 0x1ff;

			color[0] = u;
			color[1] = u;
			color[2] = save;
			color[3] = save;
		}
		break;
	case TEXEL_YUV16:
	case TEXEL_YUV32:
		{
			taddr = (tbase << 3) + s;
			int taddrlow = taddr >> 1;

			taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
			taddrlow ^= ((t & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR);
					
			taddr &= 0x7ff;
			taddrlow &= 0x3ff;
					
			uint16_t c = tc16[taddrlow];
					
			int32_t y, u, v;
			y = TMEM[taddr | 0x800];
			u = c >> 8;
			v = c & 0xff;

			u = (u - 0x80) & 0x1ff;
			v = (v - 0x80) & 0x1ff;



			color[0] = u;
			color[1] = v;
			color[2] = y;
			color[3] = y;
		}
		break;
	case TEXEL_CI4:
		{
			taddr = ((tbase << 4) + s) >> 1;
			taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
			
			uint8_t p;

			
			
			p = TMEM[taddr & 0xfff];
			p = (s & 1) ? (p & 0xf) : (p >> 4);
			p = (tpal << 4) | p;
			color[0] = color[1] = color[2] = color[3] = p;
		}
		break;
	case TEXEL_CI8:
		{
			taddr = (tbase << 3) + s;
			taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
			
			uint8_t p;

			
			p = TMEM[taddr & 0xfff];
			color[0] = p;
			color[1] = p;
			color[2] = p;
			color[3] = p;
		}
		break;
	case TEXEL_CI16:
		{
			taddr = (tbase << 2) + s;
			taddr ^= ((t & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR);
								
			uint16_t c;
					
			c = tc16[taddr & 0x7ff];
			color[0] = c >> 8;
			color[1] = c & 0xff;
			color[2] = color[0];
			color[3] = (c & 1) ? 0xff : 0;
		}
		break;
	case TEXEL_CI32:
		{
			taddr = (tbase << 2) + s;
			taddr ^= ((t & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR);
					
			uint16_t c;
					
			c = tc16[taddr & 0x7ff];
			color[0] = c >> 8;
			color[1] = c & 0xff;
			color[2] = color[0];
			color[3] = (c & 1) ? 0xff : 0;
			
		}
        break;
	case TEXEL_IA4:
		{
			taddr = ((tbase << 4) + s) >> 1;
			taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
			
			uint8_t p, i; 
					
			
			p = TMEM[taddr & 0xfff];
			p = (s & 1) ? (p & 0xf) : (p >> 4);
			i = p & 0xe;
			i = (i << 4) | (i << 1) | (i >> 2);
			color[0] = i;
			color[1] = i;
			color[2] = i;
			color[3] = (p & 0x1) ? 0xff : 0;
		}
		break;
	case TEXEL_IA8:
		{
			taddr = (tbase << 3) + s;
			taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
			
			uint8_t p, i;

			
			p = TMEM[taddr & 0xfff];
			i = p & 0xf0;
			i |= (i >> 4);
			color[0] = i;
			color[1] = i;
			color[2] = i;
			color[3] = ((p & 0xf) << 4) | (p & 0xf);
		}
		break;
	case TEXEL_IA16:
		{
		
		
			taddr = (tbase << 2) + s;
			taddr ^= ((t & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR);
			
			uint16_t c; 
										
			c = tc16[taddr & 0x7ff];
			color[0] = color[1] = color[2] = (c >> 8);
			color[3] = c & 0xff;
		}
		break;
	case TEXEL_IA32:
		{
			taddr = (tbase << 2) + s;
			taddr ^= ((t & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR);
					
			uint16_t c;
					
			c = tc16[taddr & 0x7ff];
			color[0] = c >> 8;
			color[1] = c & 0xff;
			color[2] = color[0];
			color[3] = (c & 1) ? 0xff : 0;
		}
		break;
	case TEXEL_I4:
		{
			taddr = ((tbase << 4) + s) >> 1;
			taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
			
			uint8_t byteval, c; 
													
			byteval = TMEM[taddr & 0xfff];
			c = (s & 1) ? (byteval & 0xf) : (byteval >> 4);
			c |= (c << 4);
			color[0] = c;
			color[1] = c;
			color[2] = c;
			color[3] = c;
		}
		break;
	case TEXEL_I8:
		{
			taddr = (tbase << 3) + s;
			taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
			
			uint8_t c; 

			c = TMEM[taddr & 0xfff];
			color[0] = c;
			color[1] = c;
			color[2] = c;
			color[3] = c;
		}
		break;
	case TEXEL_I16:
		{
			taddr = (tbase << 2) + s;
			taddr ^= ((t & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR);
								
			uint16_t c;
					
			c = tc16[taddr & 0x7ff];
			color[0] = c >> 8;
			color[1] = c & 0xff;
			color[2] = color[0];
			color[3] = (c & 1) ? 0xff : 0;
		}
		break;
	case TEXEL_I32:
		{
			taddr = (tbase << 2) + s;
			taddr ^= ((t & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR);
					
			uint16_t c;
					
			c = tc16[taddr & 0x7ff];
			color[0] = c >> 8;
			color[1] = c & 0xff;
			color[2] = color[0];
			color[3] = (c & 1) ? 0xff : 0;
		}
		break;
	default:
		debug("fetch_texel: unknown texture format %d, size %d, tilenum %d\n", tile[tilenum].format, tile[tilenum].size, tilenum);
		break;
	}
}

static void fetch_texel_entlut(COLOR color, int s, int t, uint32_t tilenum)
{
	uint32_t tbase = tile[tilenum].line * t + tile[tilenum].tmem;
	uint32_t tpal	= tile[tilenum].palette << 4;
	uint16_t *tc16 = (uint16_t*)TMEM;
	uint32_t taddr = 0;
	uint32_t c;

	
	
	switch(tile[tilenum].f.tlutswitch)
	{
	case 0:
	case 1:
	case 2:
		{
			taddr = ((tbase << 4) + s) >> 1;
			taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
			c = TMEM[taddr & 0x7ff];
			c = (s & 1) ? (c & 0xf) : (c >> 4);
			c = tlut[((tpal | c) << 2) ^ WORD_ADDR_XOR];
		}
		break;
	case 3:
		{
			taddr = (tbase << 3) + s;
			taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
			c = TMEM[taddr & 0x7ff];
			c = (s & 1) ? (c & 0xf) : (c >> 4);
			c = tlut[((tpal | c) << 2) ^ WORD_ADDR_XOR];
		}
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		{
			taddr = (tbase << 3) + s;
			taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
			c = TMEM[taddr & 0x7ff];
			c = tlut[(c << 2) ^ WORD_ADDR_XOR];
		}
		break;
	case 8:
	case 9:
	case 10:
		{
			taddr = (tbase << 2) + s;
			taddr ^= ((t & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR);
			c = tc16[taddr & 0x3ff];
			c = tlut[((c >> 6) & ~3) ^ WORD_ADDR_XOR];
			
		}
		break;
	case 11:
		{
			taddr = (tbase << 3) + s;
			taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
			c = TMEM[taddr & 0x7ff];
			c = tlut[(c << 2) ^ WORD_ADDR_XOR];
		}
		break;
	case 12:
	case 13:
	case 14:
		{
			taddr = (tbase << 2) + s;
			taddr ^= ((t & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR);
			c = tc16[taddr & 0x3ff];
			c = tlut[((c >> 6) & ~3) ^ WORD_ADDR_XOR];
		}
		break;
	case 15:
		{
			taddr = (tbase << 3) + s;
			taddr ^= ((t & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR);
			c = TMEM[taddr & 0x7ff];
			c = tlut[(c << 2) ^ WORD_ADDR_XOR];
		}
		break;
	default:
		debug("fetch_texel_entlut: unknown texture format %d, size %d, tilenum %d\n", tile[tilenum].format, tile[tilenum].size, tilenum);
		break;
	}

	if (!other_modes.tlut_type)
	{
		color[0] = GET_HI_RGBA16_TMEM(c);
		color[1] = GET_MED_RGBA16_TMEM(c);
		color[2] = GET_LOW_RGBA16_TMEM(c);
		color[3] = (c & 1) ? 0xff : 0;
	}
	else
	{
		color[0] = color[1] = color[2] = c >> 8;
		color[3] = c & 0xff;
	}

}

static void fetch_texel_quadro_rgba16(COLOR color0, COLOR color1, COLOR color2, COLOR color3, const int16_t colors[4]) {
  __m128i al_v, hi_v, md_v, lo_v;
  __m128i hi_md_unpack, lo_al_unpack;
  __m128i color_01, color_23;

  // Shift bits into high left part of halfword.
  al_v = hi_v = md_v = lo_v = _mm_loadl_epi64(colors);
  md_v = _mm_slli_epi16(md_v, 5);
  lo_v = _mm_slli_epi16(lo_v, 10);
  hi_md_unpack = _mm_unpacklo_epi16(hi_v, md_v);

  // Perform a replicated_rgba lookup.
  // hi[0], md[0], hi[1], md[1], ..., hi[3], md[3]
  // lo[0], al[0], lo[1], al[1], ..., lo[3], al[3]
  al_v = _mm_srli_epi16(_mm_srai_epi16(_mm_slli_epi16(al_v, 15), 7), 8);
  lo_v = _mm_or_si128(_mm_srli_epi16(lo_v, 13), _mm_slli_epi16(_mm_srli_epi16(lo_v, 11), 3));
  lo_al_unpack = _mm_unpacklo_epi16(lo_v, al_v);

  hi_md_unpack = _mm_or_si128(_mm_srli_epi16(hi_md_unpack, 13), _mm_slli_epi16(_mm_srli_epi16(hi_md_unpack, 11), 3));

  // Spill out the color to memory.
  color_01 = _mm_unpacklo_epi32(hi_md_unpack, lo_al_unpack);
  color_23 = _mm_unpackhi_epi32(hi_md_unpack, lo_al_unpack);

  _mm_store_si128(color0, _mm_cvtepu16_epi32(color_01));
  _mm_store_si128(color2, _mm_cvtepu16_epi32(color_23));
  _mm_store_si128(color1, _mm_cvtepu16_epi32(_mm_srli_si128(color_01, 8)));
  _mm_store_si128(color3, _mm_cvtepu16_epi32(_mm_srli_si128(color_23, 8)));
}

static void fetch_texel_quadro(COLOR color0, COLOR color1, COLOR color2, COLOR color3, int s0, int s1, int t0, int t1, uint32_t tilenum)
{

	uint32_t tbase0 = tile[tilenum].line * t0 + tile[tilenum].tmem;
	uint32_t tbase2 = tile[tilenum].line * t1 + tile[tilenum].tmem;
	uint32_t tpal	= tile[tilenum].palette;
	uint32_t xort = 0, ands = 0;

	
	

	uint16_t *tc16 = (uint16_t*)TMEM;
	uint32_t taddr0 = 0, taddr1 = 0, taddr2 = 0, taddr3 = 0;
	uint32_t taddrlow0 = 0, taddrlow1 = 0, taddrlow2 = 0, taddrlow3 = 0;

	switch (tile[tilenum].f.notlutswitch)
	{
	case TEXEL_RGBA4:
		{
			taddr0 = ((tbase0 << 4) + s0) >> 1;
			taddr1 = ((tbase0 << 4) + s1) >> 1;
			taddr2 = ((tbase2 << 4) + s0) >> 1;
			taddr3 = ((tbase2 << 4) + s1) >> 1;
			xort = (t0 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
			
			uint32_t byteval, c; 
													
			taddr0 &= 0xfff;
			taddr1 &= 0xfff;
			taddr2 &= 0xfff;
			taddr3 &= 0xfff;
			ands = s0 & 1;
			byteval = TMEM[taddr0];
			c = (ands) ? (byteval & 0xf) : (byteval >> 4);
			c |= (c << 4);
			color0[0] = c;
			color0[1] = c;
			color0[2] = c;
			color0[3] = c;
			byteval = TMEM[taddr2];
			c = (ands) ? (byteval & 0xf) : (byteval >> 4);
			c |= (c << 4);
			color2[0] = c;
			color2[1] = c;
			color2[2] = c;
			color2[3] = c;

			ands = s1 & 1;
			byteval = TMEM[taddr1];
			c = (ands) ? (byteval & 0xf) : (byteval >> 4);
			c |= (c << 4);
			color1[0] = c;
			color1[1] = c;
			color1[2] = c;
			color1[3] = c;
			byteval = TMEM[taddr3];
			c = (ands) ? (byteval & 0xf) : (byteval >> 4);
			c |= (c << 4);
			color3[0] = c;
			color3[1] = c;
			color3[2] = c;
			color3[3] = c;
		}
		break;
	case TEXEL_RGBA8:
		{
			taddr0 = ((tbase0 << 3) + s0);
			taddr1 = ((tbase0 << 3) + s1);
			taddr2 = ((tbase2 << 3) + s0);
			taddr3 = ((tbase2 << 3) + s1);
			xort = (t0 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
			
			uint32_t p; 
			
			taddr0 &= 0xfff;
			taddr1 &= 0xfff;
			taddr2 &= 0xfff;
			taddr3 &= 0xfff;
			p = TMEM[taddr0];
			color0[0] = p;
			color0[1] = p;
			color0[2] = p;
			color0[3] = p;
			p = TMEM[taddr2];
			color2[0] = p;
			color2[1] = p;
			color2[2] = p;
			color2[3] = p;
			p = TMEM[taddr1];
			color1[0] = p;
			color1[1] = p;
			color1[2] = p;
			color1[3] = p;
			p = TMEM[taddr3];
			color3[0] = p;
			color3[1] = p;
			color3[2] = p;
			color3[3] = p;
		}
		break;
	case TEXEL_RGBA16:
		{
			taddr0 = ((tbase0 << 2) + s0);
			taddr1 = ((tbase0 << 2) + s1);
			taddr2 = ((tbase2 << 2) + s0);
			taddr3 = ((tbase2 << 2) + s1);
			xort = (t0 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
					
			uint32_t c0, c1, c2, c3;
					
			taddr0 &= 0x7ff;
			taddr1 &= 0x7ff;
			taddr2 &= 0x7ff;
			taddr3 &= 0x7ff;

      int16_t colors[4];

			colors[0] = tc16[taddr0];
			colors[1] = tc16[taddr1];
			colors[2] = tc16[taddr2];
			colors[3] = tc16[taddr3];
      fetch_texel_quadro_rgba16(color0, color1, color2, color3, colors);
    }
		break;
	case TEXEL_RGBA32:
		{
			taddr0 = ((tbase0 << 2) + s0);
			taddr1 = ((tbase0 << 2) + s1);
			taddr2 = ((tbase2 << 2) + s0);
			taddr3 = ((tbase2 << 2) + s1);
			xort = (t0 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
					
			uint16_t c0, c1, c2, c3;

			taddr0 &= 0x3ff;
			taddr1 &= 0x3ff;
			taddr2 &= 0x3ff;
			taddr3 &= 0x3ff;
			c0 = tc16[taddr0];
			color0[0] = c0 >> 8;
			color0[1] = c0 & 0xff;
			c0 = tc16[taddr0 | 0x400];
			color0[2] = c0 >>  8;
			color0[3] = c0 & 0xff;
			c1 = tc16[taddr1];
			color1[0] = c1 >> 8;
			color1[1] = c1 & 0xff;
			c1 = tc16[taddr1 | 0x400];
			color1[2] = c1 >>  8;
			color1[3] = c1 & 0xff;
			c2 = tc16[taddr2];
			color2[0] = c2 >> 8;
			color2[1] = c2 & 0xff;
			c2 = tc16[taddr2 | 0x400];
			color2[2] = c2 >>  8;
			color2[3] = c2 & 0xff;
			c3 = tc16[taddr3];
			color3[0] = c3 >> 8;
			color3[1] = c3 & 0xff;
			c3 = tc16[taddr3 | 0x400];
			color3[2] = c3 >>  8;
			color3[3] = c3 & 0xff;
		}
		break;
	case TEXEL_YUV4:
	case TEXEL_YUV8:
		{
			taddr0 = (tbase0 << 3) + s0;
			taddr1 = (tbase0 << 3) + s1;
			taddr2 = (tbase2 << 3) + s0;
			taddr3 = (tbase2 << 3) + s1;

			xort = (t0 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;

			int32_t u0, u1, u2, u3, save0, save1, save2, save3;

			save0 = u0 = TMEM[taddr0 & 0x7ff];
			u0 = (u0 - 0x80) & 0x1ff;
			save1 = u1 = TMEM[taddr1 & 0x7ff];
			u1 = (u1 - 0x80) & 0x1ff;
			save2 = u2 = TMEM[taddr2 & 0x7ff];
			u2 = (u2 - 0x80) & 0x1ff;
			save3 = u3 = TMEM[taddr3 & 0x7ff];
			u3 = (u3 - 0x80) & 0x1ff;

			color0[0] = u0;
			color0[1] = u0;
			color0[2] = save0;
			color0[3] = save0;
			color1[0] = u1;
			color1[1] = u1;
			color1[2] = save1;
			color1[3] = save1;
			color2[0] = u2;
			color2[1] = u2;
			color2[2] = save2;
			color2[3] = save2;
			color3[0] = u3;
			color3[1] = u3;
			color3[2] = save3;
			color3[3] = save3;
		}
		break;
	case TEXEL_YUV16:
	case TEXEL_YUV32:
		{
			taddr0 = ((tbase0 << 3) + s0);
			taddr1 = ((tbase0 << 3) + s1);
			taddr2 = ((tbase2 << 3) + s0);
			taddr3 = ((tbase2 << 3) + s1);
			taddrlow0 = taddr0 >> 1;
			taddrlow1 = taddr1 >> 1;
			taddrlow2 = taddr2 >> 1;
			taddrlow3 = taddr3 >> 1;

			xort = (t0 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
			xort = (t0 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddrlow0 ^= xort;
			taddrlow1 ^= xort;
			xort = (t1 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddrlow2 ^= xort;
			taddrlow3 ^= xort;

			taddr0 &= 0x7ff;
			taddr1 &= 0x7ff;
			taddr2 &= 0x7ff;
			taddr3 &= 0x7ff;
			taddrlow0 &= 0x3ff;
			taddrlow1 &= 0x3ff;
			taddrlow2 &= 0x3ff;
			taddrlow3 &= 0x3ff;

			uint16_t c0, c1, c2, c3;
			int32_t y0, y1, y2, y3, u0, u1, u2, u3, v0, v1, v2, v3;

			c0 = tc16[taddrlow0];
			c1 = tc16[taddrlow1];
			c2 = tc16[taddrlow2];
			c3 = tc16[taddrlow3];					
			
			y0 = TMEM[taddr0 | 0x800];
			u0 = c0 >> 8;
			v0 = c0 & 0xff;
			y1 = TMEM[taddr1 | 0x800];
			u1 = c1 >> 8;
			v1 = c1 & 0xff;
			y2 = TMEM[taddr2 | 0x800];
			u2 = c2 >> 8;
			v2 = c2 & 0xff;
			y3 = TMEM[taddr3 | 0x800];
			u3 = c3 >> 8;
			v3 = c3 & 0xff;

			u0 = (u0 - 0x80) & 0x1ff;
			v0 = (v0 - 0x80) & 0x1ff;
			u1 = (u1 - 0x80) & 0x1ff;
			v1 = (v1 - 0x80) & 0x1ff;
			u2 = (u2 - 0x80) & 0x1ff;
			v2 = (v2 - 0x80) & 0x1ff;
			u3 = (u3 - 0x80) & 0x1ff;
			v3 = (v3 - 0x80) & 0x1ff;

			color0[0] = u0;
			color0[1] = v0;
			color0[2] = y0;
			color0[3] = y0;
			color1[0] = u1;
			color1[1] = v1;
			color1[2] = y1;
			color1[3] = y1;
			color2[0] = u2;
			color2[1] = v2;
			color2[2] = y2;
			color2[3] = y2;
			color3[0] = u3;
			color3[1] = v3;
			color3[2] = y3;
			color3[3] = y3;
		}
		break;
	case TEXEL_CI4:
		{
			taddr0 = ((tbase0 << 4) + s0) >> 1;
			taddr1 = ((tbase0 << 4) + s1) >> 1;
			taddr2 = ((tbase2 << 4) + s0) >> 1;
			taddr3 = ((tbase2 << 4) + s1) >> 1;
			xort = (t0 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
			
			uint32_t p;
															
			taddr0 &= 0xfff;
			taddr1 &= 0xfff;
			taddr2 &= 0xfff;
			taddr3 &= 0xfff;
			ands = s0 & 1;
			p = TMEM[taddr0];
			p = (ands) ? (p & 0xf) : (p >> 4);
			p = (tpal << 4) | p;
			color0[0] = color0[1] = color0[2] = color0[3] = p;
			p = TMEM[taddr2];
			p = (ands) ? (p & 0xf) : (p >> 4);
			p = (tpal << 4) | p;
			color2[0] = color2[1] = color2[2] = color2[3] = p;

			ands = s1 & 1;
			p = TMEM[taddr1];
			p = (ands) ? (p & 0xf) : (p >> 4);
			p = (tpal << 4) | p;
			color1[0] = color1[1] = color1[2] = color1[3] = p;
			p = TMEM[taddr3];
			p = (ands) ? (p & 0xf) : (p >> 4);
			p = (tpal << 4) | p;
			color3[0] = color3[1] = color3[2] = color3[3] = p;
		}
		break;
	case TEXEL_CI8:
		{
			taddr0 = ((tbase0 << 3) + s0);
			taddr1 = ((tbase0 << 3) + s1);
			taddr2 = ((tbase2 << 3) + s0);
			taddr3 = ((tbase2 << 3) + s1);
			xort = (t0 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
			
			uint32_t p;

			taddr0 &= 0xfff;
			taddr1 &= 0xfff;
			taddr2 &= 0xfff;
			taddr3 &= 0xfff;
			p = TMEM[taddr0];
			color0[0] = p;
			color0[1] = p;
			color0[2] = p;
			color0[3] = p;
			p = TMEM[taddr2];
			color2[0] = p;
			color2[1] = p;
			color2[2] = p;
			color2[3] = p;
			p = TMEM[taddr1];
			color1[0] = p;
			color1[1] = p;
			color1[2] = p;
			color1[3] = p;
			p = TMEM[taddr3];
			color3[0] = p;
			color3[1] = p;
			color3[2] = p;
			color3[3] = p;
		}
		break;
	case TEXEL_CI16:
		{
			taddr0 = ((tbase0 << 2) + s0);
			taddr1 = ((tbase0 << 2) + s1);
			taddr2 = ((tbase2 << 2) + s0);
			taddr3 = ((tbase2 << 2) + s1);
			xort = (t0 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
								
			uint16_t c0, c1, c2, c3;
					
			taddr0 &= 0x7ff;
			taddr1 &= 0x7ff;
			taddr2 &= 0x7ff;
			taddr3 &= 0x7ff;
			c0 = tc16[taddr0];
			color0[0] = c0 >> 8;
			color0[1] = c0 & 0xff;
			color0[2] = c0 >> 8;
			color0[3] = (c0 & 1) ? 0xff : 0;
			c1 = tc16[taddr1];
			color1[0] = c1 >> 8;
			color1[1] = c1 & 0xff;
			color1[2] = c1 >> 8;
			color1[3] = (c1 & 1) ? 0xff : 0;
			c2 = tc16[taddr2];
			color2[0] = c2 >> 8;
			color2[1] = c2 & 0xff;
			color2[2] = c2 >> 8;
			color2[3] = (c2 & 1) ? 0xff : 0;
			c3 = tc16[taddr3];
			color3[0] = c3 >> 8;
			color3[1] = c3 & 0xff;
			color3[2] = c3 >> 8;
			color3[3] = (c3 & 1) ? 0xff : 0;
			
		}
		break;
	case TEXEL_CI32:
		{
			taddr0 = ((tbase0 << 2) + s0);
			taddr1 = ((tbase0 << 2) + s1);
			taddr2 = ((tbase2 << 2) + s0);
			taddr3 = ((tbase2 << 2) + s1);
			xort = (t0 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
								
			uint16_t c0, c1, c2, c3;
					
			taddr0 &= 0x7ff;
			taddr1 &= 0x7ff;
			taddr2 &= 0x7ff;
			taddr3 &= 0x7ff;
			c0 = tc16[taddr0];
			color0[0] = c0 >> 8;
			color0[1] = c0 & 0xff;
			color0[2] = c0 >> 8;
			color0[3] = (c0 & 1) ? 0xff : 0;
			c1 = tc16[taddr1];
			color1[0] = c1 >> 8;
			color1[1] = c1 & 0xff;
			color1[2] = c1 >> 8;
			color1[3] = (c1 & 1) ? 0xff : 0;
			c2 = tc16[taddr2];
			color2[0] = c2 >> 8;
			color2[1] = c2 & 0xff;
			color2[2] = c2 >> 8;
			color2[3] = (c2 & 1) ? 0xff : 0;
			c3 = tc16[taddr3];
			color3[0] = c3 >> 8;
			color3[1] = c3 & 0xff;
			color3[2] = c3 >> 8;
			color3[3] = (c3 & 1) ? 0xff : 0;
			
		}
        break;
	case TEXEL_IA4:
		{
			taddr0 = ((tbase0 << 4) + s0) >> 1;
			taddr1 = ((tbase0 << 4) + s1) >> 1;
			taddr2 = ((tbase2 << 4) + s0) >> 1;
			taddr3 = ((tbase2 << 4) + s1) >> 1;
			xort = (t0 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
			
			uint32_t p, i; 
					
			taddr0 &= 0xfff;
			taddr1 &= 0xfff;
			taddr2 &= 0xfff;
			taddr3 &= 0xfff;
			ands = s0 & 1;
			p = TMEM[taddr0];
			p = ands ? (p & 0xf) : (p >> 4);
			i = p & 0xe;
			i = (i << 4) | (i << 1) | (i >> 2);
			color0[0] = i;
			color0[1] = i;
			color0[2] = i;
			color0[3] = (p & 0x1) ? 0xff : 0;
			p = TMEM[taddr2];
			p = ands ? (p & 0xf) : (p >> 4);
			i = p & 0xe;
			i = (i << 4) | (i << 1) | (i >> 2);
			color2[0] = i;
			color2[1] = i;
			color2[2] = i;
			color2[3] = (p & 0x1) ? 0xff : 0;

			ands = s1 & 1;
			p = TMEM[taddr1];
			p = ands ? (p & 0xf) : (p >> 4);
			i = p & 0xe;
			i = (i << 4) | (i << 1) | (i >> 2);
			color1[0] = i;
			color1[1] = i;
			color1[2] = i;
			color1[3] = (p & 0x1) ? 0xff : 0;
			p = TMEM[taddr3];
			p = ands ? (p & 0xf) : (p >> 4);
			i = p & 0xe;
			i = (i << 4) | (i << 1) | (i >> 2);
			color3[0] = i;
			color3[1] = i;
			color3[2] = i;
			color3[3] = (p & 0x1) ? 0xff : 0;
			
		}
		break;
	case TEXEL_IA8:
		{
			taddr0 = ((tbase0 << 3) + s0);
			taddr1 = ((tbase0 << 3) + s1);
			taddr2 = ((tbase2 << 3) + s0);
			taddr3 = ((tbase2 << 3) + s1);
			xort = (t0 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
			
			uint32_t p, i;

			taddr0 &= 0xfff;
			taddr1 &= 0xfff;
			taddr2 &= 0xfff;
			taddr3 &= 0xfff;
			p = TMEM[taddr0];
			i = p & 0xf0;
			i |= (i >> 4);
			color0[0] = i;
			color0[1] = i;
			color0[2] = i;
			color0[3] = ((p & 0xf) << 4) | (p & 0xf);
			p = TMEM[taddr1];
			i = p & 0xf0;
			i |= (i >> 4);
			color1[0] = i;
			color1[1] = i;
			color1[2] = i;
			color1[3] = ((p & 0xf) << 4) | (p & 0xf);
			p = TMEM[taddr2];
			i = p & 0xf0;
			i |= (i >> 4);
			color2[0] = i;
			color2[1] = i;
			color2[2] = i;
			color2[3] = ((p & 0xf) << 4) | (p & 0xf);
			p = TMEM[taddr3];
			i = p & 0xf0;
			i |= (i >> 4);
			color3[0] = i;
			color3[1] = i;
			color3[2] = i;
			color3[3] = ((p & 0xf) << 4) | (p & 0xf);
			
			
		}
		break;
	case TEXEL_IA16:
		{
			taddr0 = ((tbase0 << 2) + s0);
			taddr1 = ((tbase0 << 2) + s1);
			taddr2 = ((tbase2 << 2) + s0);
			taddr3 = ((tbase2 << 2) + s1);
			xort = (t0 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
								
			uint16_t c0, c1, c2, c3;
										
			taddr0 &= 0x7ff;
			taddr1 &= 0x7ff;
			taddr2 &= 0x7ff;
			taddr3 &= 0x7ff;
			c0 = tc16[taddr0];
			color0[0] = color0[1] = color0[2] = c0 >> 8;
			color0[3] = c0 & 0xff;
			c1 = tc16[taddr1];
			color1[0] = color1[1] = color1[2] = c1 >> 8;
			color1[3] = c1 & 0xff;
			c2 = tc16[taddr2];
			color2[0] = color2[1] = color2[2] = c2 >> 8;
			color2[3] = c2 & 0xff;
			c3 = tc16[taddr3];
			color3[0] = color3[1] = color3[2] = c3 >> 8;
			color3[3] = c3 & 0xff;
				
		}
		break;
	case TEXEL_IA32:
		{
			taddr0 = ((tbase0 << 2) + s0);
			taddr1 = ((tbase0 << 2) + s1);
			taddr2 = ((tbase2 << 2) + s0);
			taddr3 = ((tbase2 << 2) + s1);
			xort = (t0 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
								
			uint16_t c0, c1, c2, c3;
					
			taddr0 &= 0x7ff;
			taddr1 &= 0x7ff;
			taddr2 &= 0x7ff;
			taddr3 &= 0x7ff;
			c0 = tc16[taddr0];
			color0[0] = c0 >> 8;
			color0[1] = c0 & 0xff;
			color0[2] = c0 >> 8;
			color0[3] = (c0 & 1) ? 0xff : 0;
			c1 = tc16[taddr1];
			color1[0] = c1 >> 8;
			color1[1] = c1 & 0xff;
			color1[2] = c1 >> 8;
			color1[3] = (c1 & 1) ? 0xff : 0;
			c2 = tc16[taddr2];
			color2[0] = c2 >> 8;
			color2[1] = c2 & 0xff;
			color2[2] = c2 >> 8;
			color2[3] = (c2 & 1) ? 0xff : 0;
			c3 = tc16[taddr3];
			color3[0] = c3 >> 8;
			color3[1] = c3 & 0xff;
			color3[2] = c3 >> 8;
			color3[3] = (c3 & 1) ? 0xff : 0;
						
		}
		break;
	case TEXEL_I4:
		{
			taddr0 = ((tbase0 << 4) + s0) >> 1;
			taddr1 = ((tbase0 << 4) + s1) >> 1;
			taddr2 = ((tbase2 << 4) + s0) >> 1;
			taddr3 = ((tbase2 << 4) + s1) >> 1;
			xort = (t0 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
			
			uint32_t p, c0, c1, c2, c3; 
			
			taddr0 &= 0xfff;
			taddr1 &= 0xfff;
			taddr2 &= 0xfff;
			taddr3 &= 0xfff;
			ands = s0 & 1;
			p = TMEM[taddr0];
			c0 = ands ? (p & 0xf) : (p >> 4);
			c0 |= (c0 << 4);
			color0[0] = color0[1] = color0[2] = color0[3] = c0;
			p = TMEM[taddr2];
			c2 = ands ? (p & 0xf) : (p >> 4);
			c2 |= (c2 << 4);
			color2[0] = color2[1] = color2[2] = color2[3] = c2;

			ands = s1 & 1;
			p = TMEM[taddr1];
			c1 = ands ? (p & 0xf) : (p >> 4);
			c1 |= (c1 << 4);
			color1[0] = color1[1] = color1[2] = color1[3] = c1;
			p = TMEM[taddr3];
			c3 = ands ? (p & 0xf) : (p >> 4);
			c3 |= (c3 << 4);
			color3[0] = color3[1] = color3[2] = color3[3] = c3;
				
		}
		break;
	case TEXEL_I8:
		{
			taddr0 = ((tbase0 << 3) + s0);
			taddr1 = ((tbase0 << 3) + s1);
			taddr2 = ((tbase2 << 3) + s0);
			taddr3 = ((tbase2 << 3) + s1);
			xort = (t0 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
			
			uint32_t p;

			taddr0 &= 0xfff;
			taddr1 &= 0xfff;
			taddr2 &= 0xfff;
			taddr3 &= 0xfff;
			p = TMEM[taddr0];
			color0[0] = p;
			color0[1] = p;
			color0[2] = p;
			color0[3] = p;
			p = TMEM[taddr1];
			color1[0] = p;
			color1[1] = p;
			color1[2] = p;
			color1[3] = p;
			p = TMEM[taddr2];
			color2[0] = p;
			color2[1] = p;
			color2[2] = p;
			color2[3] = p;
			p = TMEM[taddr3];
			color3[0] = p;
			color3[1] = p;
			color3[2] = p;
			color3[3] = p;
		}
		break;
	case TEXEL_I16:
		{
			taddr0 = ((tbase0 << 2) + s0);
			taddr1 = ((tbase0 << 2) + s1);
			taddr2 = ((tbase2 << 2) + s0);
			taddr3 = ((tbase2 << 2) + s1);
			xort = (t0 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
								
			uint16_t c0, c1, c2, c3;
					
			taddr0 &= 0x7ff;
			taddr1 &= 0x7ff;
			taddr2 &= 0x7ff;
			taddr3 &= 0x7ff;
			c0 = tc16[taddr0];
			color0[0] = c0 >> 8;
			color0[1] = c0 & 0xff;
			color0[2] = c0 >> 8;
			color0[3] = (c0 & 1) ? 0xff : 0;
			c1 = tc16[taddr1];
			color1[0] = c1 >> 8;
			color1[1] = c1 & 0xff;
			color1[2] = c1 >> 8;
			color1[3] = (c1 & 1) ? 0xff : 0;
			c2 = tc16[taddr2];
			color2[0] = c2 >> 8;
			color2[1] = c2 & 0xff;
			color2[2] = c2 >> 8;
			color2[3] = (c2 & 1) ? 0xff : 0;
			c3 = tc16[taddr3];
			color3[0] = c3 >> 8;
			color3[1] = c3 & 0xff;
			color3[2] = c3 >> 8;
			color3[3] = (c3 & 1) ? 0xff : 0;
		}
		break;
	case TEXEL_I32:
		{
			taddr0 = ((tbase0 << 2) + s0);
			taddr1 = ((tbase0 << 2) + s1);
			taddr2 = ((tbase2 << 2) + s0);
			taddr3 = ((tbase2 << 2) + s1);
			xort = (t0 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
								
			uint16_t c0, c1, c2, c3;
					
			taddr0 &= 0x7ff;
			taddr1 &= 0x7ff;
			taddr2 &= 0x7ff;
			taddr3 &= 0x7ff;
			c0 = tc16[taddr0];
			color0[0] = c0 >> 8;
			color0[1] = c0 & 0xff;
			color0[2] = c0 >> 8;
			color0[3] = (c0 & 1) ? 0xff : 0;
			c1 = tc16[taddr1];
			color1[0] = c1 >> 8;
			color1[1] = c1 & 0xff;
			color1[2] = c1 >> 8;
			color1[3] = (c1 & 1) ? 0xff : 0;
			c2 = tc16[taddr2];
			color2[0] = c2 >> 8;
			color2[1] = c2 & 0xff;
			color2[2] = c2 >> 8;
			color2[3] = (c2 & 1) ? 0xff : 0;
			c3 = tc16[taddr3];
			color3[0] = c3 >> 8;
			color3[1] = c3 & 0xff;
			color3[2] = c3 >> 8;
			color3[3] = (c3 & 1) ? 0xff : 0;
		}
		break;
	default:
		debug("fetch_texel_quadro: unknown texture format %d, size %d, tilenum %d\n", tile[tilenum].format, tile[tilenum].size, tilenum);
		break;
	}
}

static void fetch_texel_entlut_quadro(COLOR color0, COLOR color1, COLOR color2, COLOR color3, int s0, int s1, int t0, int t1, uint32_t tilenum)
{
	uint32_t tbase0 = tile[tilenum].line * t0 + tile[tilenum].tmem;
	uint32_t tbase2 = tile[tilenum].line * t1 + tile[tilenum].tmem;
	uint32_t tpal	= tile[tilenum].palette << 4;
	uint32_t xort = 0, ands = 0;

	uint16_t *tc16 = (uint16_t*)TMEM;
	uint32_t taddr0 = 0, taddr1 = 0, taddr2 = 0, taddr3 = 0;
  int16_t colors[4];
	
	switch(tile[tilenum].f.tlutswitch)
	{
	case 0:
	case 1:
	case 2:
		{
			taddr0 = ((tbase0 << 4) + s0) >> 1;
			taddr1 = ((tbase0 << 4) + s1) >> 1;
			taddr2 = ((tbase2 << 4) + s0) >> 1;
			taddr3 = ((tbase2 << 4) + s1) >> 1;
			xort = (t0 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
															
			ands = s0 & 1;
			colors[0] = TMEM[taddr0 & 0x7ff];
			colors[0] = (ands) ? (colors[0] & 0xf) : (colors[0] >> 4);
			colors[0] = tlut[((tpal | colors[0]) << 2) ^ WORD_ADDR_XOR];
			colors[2] = TMEM[taddr2 & 0x7ff];
			colors[2] = (ands) ? (colors[2] & 0xf) : (colors[2] >> 4);
			colors[2] = tlut[((tpal | colors[2]) << 2) ^ WORD_ADDR_XOR];

			ands = s1 & 1;
			colors[1] = TMEM[taddr1 & 0x7ff];
			colors[1] = (ands) ? (colors[1] & 0xf) : (colors[1] >> 4);
			colors[1] = tlut[((tpal | colors[1]) << 2) ^ WORD_ADDR_XOR];
			colors[3] = TMEM[taddr3 & 0x7ff];
			colors[3] = (ands) ? (colors[3] & 0xf) : (colors[3] >> 4);
			colors[3] = tlut[((tpal | colors[3]) << 2) ^ WORD_ADDR_XOR];
		}
		break;
	case 3:
		{
			taddr0 = ((tbase0 << 3) + s0);
			taddr1 = ((tbase0 << 3) + s1);
			taddr2 = ((tbase2 << 3) + s0);
			taddr3 = ((tbase2 << 3) + s1);
			xort = (t0 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
															
			ands = s0 & 1;
			colors[0] = TMEM[taddr0 & 0x7ff];
			colors[0] = (ands) ? (colors[0] & 0xf) : (colors[0] >> 4);
			colors[0] = tlut[((tpal | colors[0]) << 2) ^ WORD_ADDR_XOR];
			colors[2] = TMEM[taddr2 & 0x7ff];
			colors[2] = (ands) ? (colors[2] & 0xf) : (colors[2] >> 4);
			colors[2] = tlut[((tpal | colors[2]) << 2) ^ WORD_ADDR_XOR];

			ands = s1 & 1;
			colors[1] = TMEM[taddr1 & 0x7ff];
			colors[1] = (ands) ? (colors[1] & 0xf) : (colors[1] >> 4);
			colors[1] = tlut[((tpal | colors[1]) << 2) ^ WORD_ADDR_XOR];
			colors[3] = TMEM[taddr3 & 0x7ff];
			colors[3] = (ands) ? (colors[3] & 0xf) : (colors[3] >> 4);
			colors[3] = tlut[((tpal | colors[3]) << 2) ^ WORD_ADDR_XOR];
		}
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		{
			taddr0 = ((tbase0 << 3) + s0);
			taddr1 = ((tbase0 << 3) + s1);
			taddr2 = ((tbase2 << 3) + s0);
			taddr3 = ((tbase2 << 3) + s1);
			xort = (t0 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
			
			colors[0] = TMEM[taddr0 & 0x7ff];
			colors[0] = tlut[(colors[0] << 2) ^ WORD_ADDR_XOR];
			colors[2] = TMEM[taddr2 & 0x7ff];
			colors[2] = tlut[(colors[2] << 2) ^ WORD_ADDR_XOR];
			colors[1] = TMEM[taddr1 & 0x7ff];
			colors[1] = tlut[(colors[1] << 2) ^ WORD_ADDR_XOR];
			colors[3] = TMEM[taddr3 & 0x7ff];
			colors[3] = tlut[(colors[3] << 2) ^ WORD_ADDR_XOR];
		}
		break;
	case 8:
	case 9:
	case 10:
		{
			taddr0 = ((tbase0 << 2) + s0);
			taddr1 = ((tbase0 << 2) + s1);
			taddr2 = ((tbase2 << 2) + s0);
			taddr3 = ((tbase2 << 2) + s1);
			xort = (t0 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
					
			colors[0] = tc16[taddr0 & 0x3ff];
			colors[0] = tlut[((colors[0] >> 6) & ~3) ^ WORD_ADDR_XOR];
			colors[1] = tc16[taddr1 & 0x3ff];
			colors[1] = tlut[((colors[1] >> 6) & ~3) ^ WORD_ADDR_XOR];
			colors[2] = tc16[taddr2 & 0x3ff];
			colors[2] = tlut[((colors[2] >> 6) & ~3) ^ WORD_ADDR_XOR];
			colors[3] = tc16[taddr3 & 0x3ff];
			colors[3] = tlut[((colors[3] >> 6) & ~3) ^ WORD_ADDR_XOR];
		}
		break;
	case 11:
		{
			taddr0 = ((tbase0 << 3) + s0);
			taddr1 = ((tbase0 << 3) + s1);
			taddr2 = ((tbase2 << 3) + s0);
			taddr3 = ((tbase2 << 3) + s1);
			xort = (t0 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
			
			colors[0] = TMEM[taddr0 & 0x7ff];
			colors[0] = tlut[(colors[0] << 2) ^ WORD_ADDR_XOR];
			colors[2] = TMEM[taddr2 & 0x7ff];
			colors[2] = tlut[(colors[2] << 2) ^ WORD_ADDR_XOR];
			colors[1] = TMEM[taddr1 & 0x7ff];
			colors[1] = tlut[(colors[1] << 2) ^ WORD_ADDR_XOR];
			colors[3] = TMEM[taddr3 & 0x7ff];
			colors[3] = tlut[(colors[3] << 2) ^ WORD_ADDR_XOR];
		}
		break;
	case 12:
	case 13:
	case 14:
		{
			taddr0 = ((tbase0 << 2) + s0);
			taddr1 = ((tbase0 << 2) + s1);
			taddr2 = ((tbase2 << 2) + s0);
			taddr3 = ((tbase2 << 2) + s1);
			xort = (t0 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? WORD_XOR_DWORD_SWAP : WORD_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
								
			colors[0] = tc16[taddr0 & 0x3ff];
			colors[0] = tlut[((colors[0] >> 6) & ~3) ^ WORD_ADDR_XOR];
			colors[1] = tc16[taddr1 & 0x3ff];
			colors[1] = tlut[((colors[1] >> 6) & ~3) ^ WORD_ADDR_XOR];
			colors[2] = tc16[taddr2 & 0x3ff];
			colors[2] = tlut[((colors[2] >> 6) & ~3) ^ WORD_ADDR_XOR];
			colors[3] = tc16[taddr3 & 0x3ff];
			colors[3] = tlut[((colors[3] >> 6) & ~3) ^ WORD_ADDR_XOR];
		}
		break;
	case 15:
		{
			taddr0 = ((tbase0 << 3) + s0);
			taddr1 = ((tbase0 << 3) + s1);
			taddr2 = ((tbase2 << 3) + s0);
			taddr3 = ((tbase2 << 3) + s1);
			xort = (t0 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr0 ^= xort;
			taddr1 ^= xort;
			xort = (t1 & 1) ? BYTE_XOR_DWORD_SWAP : BYTE_ADDR_XOR;
			taddr2 ^= xort;
			taddr3 ^= xort;
			
			colors[0] = TMEM[taddr0 & 0x7ff];
			colors[0] = tlut[(colors[0] << 2) ^ WORD_ADDR_XOR];
			colors[2] = TMEM[taddr2 & 0x7ff];
			colors[2] = tlut[(colors[2] << 2) ^ WORD_ADDR_XOR];
			colors[1] = TMEM[taddr1 & 0x7ff];
			colors[1] = tlut[(colors[1] << 2) ^ WORD_ADDR_XOR];
			colors[3] = TMEM[taddr3 & 0x7ff];
			colors[3] = tlut[(colors[3] << 2) ^ WORD_ADDR_XOR];
		}
		break;
	default:
		debug("fetch_texel_entlut_quadro: unknown texture format %d, size %d, tilenum %d\n", tile[tilenum].format, tile[tilenum].size, tilenum);
		break;
	}

	if (!other_modes.tlut_type)
	{
#if 0
		color0[0] = GET_HI_RGBA16_TMEM(c0);
		color0[1] = GET_MED_RGBA16_TMEM(c0);
		color0[2] = GET_LOW_RGBA16_TMEM(c0);
		color0[3] = (c0 & 1) ? 0xff : 0;
		color1[0] = GET_HI_RGBA16_TMEM(c1);
		color1[1] = GET_MED_RGBA16_TMEM(c1);
		color1[2] = GET_LOW_RGBA16_TMEM(c1);
		color1[3] = (c1 & 1) ? 0xff : 0;
		color2[0] = GET_HI_RGBA16_TMEM(c2);
		color2[1] = GET_MED_RGBA16_TMEM(c2);
		color2[2] = GET_LOW_RGBA16_TMEM(c2);
		color2[3] = (c2 & 1) ? 0xff : 0;
		color3[0] = GET_HI_RGBA16_TMEM(c3);
		color3[1] = GET_MED_RGBA16_TMEM(c3);
		color3[2] = GET_LOW_RGBA16_TMEM(c3);
		color3[3] = (c3 & 1) ? 0xff : 0;
#endif
    fetch_texel_quadro_rgba16(color0, color1, color2, color3, colors);
	}
	else
	{
		color0[0] = color0[1] = color0[2] = colors[0] >> 8;
		color0[3] = colors[0] & 0xff;
		color1[0] = color1[1] = color1[2] = colors[1] >> 8;
		color1[3] = colors[1] & 0xff;
		color2[0] = color2[1] = color2[2] = colors[2] >> 8;
		color2[3] = colors[2] & 0xff;
		color3[0] = color3[1] = color3[2] = colors[3] >> 8;
		color3[3] = colors[3] & 0xff;
	}
}


void get_tmem_idx(int s, int t, uint32_t tilenum, uint32_t* idx0, uint32_t* idx1, uint32_t* idx2, uint32_t* idx3, uint32_t* bit3flipped, uint32_t* hibit)
{
	uint32_t tbase = (tile[tilenum].line * t) & 0x1ff;
	tbase += tile[tilenum].tmem;
	uint32_t tsize = tile[tilenum].size;
	uint32_t tformat = tile[tilenum].format;
	uint32_t sshorts = 0;

	
	if (tsize == PIXEL_SIZE_8BIT || tformat == FORMAT_YUV)
		sshorts = s >> 1;
	else if (tsize >= PIXEL_SIZE_16BIT)
		sshorts = s;
	else
		sshorts = s >> 2;
	sshorts &= 0x7ff;

	*bit3flipped = ((sshorts & 2) ? 1 : 0) ^ (t & 1);
		
	int tidx_a = ((tbase << 2) + sshorts) & 0x7fd;
	int tidx_b = (tidx_a + 1) & 0x7ff;
	int tidx_c = (tidx_a + 2) & 0x7ff;
	int tidx_d = (tidx_a + 3) & 0x7ff;

	*hibit = (tidx_a & 0x400) ? 1 : 0;

	if (t & 1)
	{
		tidx_a ^= 2;
		tidx_b ^= 2;
		tidx_c ^= 2;
		tidx_d ^= 2;
	}

	
	sort_tmem_idx(idx0, tidx_a, tidx_b, tidx_c, tidx_d, 0);
	sort_tmem_idx(idx1, tidx_a, tidx_b, tidx_c, tidx_d, 1);
	sort_tmem_idx(idx2, tidx_a, tidx_b, tidx_c, tidx_d, 2);
	sort_tmem_idx(idx3, tidx_a, tidx_b, tidx_c, tidx_d, 3);
}







void read_tmem_copy(int s, int s1, int s2, int s3, int t, uint32_t tilenum, uint32_t* sortshort, int* hibits, int* lowbits)
{
	uint32_t tbase = (tile[tilenum].line * t) & 0x1ff;
	tbase += tile[tilenum].tmem;
	uint32_t tsize = tile[tilenum].size;
	uint32_t tformat = tile[tilenum].format;
	uint32_t shbytes = 0, shbytes1 = 0, shbytes2 = 0, shbytes3 = 0;
	int32_t delta = 0;
	uint32_t sortidx[8];

	
	if (tsize == PIXEL_SIZE_8BIT || tformat == FORMAT_YUV)
	{
		shbytes = s << 1;
		shbytes1 = s1 << 1;
		shbytes2 = s2 << 1;
		shbytes3 = s3 << 1;
	}
	else if (tsize >= PIXEL_SIZE_16BIT)
	{
		shbytes = s << 2;
		shbytes1 = s1 << 2;
		shbytes2 = s2 << 2;
		shbytes3 = s3 << 2;
	}
	else
	{
		shbytes = s;
		shbytes1 = s1;
		shbytes2 = s2;
		shbytes3 = s3;
	}

	shbytes &= 0x1fff;
	shbytes1 &= 0x1fff;
	shbytes2 &= 0x1fff;
	shbytes3 &= 0x1fff;

	int tidx_a, tidx_blow, tidx_bhi, tidx_c, tidx_dlow, tidx_dhi;

	tbase <<= 4;
	tidx_a = (tbase + shbytes) & 0x1fff;
	tidx_bhi = (tbase + shbytes1) & 0x1fff;
	tidx_c = (tbase + shbytes2) & 0x1fff;
	tidx_dhi = (tbase + shbytes3) & 0x1fff;

	if (tformat == FORMAT_YUV)
	{
		delta = shbytes1 - shbytes;
		tidx_blow = (tidx_a + (delta << 1)) & 0x1fff;
		tidx_dlow = (tidx_blow + shbytes3 - shbytes) & 0x1fff;
	}
	else
	{
		tidx_blow = tidx_bhi;
		tidx_dlow = tidx_dhi;
	}

	if (t & 1)
	{
		tidx_a ^= 8;
		tidx_blow ^= 8;
		tidx_bhi ^= 8;
		tidx_c ^= 8;
		tidx_dlow ^= 8;
		tidx_dhi ^= 8;
	}

	hibits[0] = (tidx_a & 0x1000) ? 1 : 0;
	hibits[1] = (tidx_blow & 0x1000) ? 1 : 0; 
	hibits[2] =	(tidx_bhi & 0x1000) ? 1 : 0;
	hibits[3] =	(tidx_c & 0x1000) ? 1 : 0;
	hibits[4] =	(tidx_dlow & 0x1000) ? 1 : 0;
	hibits[5] = (tidx_dhi & 0x1000) ? 1 : 0;
	lowbits[0] = tidx_a & 0xf;
	lowbits[1] = tidx_blow & 0xf;
	lowbits[2] = tidx_bhi & 0xf;
	lowbits[3] = tidx_c & 0xf;
	lowbits[4] = tidx_dlow & 0xf;
	lowbits[5] = tidx_dhi & 0xf;

	uint16_t* tmem16 = (uint16_t*)TMEM;
	uint32_t short0, short1, short2, short3;

	
	tidx_a >>= 2;
	tidx_blow >>= 2;
	tidx_bhi >>= 2;
	tidx_c >>= 2;
	tidx_dlow >>= 2;
	tidx_dhi >>= 2;

	
	sort_tmem_idx(&sortidx[0], tidx_a, tidx_blow, tidx_c, tidx_dlow, 0);
	sort_tmem_idx(&sortidx[1], tidx_a, tidx_blow, tidx_c, tidx_dlow, 1);
	sort_tmem_idx(&sortidx[2], tidx_a, tidx_blow, tidx_c, tidx_dlow, 2);
	sort_tmem_idx(&sortidx[3], tidx_a, tidx_blow, tidx_c, tidx_dlow, 3);

	short0 = tmem16[sortidx[0] ^ WORD_ADDR_XOR];
	short1 = tmem16[sortidx[1] ^ WORD_ADDR_XOR];
	short2 = tmem16[sortidx[2] ^ WORD_ADDR_XOR];
	short3 = tmem16[sortidx[3] ^ WORD_ADDR_XOR];

	
	sort_tmem_shorts_lowhalf(&sortshort[0], short0, short1, short2, short3, lowbits[0] >> 2);
	sort_tmem_shorts_lowhalf(&sortshort[1], short0, short1, short2, short3, lowbits[1] >> 2);
	sort_tmem_shorts_lowhalf(&sortshort[2], short0, short1, short2, short3, lowbits[3] >> 2);
	sort_tmem_shorts_lowhalf(&sortshort[3], short0, short1, short2, short3, lowbits[4] >> 2);

	if (other_modes.en_tlut)
	{
	 	
		compute_color_index(&short0, sortshort[0], lowbits[0] & 3, tilenum);
		compute_color_index(&short1, sortshort[1], lowbits[1] & 3, tilenum);
		compute_color_index(&short2, sortshort[2], lowbits[3] & 3, tilenum);
		compute_color_index(&short3, sortshort[3], lowbits[4] & 3, tilenum);

		
		sortidx[4] = (short0 << 2);
		sortidx[5] = (short1 << 2) | 1;
		sortidx[6] = (short2 << 2) | 2;
		sortidx[7] = (short3 << 2) | 3;
	}
	else
	{
		sort_tmem_idx(&sortidx[4], tidx_a, tidx_bhi, tidx_c, tidx_dhi, 0);
		sort_tmem_idx(&sortidx[5], tidx_a, tidx_bhi, tidx_c, tidx_dhi, 1);
		sort_tmem_idx(&sortidx[6], tidx_a, tidx_bhi, tidx_c, tidx_dhi, 2);
		sort_tmem_idx(&sortidx[7], tidx_a, tidx_bhi, tidx_c, tidx_dhi, 3);
	}

	short0 = tmem16[(sortidx[4] | 0x400) ^ WORD_ADDR_XOR];
	short1 = tmem16[(sortidx[5] | 0x400) ^ WORD_ADDR_XOR];
	short2 = tmem16[(sortidx[6] | 0x400) ^ WORD_ADDR_XOR];
	short3 = tmem16[(sortidx[7] | 0x400) ^ WORD_ADDR_XOR];

	
	
	if (other_modes.en_tlut)
	{
		sort_tmem_shorts_lowhalf(&sortshort[4], short0, short1, short2, short3, 0);
		sort_tmem_shorts_lowhalf(&sortshort[5], short0, short1, short2, short3, 1);
		sort_tmem_shorts_lowhalf(&sortshort[6], short0, short1, short2, short3, 2);
		sort_tmem_shorts_lowhalf(&sortshort[7], short0, short1, short2, short3, 3);
	}
	else
	{
		sort_tmem_shorts_lowhalf(&sortshort[4], short0, short1, short2, short3, lowbits[0] >> 2);
		sort_tmem_shorts_lowhalf(&sortshort[5], short0, short1, short2, short3, lowbits[2] >> 2);
		sort_tmem_shorts_lowhalf(&sortshort[6], short0, short1, short2, short3, lowbits[3] >> 2);
		sort_tmem_shorts_lowhalf(&sortshort[7], short0, short1, short2, short3, lowbits[5] >> 2);
	}
}




void sort_tmem_idx(uint32_t *idx, uint32_t idxa, uint32_t idxb, uint32_t idxc, uint32_t idxd, uint32_t bankno)
{
	if ((idxa & 3) == bankno)
		*idx = idxa & 0x3ff;
	else if ((idxb & 3) == bankno)
		*idx = idxb & 0x3ff;
	else if ((idxc & 3) == bankno)
		*idx = idxc & 0x3ff;
	else if ((idxd & 3) == bankno)
		*idx = idxd & 0x3ff;
	else
		*idx = 0;
}


void sort_tmem_shorts_lowhalf(uint32_t* bindshort, uint32_t short0, uint32_t short1, uint32_t short2, uint32_t short3, uint32_t bankno)
{
	switch(bankno)
	{
	case 0:
		*bindshort = short0;
		break;
	case 1:
		*bindshort = short1;
		break;
	case 2:
		*bindshort = short2;
		break;
	case 3:
		*bindshort = short3;
		break;
	}
}



void compute_color_index(uint32_t* cidx, uint32_t readshort, uint32_t nybbleoffset, uint32_t tilenum)
{
	uint32_t lownib, hinib;
	if (tile[tilenum].size == PIXEL_SIZE_4BIT)
	{
		lownib = (nybbleoffset ^ 3) << 2;
		hinib = tile[tilenum].palette;
	}
	else
	{
		lownib = ((nybbleoffset & 2) ^ 2) << 2;
		hinib = lownib ? ((readshort >> 12) & 0xf) : ((readshort >> 4) & 0xf);
	}
	lownib = (readshort >> lownib) & 0xf;
	*cidx = (hinib << 4) | lownib;
}


void replicate_for_copy(uint32_t* outbyte, uint32_t inshort, uint32_t nybbleoffset, uint32_t tilenum, uint32_t tformat, uint32_t tsize)
{
	uint32_t lownib, hinib;
	switch(tsize)
	{
	case PIXEL_SIZE_4BIT:
		lownib = (nybbleoffset ^ 3) << 2;
		lownib = hinib = (inshort >> lownib) & 0xf;
		if (tformat == FORMAT_CI)
		{
			*outbyte = (tile[tilenum].palette << 4) | lownib;
		}
		else if (tformat == FORMAT_IA)
		{
			lownib = (lownib << 4) | lownib;
			*outbyte = (lownib & 0xe0) | ((lownib & 0xe0) >> 3) | ((lownib & 0xc0) >> 6);
		}
		else
			*outbyte = (lownib << 4) | lownib;
		break;
	case PIXEL_SIZE_8BIT:
		hinib = ((nybbleoffset ^ 3) | 1) << 2;
		if (tformat == FORMAT_IA)
		{
			lownib = (inshort >> hinib) & 0xf;
			*outbyte = (lownib << 4) | lownib;
		}
		else
		{
			lownib = (inshort >> (hinib & ~4)) & 0xf;
			hinib = (inshort >> hinib) & 0xf;
			*outbyte = (hinib << 4) | lownib;
		}
		break;
	default:
		*outbyte = (inshort >> 8) & 0xff;
		break;
	}
}

void fetch_qword_copy(uint32_t* hidword, uint32_t* lowdword, int32_t ssss, int32_t ssst, uint32_t tilenum)
{
	uint32_t shorta, shortb, shortc, shortd;
	uint32_t sortshort[8];
	int hibits[6];
	int lowbits[6];
	int32_t sss = ssss, sst = ssst, sss1 = 0, sss2 = 0, sss3 = 0;
	int largetex = 0;

	uint32_t tformat, tsize;
	if (other_modes.en_tlut)
	{
		tsize = PIXEL_SIZE_16BIT;
		tformat = other_modes.tlut_type ? FORMAT_IA : FORMAT_RGBA;
	}
	else
	{
		tsize = tile[tilenum].size;
		tformat = tile[tilenum].format;
	}

	tc_pipeline_copy(&sss, &sss1, &sss2, &sss3, &sst, tilenum);
	read_tmem_copy(sss, sss1, sss2, sss3, sst, tilenum, sortshort, hibits, lowbits);
	largetex = (tformat == FORMAT_YUV || (tformat == FORMAT_RGBA && tsize == PIXEL_SIZE_32BIT));

	
	if (other_modes.en_tlut)
	{
		shorta = sortshort[4];
		shortb = sortshort[5];
		shortc = sortshort[6];
		shortd = sortshort[7];
	}
	else if (largetex)
	{
		shorta = sortshort[0];
		shortb = sortshort[1];
		shortc = sortshort[2];
		shortd = sortshort[3];
	}
	else
	{
		shorta = hibits[0] ? sortshort[4] : sortshort[0];
		shortb = hibits[1] ? sortshort[5] : sortshort[1];
		shortc = hibits[3] ? sortshort[6] : sortshort[2];
		shortd = hibits[4] ? sortshort[7] : sortshort[3];
	}

	*lowdword = (shortc << 16) | shortd;

	if (tsize == PIXEL_SIZE_16BIT)
		*hidword = (shorta << 16) | shortb;
	else
	{
		replicate_for_copy(&shorta, shorta, lowbits[0] & 3, tilenum, tformat, tsize);
		replicate_for_copy(&shortb, shortb, lowbits[1] & 3, tilenum, tformat, tsize);
		replicate_for_copy(&shortc, shortc, lowbits[3] & 3, tilenum, tformat, tsize);
		replicate_for_copy(&shortd, shortd, lowbits[4] & 3, tilenum, tformat, tsize);
		*hidword = (shorta << 24) | (shortb << 16) | (shortc << 8) | shortd;
	}
}

static inline void texture_pipeline_cycle(COLOR TEX, COLOR prev, int32_t SSS, int32_t SST, uint32_t tilenum, uint32_t cycle)
{
#define TRELATIVE(x, y) 	((x) - ((y) << 3));


#define UPPER ((sfrac + tfrac) & 0x20)




	int32_t maxs, maxt, invt0r, invt0g, invt0b, invt0a;
	int32_t sfrac, tfrac, invsf, invtf;
	int upper = 0;
	int bilerp = cycle ? other_modes.bi_lerp1 : other_modes.bi_lerp0;
	int convert = other_modes.convert_one && cycle;
	COLOR t0, t1, t2, t3;
	int sss1, sst1, sss2, sst2;

	sss1 = SSS;
	sst1 = SST;

	tcshift_cycle(&sss1, &sst1, &maxs, &maxt, tilenum);

	sss1 = TRELATIVE(sss1, tile[tilenum].sl);
	sst1 = TRELATIVE(sst1, tile[tilenum].tl);

	if (other_modes.sample_type)
	{	
		sfrac = sss1 & 0x1f;
		tfrac = sst1 & 0x1f;

		tcclamp_cycle(&sss1, &sst1, &sfrac, &tfrac, maxs, maxt, tilenum);
		
	
		if (tile[tilenum].format != FORMAT_YUV)
			sss2 = sss1 + 1;
		else
			sss2 = sss1 + 2;
		
		
		

		sst2 = sst1 + 1;
		

		
		tcmask_coupled(&sss1, &sss2, &sst1, &sst2, tilenum);
		
		

		
		
		
		

		
    __m128i tex_v;
		if (bilerp)
		{			
			if (!other_modes.en_tlut)
				fetch_texel_quadro(t0, t1, t2, t3, sss1, sss2, sst1, sst2, tilenum);
			else
				fetch_texel_entlut_quadro(t0, t1, t2, t3, sss1, sss2, sst1, sst2, tilenum);

			if (tile[tilenum].format == FORMAT_YUV)
			{
				t0[0] = SIGN(t0[0], 9);
				t0[1] = SIGN(t0[1], 9);
				t1[0] = SIGN(t1[0], 9);
				t1[1] = SIGN(t1[1], 9);
				t2[0] = SIGN(t2[0], 9);
				t2[1] = SIGN(t2[1], 9);
				t3[0] = SIGN(t3[0], 9);
				t3[1] = SIGN(t3[1], 9);
			}

      __m128i t0_v = _mm_load_si128(t0);
      __m128i t1_v = _mm_load_si128(t1);
      __m128i t2_v = _mm_load_si128(t2);
      __m128i t3_v = _mm_load_si128(t3);

			if (!other_modes.mid_texel || sfrac != 0x10 || tfrac != 0x10)
			{
				if (!convert)
				{
          __m128i cv_v = _mm_set1_epi32(0x10);
          __m128i prod_a, prod_b, summand;

					if (UPPER)
					{
						
						invsf = 0x20 - sfrac;
						invtf = 0x20 - tfrac;
#if 0
						TEX[0] = t3[0] + ((((invsf * (t2[0] - t3[0])) + (invtf * (t1[0] - t3[0]))) + 0x10) >> 5);	
						TEX[1] = t3[1] + ((((invsf * (t2[1] - t3[1])) + (invtf * (t1[1] - t3[1]))) + 0x10) >> 5);																		
						TEX[2] = t3[2] + ((((invsf * (t2[2] - t3[2])) + (invtf * (t1[2] - t3[2]))) + 0x10) >> 5);																
						TEX[3] = t3[3] + ((((invsf * (t2[3] - t3[3])) + (invtf * (t1[3] - t3[3]))) + 0x10) >> 5);
#endif
            __m128i invsf_v = _mm_set1_epi32(invsf);
            __m128i invtf_v = _mm_set1_epi32(invtf);

            prod_a = _mm_mullo_epi32(invsf_v, _mm_sub_epi32(t2_v, t3_v));
            prod_b = _mm_mullo_epi32(invtf_v, _mm_sub_epi32(t1_v, t3_v));
            summand = t3_v;
					}
					else
					{
#if 0
						TEX[0] = t0[0] + ((((sfrac * (t1[0] - t0[0])) + (tfrac * (t2[0] - t0[0]))) + 0x10) >> 5);											
						TEX[1] = t0[1] + ((((sfrac * (t1[1] - t0[1])) + (tfrac * (t2[1] - t0[1]))) + 0x10) >> 5);											
						TEX[2] = t0[2] + ((((sfrac * (t1[2] - t0[2])) + (tfrac * (t2[2] - t0[2]))) + 0x10) >> 5);									
						TEX[3] = t0[3] + ((((sfrac * (t1[3] - t0[3])) + (tfrac * (t2[3] - t0[3]))) + 0x10) >> 5);
#endif
            __m128i sfrac_v = _mm_set1_epi32(sfrac);
            __m128i tfrac_v = _mm_set1_epi32(tfrac);

            prod_a = _mm_mullo_epi32(sfrac_v, _mm_sub_epi32(t1_v, t0_v));
            prod_b = _mm_mullo_epi32(tfrac_v, _mm_sub_epi32(t2_v, t0_v));
            summand = t0_v;
					}

          __m128i sum = _mm_add_epi32(_mm_add_epi32(prod_a, prod_b), cv_v);
          tex_v = _mm_add_epi32(summand, _mm_srai_epi32(sum, 5));
				}
				else
				{
          __m128i prev0_v = _mm_set1_epi32(prev[0]);
          __m128i prev1_v = _mm_set1_epi32(prev[1]);
          __m128i prev2_v = _mm_set1_epi32(prev[2]);
          __m128i cv_v = _mm_set1_epi32(0x80);

          __m128i prev0_prod, prev1_prod;

					if (UPPER)
					{
#if 0
						TEX[0] = prev[2] + ((((prev[0] * (t2[0] - t3[0])) + (prev[1] * (t1[0] - t3[0]))) + 0x80) >> 8);	
						TEX[1] = prev[2] + ((((prev[0] * (t2[1] - t3[1])) + (prev[1] * (t1[1] - t3[1]))) + 0x80) >> 8);																		
						TEX[2] = prev[2] + ((((prev[0] * (t2[2] - t3[2])) + (prev[1] * (t1[2] - t3[2]))) + 0x80) >> 8);																
						TEX[3] = prev[2] + ((((prev[0] * (t2[3] - t3[3])) + (prev[1] * (t1[3] - t3[3]))) + 0x80) >> 8);
#endif
            prev0_prod = _mm_mullo_epi32(prev0_v, _mm_sub_epi32(t2_v, t3_v));
            prev1_prod = _mm_mullo_epi32(prev1_v, _mm_sub_epi32(t1_v, t3_v));
					}
					else
					{
#if 0
						TEX[0] = prev[2] + ((((prev[0] * (t1[0] - t0[0])) + (prev[1] * (t2[0] - t0[0]))) + 0x80) >> 8);											
						TEX[1] = prev[2] + ((((prev[0] * (t1[1] - t0[1])) + (prev[1] * (t2[1] - t0[1]))) + 0x80) >> 8);											
						TEX[2] = prev[2] + ((((prev[0] * (t1[2] - t0[2])) + (prev[1] * (t2[2] - t0[2]))) + 0x80) >> 8);									
						TEX[3] = prev[2] + ((((prev[0] * (t1[3] - t0[3])) + (prev[1] * (t2[3] - t0[3]))) + 0x80) >> 8);
#endif
            prev0_prod = _mm_mullo_epi32(prev0_v, _mm_sub_epi32(t2_v, t3_v));
            prev1_prod = _mm_mullo_epi32(prev1_v, _mm_sub_epi32(t1_v, t3_v));
					}	

          __m128i sum = _mm_add_epi32(_mm_add_epi32(prev0_prod, prev1_prod), cv_v);
          tex_v = _mm_add_epi32(prev2_v, _mm_srai_epi32(sum, 8));
				}
				
			}
			else
			{
#if 0
				invt0r  = ~t0[0]; invt0g = ~t0[1]; invt0b = ~t0[2]; invt0a = ~t0[3];
#endif
        __m128i invt0r_v = _mm_xor_si128(t0_v, _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
        __m128i invt0r_shiftsum = _mm_slli_epi32(_mm_add_epi32(invt0r_v, t3_v), 6);
        __m128i cv_v = _mm_set1_epi32(0xc0);
        __m128i prod_a, prod_b;

				if (!convert)
				{
					sfrac <<= 2;
					tfrac <<= 2;

          __m128i sfrac_v = _mm_set1_epi32(sfrac);
          __m128i tfrac_v = _mm_set1_epi32(tfrac);
#if 0
					TEX[0] = t0[0] + ((((sfrac * (t1[0] - t0[0])) + (tfrac * (t2[0] - t0[0]))) + ((invt0r + t3[0]) << 6) + 0xc0) >> 8);											
					TEX[1] = t0[1] + ((((sfrac * (t1[1] - t0[1])) + (tfrac * (t2[1] - t0[1]))) + ((invt0g + t3[1]) << 6) + 0xc0) >> 8);											
					TEX[2] = t0[2] + ((((sfrac * (t1[2] - t0[2])) + (tfrac * (t2[2] - t0[2]))) + ((invt0b + t3[2]) << 6) + 0xc0) >> 8);									
					TEX[3] = t0[3] + ((((sfrac * (t1[3] - t0[3])) + (tfrac * (t2[3] - t0[3]))) + ((invt0a + t3[3]) << 6) + 0xc0) >> 8);
#endif
          prod_a = _mm_mullo_epi32(sfrac_v, _mm_sub_epi32(t1_v, t0_v));
          prod_b = _mm_mullo_epi32(tfrac_v, _mm_sub_epi32(t2_v, t0_v));
				}
				else
				{
#if 0
					TEX[0] = prev[2] + ((((prev[0] * (t1[0] - t0[0])) + (prev[1] * (t2[0] - t0[0]))) + ((invt0r + t3[0]) << 6) + 0xc0) >> 8);											
					TEX[1] = prev[2] + ((((prev[0] * (t1[1] - t0[1])) + (prev[1] * (t2[1] - t0[1]))) + ((invt0g + t3[1]) << 6) + 0xc0) >> 8);											
					TEX[2] = prev[2] + ((((prev[0] * (t1[2] - t0[2])) + (prev[1] * (t2[2] - t0[2]))) + ((invt0b + t3[2]) << 6) + 0xc0) >> 8);									
					TEX[3] = prev[2] + ((((prev[0] * (t1[3] - t0[3])) + (prev[1] * (t2[3] - t0[3]))) + ((invt0a + t3[3]) << 6) + 0xc0) >> 8);
#endif

          __m128i prev0_v = _mm_set1_epi32(prev[0]);
          __m128i prev1_v = _mm_set1_epi32(prev[1]);
          __m128i prev2_v = _mm_set1_epi32(prev[2]);

          prod_a = _mm_mullo_epi32(prev0_v, _mm_sub_epi32(t1_v, t0_v));
          prod_b = _mm_mullo_epi32(prev1_v, _mm_sub_epi32(t2_v, t0_v));
				}

        __m128i sum = _mm_add_epi32(_mm_add_epi32(_mm_add_epi32(prod_a, prod_b), invt0r_shiftsum), cv_v);
        tex_v = _mm_add_epi32(t0_v, _mm_srai_epi32(sum, 8));
			}
			
		}
		else
		{
			if (!other_modes.en_tlut)
				fetch_texel(t0, sss1, sst1, tilenum);
			else
				fetch_texel_entlut(t0, sss1, sst1, tilenum);
			if (convert)
        memcpy(t0, prev, sizeof(COLOR));

			if (tile[tilenum].format == FORMAT_YUV)
			{
				t0[0] = SIGN(t0[0], 9);
				t0[1] = SIGN(t0[1], 9);
			}


			TEX[0] = t0[2] + ((k0_tf * t0[1] + 0x80) >> 8);
			TEX[1] = t0[2] + ((k1_tf * t0[0] + k2_tf * t0[1] + 0x80) >> 8);
			TEX[2] = t0[2] + ((k3_tf * t0[0] + 0x80) >> 8);
			TEX[3] = t0[2];
      tex_v = _mm_load_si128(TEX);
		}

#if 0
		TEX[0] &= 0x1ff;
		TEX[1] &= 0x1ff;
		TEX[2] &= 0x1ff;
		TEX[3] &= 0x1ff;
#endif
		tex_v = _mm_and_si128(tex_v, _mm_set1_epi32(0x1ff));
		_mm_store_si128(TEX, tex_v);
	}
	else																								
	{																										
		
		
		

		tcclamp_cycle_light(&sss1, &sst1, maxs, maxt, tilenum);
		
        tcmask(&sss1, &sst1, tilenum);	
																										
			
		if (!other_modes.en_tlut)
			fetch_texel(t0, sss1, sst1, tilenum);
		else
			fetch_texel_entlut(t0, sss1, sst1, tilenum);
		
		if (bilerp)
		{
			if (!convert)
        memcpy(TEX, t0, sizeof(COLOR));
			else
				TEX[0] = TEX[1] = TEX[2] = TEX[3] = prev[2];
		}
		else
		{
			if (convert)
        memcpy(t0, prev, sizeof(COLOR));
			t0[0] = SIGN(t0[0], 9);
			t0[1] = SIGN(t0[1], 9); 
			t0[2] = SIGN(t0[2], 9);
			if (tile[tilenum].format == FORMAT_YUV)
			{
				t0[0] = SIGN(t0[0], 9);
				t0[1] = SIGN(t0[1], 9);
			}

			TEX[0] = t0[2] + ((k0_tf * t0[1] + 0x80) >> 8);
			TEX[1] = t0[2] + ((k1_tf * t0[0] + k2_tf * t0[1] + 0x80) >> 8);
			TEX[2] = t0[2] + ((k3_tf * t0[0] + 0x80) >> 8);
			TEX[3] = t0[2];
			TEX[0] &= 0x1ff;
			TEX[1] &= 0x1ff;
			TEX[2] &= 0x1ff;
			TEX[3] &= 0x1ff;
		}
	}
																									
}


static inline void tc_pipeline_copy(int32_t* sss0, int32_t* sss1, int32_t* sss2, int32_t* sss3, int32_t* sst, int tilenum)											
{
	int ss0 = *sss0, ss1 = 0, ss2 = 0, ss3 = 0, st = *sst;

	tcshift_copy(&ss0, &st, tilenum);
	
	

	ss0 = TRELATIVE(ss0, tile[tilenum].sl);
	st = TRELATIVE(st, tile[tilenum].tl);
	ss0 = (ss0 >> 5);
	st = (st >> 5);

	ss1 = ss0 + 1;
	ss2 = ss0 + 2;
	ss3 = ss0 + 3;

	tcmask_copy(&ss0, &ss1, &ss2, &ss3, &st, tilenum);	

	*sss0 = ss0;
	*sss1 = ss1;
	*sss2 = ss2;
	*sss3 = ss3;
	*sst = st;
}

static inline void tc_pipeline_load(int32_t* sss, int32_t* sst, int tilenum, int coord_quad)
{
	int sss1 = *sss, sst1 = *sst;
	sss1 = SIGN16(sss1);
	sst1 = SIGN16(sst1);

	
	sss1 = TRELATIVE(sss1, tile[tilenum].sl);
	sst1 = TRELATIVE(sst1, tile[tilenum].tl);
	

	
	if (!coord_quad)
	{
		sss1 = (sss1 >> 5);
		sst1 = (sst1 >> 5);
	}
	else
	{
		sss1 = (sss1 >> 3);
		sst1 = (sst1 >> 3);
	}
	
	*sss = sss1;
	*sst = sst1;
}



void render_spans_1cycle_complete(int start, int end, int tilenum, int flip, __m128i spans_drgba_v, __m128i spans_dstwz_v, __m128i spans_cdrgba_drgbady_v)
{
	int zb = zb_address >> 1;
	int zbcur;
	uint8_t offx, offy;
	SPANSIGS sigs;
	uint32_t blend_en;
	uint32_t prewrap;
	uint32_t curpixel_cvg, curpixel_cvbit, curpixel_memcvg;

	int prim_tile = tilenum;
	int tile1 = tilenum;
	int newtile = tilenum; 
	int news, newt;

	int i, j;
	
	int drinc, dginc, dbinc, dainc, dzinc, dsinc, dtinc, dwinc;
	int xinc;

	if (flip)
	{
		drinc = spans_drgba[0];
		dginc = spans_drgba[1];
		dbinc = spans_drgba[2];
		dainc = spans_drgba[3];
		dzinc = spans_dstwz[3];
		dsinc = spans_dstwz[0];
		dtinc = spans_dstwz[1];
		dwinc = spans_dstwz[2];
		xinc = 1;
	}
	else
	{
		drinc = -spans_drgba[0];
		dginc = -spans_drgba[1];
		dbinc = -spans_drgba[2];
		dainc = -spans_drgba[3];
		dzinc = -spans_dstwz[3];
		dsinc = -spans_dstwz[0];
		dtinc = -spans_dstwz[1];
		dwinc = -spans_dstwz[2];
		xinc = -1;
	}

	int dzpix;
	if (!other_modes.z_source_sel)
		dzpix = spans_dzpix;
	else
	{
		dzpix = primitive_delta_z;
		dzinc = spans_cdz = spans_dstwzdy[3] = 0;
	}
	int dzpixenc = dz_compress(dzpix);

	int cdith = 7, adith = 0;
	int r, g, b, a, z, s, t, w;
	int sr, sg, sb, sa, sz, ss, st, sw;
	int xstart, xend, xendsc;
	int sss = 0, sst = 0;
	int32_t prelodfrac;
	int curpixel = 0;
	int x, length, scdiff;
	uint32_t fir, fig, fib;
					
	for (i = start; i <= end; i++)
	{
		if (span[i].validline)
		{

		xstart = span[i].lx;
		xend = span[i].unscrx;
		xendsc = span[i].rx;
		r = span[i].rgbastwz[0];
		g = span[i].rgbastwz[1];
		b = span[i].rgbastwz[2];
		a = span[i].rgbastwz[3];
		z = other_modes.z_source_sel ? primitive_z : span[i].rgbastwz[7];
		s = span[i].rgbastwz[4];
		t = span[i].rgbastwz[5];
		w = span[i].rgbastwz[6];

		x = xendsc;
		curpixel = fb_width * i + x;
		zbcur = zb + curpixel;

		if (!flip)
		{
			length = xendsc - xstart;
			scdiff = xend - xendsc;
			compute_cvg_noflip(i);
		}
		else
		{
			length = xstart - xendsc;
			scdiff = xendsc - xend;
			compute_cvg_flip(i);
		}

		sigs.longspan = (length > 7);
		sigs.midspan = (length == 7);
		sigs.onelessthanmid = (length == 6);



		if (scdiff)
		{
			r += (drinc * scdiff);
			g += (dginc * scdiff);
			b += (dbinc * scdiff);
			a += (dainc * scdiff);
			z += (dzinc * scdiff);
			s += (dsinc * scdiff);
			t += (dtinc * scdiff);
			w += (dwinc * scdiff);
		}
		sigs.startspan = 1;

		for (j = 0; j <= length; j++)
		{
			sr = r >> 14;
			sg = g >> 14;
			sb = b >> 14;
			sa = a >> 14;
			ss = s >> 16;
			st = t >> 16;
			sw = w >> 16;
			sz = (z >> 10) & 0x3fffff;
			

			sigs.endspan = (j == length);
			sigs.preendspan = (j == (length - 1));

			lookup_cvmask_derivatives(cvgbuf[x], &offx, &offy, &curpixel_cvg, &curpixel_cvbit);
			

			get_texel1_1cycle(&news, &newt, s, t, w, dsinc, dtinc, dwinc, i, &sigs);

			
			
			if (!sigs.startspan)
			{
        memcpy(texel0_color, texel1_color, sizeof(COLOR));
				lod_frac = prelodfrac;
			}
			else
			{
				tcdiv_ptr(ss, st, sw, &sss, &sst);

				
				tclod_1cycle_current(&sss, &sst, news, newt, s, t, w, dsinc, dtinc, dwinc, i, prim_tile, &tile1, &sigs);
				
				
				
				
				texture_pipeline_cycle(texel0_color, texel0_color, sss, sst, tile1, 0);

				
				sigs.startspan = 0;
			}
			
			sigs.nextspan = sigs.endspan;
			sigs.endspan = sigs.preendspan;
			sigs.preendspan = (j == (length - 2));

			s += dsinc;
			t += dtinc;
			w += dwinc;

			tclod_1cycle_next(&news, &newt, s, t, w, dsinc, dtinc, dwinc, i, prim_tile, &newtile, &sigs, &prelodfrac);

			texture_pipeline_cycle(texel1_color, texel1_color, news, newt, newtile, 0);

			rgbaz_correct_clip(offx, offy, sr, sg, sb, sa, &sz, curpixel_cvg);

			get_dither_noise_ptr(x, i, &cdith, &adith);
			combiner_1cycle(adith, &curpixel_cvg);
				
			fbread1_ptr(curpixel, &curpixel_memcvg);
			if (z_compare(zbcur, sz, dzpix, dzpixenc, &blend_en, &prewrap, &curpixel_cvg, curpixel_memcvg))
			{
				if (blender_1cycle(&fir, &fig, &fib, cdith, blend_en, prewrap, curpixel_cvg, curpixel_cvbit))
				{
					fbwrite_ptr(curpixel, fir, fig, fib, blend_en, curpixel_cvg, curpixel_memcvg);
					if (other_modes.z_update_en)
						z_store(zbcur, sz, dzpixenc);
				}
			}

			
			
			
			r += drinc;
			g += dginc;
			b += dbinc;
			a += dainc;
			z += dzinc;
			
			x += xinc;
			curpixel += xinc;
			zbcur += xinc;
		}
		}
	}
}


void render_spans_1cycle_notexel1(int start, int end, int tilenum, int flip, __m128i spans_drgba_v, __m128i spans_dstwz_v, __m128i spans_cdrgba_drgbady_v)
{
	int zb = zb_address >> 1;
	int zbcur;
	uint8_t offx, offy;
	SPANSIGS sigs;
	uint32_t blend_en;
	uint32_t prewrap;
	uint32_t curpixel_cvg, curpixel_cvbit, curpixel_memcvg;

	int prim_tile = tilenum;
	int tile1 = tilenum;

	int i, j;

	int drinc, dginc, dbinc, dainc, dzinc, dsinc, dtinc, dwinc;
	int xinc;
	if (flip)
	{
		drinc = spans_drgba[0];
		dginc = spans_drgba[1];
		dbinc = spans_drgba[2];
		dainc = spans_drgba[3];
		dzinc = spans_dstwz[3];
		dsinc = spans_dstwz[0];
		dtinc = spans_dstwz[1];
		dwinc = spans_dstwz[2];
		xinc = 1;
	}
	else
	{
		drinc = -spans_drgba[0];
		dginc = -spans_drgba[1];
		dbinc = -spans_drgba[2];
		dainc = -spans_drgba[3];
		dzinc = -spans_dstwz[3];
		dsinc = -spans_dstwz[0];
		dtinc = -spans_dstwz[1];
		dwinc = -spans_dstwz[2];
		xinc = -1;
	}

	int dzpix;
	if (!other_modes.z_source_sel)
		dzpix = spans_dzpix;
	else
	{
		dzpix = primitive_delta_z;
		dzinc = spans_cdz = spans_dstwzdy[3] = 0;
	}
	int dzpixenc = dz_compress(dzpix);

	int cdith = 7, adith = 0;
	int r, g, b, a, z, s, t, w;
	int sr, sg, sb, sa, sz, ss, st, sw;
	int xstart, xend, xendsc;
	int sss = 0, sst = 0;
	int curpixel = 0;
	int x, length, scdiff;
	uint32_t fir, fig, fib;
					
	for (i = start; i <= end; i++)
	{
		if (span[i].validline)
		{

		xstart = span[i].lx;
		xend = span[i].unscrx;
		xendsc = span[i].rx;
		r = span[i].rgbastwz[0];
		g = span[i].rgbastwz[1];
		b = span[i].rgbastwz[2];
		a = span[i].rgbastwz[3];
		z = other_modes.z_source_sel ? primitive_z : span[i].rgbastwz[7];
		s = span[i].rgbastwz[4];
		t = span[i].rgbastwz[5];
		w = span[i].rgbastwz[6];

		x = xendsc;
		curpixel = fb_width * i + x;
		zbcur = zb + curpixel;

		if (!flip)
		{
			length = xendsc - xstart;
			scdiff = xend - xendsc;
			compute_cvg_noflip(i);
		}
		else
		{
			length = xstart - xendsc;
			scdiff = xendsc - xend;
			compute_cvg_flip(i);
		}

		sigs.longspan = (length > 7);
		sigs.midspan = (length == 7);

		if (scdiff)
		{
			r += (drinc * scdiff);
			g += (dginc * scdiff);
			b += (dbinc * scdiff);
			a += (dainc * scdiff);
			z += (dzinc * scdiff);
			s += (dsinc * scdiff);
			t += (dtinc * scdiff);
			w += (dwinc * scdiff);
		}

		for (j = 0; j <= length; j++)
		{
			sr = r >> 14;
			sg = g >> 14;
			sb = b >> 14;
			sa = a >> 14;
			ss = s >> 16;
			st = t >> 16;
			sw = w >> 16;
			sz = (z >> 10) & 0x3fffff;

			sigs.endspan = (j == length);
			sigs.preendspan = (j == (length - 1));

			lookup_cvmask_derivatives(cvgbuf[x], &offx, &offy, &curpixel_cvg, &curpixel_cvbit);

			tcdiv_ptr(ss, st, sw, &sss, &sst);

			tclod_1cycle_current_simple(&sss, &sst, s, t, w, dsinc, dtinc, dwinc, i, prim_tile, &tile1, &sigs);

			texture_pipeline_cycle(texel0_color, texel0_color, sss, sst, tile1, 0);

			rgbaz_correct_clip(offx, offy, sr, sg, sb, sa, &sz, curpixel_cvg);

			get_dither_noise_ptr(x, i, &cdith, &adith);
			combiner_1cycle(adith, &curpixel_cvg);
				
			fbread1_ptr(curpixel, &curpixel_memcvg);
			if (z_compare(zbcur, sz, dzpix, dzpixenc, &blend_en, &prewrap, &curpixel_cvg, curpixel_memcvg))
			{
				if (blender_1cycle(&fir, &fig, &fib, cdith, blend_en, prewrap, curpixel_cvg, curpixel_cvbit))
				{
					fbwrite_ptr(curpixel, fir, fig, fib, blend_en, curpixel_cvg, curpixel_memcvg);
					if (other_modes.z_update_en)
						z_store(zbcur, sz, dzpixenc);
				}
			}

			s += dsinc;
			t += dtinc;
			w += dwinc;
			r += drinc;
			g += dginc;
			b += dbinc;
			a += dainc;
			z += dzinc;
			
			x += xinc;
			curpixel += xinc;
			zbcur += xinc;
		}
		}
	}
}


void render_spans_1cycle_notex(int start, int end, int tilenum, int flip, __m128i spans_drgba_v, __m128i spans_dstwz_v, __m128i spans_cdrgba_drgbady_v)
{
	int zb = zb_address >> 1;
	int zbcur;
	uint8_t offx, offy;
	uint32_t blend_en;
	uint32_t prewrap;
	uint32_t curpixel_cvg, curpixel_cvbit, curpixel_memcvg;

	int i, j;

	int drinc, dginc, dbinc, dainc, dzinc;
	int xinc;

	if (flip)
	{
		drinc = spans_drgba[0];
		dginc = spans_drgba[1];
		dbinc = spans_drgba[2];
		dainc = spans_drgba[3];
		dzinc = spans_dstwz[3];
		xinc = 1;
	}
	else
	{
		drinc = -spans_drgba[0];
		dginc = -spans_drgba[1];
		dbinc = -spans_drgba[2];
		dainc = -spans_drgba[3];
		dzinc = -spans_dstwz[3];
		xinc = -1;
	}
	
	int dzpix;
	if (!other_modes.z_source_sel)
		dzpix = spans_dzpix;
	else
	{
		dzpix = primitive_delta_z;
		dzinc = spans_cdz = spans_dstwzdy[3] = 0;
	}
	int dzpixenc = dz_compress(dzpix);

	int cdith = 7, adith = 0;
	int r, g, b, a, z;
	int sr, sg, sb, sa, sz;
	int xstart, xend, xendsc;
	int curpixel = 0;
	int x, length, scdiff;
	uint32_t fir, fig, fib;
					
	for (i = start; i <= end; i++)
	{
		if (span[i].validline)
		{

		xstart = span[i].lx;
		xend = span[i].unscrx;
		xendsc = span[i].rx;
		r = span[i].rgbastwz[0];
		g = span[i].rgbastwz[1];
		b = span[i].rgbastwz[2];
		a = span[i].rgbastwz[3];
		z = other_modes.z_source_sel ? primitive_z : span[i].rgbastwz[7];

		x = xendsc;
		curpixel = fb_width * i + x;
		zbcur = zb + curpixel;

		if (!flip)
		{
			length = xendsc - xstart;
			scdiff = xend - xendsc;
			compute_cvg_noflip(i);
		}
		else
		{
			length = xstart - xendsc;
			scdiff = xendsc - xend;
			compute_cvg_flip(i);
		}

		if (scdiff)
		{
			r += (drinc * scdiff);
			g += (dginc * scdiff);
			b += (dbinc * scdiff);
			a += (dainc * scdiff);
			z += (dzinc * scdiff);
		}

		for (j = 0; j <= length; j++)
		{
			sr = r >> 14;
			sg = g >> 14;
			sb = b >> 14;
			sa = a >> 14;
			sz = (z >> 10) & 0x3fffff;

			lookup_cvmask_derivatives(cvgbuf[x], &offx, &offy, &curpixel_cvg, &curpixel_cvbit);

			rgbaz_correct_clip(offx, offy, sr, sg, sb, sa, &sz, curpixel_cvg);

			get_dither_noise_ptr(x, i, &cdith, &adith);
			combiner_1cycle(adith, &curpixel_cvg);
				
			fbread1_ptr(curpixel, &curpixel_memcvg);
			if (z_compare(zbcur, sz, dzpix, dzpixenc, &blend_en, &prewrap, &curpixel_cvg, curpixel_memcvg))
			{
				if (blender_1cycle(&fir, &fig, &fib, cdith, blend_en, prewrap, curpixel_cvg, curpixel_cvbit))
				{
					fbwrite_ptr(curpixel, fir, fig, fib, blend_en, curpixel_cvg, curpixel_memcvg);
					if (other_modes.z_update_en)
						z_store(zbcur, sz, dzpixenc);
				}
			}
			r += drinc;
			g += dginc;
			b += dbinc;
			a += dainc;
			z += dzinc;
			
			x += xinc;
			curpixel += xinc;
			zbcur += xinc;
		}
		}
	}
}

void render_spans_2cycle_complete(int start, int end, int tilenum, int flip, __m128i spans_drgba_v, __m128i spans_dstwz_v, __m128i spans_cdrgba_drgbady_v)
{
	int zb = zb_address >> 1;
	int zbcur;
	uint8_t offx, offy;
	SPANSIGS sigs;
	int32_t prelodfrac;
	COLOR nexttexel1_color;
	uint32_t blend_en;
	uint32_t prewrap;
	uint32_t curpixel_cvg, curpixel_cvbit, curpixel_memcvg;
	int32_t acalpha;


	int tile2 = (tilenum + 1) & 7;
	int tile1 = tilenum;
	int prim_tile = tilenum;

	int newtile1 = tile1;
	int newtile2 = tile2;
	int news, newt;

	int i, j;

	int drinc, dginc, dbinc, dainc, dzinc, dsinc, dtinc, dwinc;
	int xinc;
	if (flip)
	{
		drinc = spans_drgba[0];
		dginc = spans_drgba[1];
		dbinc = spans_drgba[2];
		dainc = spans_drgba[3];
		dzinc = spans_dstwz[3];
		dsinc = spans_dstwz[0];
		dtinc = spans_dstwz[1];
		dwinc = spans_dstwz[2];
		xinc = 1;
	}
	else
	{
		drinc = -spans_drgba[0];
		dginc = -spans_drgba[1];
		dbinc = -spans_drgba[2];
		dainc = -spans_drgba[3];
		dzinc = -spans_dstwz[3];
		dsinc = -spans_dstwz[0];
		dtinc = -spans_dstwz[1];
		dwinc = -spans_dstwz[2];
		xinc = -1;
	}

	int dzpix;
	if (!other_modes.z_source_sel)
		dzpix = spans_dzpix;
	else
	{
		dzpix = primitive_delta_z;
		dzinc = spans_cdz = spans_dstwzdy[3] = 0;
	}
	int dzpixenc = dz_compress(dzpix);

	int cdith = 7, adith = 0;
	int r, g, b, a, z, s, t, w;
	int sr, sg, sb, sa, sz, ss, st, sw;
	int xstart, xend, xendsc;
	int sss = 0, sst = 0;
	int curpixel = 0;
	
	int x, length, scdiff;
	uint32_t fir, fig, fib;
				
	for (i = start; i <= end; i++)
	{
		if (span[i].validline)
		{

		xstart = span[i].lx;
		xend = span[i].unscrx;
		xendsc = span[i].rx;
		r = span[i].rgbastwz[0];
		g = span[i].rgbastwz[1];
		b = span[i].rgbastwz[2];
		a = span[i].rgbastwz[3];
		z = other_modes.z_source_sel ? primitive_z : span[i].rgbastwz[7];
		s = span[i].rgbastwz[4];
		t = span[i].rgbastwz[5];
		w = span[i].rgbastwz[6];

		x = xendsc;
		curpixel = fb_width * i + x;
		zbcur = zb + curpixel;

		if (!flip)
		{
			length = xendsc - xstart;
			scdiff = xend - xendsc;
			compute_cvg_noflip(i);
		}
		else
		{
			length = xstart - xendsc;
			scdiff = xendsc - xend;
			compute_cvg_flip(i);
		}

		
		

		
		

		

		if (scdiff)
		{
			r += (drinc * scdiff);
			g += (dginc * scdiff);
			b += (dbinc * scdiff);
			a += (dainc * scdiff);
			z += (dzinc * scdiff);
			s += (dsinc * scdiff);
			t += (dtinc * scdiff);
			w += (dwinc * scdiff);
		}
		sigs.startspan = 1;

		for (j = 0; j <= length; j++)
		{
			sr = r >> 14;
			sg = g >> 14;
			sb = b >> 14;
			sa = a >> 14;
			ss = s >> 16;
			st = t >> 16;
			sw = w >> 16;
			sz = (z >> 10) & 0x3fffff;
			

			lookup_cvmask_derivatives(cvgbuf[x], &offx, &offy, &curpixel_cvg, &curpixel_cvbit);

			get_nexttexel0_2cycle(&news, &newt, s, t, w, dsinc, dtinc, dwinc);
			
			if (!sigs.startspan)
			{
				lod_frac = prelodfrac;
        memcpy(texel0_color, nexttexel_color, sizeof(COLOR));
        memcpy(texel1_color, nexttexel1_color, sizeof(COLOR));
			}
			else
			{
				tcdiv_ptr(ss, st, sw, &sss, &sst);

				tclod_2cycle_current(&sss, &sst, news, newt, s, t, w, dsinc, dtinc, dwinc, prim_tile, &tile1, &tile2);
				

				
				texture_pipeline_cycle(texel0_color, texel0_color, sss, sst, tile1, 0);
				texture_pipeline_cycle(texel1_color, texel0_color, sss, sst, tile2, 1);

				sigs.startspan = 0;
			}

			s += dsinc;
			t += dtinc;
			w += dwinc;

			tclod_2cycle_next(&news, &newt, s, t, w, dsinc, dtinc, dwinc, prim_tile, &newtile1, &newtile2, &prelodfrac);

			texture_pipeline_cycle(nexttexel_color, nexttexel_color, news, newt, newtile1, 0);
			texture_pipeline_cycle(nexttexel1_color, nexttexel_color, news, newt, newtile2, 1);

			rgbaz_correct_clip(offx, offy, sr, sg, sb, sa, &sz, curpixel_cvg);
					
			get_dither_noise_ptr(x, i, &cdith, &adith);
			combiner_2cycle(adith, &curpixel_cvg, &acalpha);

			fbread2_ptr(curpixel, &curpixel_memcvg);




			if (z_compare(zbcur, sz, dzpix, dzpixenc, &blend_en, &prewrap, &curpixel_cvg, curpixel_memcvg))
			{
				if (blender_2cycle(&fir, &fig, &fib, cdith, blend_en, prewrap, curpixel_cvg, curpixel_cvbit, acalpha))
				{
					fbwrite_ptr(curpixel, fir, fig, fib, blend_en, curpixel_cvg, curpixel_memcvg);
					if (other_modes.z_update_en)
						z_store(zbcur, sz, dzpixenc);
					
				}
			}


			else
				memcpy(memory_color, pre_memory_color, sizeof(COLOR));




			r += drinc;
			g += dginc;
			b += dbinc;
			a += dainc;
			z += dzinc;
			
			x += xinc;
			curpixel += xinc;
			zbcur += xinc;
		}
		}
	}
}



void render_spans_2cycle_notexelnext(int start, int end, int tilenum, int flip, __m128i spans_drgba_v, __m128i spans_dstwz_v, __m128i spans_cdrgba_drgbady_v)
{
	int zb = zb_address >> 1;
	int zbcur;
	uint8_t offx, offy;
	uint32_t blend_en;
	uint32_t prewrap;
	uint32_t curpixel_cvg, curpixel_cvbit, curpixel_memcvg;
	int32_t acalpha;

	int tile2 = (tilenum + 1) & 7;
	int tile1 = tilenum;
	int prim_tile = tilenum;

	int i, j;

  __m128i rgba_v, stwz_v;
  __m128i drgbainc = spans_drgba_v;
  __m128i dstwzinc = spans_dstwz_v;
	int xinc = 1;

  if (!flip) {
    drgbainc = _mm_sub_epi32(_mm_setzero_si128(), drgbainc);
    dstwzinc = _mm_sub_epi32(_mm_setzero_si128(), dstwzinc);
    xinc = -xinc;
  }

	int dzpix;
	if (!other_modes.z_source_sel)
		dzpix = spans_dzpix;
	else
	{
		dzpix = primitive_delta_z;
		spans_cdz = spans_dstwzdy[3] = 0;
    dstwzinc = _mm_insert_epi32(dstwzinc, spans_cdz, 3);
	}
	int dzpixenc = dz_compress(dzpix);

	int cdith = 7, adith = 0;
	int sz;
	int xstart, xend, xendsc;
  int ssst[2] = {0, 0};
	int curpixel = 0;
	
	int x, length, scdiff;
	uint32_t fir, fig, fib;
				
	for (i = start; i <= end; i++)
	{
		if (span[i].validline)
		{

		xstart = span[i].lx;
		xend = span[i].unscrx;
		xendsc = span[i].rx;
    rgba_v = _mm_load_si128(&span[i].rgbastwz[0]);
    stwz_v = _mm_load_si128(&span[i].rgbastwz[4]);
    if (other_modes.z_source_sel)
      stwz_v = _mm_insert_epi32(stwz_v, primitive_z, 3);

		x = xendsc;
		curpixel = fb_width * i + x;
		zbcur = zb + curpixel;

		if (!flip)
		{
			length = xendsc - xstart;
			scdiff = xend - xendsc;
			compute_cvg_noflip(i);
		}
		else
		{
			length = xstart - xendsc;
			scdiff = xendsc - xend;
			compute_cvg_flip(i);
		}

		if (scdiff)
		{
      __m128i scdiff_v = _mm_set1_epi32(scdiff);
      rgba_v = _mm_add_epi32(rgba_v, _mm_mullo_epi32(drgbainc, scdiff_v));
      stwz_v = _mm_add_epi32(stwz_v, _mm_mullo_epi32(dstwzinc, scdiff_v));
		}

		for (j = 0; j <= length; j++)
		{
      sz = (_mm_extract_epi32(stwz_v, 3) >> 10) & 0x3fffff;
      __m128i srgba_v = _mm_srai_epi32(rgba_v, 14);
      __m128i sstwz_v = _mm_srai_epi32(stwz_v, 16);

			lookup_cvmask_derivatives(cvgbuf[x], &offx, &offy, &curpixel_cvg, &curpixel_cvbit);

      tcdiv_ptr == tcdiv_persp ? tcdiv_persp_sse(sstwz_v, ssst) : tcdiv_nopersp_sse(sstwz_v, ssst);

			tclod_2cycle_current_simple_sse(ssst, stwz_v, dstwzinc, prim_tile, &tile1, &tile2);
				
			texture_pipeline_cycle(texel0_color, texel0_color, ssst[0], ssst[1], tile1, 0);
			texture_pipeline_cycle(texel1_color, texel0_color, ssst[0], ssst[1], tile2, 1);

			rgbaz_correct_clip_sse(offx, offy, srgba_v, &sz, curpixel_cvg, spans_cdrgba_drgbady_v);

			get_dither_noise_ptr(x, i, &cdith, &adith);
			combiner_2cycle(adith, &curpixel_cvg, &acalpha);

			fbread2_ptr(curpixel, &curpixel_memcvg);

			if (z_compare(zbcur, sz, dzpix, dzpixenc, &blend_en, &prewrap, &curpixel_cvg, curpixel_memcvg))
			{
				if (blender_2cycle(&fir, &fig, &fib, cdith, blend_en, prewrap, curpixel_cvg, curpixel_cvbit, acalpha))
				{
					fbwrite_ptr(curpixel, fir, fig, fib, blend_en, curpixel_cvg, curpixel_memcvg);
					if (other_modes.z_update_en)
						z_store(zbcur, sz, dzpixenc);
				}
			}
			else
				memcpy(memory_color, pre_memory_color, sizeof(COLOR));

      rgba_v = _mm_add_epi32(rgba_v, drgbainc);
      stwz_v = _mm_add_epi32(stwz_v, dstwzinc);
			
			x += xinc;
			curpixel += xinc;
			zbcur += xinc;
		}
		}
	}
}


void render_spans_2cycle_notexel1(int start, int end, int tilenum, int flip, __m128i spans_drgba_v, __m128i spans_dstwz_v, __m128i spans_cdrgba_drgbady_v)
{
	int zb = zb_address >> 1;
	int zbcur;
	uint8_t offx, offy;
	uint32_t blend_en;
	uint32_t prewrap;
	uint32_t curpixel_cvg, curpixel_cvbit, curpixel_memcvg;
	int32_t acalpha;

	int tile1 = tilenum;
	int prim_tile = tilenum;

	int i, j;

	int drinc, dginc, dbinc, dainc, dzinc, dsinc, dtinc, dwinc;
	int xinc;
	if (flip)
	{
		drinc = spans_drgba[0];
		dginc = spans_drgba[1];
		dbinc = spans_drgba[2];
		dainc = spans_drgba[3];
		dzinc = spans_dstwz[3];
		dsinc = spans_dstwz[0];
		dtinc = spans_dstwz[1];
		dwinc = spans_dstwz[2];
		xinc = 1;
	}
	else
	{
		drinc = -spans_drgba[0];
		dginc = -spans_drgba[1];
		dbinc = -spans_drgba[2];
		dainc = -spans_drgba[3];
		dzinc = -spans_dstwz[3];
		dsinc = -spans_dstwz[0];
		dtinc = -spans_dstwz[1];
		dwinc = -spans_dstwz[2];
		xinc = -1;
	}

	int dzpix;
	if (!other_modes.z_source_sel)
		dzpix = spans_dzpix;
	else
	{
		dzpix = primitive_delta_z;
		dzinc = spans_cdz = spans_dstwzdy[3] = 0;
	}
	int dzpixenc = dz_compress(dzpix);

	int cdith = 7, adith = 0;
	int r, g, b, a, z, s, t, w;
	int sr, sg, sb, sa, sz, ss, st, sw;
	int xstart, xend, xendsc;
	int sss = 0, sst = 0;
	int curpixel = 0;
	
	int x, length, scdiff;
	uint32_t fir, fig, fib;
				
	for (i = start; i <= end; i++)
	{
		if (span[i].validline)
		{

		xstart = span[i].lx;
		xend = span[i].unscrx;
		xendsc = span[i].rx;
		r = span[i].rgbastwz[0];
		g = span[i].rgbastwz[1];
		b = span[i].rgbastwz[2];
		a = span[i].rgbastwz[3];
		z = other_modes.z_source_sel ? primitive_z : span[i].rgbastwz[7];
		s = span[i].rgbastwz[4];
		t = span[i].rgbastwz[5];
		w = span[i].rgbastwz[6];

		x = xendsc;
		curpixel = fb_width * i + x;
		zbcur = zb + curpixel;

		if (!flip)
		{
			length = xendsc - xstart;
			scdiff = xend - xendsc;
			compute_cvg_noflip(i);
		}
		else
		{
			length = xstart - xendsc;
			scdiff = xendsc - xend;
			compute_cvg_flip(i);
		}

		if (scdiff)
		{
			r += (drinc * scdiff);
			g += (dginc * scdiff);
			b += (dbinc * scdiff);
			a += (dainc * scdiff);
			z += (dzinc * scdiff);
			s += (dsinc * scdiff);
			t += (dtinc * scdiff);
			w += (dwinc * scdiff);
		}

		for (j = 0; j <= length; j++)
		{
			sr = r >> 14;
			sg = g >> 14;
			sb = b >> 14;
			sa = a >> 14;
			ss = s >> 16;
			st = t >> 16;
			sw = w >> 16;
			sz = (z >> 10) & 0x3fffff;

			lookup_cvmask_derivatives(cvgbuf[x], &offx, &offy, &curpixel_cvg, &curpixel_cvbit);
			
			tcdiv_ptr(ss, st, sw, &sss, &sst);

			tclod_2cycle_current_notexel1(&sss, &sst, s, t, w, dsinc, dtinc, dwinc, prim_tile, &tile1);
			
			
			texture_pipeline_cycle(texel0_color, texel0_color, sss, sst, tile1, 0);

			rgbaz_correct_clip(offx, offy, sr, sg, sb, sa, &sz, curpixel_cvg);

			get_dither_noise_ptr(x, i, &cdith, &adith);
			combiner_2cycle(adith, &curpixel_cvg, &acalpha);

			fbread2_ptr(curpixel, &curpixel_memcvg);

			if (z_compare(zbcur, sz, dzpix, dzpixenc, &blend_en, &prewrap, &curpixel_cvg, curpixel_memcvg))
			{
				if (blender_2cycle(&fir, &fig, &fib, cdith, blend_en, prewrap, curpixel_cvg, curpixel_cvbit, acalpha))
				{
					fbwrite_ptr(curpixel, fir, fig, fib, blend_en, curpixel_cvg, curpixel_memcvg);
					if (other_modes.z_update_en)
						z_store(zbcur, sz, dzpixenc);
				}
			}
			else
				memcpy(memory_color, pre_memory_color, sizeof(COLOR));

			s += dsinc;
			t += dtinc;
			w += dwinc;
			r += drinc;
			g += dginc;
			b += dbinc;
			a += dainc;
			z += dzinc;
			
			x += xinc;
			curpixel += xinc;
			zbcur += xinc;
		}
		}
	}
}


void render_spans_2cycle_notex(int start, int end, int tilenum, int flip, __m128i spans_drgba_v, __m128i spans_dstwz_v, __m128i spans_cdrgba_drgbady_v)
{
	int zb = zb_address >> 1;
	int zbcur;
	uint8_t offx, offy;
	int i, j;
	uint32_t blend_en;
	uint32_t prewrap;
	uint32_t curpixel_cvg, curpixel_cvbit, curpixel_memcvg;
	int32_t acalpha;

	int drinc, dginc, dbinc, dainc, dzinc;
	int xinc;
	if (flip)
	{
		drinc = spans_drgba[0];
		dginc = spans_drgba[1];
		dbinc = spans_drgba[2];
		dainc = spans_drgba[3];
		dzinc = spans_dstwz[3];
		xinc = 1;
	}
	else
	{
		drinc = -spans_drgba[0];
		dginc = -spans_drgba[1];
		dbinc = -spans_drgba[2];
		dainc = -spans_drgba[3];
		dzinc = -spans_dstwz[3];
		xinc = -1;
	}

	int dzpix;
	if (!other_modes.z_source_sel)
		dzpix = spans_dzpix;
	else
	{
		dzpix = primitive_delta_z;
		dzinc = spans_cdz = spans_dstwzdy[3] = 0;
	}
	int dzpixenc = dz_compress(dzpix);

	int cdith = 7, adith = 0;
	int r, g, b, a, z;
	int sr, sg, sb, sa, sz;
	int xstart, xend, xendsc;
	int curpixel = 0;
	
	int x, length, scdiff;
	uint32_t fir, fig, fib;
				
	for (i = start; i <= end; i++)
	{
		if (span[i].validline)
		{

		xstart = span[i].lx;
		xend = span[i].unscrx;
		xendsc = span[i].rx;
		r = span[i].rgbastwz[0];
		g = span[i].rgbastwz[1];
		b = span[i].rgbastwz[2];
		a = span[i].rgbastwz[3];
		z = other_modes.z_source_sel ? primitive_z : span[i].rgbastwz[7];

		x = xendsc;
		curpixel = fb_width * i + x;
		zbcur = zb + curpixel;

		if (!flip)
		{
			length = xendsc - xstart;
			scdiff = xend - xendsc;
			compute_cvg_noflip(i);
		}
		else
		{
			length = xstart - xendsc;
			scdiff = xendsc - xend;
			compute_cvg_flip(i);
		}

		if (scdiff)
		{
			r += (drinc * scdiff);
			g += (dginc * scdiff);
			b += (dbinc * scdiff);
			a += (dainc * scdiff);
			z += (dzinc * scdiff);
		}

		for (j = 0; j <= length; j++)
		{
			sr = r >> 14;
			sg = g >> 14;
			sb = b >> 14;
			sa = a >> 14;
			sz = (z >> 10) & 0x3fffff;

			lookup_cvmask_derivatives(cvgbuf[x], &offx, &offy, &curpixel_cvg, &curpixel_cvbit);

			rgbaz_correct_clip(offx, offy, sr, sg, sb, sa, &sz, curpixel_cvg);

			get_dither_noise_ptr(x, i, &cdith, &adith);
			combiner_2cycle(adith, &curpixel_cvg, &acalpha);

			fbread2_ptr(curpixel, &curpixel_memcvg);

			if (z_compare(zbcur, sz, dzpix, dzpixenc, &blend_en, &prewrap, &curpixel_cvg, curpixel_memcvg))
			{
				if (blender_2cycle(&fir, &fig, &fib, cdith, blend_en, prewrap, curpixel_cvg, curpixel_cvbit, acalpha))
				{
					fbwrite_ptr(curpixel, fir, fig, fib, blend_en, curpixel_cvg, curpixel_memcvg);
					if (other_modes.z_update_en)
						z_store(zbcur, sz, dzpixenc);
				}
			}
			else
				memcpy(memory_color, pre_memory_color, sizeof(COLOR));


			r += drinc;
			g += dginc;
			b += dbinc;
			a += dainc;
			z += dzinc;
			
			x += xinc;
			curpixel += xinc;
			zbcur += xinc;
		}
		}
	}
}

void render_spans_fill(int start, int end, int flip)
{
	if (unlikely(fb_size == PIXEL_SIZE_4BIT))
	{
		rdp_pipeline_crashed = 1;
		return;
	}

	int i, j;
	
	int fastkillbits = other_modes.image_read_en || other_modes.z_compare_en;
	int slowkillbits = other_modes.z_update_en && !other_modes.z_source_sel && !fastkillbits;

	int xinc = flip ? 1 : -1;

	int xstart = 0, xendsc;
	int prevxstart;
	int curpixel = 0;
	int x, length;
				
	for (i = start; i <= end; i++)
	{
		prevxstart = xstart;
		xstart = span[i].lx;
		xendsc = span[i].rx;

		x = xendsc;
		curpixel = fb_width * i + x;
		length = flip ? (xstart - xendsc) : (xendsc - xstart);

		if (span[i].validline)
		{
			if (unlikely(fastkillbits && length >= 0))
			{
				if (!onetimewarnings.fillmbitcrashes)
					debug("render_spans_fill: image_read_en %x z_update_en %x z_compare_en %x. RDP crashed",
					other_modes.image_read_en, other_modes.z_update_en, other_modes.z_compare_en);
				onetimewarnings.fillmbitcrashes = 1;
				rdp_pipeline_crashed = 1;
				return;
			}

			
			
			
			
			for (j = 0; j <= length; j++)
			{
				fbfill_ptr(curpixel);
				x += xinc;
				curpixel += xinc;
			}

			if (unlikely(slowkillbits && length >= 0))
			{
				if (!onetimewarnings.fillmbitcrashes)
					debug("render_spans_fill: image_read_en %x z_update_en %x z_compare_en %x z_source_sel %x. RDP crashed",
					other_modes.image_read_en, other_modes.z_update_en, other_modes.z_compare_en, other_modes.z_source_sel);
				onetimewarnings.fillmbitcrashes = 1;
				rdp_pipeline_crashed = 1;
				return;
			}
		}
	}
}

void render_spans_copy(int start, int end, int tilenum, int flip)
{
	int i, j, k;

	if (unlikely(fb_size == PIXEL_SIZE_32BIT))
	{
		rdp_pipeline_crashed = 1;
		return;
	}
	
	int tile1 = tilenum;
	int prim_tile = tilenum;

	int dsinc, dtinc, dwinc;
	int xinc;
	if (flip)
	{
		dsinc = spans_dstwz[0];
		dtinc = spans_dstwz[2];
		dwinc = spans_dstwz[3];
		xinc = 1;
	}
	else
	{
		dsinc = -spans_dstwz[0];
		dtinc = -spans_dstwz[1];
		dwinc = -spans_dstwz[2];
		xinc = -1;
	}

	int xstart = 0, xendsc;
	int s = 0, t = 0, w = 0, ss = 0, st = 0, sw = 0, sss = 0, sst = 0, ssw = 0;
	int fb_index, length;
	int diff = 0;

	uint32_t hidword = 0, lowdword = 0;
	uint32_t hidword1 = 0, lowdword1 = 0;
	int fbadvance = (fb_size == PIXEL_SIZE_4BIT) ? 8 : 16 >> fb_size;
	uint32_t fbptr = 0;
	int fbptr_advance = flip ? 8 : -8;
	uint64_t copyqword = 0;
	uint32_t tempdword = 0, tempbyte = 0;
	int copywmask = 0, alphamask = 0;
	int bytesperpixel = (fb_size == PIXEL_SIZE_4BIT) ? 1 : (1 << (fb_size - 1));
	uint32_t fbendptr = 0;
	int32_t threshold, currthreshold;

#define PIXELS_TO_BYTES_SPECIAL4(pix, siz) ((siz) ? PIXELS_TO_BYTES(pix, siz) : (pix))
				
	for (i = start; i <= end; i++)
	{
		if (span[i].validline)
		{

		s = span[i].rgbastwz[4];
		t = span[i].rgbastwz[5];
		w = span[i].rgbastwz[6];
		
		xstart = span[i].lx;
		xendsc = span[i].rx;

		fb_index = fb_width * i + xendsc;
		fbptr = fb_address + PIXELS_TO_BYTES_SPECIAL4(fb_index, fb_size);
		fbendptr = fb_address + PIXELS_TO_BYTES_SPECIAL4((fb_width * i + xstart), fb_size);
		length = flip ? (xstart - xendsc) : (xendsc - xstart);

		
		

		for (j = 0; j <= length; j += fbadvance)
		{
			ss = s >> 16;
			st = t >> 16;
			sw = w >> 16;

			tcdiv_ptr(ss, st, sw, &sss, &sst);
			
			tclod_copy(&sss, &sst, s, t, w, dsinc, dtinc, dwinc, prim_tile, &tile1);
			
			
			
			fetch_qword_copy(&hidword, &lowdword, sss, sst, tile1);

			
			
			if (fb_size == PIXEL_SIZE_16BIT || fb_size == PIXEL_SIZE_8BIT)
				copyqword = ((uint64_t)hidword << 32) | ((uint64_t)lowdword);
			else
				copyqword = 0;
			
			
			if (!other_modes.alpha_compare_en)
				alphamask = 0xff;
			else if (fb_size == PIXEL_SIZE_16BIT)
			{
				alphamask = 0;
				alphamask |= (((copyqword >> 48) & 1) ? 0xC0 : 0);
				alphamask |= (((copyqword >> 32) & 1) ? 0x30 : 0);
				alphamask |= (((copyqword >> 16) & 1) ? 0xC : 0);
				alphamask |= ((copyqword & 1) ? 0x3 : 0);
			}
			else if (fb_size == PIXEL_SIZE_8BIT)
			{
				alphamask = 0;
				threshold = (other_modes.dither_alpha_en) ? (irand() & 0xff) : blend_color[3];
				if (other_modes.dither_alpha_en)
				{
					currthreshold = threshold;
					alphamask |= (((copyqword >> 24) & 0xff) >= currthreshold ? 0xC0 : 0);
					currthreshold = ((threshold & 3) << 6) | (threshold >> 2);
					alphamask |= (((copyqword >> 16) & 0xff) >= currthreshold ? 0x30 : 0);
					currthreshold = ((threshold & 0xf) << 4) | (threshold >> 4);
					alphamask |= (((copyqword >> 8) & 0xff) >= currthreshold ? 0xC : 0);
					currthreshold = ((threshold & 0x3f) << 2) | (threshold >> 6);
					alphamask |= ((copyqword & 0xff) >= currthreshold ? 0x3 : 0);	
				}
				else
				{
					alphamask |= (((copyqword >> 24) & 0xff) >= threshold ? 0xC0 : 0);
					alphamask |= (((copyqword >> 16) & 0xff) >= threshold ? 0x30 : 0);
					alphamask |= (((copyqword >> 8) & 0xff) >= threshold ? 0xC : 0);
					alphamask |= ((copyqword & 0xff) >= threshold ? 0x3 : 0);
				}
			}
			else
				alphamask = 0;

			copywmask = (flip) ? (fbendptr - fbptr + bytesperpixel) : (fbptr - fbendptr + bytesperpixel);
			
			if (copywmask > 8) 
				copywmask = 8;
			tempdword = fbptr;
			k = 7;
			while(copywmask > 0)
			{
				tempbyte = (uint32_t)((copyqword >> (k << 3)) & 0xff);
				if (alphamask & (1 << k))
				{
					PAIRWRITE8(tempdword, tempbyte, (tempbyte & 1) ? 3 : 0);
				}
				k--;
				tempdword += xinc;
				copywmask--;
			}
			
			s += dsinc;
			t += dtinc;
			w += dwinc;
			fbptr += fbptr_advance;
		}
		}
	}
}


void loading_pipeline(int start, int end, int tilenum, int coord_quad, int ltlut)
{


	int localdebugmode = 0, cnt = 0;
	int i, j;

	int dsinc, dtinc;
	dsinc = spans_dstwz[0];
	dtinc = spans_dstwz[1];

	int s, t;
	int ss, st;
	int xstart, xend, xendsc;
	int sss = 0, sst = 0;
	int ti_index, length;

	uint32_t tmemidx0 = 0, tmemidx1 = 0, tmemidx2 = 0, tmemidx3 = 0;
	int dswap = 0;
	uint16_t* tmem16 = (uint16_t*)TMEM;
	uint32_t readval0, readval1, readval2, readval3;
	uint32_t readidx32;
	uint64_t loadqword;
	uint16_t tempshort;
	int tmem_formatting = 0;
	uint32_t bit3fl = 0, hibit = 0;

	if (unlikely(end > start && ltlut))
	{
		rdp_pipeline_crashed = 1;
		return;
	}

	if (tile[tilenum].format == FORMAT_YUV)
		tmem_formatting = 0;
	else if (tile[tilenum].format == FORMAT_RGBA && tile[tilenum].size == PIXEL_SIZE_32BIT)
		tmem_formatting = 1;
	else
		tmem_formatting = 2;

	int tiadvance = 0, spanadvance = 0;
	int tiptr = 0;
	switch (ti_size)
	{
	case PIXEL_SIZE_4BIT:
		rdp_pipeline_crashed = 1;
		return;
		break;
	case PIXEL_SIZE_8BIT:
		tiadvance = 8;
		spanadvance = 8;
		break;
	case PIXEL_SIZE_16BIT:
		if (!ltlut)
		{
			tiadvance = 8;
			spanadvance = 4;
		}
		else
		{
			tiadvance = 2;
			spanadvance = 1;
		}
		break;
	case PIXEL_SIZE_32BIT:
		tiadvance = 8;
		spanadvance = 2;
		break;
	}

	for (i = start; i <= end; i++)
	{
		xstart = span[i].lx;
		xend = span[i].unscrx;
		xendsc = span[i].rx;
		s = span[i].rgbastwz[4];
		t = span[i].rgbastwz[5];

		ti_index = ti_width * i + xend;
		tiptr = ti_address + PIXELS_TO_BYTES(ti_index, ti_size);

		length = (xstart - xend + 1) & 0xfff;

		
		for (j = 0; j < length; j+= spanadvance)
		{
			ss = s >> 16;
			st = t >> 16;

			
			
			
			
			
			
			sss = ss & 0xffff;
			sst = st & 0xffff;

			tc_pipeline_load(&sss, &sst, tilenum, coord_quad);

			dswap = sst & 1;

			
			get_tmem_idx(sss, sst, tilenum, &tmemidx0, &tmemidx1, &tmemidx2, &tmemidx3, &bit3fl, &hibit);

			readidx32 = (tiptr >> 2) & ~1;
			RREADIDX32(readval0, readidx32);
			readidx32++;
			RREADIDX32(readval1, readidx32);
			readidx32++;
			RREADIDX32(readval2, readidx32);
			readidx32++;
			RREADIDX32(readval3, readidx32);

			
			switch(tiptr & 7)
			{
			case 0:
				if (!ltlut)
					loadqword = ((uint64_t)readval0 << 32) | readval1;
				else
				{
					tempshort = readval0 >> 16;
					loadqword = ((uint64_t)tempshort << 48) | ((uint64_t) tempshort << 32) | ((uint64_t) tempshort << 16) | tempshort;
				}
				break;
			case 1:
				loadqword = ((uint64_t)readval0 << 40) | ((uint64_t)readval1 << 8) | (readval2 >> 24);
				break;
			case 2:
				if (!ltlut)
					loadqword = ((uint64_t)readval0 << 48) | ((uint64_t)readval1 << 16) | (readval2 >> 16);
				else
				{
					tempshort = readval0 & 0xffff;
					loadqword = ((uint64_t)tempshort << 48) | ((uint64_t) tempshort << 32) | ((uint64_t) tempshort << 16) | tempshort;
				}
				break;
			case 3:
				loadqword = ((uint64_t)readval0 << 56) | ((uint64_t)readval1 << 24) | (readval2 >> 8);
				break;
			case 4:
				if (!ltlut)
					loadqword = ((uint64_t)readval1 << 32) | readval2;
				else
				{
					tempshort = readval1 >> 16;
					loadqword = ((uint64_t)tempshort << 48) | ((uint64_t) tempshort << 32) | ((uint64_t) tempshort << 16) | tempshort;
				}
				break;
			case 5:
				loadqword = ((uint64_t)readval1 << 40) | ((uint64_t)readval2 << 8) | (readval3 >> 24);
				break;
			case 6:
				if (!ltlut)
					loadqword = ((uint64_t)readval1 << 48) | ((uint64_t)readval2 << 16) | (readval3 >> 16);
				else
				{
					tempshort = readval1 & 0xffff;
					loadqword = ((uint64_t)tempshort << 48) | ((uint64_t) tempshort << 32) | ((uint64_t) tempshort << 16) | tempshort;
				}
				break;
			case 7:
				loadqword = ((uint64_t)readval1 << 56) | ((uint64_t)readval2 << 24) | (readval3 >> 8);
				break;
			}

			
			switch(tmem_formatting)
			{
			case 0:
				readval0 = (uint32_t)((((loadqword >> 56) & 0xff) << 24) | (((loadqword >> 40) & 0xff) << 16) | (((loadqword >> 24) & 0xff) << 8) | (((loadqword >> 8) & 0xff) << 0));
				readval1 = (uint32_t)((((loadqword >> 48) & 0xff) << 24) | (((loadqword >> 32) & 0xff) << 16) | (((loadqword >> 16) & 0xff) << 8) | (((loadqword >> 0) & 0xff) << 0));
				if (bit3fl)
				{
					tmem16[tmemidx2 ^ WORD_ADDR_XOR] = (uint16_t)(readval0 >> 16);
					tmem16[tmemidx3 ^ WORD_ADDR_XOR] = (uint16_t)(readval0 & 0xffff);
					tmem16[(tmemidx2 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(readval1 >> 16);
					tmem16[(tmemidx3 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(readval1 & 0xffff);
				}
				else
				{
					tmem16[tmemidx0 ^ WORD_ADDR_XOR] = (uint16_t)(readval0 >> 16);
					tmem16[tmemidx1 ^ WORD_ADDR_XOR] = (uint16_t)(readval0 & 0xffff);
					tmem16[(tmemidx0 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(readval1 >> 16);
					tmem16[(tmemidx1 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(readval1 & 0xffff);
				}
				break;
			case 1:
				readval0 = (uint32_t)(((loadqword >> 48) << 16) | ((loadqword >> 16) & 0xffff));
				readval1 = (uint32_t)((((loadqword >> 32) & 0xffff) << 16) | (loadqword & 0xffff));

				if (bit3fl)
				{
					tmem16[tmemidx2 ^ WORD_ADDR_XOR] = (uint16_t)(readval0 >> 16);
					tmem16[tmemidx3 ^ WORD_ADDR_XOR] = (uint16_t)(readval0 & 0xffff);
					tmem16[(tmemidx2 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(readval1 >> 16);
					tmem16[(tmemidx3 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(readval1 & 0xffff);
				}
				else
				{
					tmem16[tmemidx0 ^ WORD_ADDR_XOR] = (uint16_t)(readval0 >> 16);
					tmem16[tmemidx1 ^ WORD_ADDR_XOR] = (uint16_t)(readval0 & 0xffff);
					tmem16[(tmemidx0 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(readval1 >> 16);
					tmem16[(tmemidx1 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(readval1 & 0xffff);
				}
				break;
			case 2:
				if (!dswap)
				{
					if (!hibit)
					{
						tmem16[tmemidx0 ^ WORD_ADDR_XOR] = (uint16_t)(loadqword >> 48);
						tmem16[tmemidx1 ^ WORD_ADDR_XOR] = (uint16_t)(loadqword >> 32);
						tmem16[tmemidx2 ^ WORD_ADDR_XOR] = (uint16_t)(loadqword >> 16);
						tmem16[tmemidx3 ^ WORD_ADDR_XOR] = (uint16_t)(loadqword & 0xffff);
					}
					else
					{
						tmem16[(tmemidx0 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(loadqword >> 48);
						tmem16[(tmemidx1 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(loadqword >> 32);
						tmem16[(tmemidx2 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(loadqword >> 16);
						tmem16[(tmemidx3 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(loadqword & 0xffff);
					}
				}
				else
				{
					if (!hibit)
					{
						tmem16[tmemidx0 ^ WORD_ADDR_XOR] = (uint16_t)(loadqword >> 16);
						tmem16[tmemidx1 ^ WORD_ADDR_XOR] = (uint16_t)(loadqword & 0xffff);
						tmem16[tmemidx2 ^ WORD_ADDR_XOR] = (uint16_t)(loadqword >> 48);
						tmem16[tmemidx3 ^ WORD_ADDR_XOR] = (uint16_t)(loadqword >> 32);
					}
					else
					{
						tmem16[(tmemidx0 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(loadqword >> 16);
						tmem16[(tmemidx1 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(loadqword & 0xffff);
						tmem16[(tmemidx2 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(loadqword >> 48);
						tmem16[(tmemidx3 | 0x400) ^ WORD_ADDR_XOR] = (uint16_t)(loadqword >> 32);
					}
				}
			break;
			}


			s = (s + dsinc) & ~0x1f;
			t = (t + dtinc) & ~0x1f;
			tiptr += tiadvance;
		}
	}
}

static void edgewalker_for_prims_unpack(const int32_t* ewdata, __m128i *out1, __m128i *out2) {
  __m128i v1 = _mm_load_si128(ewdata + 8);
  __m128i v2 = _mm_load_si128(ewdata + 12);
  __m128i lshift, rshift;

  lshift = _mm_slli_epi32(v1, 16);
  rshift = _mm_srli_epi32(v1, 16);
  v1 = _mm_or_si128(lshift, rshift);

  lshift = _mm_slli_epi32(v2, 16);
  rshift = _mm_srli_epi32(v2, 16);
  v2 = _mm_or_si128(lshift, rshift);

  *out1 = _mm_unpacklo_epi16(v2, v1);
  *out2 = _mm_unpackhi_epi16(v2, v1);
}

static void edgewalker_for_prims(int32_t* ewdata)
{
  __m128i rgba_v, drgbadx_v, drgbade_v, drgbady_v;
  __m128i stwz_v, dstwzdx_v, dstwzde_v, dstwzdy_v;

	int j = 0;
	int xleft = 0, xright = 0, xleft_inc = 0, xright_inc = 0;
	int dr = 0, dg = 0, db = 0, da = 0;
	int tilenum = 0, flip = 0;
	int32_t yl = 0, ym = 0, yh = 0;
	int32_t xl = 0, xm = 0, xh = 0;
	int32_t dxldy = 0, dxhdy = 0, dxmdy = 0;

	if (unlikely(other_modes.f.stalederivs))
	{
		deduce_derivatives();
		other_modes.f.stalederivs = 0;
	}

	
	flip = (ewdata[0] & 0x800000) ? 1 : 0;
	max_level = (ewdata[0] >> 19) & 7;
	tilenum = (ewdata[0] >> 16) & 7;

	
	yl = SIGN(ewdata[0], 14); 
	ym = ewdata[1] >> 16;
	ym = SIGN(ym, 14);
	yh = SIGN(ewdata[1], 14); 
	
	xl = SIGN(ewdata[2], 28);
	xh = SIGN(ewdata[4], 28);
	xm = SIGN(ewdata[6], 28);
	
	dxldy = SIGN(ewdata[3], 30);
	
	dxhdy = SIGN(ewdata[5], 30);
	dxmdy = SIGN(ewdata[7], 30);

  edgewalker_for_prims_unpack(ewdata, &rgba_v, &drgbadx_v);
  edgewalker_for_prims_unpack(ewdata + 8, &drgbade_v, &drgbady_v);
  edgewalker_for_prims_unpack(ewdata + 16, &stwz_v, &dstwzdx_v);
  edgewalker_for_prims_unpack(ewdata + 24, &dstwzde_v, &dstwzdy_v);

  // TODO: Get rid of these by adjusting the decoder.
  stwz_v    = _mm_insert_epi32(stwz_v, ewdata[40], 3);
  dstwzdx_v = _mm_insert_epi32(dstwzdx_v, ewdata[41], 3);
  dstwzde_v = _mm_insert_epi32(dstwzde_v, ewdata[42], 3);
  dstwzdy_v = _mm_insert_epi32(dstwzdy_v, ewdata[43], 3);

  __m128i spans_drgba_v = _mm_slli_epi32(_mm_srli_epi32(drgbadx_v, 5), 5);
  __m128i spans_dstwz_v = _mm_slli_epi32(_mm_srli_epi32(dstwzdx_v, 5), 5);
  spans_dstwz_v = _mm_insert_epi32(spans_dstwz_v, ewdata[41], 3);
  // TODO: Remove after vectorizing all render spans.
  _mm_store_si128(spans_drgba, spans_drgba_v);
  _mm_store_si128(spans_dstwz, spans_dstwz_v);

  __m128i spans_drgbady_v = _mm_srai_epi32(_mm_slli_epi32(drgbady_v, 5), 19); 
  __m128i spans_cdrgba_v = _mm_srai_epi32(_mm_slli_epi32(spans_drgba_v, 5), 19);
  __m128i spans_cdrgba_drgbady_v =  _mm_packs_epi32(
    _mm_unpacklo_epi32(spans_cdrgba_v, spans_drgbady_v),
    _mm_unpackhi_epi32(spans_cdrgba_v, spans_drgbady_v));

  // TODO: Remove after vectorizing all the rgbaz_correct_clips.
  _mm_store_si128(spans_cdrgba_drgbady, spans_cdrgba_drgbady_v);

	spans_cdz = spans_dstwz[3] >> 10;
	spans_cdz = SIGN(spans_cdz, 22);
	
  _mm_store_si128(spans_dstwzdy, _mm_slli_epi32(_mm_srli_epi32(dstwzdy_v, 15), 15));
	spans_dstwzdy[3] = /* dzdy */ ewdata[43] >> 10;
	spans_dstwzdy[3] = SIGN(spans_dstwzdy[3], 22);

	
	int dzdy_dz = (/* dzdy */ ewdata[43] >> 16) & 0xffff;
	int dzdx_dz = (/* dzdx */ ewdata[41] >> 16) & 0xffff;
	
	spans_dzpix = ((dzdy_dz & 0x8000) ? ((~dzdy_dz) & 0x7fff) : dzdy_dz) + ((dzdx_dz & 0x8000) ? ((~dzdx_dz) & 0x7fff) : dzdx_dz);
	spans_dzpix = normalize_dzpix(spans_dzpix & 0xffff) & 0xffff;
	

	
	xleft_inc = (dxmdy >> 2) & ~0x1;
	xright_inc = (dxhdy >> 2) & ~0x1;
	
	
	
	xright = xh & ~0x1;
	xleft = xm & ~0x1;
		
	int k = 0;

  __m128i drgbadiff_v, dstwzdiff_v;
	int sign_dxhdy = (ewdata[5] & 0x80000000) ? 1 : 0;
	int do_offset = !(sign_dxhdy ^ flip);

  drgbadiff_v = _mm_setzero_si128();
  dstwzdiff_v = _mm_setzero_si128();

	if (do_offset)
	{
    __m128i drgbadeh_v = _mm_slli_epi32(_mm_srli_epi32(drgbade_v, 9), 9);
    __m128i drgbadyh_v = _mm_slli_epi32(_mm_srli_epi32(drgbady_v, 9), 9);
    __m128i dstwzdeh_v = _mm_slli_epi32(_mm_srli_epi32(dstwzde_v, 9), 9);
    __m128i dstwzdyh_v = _mm_slli_epi32(_mm_srli_epi32(dstwzdy_v, 9), 9);
		
    drgbadiff_v = _mm_sub_epi32(drgbadeh_v, drgbadyh_v);
    drgbadiff_v = _mm_sub_epi32(drgbadiff_v, _mm_srai_epi32(drgbadiff_v, 2));
    dstwzdiff_v = _mm_sub_epi32(dstwzdeh_v, dstwzdyh_v);
    dstwzdiff_v = _mm_sub_epi32(dstwzdiff_v, _mm_srai_epi32(dstwzdiff_v, 2));
	}

	int xfrac = 0;

  __m128i drgbadxh_v, dstwzdxh_v;

  drgbadxh_v = _mm_setzero_si128();
  dstwzdxh_v = _mm_setzero_si128();

	if (other_modes.cycle_type != CYCLE_TYPE_COPY)
	{
    drgbadxh_v = _mm_slli_epi32(_mm_srai_epi32(drgbadx_v, 9), 1);
    dstwzdxh_v = _mm_slli_epi32(_mm_srai_epi32(dstwzdx_v, 9), 1);
	}





#define ADJUST_ATTR_PRIM()		\
{	\
  __m128i xfrac_v = _mm_set1_epi32(xfrac); \
  __m128i rgba_adjust = _mm_slli_epi32(_mm_srli_epi32(rgba_v, 9), 9); \
  __m128i stwz_adjust = _mm_slli_epi32(_mm_srli_epi32(stwz_v, 9), 9); \
  __m128i rgba_xfrac_adjust = _mm_mullo_epi32(xfrac_v, drgbadxh_v); \
  __m128i stwz_xfrac_adjust = _mm_mullo_epi32(xfrac_v, dstwzdxh_v); \
  __m128i rgba_sum = _mm_sub_epi32(_mm_add_epi32(rgba_adjust, drgbadiff_v), rgba_xfrac_adjust); \
  __m128i stwz_sum = _mm_sub_epi32(_mm_add_epi32(stwz_adjust, dstwzdiff_v), stwz_xfrac_adjust); \
  __m128i rgba_result = _mm_slli_epi32(_mm_srli_epi32(rgba_sum, 10), 10); \
  __m128i stwz_result = _mm_slli_epi32(_mm_srli_epi32(stwz_sum, 10), 10); \
  _mm_store_si128(span[j].rgbastwz, rgba_result); \
  _mm_store_si128(&span[j].rgbastwz[4], stwz_result); \
}


#define ADDVALUES_PRIM() {	\
      rgba_v = _mm_add_epi32(rgba_v, drgbade_v); \
      stwz_v = _mm_add_epi32(stwz_v, dstwzde_v); \
}

	int32_t maxxmx, minxmx, maxxhx, minxhx;

	int spix = 0;
	int ycur =	yh & ~3;
	int ldflag = (sign_dxhdy ^ flip) ? 0 : 3;
	int invaly = 1;
	int length = 0;
	int32_t xrsc = 0, xlsc = 0, stickybit = 0;
	int32_t yllimit = 0, yhlimit = 0;
	if (yl & 0x2000)
		yllimit = 1;
	else if (yl & 0x1000)
		yllimit = 0;
	else
		yllimit = (yl & 0xfff) < clip.yl;
	yllimit = yllimit ? yl : clip.yl;

	int ylfar = yllimit | 3;
	if ((yl >> 2) > (ylfar >> 2))
		ylfar += 4;
	else if ((yllimit >> 2) >= 0 && (yllimit >> 2) < 1023)
		span[(yllimit >> 2) + 1].validline = 0;
	
	
	if (yh & 0x2000)
		yhlimit = 0;
	else if (yh & 0x1000)
		yhlimit = 1;
	else
		yhlimit = (yh >= clip.yh);
	yhlimit = yhlimit ? yh : clip.yh;

	int yhclose = yhlimit & ~3;

	int32_t clipxlshift = clip.xl << 1;
	int32_t clipxhshift = clip.xh << 1;
	int allover = 1, allunder = 1, curover = 0, curunder = 0;
	int allinval = 1;
	int32_t curcross = 0;

	xfrac = ((xright >> 8) & 0xff);

	
	if (flip)
	{
	for (k = ycur; k <= ylfar; k++)
	{
		if (k == ym)
		{
		
			xleft = xl & ~1;
			xleft_inc = (dxldy >> 2) & ~1;
		}
		
		spix = k & 3;
						
		if (k >= yhclose)
		{
			invaly = k < yhlimit || k >= yllimit;
			
			j = k >> 2;

			if (spix == 0)
			{
				maxxmx = 0;
				minxhx = 0xfff;
				allover = allunder = 1;
				allinval = 1;
			}

			stickybit = ((xright >> 1) & 0x1fff) > 0;
			xrsc = ((xright >> 13) & 0x1ffe) | stickybit;
			curunder = ((xright & 0x8000000) || xrsc < clipxhshift); 
			xrsc = curunder ? clipxhshift : (((xright >> 13) & 0x3ffe) | stickybit);
			curover = ((xrsc & 0x2000) || (xrsc & 0x1fff) >= clipxlshift);
			xrsc = curover ? clipxlshift : xrsc;
			span[j].majorx[spix] = xrsc & 0x1fff;
			allover &= curover;
			allunder &= curunder; 

			stickybit = ((xleft >> 1) & 0x1fff) > 0;
			xlsc = ((xleft >> 13) & 0x1ffe) | stickybit;
			curunder = ((xleft & 0x8000000) || xlsc < clipxhshift);
			xlsc = curunder ? clipxhshift : (((xleft >> 13) & 0x3ffe) | stickybit);
			curover = ((xlsc & 0x2000) || (xlsc & 0x1fff) >= clipxlshift);
			xlsc = curover ? clipxlshift : xlsc;
			span[j].minorx[spix] = xlsc & 0x1fff;
			allover &= curover;
			allunder &= curunder; 
			
			
			
			curcross = ((xleft ^ (1 << 27)) & (0x3fff << 14)) < ((xright ^ (1 << 27)) & (0x3fff << 14));
			

			invaly |= curcross;
			span[j].invalyscan[spix] = invaly;
			allinval &= invaly;

			if (!invaly)
			{
				maxxmx = (((xlsc >> 3) & 0xfff) > maxxmx) ? (xlsc >> 3) & 0xfff : maxxmx;
				minxhx = (((xrsc >> 3) & 0xfff) < minxhx) ? (xrsc >> 3) & 0xfff : minxhx;
			}
			
			if (spix == ldflag)
			{
				span[j].unscrx = xright >> 16;
				xfrac = (xright >> 8) & 0xff;
				ADJUST_ATTR_PRIM();
			}

			if (spix == 3)
			{
				span[j].lx = maxxmx;
				span[j].rx = minxhx;
				span[j].validline  = !allinval && !allover && !allunder && (!scfield || (scfield && !(sckeepodd ^ (j & 1))));
				
			}
			
						           
		}

		if (spix == 3)
		{
			ADDVALUES_PRIM();
		}

		xleft += xleft_inc;
		xright += xright_inc;

	}
	}
	else
	{
	for (k = ycur; k <= ylfar; k++)
	{
		if (k == ym)
		{
			xleft = xl & ~1;
			xleft_inc = (dxldy >> 2) & ~1;
		}
		
		spix = k & 3;
						
		if (k >= yhclose)
		{
			invaly = k < yhlimit || k >= yllimit;
			j = k >> 2;

			if (spix == 0)
			{
				maxxhx = 0;
				minxmx = 0xfff;
				allover = allunder = 1;
				allinval = 1;
			}

			stickybit = ((xright >> 1) & 0x1fff) > 0;
			xrsc = ((xright >> 13) & 0x1ffe) | stickybit;
			curunder = ((xright & 0x8000000) || xrsc < clipxhshift); 
			xrsc = curunder ? clipxhshift : (((xright >> 13) & 0x3ffe) | stickybit);
			curover = ((xrsc & 0x2000) || (xrsc & 0x1fff) >= clipxlshift);
			xrsc = curover ? clipxlshift : xrsc;
			span[j].majorx[spix] = xrsc & 0x1fff;
			allover &= curover;
			allunder &= curunder; 

			stickybit = ((xleft >> 1) & 0x1fff) > 0;
			xlsc = ((xleft >> 13) & 0x1ffe) | stickybit;
			curunder = ((xleft & 0x8000000) || xlsc < clipxhshift);
			xlsc = curunder ? clipxhshift : (((xleft >> 13) & 0x3ffe) | stickybit);
			curover = ((xlsc & 0x2000) || (xlsc & 0x1fff) >= clipxlshift);
			xlsc = curover ? clipxlshift : xlsc;
			span[j].minorx[spix] = xlsc & 0x1fff;
			allover &= curover;
			allunder &= curunder; 

			curcross = ((xright ^ (1 << 27)) & (0x3fff << 14)) < ((xleft ^ (1 << 27)) & (0x3fff << 14));
            
			invaly |= curcross;
			span[j].invalyscan[spix] = invaly;
			allinval &= invaly;

			if (!invaly)
			{
				minxmx = (((xlsc >> 3) & 0xfff) < minxmx) ? (xlsc >> 3) & 0xfff : minxmx;
				maxxhx = (((xrsc >> 3) & 0xfff) > maxxhx) ? (xrsc >> 3) & 0xfff : maxxhx;
			}

			if (spix == ldflag)
			{
				span[j].unscrx  = xright >> 16;
				xfrac = (xright >> 8) & 0xff;
				ADJUST_ATTR_PRIM();
			}

			if (spix == 3)
			{
				span[j].lx = minxmx;
				span[j].rx = maxxhx;
				span[j].validline  = !allinval && !allover && !allunder && (!scfield || (scfield && !(sckeepodd ^ (j & 1))));
			}
			
		}

		if (spix == 3)
		{
			ADDVALUES_PRIM();
		}

		xleft += xleft_inc;
		xright += xright_inc;

	}
	}

	
	

	switch(other_modes.cycle_type)
	{
		case CYCLE_TYPE_1: render_spans_1cycle_ptr(yhlimit >> 2, yllimit >> 2, tilenum, flip, spans_drgba_v, spans_dstwz_v, spans_cdrgba_drgbady_v); break;
		case CYCLE_TYPE_2: render_spans_2cycle_ptr(yhlimit >> 2, yllimit >> 2, tilenum, flip, spans_drgba_v, spans_dstwz_v, spans_cdrgba_drgbady_v); break;
		case CYCLE_TYPE_COPY: render_spans_copy(yhlimit >> 2, yllimit >> 2, tilenum, flip); break;
		case CYCLE_TYPE_FILL: render_spans_fill(yhlimit >> 2, yllimit >> 2, flip); break;
		default: debug("cycle_type %d", other_modes.cycle_type); break;
	}
	
	
}



static void edgewalker_for_loads(int32_t* lewdata)
{
	int j = 0;
	int xleft = 0, xright = 0;
	int xstart = 0, xend = 0;
	int s = 0, t = 0, w = 0;
	int dsdx = 0, dtdx = 0;
	int dsdy = 0, dtdy = 0;
	int dsde = 0, dtde = 0;
	int tilenum = 0, flip = 0;
	int32_t yl = 0, ym = 0, yh = 0;
	int32_t xl = 0, xm = 0, xh = 0;
	int32_t dxldy = 0, dxhdy = 0, dxmdy = 0;

	int commandcode = (lewdata[0] >> 24) & 0x3f;
	int ltlut = (commandcode == 0x30);
	int coord_quad = ltlut || (commandcode == 0x33);
	flip = 1;
	max_level = 0;
	tilenum = (lewdata[0] >> 16) & 7;

	
	yl = SIGN(lewdata[0], 14); 
	ym = lewdata[1] >> 16;
	ym = SIGN(ym, 14);
	yh = SIGN(lewdata[1], 14); 
	
	xl = SIGN(lewdata[2], 28);
	xh = SIGN(lewdata[3], 28);
	xm = SIGN(lewdata[4], 28);
	
	dxldy = 0;
	dxhdy = 0;
	dxmdy = 0;

	
	s    = lewdata[5] & 0xffff0000;
	t    = (lewdata[5] & 0xffff) << 16;
	w    = 0;
	dsdx = (lewdata[7] & 0xffff0000) | ((lewdata[6] >> 16) & 0xffff);
	dtdx = ((lewdata[7] << 16) & 0xffff0000)	| (lewdata[6] & 0xffff);
	dsde = 0;
	dtde = (lewdata[9] & 0xffff) << 16;
	dsdy = 0;
	dtdy = (lewdata[8] & 0xffff) << 16;

	spans_dstwz[0] = dsdx & ~0x1f;
	spans_dstwz[1] = dtdx & ~0x1f;
	spans_dstwz[2] = 0;

	
	

	
	
	xright = xh & ~0x1;
	xleft = xm & ~0x1;
		
	int k = 0;

	int sign_dxhdy = 0;

	int do_offset = 0;

	int xfrac = 0;






#define ADJUST_ATTR_LOAD()										\
{																\
	span[j].rgbastwz[4] = s & ~0x3ff;										\
	span[j].rgbastwz[5] = t & ~0x3ff;										\
}


#define ADDVALUES_LOAD() {	\
			t += dtde;		\
}

	int32_t maxxmx, minxhx;

	int spix = 0;
	int ycur =	yh & ~3;
	int ylfar = yl | 3;
	
	int valid_y = 1;
	int length = 0;
	int32_t xrsc = 0, xlsc = 0, stickybit = 0;
	int32_t yllimit = yl;
	int32_t yhlimit = yh;

	xfrac = 0;
	xend = xright >> 16;

	
	for (k = ycur; k <= ylfar; k++)
	{
		if (k == ym)
			xleft = xl & ~1;
		
		spix = k & 3;
						
		if (!(k & ~0xfff))
		{
			j = k >> 2;
			valid_y = !(k < yhlimit || k >= yllimit);

			if (spix == 0)
			{
				maxxmx = 0;
				minxhx = 0xfff;
			}

			xrsc = (xright >> 13) & 0x7ffe;
			
			

			xlsc = (xleft >> 13) & 0x7ffe;

			if (valid_y)
			{
				maxxmx = (((xlsc >> 3) & 0xfff) > maxxmx) ? (xlsc >> 3) & 0xfff : maxxmx;
				minxhx = (((xrsc >> 3) & 0xfff) < minxhx) ? (xrsc >> 3) & 0xfff : minxhx;
			}

			if (spix == 0)
			{
				span[j].unscrx = xend;
				ADJUST_ATTR_LOAD();
			}

			if (spix == 3)
			{
				span[j].lx = maxxmx;
				span[j].rx = minxhx;
				
				
			}
			
						           
		}

		if (spix == 3)
		{
			ADDVALUES_LOAD();
		}

		

	}

	loading_pipeline(yhlimit >> 2, yllimit >> 2, tilenum, coord_quad, ltlut);
}


static const char *const image_format[] = { "RGBA", "YUV", "CI", "IA", "I", "???", "???", "???" };
static const char *const image_size[] = { "4-bit", "8-bit", "16-bit", "32-bit" };

static const uint32_t rdp_command_length[64] =
{
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	32,			
	32+16,		
	32+64,		
	32+64+16,	
	32+64,		
	32+64+16,	
	32+64+64,	
	32+64+64+16,
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	16,			
	16,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8,			
	8			
};

static int rdp_dasm(char *buffer)
{
	int tile;
	const char *format, *size;
	char sl[32], tl[32], sh[32], th[32];
	char s[32], t[32];
	char dsdx[32], dtdy[32];
#if DETAILED_LOGGING
	int i;
	char dtdx[32], dwdx[32];
	char dsdy[32], dwdy[32];
	char dsde[32], dtde[32], dwde[32];
	char yl[32], yh[32], ym[32], xl[32], xh[32], xm[32];
	char dxldy[32], dxhdy[32], dxmdy[32];
	char rt[32], gt[32], bt[32], at[32];
	char drdx[32], dgdx[32], dbdx[32], dadx[32];
	char drdy[32], dgdy[32], dbdy[32], dady[32];
	char drde[32], dgde[32], dbde[32], dade[32];
#endif
	uint32_t r,g,b,a;

	uint32_t cmd[64];
	uint32_t length;
	uint32_t command;

	length = rdp_cmd_ptr * 4;
	if (length < 8)
	{
		sprintf(buffer, "ERROR: length = %d\n", length);
		return 0;
	}

	cmd[0] = rdp_cmd_data[rdp_cmd_cur+0];
	cmd[1] = rdp_cmd_data[rdp_cmd_cur+1];

	tile = (cmd[1] >> 24) & 0x7;
	sprintf(sl, "%4.2f", (float)((cmd[0] >> 12) & 0xfff) / 4.0f);
	sprintf(tl, "%4.2f", (float)((cmd[0] >>  0) & 0xfff) / 4.0f);
	sprintf(sh, "%4.2f", (float)((cmd[1] >> 12) & 0xfff) / 4.0f);
	sprintf(th, "%4.2f", (float)((cmd[1] >>  0) & 0xfff) / 4.0f);

	format = image_format[(cmd[0] >> 21) & 0x7];
	size = image_size[(cmd[0] >> 19) & 0x3];

	r = (cmd[1] >> 24) & 0xff;
	g = (cmd[1] >> 16) & 0xff;
	b = (cmd[1] >>  8) & 0xff;
	a = (cmd[1] >>  0) & 0xff;

	command = (cmd[0] >> 24) & 0x3f;
	switch (command)
	{
		case 0x00:	sprintf(buffer, "No Op"); break;
		case 0x08:
			sprintf(buffer, "Tri_NoShade (%08X %08X)", cmd[0], cmd[1]); break;
		case 0x0a:
			sprintf(buffer, "Tri_Tex (%08X %08X)", cmd[0], cmd[1]); break;
		case 0x0c:
			sprintf(buffer, "Tri_Shade (%08X %08X)", cmd[0], cmd[1]); break;
		case 0x0e:
			sprintf(buffer, "Tri_TexShade (%08X %08X)", cmd[0], cmd[1]); break;
		case 0x09:
			sprintf(buffer, "TriZ_NoShade (%08X %08X)", cmd[0], cmd[1]); break;
		case 0x0b:
			sprintf(buffer, "TriZ_Tex (%08X %08X)", cmd[0], cmd[1]); break;
		case 0x0d:
			sprintf(buffer, "TriZ_Shade (%08X %08X)", cmd[0], cmd[1]); break;
		case 0x0f:
			sprintf(buffer, "TriZ_TexShade (%08X %08X)", cmd[0], cmd[1]); break;
#if DETAILED_LOGGING
		case 0x08:		
		{
			int lft = (command >> 23) & 0x1;

			if (length != rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_NoShade length = %d\n", length);
				return 0;
			}

			cmd[2] = rdp_cmd_data[rdp_cmd_cur+2];
			cmd[3] = rdp_cmd_data[rdp_cmd_cur+3];
			cmd[4] = rdp_cmd_data[rdp_cmd_cur+4];
			cmd[5] = rdp_cmd_data[rdp_cmd_cur+5];
			cmd[6] = rdp_cmd_data[rdp_cmd_cur+6];
			cmd[7] = rdp_cmd_data[rdp_cmd_cur+7];

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)(cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)(cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)(cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)(cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)(cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)(cmd[7] / 65536.0f));

					sprintf(buffer, "Tri_NoShade            %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			break;
		}
		case 0x0a:		
		{
			int lft = (command >> 23) & 0x1;

			if (length < rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_Tex length = %d\n", length);
				return 0;
			}

			for (i=2; i < 24; i++)
			{
				cmd[i] = rdp_cmd_data[rdp_cmd_cur+i];
			}

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)((int32_t)cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)((int32_t)cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)((int32_t)cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)((int32_t)cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)((int32_t)cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)((int32_t)cmd[7] / 65536.0f));

			sprintf(s,		"%4.4f", (float)(int32_t)((cmd[ 8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(t,		"%4.4f", (float)(int32_t)(((cmd[ 8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(w,		"%4.4f", (float)(int32_t)((cmd[ 9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,	"%4.4f", (float)(int32_t)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdx,	"%4.4f", (float)(int32_t)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dwdx,	"%4.4f", (float)(int32_t)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,	"%4.4f", (float)(int32_t)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtde,	"%4.4f", (float)(int32_t)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dwde,	"%4.4f", (float)(int32_t)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,	"%4.4f", (float)(int32_t)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdy,	"%4.4f", (float)(int32_t)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dwdy,	"%4.4f", (float)(int32_t)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);


			buffer+=sprintf(buffer, "Tri_Tex               %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       S: %s, T: %s, W: %s\n", s, t, w);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDX: %s, DTDX: %s, DWDX: %s\n", dsdx, dtdx, dwdx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDE: %s, DTDE: %s, DWDE: %s\n", dsde, dtde, dwde);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDY: %s, DTDY: %s, DWDY: %s\n", dsdy, dtdy, dwdy);
			break;
		}
		case 0x0c:		
		{
			int lft = (command >> 23) & 0x1;

			if (length != rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_Shade length = %d\n", length);
				return 0;
			}

			for (i=2; i < 24; i++)
			{
				cmd[i] = rdp_cmd_data[i];
			}

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)((int32_t)cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)((int32_t)cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)((int32_t)cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)((int32_t)cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)((int32_t)cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)((int32_t)cmd[7] / 65536.0f));
			sprintf(rt,		"%4.4f", (float)(int32_t)((cmd[8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(gt,		"%4.4f", (float)(int32_t)(((cmd[8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(bt,		"%4.4f", (float)(int32_t)((cmd[9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,		"%4.4f", (float)(int32_t)(((cmd[9] & 0xffff) << 16) | (cmd[13] & 0xffff)) / 65536.0f);
			sprintf(drdx,	"%4.4f", (float)(int32_t)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdx,	"%4.4f", (float)(int32_t)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dbdx,	"%4.4f", (float)(int32_t)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,	"%4.4f", (float)(int32_t)(((cmd[11] & 0xffff) << 16) | (cmd[15] & 0xffff)) / 65536.0f);
			sprintf(drde,	"%4.4f", (float)(int32_t)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgde,	"%4.4f", (float)(int32_t)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dbde,	"%4.4f", (float)(int32_t)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,	"%4.4f", (float)(int32_t)(((cmd[17] & 0xffff) << 16) | (cmd[21] & 0xffff)) / 65536.0f);
			sprintf(drdy,	"%4.4f", (float)(int32_t)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdy,	"%4.4f", (float)(int32_t)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dbdy,	"%4.4f", (float)(int32_t)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,	"%4.4f", (float)(int32_t)(((cmd[19] & 0xffff) << 16) | (cmd[23] & 0xffff)) / 65536.0f);

			buffer+=sprintf(buffer, "Tri_Shade              %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       R: %s, G: %s, B: %s, A: %s\n", rt, gt, bt, at);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDX: %s, DGDX: %s, DBDX: %s, DADX: %s\n", drdx, dgdx, dbdx, dadx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDE: %s, DGDE: %s, DBDE: %s, DADE: %s\n", drde, dgde, dbde, dade);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDY: %s, DGDY: %s, DBDY: %s, DADY: %s\n", drdy, dgdy, dbdy, dady);
			break;
		}
		case 0x0e:		
		{
			int lft = (command >> 23) & 0x1;

			if (length < rdp_command_length[command])
			{
				sprintf(buffer, "ERROR: Tri_TexShade length = %d\n", length);
				return 0;
			}

			for (i=2; i < 40; i++)
			{
				cmd[i] = rdp_cmd_data[rdp_cmd_cur+i];
			}

			sprintf(yl,		"%4.4f", (float)((cmd[0] >>  0) & 0x1fff) / 4.0f);
			sprintf(ym,		"%4.4f", (float)((cmd[1] >> 16) & 0x1fff) / 4.0f);
			sprintf(yh,		"%4.4f", (float)((cmd[1] >>  0) & 0x1fff) / 4.0f);
			sprintf(xl,		"%4.4f", (float)((int32_t)cmd[2] / 65536.0f));
			sprintf(dxldy,	"%4.4f", (float)((int32_t)cmd[3] / 65536.0f));
			sprintf(xh,		"%4.4f", (float)((int32_t)cmd[4] / 65536.0f));
			sprintf(dxhdy,	"%4.4f", (float)((int32_t)cmd[5] / 65536.0f));
			sprintf(xm,		"%4.4f", (float)((int32_t)cmd[6] / 65536.0f));
			sprintf(dxmdy,	"%4.4f", (float)((int32_t)cmd[7] / 65536.0f));
			sprintf(rt,		"%4.4f", (float)(int32_t)((cmd[8] & 0xffff0000) | ((cmd[12] >> 16) & 0xffff)) / 65536.0f);
			sprintf(gt,		"%4.4f", (float)(int32_t)(((cmd[8] & 0xffff) << 16) | (cmd[12] & 0xffff)) / 65536.0f);
			sprintf(bt,		"%4.4f", (float)(int32_t)((cmd[9] & 0xffff0000) | ((cmd[13] >> 16) & 0xffff)) / 65536.0f);
			sprintf(at,		"%4.4f", (float)(int32_t)(((cmd[9] & 0xffff) << 16) | (cmd[13] & 0xffff)) / 65536.0f);
			sprintf(drdx,	"%4.4f", (float)(int32_t)((cmd[10] & 0xffff0000) | ((cmd[14] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdx,	"%4.4f", (float)(int32_t)(((cmd[10] & 0xffff) << 16) | (cmd[14] & 0xffff)) / 65536.0f);
			sprintf(dbdx,	"%4.4f", (float)(int32_t)((cmd[11] & 0xffff0000) | ((cmd[15] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dadx,	"%4.4f", (float)(int32_t)(((cmd[11] & 0xffff) << 16) | (cmd[15] & 0xffff)) / 65536.0f);
			sprintf(drde,	"%4.4f", (float)(int32_t)((cmd[16] & 0xffff0000) | ((cmd[20] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgde,	"%4.4f", (float)(int32_t)(((cmd[16] & 0xffff) << 16) | (cmd[20] & 0xffff)) / 65536.0f);
			sprintf(dbde,	"%4.4f", (float)(int32_t)((cmd[17] & 0xffff0000) | ((cmd[21] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dade,	"%4.4f", (float)(int32_t)(((cmd[17] & 0xffff) << 16) | (cmd[21] & 0xffff)) / 65536.0f);
			sprintf(drdy,	"%4.4f", (float)(int32_t)((cmd[18] & 0xffff0000) | ((cmd[22] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dgdy,	"%4.4f", (float)(int32_t)(((cmd[18] & 0xffff) << 16) | (cmd[22] & 0xffff)) / 65536.0f);
			sprintf(dbdy,	"%4.4f", (float)(int32_t)((cmd[19] & 0xffff0000) | ((cmd[23] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dady,	"%4.4f", (float)(int32_t)(((cmd[19] & 0xffff) << 16) | (cmd[23] & 0xffff)) / 65536.0f);

			sprintf(s,		"%4.4f", (float)(int32_t)((cmd[24] & 0xffff0000) | ((cmd[28] >> 16) & 0xffff)) / 65536.0f);
			sprintf(t,		"%4.4f", (float)(int32_t)(((cmd[24] & 0xffff) << 16) | (cmd[28] & 0xffff)) / 65536.0f);
			sprintf(w,		"%4.4f", (float)(int32_t)((cmd[25] & 0xffff0000) | ((cmd[29] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdx,	"%4.4f", (float)(int32_t)((cmd[26] & 0xffff0000) | ((cmd[30] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdx,	"%4.4f", (float)(int32_t)(((cmd[26] & 0xffff) << 16) | (cmd[30] & 0xffff)) / 65536.0f);
			sprintf(dwdx,	"%4.4f", (float)(int32_t)((cmd[27] & 0xffff0000) | ((cmd[31] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsde,	"%4.4f", (float)(int32_t)((cmd[32] & 0xffff0000) | ((cmd[36] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtde,	"%4.4f", (float)(int32_t)(((cmd[32] & 0xffff) << 16) | (cmd[36] & 0xffff)) / 65536.0f);
			sprintf(dwde,	"%4.4f", (float)(int32_t)((cmd[33] & 0xffff0000) | ((cmd[37] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dsdy,	"%4.4f", (float)(int32_t)((cmd[34] & 0xffff0000) | ((cmd[38] >> 16) & 0xffff)) / 65536.0f);
			sprintf(dtdy,	"%4.4f", (float)(int32_t)(((cmd[34] & 0xffff) << 16) | (cmd[38] & 0xffff)) / 65536.0f);
			sprintf(dwdy,	"%4.4f", (float)(int32_t)((cmd[35] & 0xffff0000) | ((cmd[39] >> 16) & 0xffff)) / 65536.0f);


			buffer+=sprintf(buffer, "Tri_TexShade           %d, XL: %s, XM: %s, XH: %s, YL: %s, YM: %s, YH: %s\n", lft, xl,xm,xh,yl,ym,yh);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       R: %s, G: %s, B: %s, A: %s\n", rt, gt, bt, at);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDX: %s, DGDX: %s, DBDX: %s, DADX: %s\n", drdx, dgdx, dbdx, dadx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDE: %s, DGDE: %s, DBDE: %s, DADE: %s\n", drde, dgde, dbde, dade);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DRDY: %s, DGDY: %s, DBDY: %s, DADY: %s\n", drdy, dgdy, dbdy, dady);

			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       S: %s, T: %s, W: %s\n", s, t, w);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDX: %s, DTDX: %s, DWDX: %s\n", dsdx, dtdx, dwdx);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDE: %s, DTDE: %s, DWDE: %s\n", dsde, dtde, dwde);
			buffer+=sprintf(buffer, "                              ");
			buffer+=sprintf(buffer, "                       DSDY: %s, DTDY: %s, DWDY: %s\n", dsdy, dtdy, dwdy);
			break;
		}
#endif
		case 0x24: 
		case 0x25:
		{
			if (length < 16)
			{
				sprintf(buffer, "ERROR: Texture_Rectangle length = %d\n", length);
				return 0;
			}
			cmd[2] = rdp_cmd_data[rdp_cmd_cur+2];
			cmd[3] = rdp_cmd_data[rdp_cmd_cur+3];
			sprintf(s,    "%4.4f", (float)(int16_t)((cmd[2] >> 16) & 0xffff) / 32.0f);
			sprintf(t,    "%4.4f", (float)(int16_t)((cmd[2] >>  0) & 0xffff) / 32.0f);
			sprintf(dsdx, "%4.4f", (float)(int16_t)((cmd[3] >> 16) & 0xffff) / 1024.0f);
			sprintf(dtdy, "%4.4f", (float)(int16_t)((cmd[3] >> 16) & 0xffff) / 1024.0f);

			if (command == 0x24)
					sprintf(buffer, "Texture_Rectangle      %d, %s, %s, %s, %s,  %s, %s, %s, %s", tile, sh, th, sl, tl, s, t, dsdx, dtdy);
			else
					sprintf(buffer, "Texture_Rectangle_Flip %d, %s, %s, %s, %s,  %s, %s, %s, %s", tile, sh, th, sl, tl, s, t, dsdx, dtdy);

			break;
		}
		case 0x26:	sprintf(buffer, "Sync_Load"); break;
		case 0x27:	sprintf(buffer, "Sync_Pipe"); break;
		case 0x28:	sprintf(buffer, "Sync_Tile"); break;
		case 0x29:	sprintf(buffer, "Sync_Full"); break;
		case 0x2a:  sprintf(buffer, "Set_Key_GB"); break;
		case 0x2b:	sprintf(buffer, "Set_Key_R"); break;
		case 0x2c:	sprintf(buffer, "Set_Convert"); break;
		case 0x2d:	sprintf(buffer, "Set_Scissor            %s, %s, %s, %s", sl, tl, sh, th); break;
		case 0x2e:	sprintf(buffer, "Set_Prim_Depth         %04X, %04X", (cmd[1] >> 16) & 0xffff, cmd[1] & 0xffff); break;
		case 0x2f:	sprintf(buffer, "Set_Other_Modes        %08X %08X", cmd[0], cmd[1]); break;
		case 0x30:	sprintf(buffer, "Load_TLUT              %d, %s, %s, %s, %s", tile, sl, tl, sh, th); break;
		case 0x32:	sprintf(buffer, "Set_Tile_Size          %d, %s, %s, %s, %s", tile, sl, tl, sh, th); break;
		case 0x33:	sprintf(buffer, "Load_Block             %d, %03X, %03X, %03X, %03X", tile, (cmd[0] >> 12) & 0xfff, cmd[0] & 0xfff, (cmd[1] >> 12) & 0xfff, cmd[1] & 0xfff); break;
		case 0x34:	sprintf(buffer, "Load_Tile              %d, %s, %s, %s, %s", tile, sl, tl, sh, th); break;
		case 0x35:	sprintf(buffer, "Set_Tile               %d, %s, %s, %d, %04X", tile, format, size, ((cmd[0] >> 9) & 0x1ff) * 8, (cmd[0] & 0x1ff) * 8); break;
		case 0x36:	sprintf(buffer, "Fill_Rectangle         %s, %s, %s, %s", sh, th, sl, tl); break;
		case 0x37:	sprintf(buffer, "Set_Fill_Color         R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x38:	sprintf(buffer, "Set_Fog_Color          R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x39:	sprintf(buffer, "Set_Blend_Color        R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x3a:	sprintf(buffer, "Set_Prim_Color         %d, %d, R: %d, G: %d, B: %d, A: %d", (cmd[0] >> 8) & 0x1f, cmd[0] & 0xff, r, g, b, a); break;
		case 0x3b:	sprintf(buffer, "Set_Env_Color          R: %d, G: %d, B: %d, A: %d", r, g, b, a); break;
		case 0x3c:	sprintf(buffer, "Set_Combine            %08X %08X", cmd[0], cmd[1]); break;
		case 0x3d:	sprintf(buffer, "Set_Texture_Image      %s, %s, %d, %08X", format, size, (cmd[0] & 0x1ff)+1, cmd[1]); break;
		case 0x3e:	sprintf(buffer, "Set_Mask_Image         %08X", cmd[1]); break;
		case 0x3f:	sprintf(buffer, "Set_Color_Image        %s, %s, %d, %08X", format, size, (cmd[0] & 0x1ff)+1, cmd[1]); break;
		default:	sprintf(buffer, "Unknown command 0x%06X (%08X %08X)", command, cmd[0], cmd[1]); break;
	}

	return rdp_command_length[command];
}







static void rdp_invalid(uint32_t w1, uint32_t w2)
{
}

static void rdp_noop(uint32_t w1, uint32_t w2)
{
}

static void rdp_tri_noshade(uint32_t w1, uint32_t w2)
{
	int32_t ewdata[44];
	memcpy(&ewdata[0], &rdp_cmd_data[rdp_cmd_cur], 8 * sizeof(int32_t));
	memset(&ewdata[8], 0, 36 * sizeof(int32_t));
	edgewalker_for_prims(ewdata);
}

static void rdp_tri_noshade_z(uint32_t w1, uint32_t w2)
{
	int32_t ewdata[44];
	memcpy(&ewdata[0], &rdp_cmd_data[rdp_cmd_cur], 8 * sizeof(int32_t));
	memset(&ewdata[8], 0, 32 * sizeof(int32_t));
	memcpy(&ewdata[40], &rdp_cmd_data[rdp_cmd_cur + 8], 4 * sizeof(int32_t));
	edgewalker_for_prims(ewdata);
}

static void rdp_tri_tex(uint32_t w1, uint32_t w2)
{
	int32_t ewdata[44];
	memcpy(&ewdata[0], &rdp_cmd_data[rdp_cmd_cur], 8 * sizeof(int32_t));
	memset(&ewdata[8], 0, 16 * sizeof(int32_t));
	memcpy(&ewdata[24], &rdp_cmd_data[rdp_cmd_cur + 8], 16 * sizeof(int32_t));
	memset(&ewdata[40], 0, 4 * sizeof(int32_t));
	edgewalker_for_prims(ewdata);
}

static void rdp_tri_tex_z(uint32_t w1, uint32_t w2)
{
	int32_t ewdata[44];
	memcpy(&ewdata[0], &rdp_cmd_data[rdp_cmd_cur], 8 * sizeof(int32_t));
	memset(&ewdata[8], 0, 16 * sizeof(int32_t));
	memcpy(&ewdata[24], &rdp_cmd_data[rdp_cmd_cur + 8], 16 * sizeof(int32_t));
	memcpy(&ewdata[40], &rdp_cmd_data[rdp_cmd_cur + 24], 4 * sizeof(int32_t));
	edgewalker_for_prims(ewdata);
}

static void rdp_tri_shade(uint32_t w1, uint32_t w2)
{
	int32_t ewdata[44];
	memcpy(&ewdata[0], &rdp_cmd_data[rdp_cmd_cur], 24 * sizeof(int32_t));
	memset(&ewdata[24], 0, 20 * sizeof(int32_t));
	edgewalker_for_prims(ewdata);
}

static void rdp_tri_shade_z(uint32_t w1, uint32_t w2)
{
	int32_t ewdata[44];
	memcpy(&ewdata[0], &rdp_cmd_data[rdp_cmd_cur], 24 * sizeof(int32_t));
	memset(&ewdata[24], 0, 16 * sizeof(int32_t));
	memcpy(&ewdata[40], &rdp_cmd_data[rdp_cmd_cur + 24], 4 * sizeof(int32_t));
	edgewalker_for_prims(ewdata);
}

static void rdp_tri_texshade(uint32_t w1, uint32_t w2)
{
	int32_t ewdata[44];
	memcpy(&ewdata[0], &rdp_cmd_data[rdp_cmd_cur], 40 * sizeof(int32_t));
	memset(&ewdata[40], 0, 4 * sizeof(int32_t));
	edgewalker_for_prims(ewdata);
}

static void rdp_tri_texshade_z(uint32_t w1, uint32_t w2)
{
	int32_t ewdata[44];
	memcpy(&ewdata[0], &rdp_cmd_data[rdp_cmd_cur], 44 * sizeof(int32_t));
	edgewalker_for_prims(ewdata);
}

static void rdp_tex_rect(uint32_t w1, uint32_t w2)
{
	uint32_t w3 = rdp_cmd_data[rdp_cmd_cur + 2];
	uint32_t w4 = rdp_cmd_data[rdp_cmd_cur + 3];

	
	uint32_t tilenum	= (w2 >> 24) & 0x7;
	uint32_t xl = (w1 >> 12) & 0xfff;
	uint32_t yl	= (w1 >>  0) & 0xfff;
	uint32_t xh	= (w2 >> 12) & 0xfff;
	uint32_t yh	= (w2 >>  0) & 0xfff;
	
	int32_t s = (w3 >> 16) & 0xffff;
	int32_t t = (w3 >>  0) & 0xffff;
	int32_t dsdx = (w4 >> 16) & 0xffff;
	int32_t dtdy = (w4 >>  0) & 0xffff;
  uint32_t xlint, xhint;

	dsdx = SIGN16(dsdx);
	dtdy = SIGN16(dtdy);
	
	if (other_modes.cycle_type == CYCLE_TYPE_FILL || other_modes.cycle_type == CYCLE_TYPE_COPY)
		yl |= 3;

	xlint = (xl >> 2) & 0x3ff;
	xhint = (xh >> 2) & 0x3ff;

	int32_t ewdata[44];
	ewdata[0] = (0x24 << 24) | ((0x80 | tilenum) << 16) | yl;
	ewdata[1] = (yl << 16) | yh;
	ewdata[2] = (xlint << 16) | ((xl & 3) << 14);
	ewdata[3] = 0;
	ewdata[4] = (xhint << 16) | ((xh & 3) << 14);
	ewdata[5] = 0;
	ewdata[6] = (xlint << 16) | ((xl & 3) << 14);
	ewdata[7] = 0;
	memset(&ewdata[8], 0, 16 * sizeof(uint32_t));
	ewdata[24] = (s << 16) | t;
	ewdata[25] = 0;
	ewdata[26] = ((dsdx >> 5) << 16);
	ewdata[27] = 0;
	ewdata[28] = 0;
	ewdata[29] = 0;
	ewdata[30] = ((dsdx & 0x1f) << 11) << 16;
	ewdata[31] = 0;
	ewdata[32] = (dtdy >> 5) & 0xffff;
	ewdata[33] = 0;
	ewdata[34] = (dtdy >> 5) & 0xffff;
	ewdata[35] = 0;
	ewdata[36] = (dtdy & 0x1f) << 11;
	ewdata[37] = 0;
	ewdata[38] = (dtdy & 0x1f) << 11;
	ewdata[39] = 0;
	memset(&ewdata[40], 0, 4 * sizeof(int32_t));

	

	edgewalker_for_prims(ewdata);
}

static void rdp_tex_rect_flip(uint32_t w1, uint32_t w2)
{
	uint32_t w3 = rdp_cmd_data[rdp_cmd_cur+2];
	uint32_t w4 = rdp_cmd_data[rdp_cmd_cur+3];
	
	
	uint32_t tilenum	= (w2 >> 24) & 0x7;
	uint32_t xl = (w1 >> 12) & 0xfff;
	uint32_t yl	= (w1 >>  0) & 0xfff;
	uint32_t xh	= (w2 >> 12) & 0xfff;
	uint32_t yh	= (w2 >>  0) & 0xfff;
	
	int32_t s = (w3 >> 16) & 0xffff;
	int32_t t = (w3 >>  0) & 0xffff;
	int32_t dsdx = (w4 >> 16) & 0xffff;
	int32_t dtdy = (w4 >>  0) & 0xffff;
  uint32_t xlint, xhint;

	dsdx = SIGN16(dsdx);
	dtdy = SIGN16(dtdy);

	if (other_modes.cycle_type == CYCLE_TYPE_FILL || other_modes.cycle_type == CYCLE_TYPE_COPY)
		yl |= 3;

	xlint = (xl >> 2) & 0x3ff;
	xhint = (xh >> 2) & 0x3ff;

	int32_t ewdata[44];
	ewdata[0] = (0x25 << 24) | ((0x80 | tilenum) << 16) | yl;
	ewdata[1] = (yl << 16) | yh;
	ewdata[2] = (xlint << 16) | ((xl & 3) << 14);
	ewdata[3] = 0;
	ewdata[4] = (xhint << 16) | ((xh & 3) << 14);
	ewdata[5] = 0;
	ewdata[6] = (xlint << 16) | ((xl & 3) << 14);
	ewdata[7] = 0;
	memset(&ewdata[8], 0, 16 * sizeof(int32_t));
	ewdata[24] = (s << 16) | t;
	ewdata[25] = 0;
	
	ewdata[26] = (dtdy >> 5) & 0xffff;
	ewdata[27] = 0;
	ewdata[28] = 0;
	ewdata[29] = 0;
	ewdata[30] = ((dtdy & 0x1f) << 11);
	ewdata[31] = 0;
	ewdata[32] = (dsdx >> 5) << 16;
	ewdata[33] = 0;
	ewdata[34] = (dsdx >> 5) << 16;
	ewdata[35] = 0;
	ewdata[36] = (dsdx & 0x1f) << 27;
	ewdata[37] = 0;
	ewdata[38] = (dsdx & 0x1f) << 27;
	ewdata[39] = 0;
	memset(&ewdata[40], 0, 4 * sizeof(int32_t));

	edgewalker_for_prims(ewdata);
}

static void rdp_sync_load(uint32_t w1, uint32_t w2)
{
	
}

static void rdp_sync_pipe(uint32_t w1, uint32_t w2)
{
	
	
}

static void rdp_sync_tile(uint32_t w1, uint32_t w2)
{
	
}

static void rdp_sync_full(uint32_t w1, uint32_t w2)
{
	
	

	
	
	
	
	
	
	
	
	
	

	
	
	
	
	
	
	
	
	

	z64gl_command = 0;

  signal_rcp_interrupt(cen64->bus.vr4300, MI_INTR_DP);
}

static void rdp_set_key_gb(uint32_t w1, uint32_t w2)
{
	key_width[1] = (w1 >> 12) & 0xfff;
	key_width[2] = w1 & 0xfff;
	key_center[1] = (w2 >> 24) & 0xff;
	key_scale[1] = (w2 >> 16) & 0xff;
	key_center[2] = (w2 >> 8) & 0xff;
	key_scale[2] = w2 & 0xff;
}

static void rdp_set_key_r(uint32_t w1, uint32_t w2)
{
	key_width[0] = (w2 >> 16) & 0xfff;
	key_center[0] = (w2 >> 8) & 0xff;
	key_scale[0] = w2 & 0xff;
}

static void rdp_set_convert(uint32_t w1, uint32_t w2)
{
	int32_t k0 = (w1 >> 13) & 0x1ff;
	int32_t k1 = (w1 >> 4) & 0x1ff;
	int32_t k2 = ((w1 & 0xf) << 5) | ((w2 >> 27) & 0x1f);
	int32_t k3 = (w2 >> 18) & 0x1ff;
	k0_tf = (SIGN(k0, 9) << 1) + 1;
	k1_tf = (SIGN(k1, 9) << 1) + 1;
	k2_tf = (SIGN(k2, 9) << 1) + 1;
	k3_tf = (SIGN(k3, 9) << 1) + 1;
	k4 = (w2 >> 9) & 0x1ff;
	k5 = w2 & 0x1ff;
}

static void rdp_set_scissor(uint32_t w1, uint32_t w2)
{
	clip.xh = (w1 >> 12) & 0xfff;
	clip.yh = (w1 >>  0) & 0xfff;
	clip.xl = (w2 >> 12) & 0xfff;
	clip.yl = (w2 >>  0) & 0xfff;
	
	scfield = (w2 >> 25) & 1;
	sckeepodd = (w2 >> 24) & 1;
}

static void rdp_set_prim_depth(uint32_t w1, uint32_t w2)
{
	primitive_z = w2 & (0x7fff << 16);
	

	primitive_delta_z = (uint16_t)(w2);
}

static void rdp_set_other_modes(uint32_t w1, uint32_t w2)
{
	other_modes.cycle_type			= (w1 >> 20) & 0x3;
	other_modes.persp_tex_en 		= (w1 & 0x80000) ? 1 : 0;
	other_modes.detail_tex_en		= (w1 & 0x40000) ? 1 : 0;
	other_modes.sharpen_tex_en		= (w1 & 0x20000) ? 1 : 0;
	other_modes.tex_lod_en			= (w1 & 0x10000) ? 1 : 0;
	other_modes.en_tlut				= (w1 & 0x08000) ? 1 : 0;
	other_modes.tlut_type			= (w1 & 0x04000) ? 1 : 0;
	other_modes.sample_type			= (w1 & 0x02000) ? 1 : 0;
	other_modes.mid_texel			= (w1 & 0x01000) ? 1 : 0;
	other_modes.bi_lerp0			= (w1 & 0x00800) ? 1 : 0;
	other_modes.bi_lerp1			= (w1 & 0x00400) ? 1 : 0;
	other_modes.convert_one			= (w1 & 0x00200) ? 1 : 0;
	other_modes.key_en				= (w1 & 0x00100) ? 1 : 0;
	other_modes.rgb_dither_sel		= (w1 >> 6) & 0x3;
	other_modes.alpha_dither_sel	= (w1 >> 4) & 0x3;
	other_modes.blend_m1a_0			= (w2 >> 30) & 0x3;
	other_modes.blend_m1a_1			= (w2 >> 28) & 0x3;
	other_modes.blend_m1b_0			= (w2 >> 26) & 0x3;
	other_modes.blend_m1b_1			= (w2 >> 24) & 0x3;
	other_modes.blend_m2a_0			= (w2 >> 22) & 0x3;
	other_modes.blend_m2a_1			= (w2 >> 20) & 0x3;
	other_modes.blend_m2b_0			= (w2 >> 18) & 0x3;
	other_modes.blend_m2b_1			= (w2 >> 16) & 0x3;
	other_modes.force_blend			= (w2 >> 14) & 1;
	other_modes.alpha_cvg_select	= (w2 >> 13) & 1;
	other_modes.cvg_times_alpha		= (w2 >> 12) & 1;
	other_modes.z_mode				= (w2 >> 10) & 0x3;
	other_modes.cvg_dest			= (w2 >> 8) & 0x3;
	other_modes.color_on_cvg		= (w2 >> 7) & 1;
	other_modes.image_read_en		= (w2 >> 6) & 1;
	other_modes.z_update_en			= (w2 >> 5) & 1;
	other_modes.z_compare_en		= (w2 >> 4) & 1;
	other_modes.antialias_en		= (w2 >> 3) & 1;
	other_modes.z_source_sel		= (w2 >> 2) & 1;
	other_modes.dither_alpha_en		= (w2 >> 1) & 1;
	other_modes.alpha_compare_en	= (w2) & 1;

	SET_BLENDER_INPUT(0, 0, &blender1a_r[0], &blender1a_g[0], &blender1a_b[0], &blender1b_a[0],
					  other_modes.blend_m1a_0, other_modes.blend_m1b_0);
	SET_BLENDER_INPUT(0, 1, &blender2a_r[0], &blender2a_g[0], &blender2a_b[0], &blender2b_a[0],
					  other_modes.blend_m2a_0, other_modes.blend_m2b_0);
	SET_BLENDER_INPUT(1, 0, &blender1a_r[1], &blender1a_g[1], &blender1a_b[1], &blender1b_a[1],
					  other_modes.blend_m1a_1, other_modes.blend_m1b_1);
	SET_BLENDER_INPUT(1, 1, &blender2a_r[1], &blender2a_g[1], &blender2a_b[1], &blender2b_a[1],
					  other_modes.blend_m2a_1, other_modes.blend_m2b_1);

	other_modes.f.stalederivs = 1;
}

void deduce_derivatives()
{
	
	other_modes.f.partialreject_1cycle = (blender2b_a[0] == &inv_pixel_color[3] && blender1b_a[0] == &pixel_color[3]);
	other_modes.f.partialreject_2cycle = (blender2b_a[1] == &inv_pixel_color[3] && blender1b_a[1] == &pixel_color[3]);


	other_modes.f.special_bsel0 = (blender2b_a[0] == &memory_color[3]);
	other_modes.f.special_bsel1 = (blender2b_a[1] == &memory_color[3]);


	other_modes.f.realblendershiftersneeded = (other_modes.f.special_bsel0 && other_modes.cycle_type == CYCLE_TYPE_1) || (other_modes.f.special_bsel1 && other_modes.cycle_type == CYCLE_TYPE_2);
	other_modes.f.interpixelblendershiftersneeded = (other_modes.f.special_bsel0 && other_modes.cycle_type == CYCLE_TYPE_2);


	other_modes.f.rgb_alpha_dither = (other_modes.rgb_dither_sel << 2) | other_modes.alpha_dither_sel;

	if (other_modes.rgb_dither_sel == 3)
		rgb_dither_ptr = rgb_dither_func[1];
	else
		rgb_dither_ptr = rgb_dither_func[0];

	tcdiv_ptr = tcdiv_func[other_modes.persp_tex_en];

	
	int texel1_used_in_cc1 = 0, texel0_used_in_cc1 = 0, texel0_used_in_cc0 = 0, texel1_used_in_cc0 = 0;
	int texels_in_cc0 = 0, texels_in_cc1 = 0;
	int lod_frac_used_in_cc1 = 0, lod_frac_used_in_cc0 = 0;

	if ((combiner_rgbmul_r[1] == &lod_frac) || (combiner_alphamul[1] == &lod_frac))
		lod_frac_used_in_cc1 = 1;
	if ((combiner_rgbmul_r[0] == &lod_frac) || (combiner_alphamul[0] == &lod_frac))
		lod_frac_used_in_cc0 = 1;

	if (combiner_rgbmul_r[1] == &texel1_color[0] || combiner_rgbsub_a_r[1] == &texel1_color[0] || combiner_rgbsub_b_r[1] == &texel1_color[0] || combiner_rgbadd_r[1] == &texel1_color[0] || \
		combiner_alphamul[1] == &texel1_color[3] || combiner_alphasub_a[1] == &texel1_color[3] || combiner_alphasub_b[1] == &texel1_color[3] || combiner_alphaadd[1] == &texel1_color[3] || \
		combiner_rgbmul_r[1] == &texel1_color[3])
		texel1_used_in_cc1 = 1;
	if (combiner_rgbmul_r[1] == &texel0_color[0] || combiner_rgbsub_a_r[1] == &texel0_color[0] || combiner_rgbsub_b_r[1] == &texel0_color[0] || combiner_rgbadd_r[1] == &texel0_color[0] || \
		combiner_alphamul[1] == &texel0_color[3] || combiner_alphasub_a[1] == &texel0_color[3] || combiner_alphasub_b[1] == &texel0_color[3] || combiner_alphaadd[1] == &texel0_color[3] || \
		combiner_rgbmul_r[1] == &texel0_color[3])
		texel0_used_in_cc1 = 1;
	if (combiner_rgbmul_r[0] == &texel1_color[0] || combiner_rgbsub_a_r[0] == &texel1_color[0] || combiner_rgbsub_b_r[0] == &texel1_color[0] || combiner_rgbadd_r[0] == &texel1_color[0] || \
		combiner_alphamul[0] == &texel1_color[3] || combiner_alphasub_a[0] == &texel1_color[3] || combiner_alphasub_b[0] == &texel1_color[3] || combiner_alphaadd[0] == &texel1_color[3] || \
		combiner_rgbmul_r[0] == &texel1_color[3])
		texel1_used_in_cc0 = 1;
	if (combiner_rgbmul_r[0] == &texel0_color[0] || combiner_rgbsub_a_r[0] == &texel0_color[0] || combiner_rgbsub_b_r[0] == &texel0_color[0] || combiner_rgbadd_r[0] == &texel0_color[0] || \
		combiner_alphamul[0] == &texel0_color[3] || combiner_alphasub_a[0] == &texel0_color[3] || combiner_alphasub_b[0] == &texel0_color[3] || combiner_alphaadd[0] == &texel0_color[3] || \
		combiner_rgbmul_r[0] == &texel0_color[3])
		texel0_used_in_cc0 = 1;
	texels_in_cc0 = texel0_used_in_cc0 || texel1_used_in_cc0;
	texels_in_cc1 = texel0_used_in_cc1 || texel1_used_in_cc1;	

	
	if (texel1_used_in_cc1)
		render_spans_1cycle_ptr = render_spans_1cycle_func[2];
	else if (texel0_used_in_cc1 || lod_frac_used_in_cc1)
		render_spans_1cycle_ptr = render_spans_1cycle_func[1];
	else
		render_spans_1cycle_ptr = render_spans_1cycle_func[0];

	if (texel1_used_in_cc1)
		render_spans_2cycle_ptr = render_spans_2cycle_func[3];
	else if (texel1_used_in_cc0 || texel0_used_in_cc1)
		render_spans_2cycle_ptr = render_spans_2cycle_func[2];
	else if (texel0_used_in_cc0 || lod_frac_used_in_cc0 || lod_frac_used_in_cc1)
		render_spans_2cycle_ptr = render_spans_2cycle_func[1];
	else
		render_spans_2cycle_ptr = render_spans_2cycle_func[0];

	
	int lodfracused = 0;

	if ((other_modes.cycle_type == CYCLE_TYPE_2 && (lod_frac_used_in_cc0 || lod_frac_used_in_cc1)) || \
		(other_modes.cycle_type == CYCLE_TYPE_1 && lod_frac_used_in_cc1))
		lodfracused = 1;

	if ((other_modes.cycle_type == CYCLE_TYPE_1 && combiner_rgbsub_a_r[1] == &noise) || \
		(other_modes.cycle_type == CYCLE_TYPE_2 && (combiner_rgbsub_a_r[0] == &noise || combiner_rgbsub_a_r[1] == &noise)) || \
		other_modes.alpha_dither_sel == 2)
		get_dither_noise_ptr = get_dither_noise_func[0];
	else if (other_modes.f.rgb_alpha_dither != 0xf)
		get_dither_noise_ptr = get_dither_noise_func[1];
	else
		get_dither_noise_ptr = get_dither_noise_func[2];

	other_modes.f.dolod = other_modes.tex_lod_en || lodfracused;
}

static inline int32_t irand()
{
	iseed *= 0x343fd;
	iseed += 0x269ec3;
	return ((iseed >> 16) & 0x7fff);
}

static void rdp_set_tile_size(uint32_t w1, uint32_t w2)
{
	int tilenum = (w2 >> 24) & 0x7;
	tile[tilenum].sl = (w1 >> 12) & 0xfff;
	tile[tilenum].tl = (w1 >>  0) & 0xfff;
	tile[tilenum].sh = (w2 >> 12) & 0xfff;
	tile[tilenum].th = (w2 >>  0) & 0xfff;

	calculate_clamp_diffs(tilenum);
}
	
static void rdp_load_block(uint32_t w1, uint32_t w2)
{
	int tilenum = (w2 >> 24) & 0x7;
	int sl, sh, tl, dxt;
						
	
	tile[tilenum].sl = sl = ((w1 >> 12) & 0xfff);
	tile[tilenum].tl = tl = ((w1 >>  0) & 0xfff);
	tile[tilenum].sh = sh = ((w2 >> 12) & 0xfff);
	tile[tilenum].th = dxt	= ((w2 >>  0) & 0xfff);

	calculate_clamp_diffs(tilenum);

	int tlclamped = tl & 0x3ff;

	int32_t lewdata[10];
	
	lewdata[0] = (w1 & 0xff000000) | (0x10 << 19) | (tilenum << 16) | ((tlclamped << 2) | 3);
	lewdata[1] = (((tlclamped << 2) | 3) << 16) | (tlclamped << 2);
	lewdata[2] = sh << 16;
	lewdata[3] = sl << 16;
	lewdata[4] = sh << 16;
	lewdata[5] = ((sl << 3) << 16) | (tl << 3);
	lewdata[6] = (dxt & 0xff) << 8;
	lewdata[7] = ((0x80 >> ti_size) << 16) | (dxt >> 8);
	lewdata[8] = 0x20;
	lewdata[9] = 0x20;

	edgewalker_for_loads(lewdata);

}

static void rdp_load_tlut(uint32_t w1, uint32_t w2)
{
	

	tile_tlut_common_cs_decoder(w1, w2);
}

static void rdp_load_tile(uint32_t w1, uint32_t w2)
{
	tile_tlut_common_cs_decoder(w1, w2);
}

void tile_tlut_common_cs_decoder(uint32_t w1, uint32_t w2)
{
	int tilenum = (w2 >> 24) & 0x7;
	int sl, tl, sh, th;

	
	tile[tilenum].sl = sl = ((w1 >> 12) & 0xfff);
	tile[tilenum].tl = tl = ((w1 >>  0) & 0xfff);
	tile[tilenum].sh = sh = ((w2 >> 12) & 0xfff);
	tile[tilenum].th = th = ((w2 >>  0) & 0xfff);

	calculate_clamp_diffs(tilenum);

	
	int32_t lewdata[10];

	lewdata[0] = (w1 & 0xff000000) | (0x10 << 19) | (tilenum << 16) | (th | 3);
	lewdata[1] = ((th | 3) << 16) | (tl);
	lewdata[2] = ((sh >> 2) << 16) | ((sh & 3) << 14);
	lewdata[3] = ((sl >> 2) << 16) | ((sl & 3) << 14);
	lewdata[4] = ((sh >> 2) << 16) | ((sh & 3) << 14);
	lewdata[5] = ((sl << 3) << 16) | (tl << 3);
	lewdata[6] = 0;
	lewdata[7] = (0x200 >> ti_size) << 16;
	lewdata[8] = 0x20;
	lewdata[9] = 0x20;

	edgewalker_for_loads(lewdata);
}

static void rdp_set_tile(uint32_t w1, uint32_t w2)
{
	int tilenum = (w2 >> 24) & 0x7;
	
	tile[tilenum].format	= (w1 >> 21) & 0x7;
	tile[tilenum].size		= (w1 >> 19) & 0x3;
	tile[tilenum].line		= (w1 >>  9) & 0x1ff;
	tile[tilenum].tmem		= (w1 >>  0) & 0x1ff;
	tile[tilenum].palette	= (w2 >> 20) & 0xf;
	tile[tilenum].ct		= (w2 >> 19) & 0x1;
	tile[tilenum].mt		= (w2 >> 18) & 0x1;
	tile[tilenum].mask_t	= (w2 >> 14) & 0xf;
	tile[tilenum].shift_t	= (w2 >> 10) & 0xf;
	tile[tilenum].cs		= (w2 >>  9) & 0x1;
	tile[tilenum].ms		= (w2 >>  8) & 0x1;
	tile[tilenum].mask_s	= (w2 >>  4) & 0xf;
	tile[tilenum].shift_s	= (w2 >>  0) & 0xf;

	calculate_tile_derivs(tilenum);
}

static void rdp_fill_rect(uint32_t w1, uint32_t w2)
{
	uint32_t xl = (w1 >> 12) & 0xfff;
	uint32_t yl = (w1 >>  0) & 0xfff;
	uint32_t xh = (w2 >> 12) & 0xfff;
	uint32_t yh = (w2 >>  0) & 0xfff;
  uint32_t xlint, xhint;

	if (other_modes.cycle_type == CYCLE_TYPE_FILL || other_modes.cycle_type == CYCLE_TYPE_COPY)
		yl |= 3;

	xlint = (xl >> 2) & 0x3ff;
	xhint = (xh >> 2) & 0x3ff;

	int32_t ewdata[44];
	ewdata[0] = (0x3680 << 16) | yl;
	ewdata[1] = (yl << 16) | yh;
	ewdata[2] = (xlint << 16) | ((xl & 3) << 14);
	ewdata[3] = 0;
	ewdata[4] = (xhint << 16) | ((xh & 3) << 14);
	ewdata[5] = 0;
	ewdata[6] = (xlint << 16) | ((xl & 3) << 14);
	ewdata[7] = 0;
	memset(&ewdata[8], 0, 36 * sizeof(int32_t));

	edgewalker_for_prims(ewdata);
}

static void rdp_set_fill_color(uint32_t w1, uint32_t w2)
{
	fill_color = w2;
}

static void rdp_set_fog_color(uint32_t w1, uint32_t w2)
{
	fog_color[0] = (w2 >> 24) & 0xff;
	fog_color[1] = (w2 >> 16) & 0xff;
	fog_color[2] = (w2 >>  8) & 0xff;
	fog_color[3] = (w2 >>  0) & 0xff;
}

static void rdp_set_blend_color(uint32_t w1, uint32_t w2)
{
	blend_color[0] = (w2 >> 24) & 0xff;
	blend_color[1] = (w2 >> 16) & 0xff;
	blend_color[2] = (w2 >>  8) & 0xff;
	blend_color[3] = (w2 >>  0) & 0xff;
}

static void rdp_set_prim_color(uint32_t w1, uint32_t w2)
{
	min_level = (w1 >> 8) & 0x1f;
	primitive_lod_frac = w1 & 0xff;
	prim_color[0] = (w2 >> 24) & 0xff;
	prim_color[1] = (w2 >> 16) & 0xff;
	prim_color[2] = (w2 >>  8) & 0xff;
	prim_color[3] = (w2 >>  0) & 0xff;
}

static void rdp_set_env_color(uint32_t w1, uint32_t w2)
{
	env_color[0] = (w2 >> 24) & 0xff;
	env_color[1] = (w2 >> 16) & 0xff;
	env_color[2] = (w2 >>  8) & 0xff;
	env_color[3] = (w2 >>  0) & 0xff;
}

static void rdp_set_combine(uint32_t w1, uint32_t w2)
{
	combine.sub_a_rgb0	= (w1 >> 20) & 0xf;
	combine.mul_rgb0	= (w1 >> 15) & 0x1f;
	combine.sub_a_a0	= (w1 >> 12) & 0x7;
	combine.mul_a0		= (w1 >>  9) & 0x7;
	combine.sub_a_rgb1	= (w1 >>  5) & 0xf;
	combine.mul_rgb1	= (w1 >>  0) & 0x1f;

	combine.sub_b_rgb0	= (w2 >> 28) & 0xf;
	combine.sub_b_rgb1	= (w2 >> 24) & 0xf;
	combine.sub_a_a1	= (w2 >> 21) & 0x7;
	combine.mul_a1		= (w2 >> 18) & 0x7;
	combine.add_rgb0	= (w2 >> 15) & 0x7;
	combine.sub_b_a0	= (w2 >> 12) & 0x7;
	combine.add_a0		= (w2 >>  9) & 0x7;
	combine.add_rgb1	= (w2 >>  6) & 0x7;
	combine.sub_b_a1	= (w2 >>  3) & 0x7;
	combine.add_a1		= (w2 >>  0) & 0x7;

	
	SET_SUBA_RGB_INPUT(&combiner_rgbsub_a_r[0], &combiner_rgbsub_a_g[0], &combiner_rgbsub_a_b[0], combine.sub_a_rgb0);
	SET_SUBB_RGB_INPUT(&combiner_rgbsub_b_r[0], &combiner_rgbsub_b_g[0], &combiner_rgbsub_b_b[0], combine.sub_b_rgb0);
	SET_MUL_RGB_INPUT(&combiner_rgbmul_r[0], &combiner_rgbmul_g[0], &combiner_rgbmul_b[0], combine.mul_rgb0);
	SET_ADD_RGB_INPUT(&combiner_rgbadd_r[0], &combiner_rgbadd_g[0], &combiner_rgbadd_b[0], combine.add_rgb0);
	SET_SUB_ALPHA_INPUT(&combiner_alphasub_a[0], combine.sub_a_a0);
	SET_SUB_ALPHA_INPUT(&combiner_alphasub_b[0], combine.sub_b_a0);
	SET_MUL_ALPHA_INPUT(&combiner_alphamul[0], combine.mul_a0);
	SET_SUB_ALPHA_INPUT(&combiner_alphaadd[0], combine.add_a0);

	SET_SUBA_RGB_INPUT(&combiner_rgbsub_a_r[1], &combiner_rgbsub_a_g[1], &combiner_rgbsub_a_b[1], combine.sub_a_rgb1);
	SET_SUBB_RGB_INPUT(&combiner_rgbsub_b_r[1], &combiner_rgbsub_b_g[1], &combiner_rgbsub_b_b[1], combine.sub_b_rgb1);
	SET_MUL_RGB_INPUT(&combiner_rgbmul_r[1], &combiner_rgbmul_g[1], &combiner_rgbmul_b[1], combine.mul_rgb1);
	SET_ADD_RGB_INPUT(&combiner_rgbadd_r[1], &combiner_rgbadd_g[1], &combiner_rgbadd_b[1], combine.add_rgb1);
	SET_SUB_ALPHA_INPUT(&combiner_alphasub_a[1], combine.sub_a_a1);
	SET_SUB_ALPHA_INPUT(&combiner_alphasub_b[1], combine.sub_b_a1);
	SET_MUL_ALPHA_INPUT(&combiner_alphamul[1], combine.mul_a1);
	SET_SUB_ALPHA_INPUT(&combiner_alphaadd[1], combine.add_a1);

	other_modes.f.stalederivs = 1;
}

static void rdp_set_texture_image(uint32_t w1, uint32_t w2)
{
	ti_format	= (w1 >> 21) & 0x7;
	ti_size		= (w1 >> 19) & 0x3;
	ti_width	= (w1 & 0x3ff) + 1;
	ti_address	= w2 & 0x0ffffff;
	
	
	
}

static void rdp_set_mask_image(uint32_t w1, uint32_t w2)
{
	zb_address	= w2 & 0x0ffffff;
}

static void rdp_set_color_image(uint32_t w1, uint32_t w2)
{
	fb_format 	= (w1 >> 21) & 0x7;
	fb_size		= (w1 >> 19) & 0x3;
	fb_width	= (w1 & 0x3ff) + 1;
	fb_address	= w2 & 0x0ffffff;

	
	fbread1_ptr = fbread_func[fb_size];
	fbread2_ptr = fbread2_func[fb_size];
	fbwrite_ptr = fbwrite_func[fb_size];
	fbfill_ptr = fbfill_func[fb_size];
}



static void (*const rdp_command_table[64])(uint32_t w1, uint32_t w2) =
{
	
	rdp_noop,			rdp_invalid,			rdp_invalid,			rdp_invalid,
	rdp_invalid,		rdp_invalid,			rdp_invalid,			rdp_invalid,
	rdp_tri_noshade,	rdp_tri_noshade_z,		rdp_tri_tex,			rdp_tri_tex_z,
	rdp_tri_shade,		rdp_tri_shade_z,		rdp_tri_texshade,		rdp_tri_texshade_z,
	
	rdp_invalid,		rdp_invalid,			rdp_invalid,			rdp_invalid,
	rdp_invalid,		rdp_invalid,			rdp_invalid,			rdp_invalid,
	rdp_invalid,		rdp_invalid,			rdp_invalid,			rdp_invalid,
	rdp_invalid,		rdp_invalid,			rdp_invalid,			rdp_invalid,
	
	rdp_invalid,		rdp_invalid,			rdp_invalid,			rdp_invalid,
	rdp_tex_rect,		rdp_tex_rect_flip,		rdp_sync_load,			rdp_sync_pipe,
	rdp_sync_tile,		rdp_sync_full,			rdp_set_key_gb,			rdp_set_key_r,
	rdp_set_convert,	rdp_set_scissor,		rdp_set_prim_depth,		rdp_set_other_modes,
	
	rdp_load_tlut,		rdp_invalid,			rdp_set_tile_size,		rdp_load_block,
	rdp_load_tile,		rdp_set_tile,			rdp_fill_rect,			rdp_set_fill_color,
	rdp_set_fog_color,	rdp_set_blend_color,	rdp_set_prim_color,		rdp_set_env_color,
	rdp_set_combine,	rdp_set_texture_image,	rdp_set_mask_image,		rdp_set_color_image
};

void rdp_process_list(void)
{
	int i, length;
	uint32_t cmd, cmd_length;
	uint32_t dp_current_al = dp_current & ~7, dp_end_al = dp_end & ~7; 

	dp_status &= ~DP_STATUS_FREEZE;
	
	
	
	

	

	if (dp_end_al <= dp_current_al)
	{
		
		
		
		
		
		
		return;
	}

	length = (dp_end_al - dp_current_al) >> 2;

	ptr_onstart = rdp_cmd_ptr;



	uint32_t remaining_length = length;


	dp_current_al >>= 2;

	while (remaining_length)
	{

	int toload = remaining_length > 0x10000 ? 0x10000 : remaining_length;


	if (dp_status & DP_STATUS_XBUS_DMA)
	{
		for (i = 0; i < toload; i ++)
		{
			rdp_cmd_data[rdp_cmd_ptr] = byteswap_32(rsp_dmem[dp_current_al & 0x3ff]);
			rdp_cmd_ptr++;
			dp_current_al++;
		}
	}
	else
	{
		for (i = 0; i < toload; i ++)
		{
			RREADIDX32(rdp_cmd_data[rdp_cmd_ptr], dp_current_al);
			rdp_cmd_ptr++;
			dp_current_al++;
		}
	}

	remaining_length -= toload;

	while (rdp_cmd_cur < rdp_cmd_ptr && !rdp_pipeline_crashed)
	{
		cmd = (rdp_cmd_data[rdp_cmd_cur] >> 24) & 0x3f;
		cmd_length = rdp_command_length[cmd] >> 2;

		
		
		if ((rdp_cmd_ptr - rdp_cmd_cur) < cmd_length)
		{

			if (!remaining_length)
			{

				dp_start = dp_current = dp_end;
				return;
			}
			else
			{
				dp_current_al -= (rdp_cmd_ptr - rdp_cmd_cur);
				remaining_length += (rdp_cmd_ptr - rdp_cmd_cur);
				break;
			}
		}
		
		if (LOG_RDP_EXECUTION)
		{
			char string[4000];
			if (0)
			{
			z64gl_command += cmd_length;


			rdp_dasm(string);
			fprintf(rdp_exec, "%08X: %08X %08X   %s\n", command_counter, rdp_cmd_data[rdp_cmd_cur+0], rdp_cmd_data[rdp_cmd_cur+1], string);
			}
			command_counter++;
		}

		
		
		

		
		rdp_command_table[cmd](rdp_cmd_data[rdp_cmd_cur+0], rdp_cmd_data[rdp_cmd_cur + 1]);
		
		rdp_cmd_cur += cmd_length;
	};
	rdp_cmd_ptr = 0;
	rdp_cmd_cur = 0;
	};

	dp_start = dp_current = dp_end;


}

static inline int alpha_compare(int32_t comb_alpha)
{
	int32_t threshold;
	if (!other_modes.alpha_compare_en)
		return 1;
	else
	{
		if (!other_modes.dither_alpha_en)
			threshold = blend_color[3];
		else
			threshold = irand() & 0xff;
		if (comb_alpha >= threshold)
			return 1;
		else
			return 0;
	}
}

static inline int32_t color_combiner_equation(int32_t a, int32_t b, int32_t c, int32_t d)
{





	a = special_9bit_exttable[a];
	b = special_9bit_exttable[b];
	c = SIGNF(c, 9);
	d = special_9bit_exttable[d];
	a = ((a - b) * c) + (d << 8) + 0x80;
	return (a & 0x1ffff);
}

static inline int32_t alpha_combiner_equation(int32_t a, int32_t b, int32_t c, int32_t d)
{
	a = special_9bit_exttable[a];
	b = special_9bit_exttable[b];
	c = SIGNF(c, 9);
	d = special_9bit_exttable[d];
	a = (((a - b) * c) + (d << 8) + 0x80) >> 8;
	return (a & 0x1ff);
}


static inline void blender_equation_cycle0(int* r, int* g, int* b)
{
	int blend1a, blend2a;
	int blr, blg, blb, sum;
	blend1a = *blender1b_a[0] >> 3;
	blend2a = *blender2b_a[0] >> 3;

	int mulb;
    
	
	
	if (other_modes.f.special_bsel0)
	{
		blend1a = (blend1a >> blshifta) & 0x3C;
		blend2a = (blend2a >> blshiftb) | 3;
	}
	
	mulb = blend2a + 1;

	
	blr = (*blender1a_r[0]) * blend1a + (*blender2a_r[0]) * mulb;
	blg = (*blender1a_g[0]) * blend1a + (*blender2a_g[0]) * mulb;
	blb = (*blender1a_b[0]) * blend1a + (*blender2a_b[0]) * mulb;
	
	

	if (!other_modes.force_blend)
	{
		
		
		
		
		
		sum = ((blend1a & ~3) + (blend2a & ~3) + 4) << 9;
		*r = bldiv_hwaccurate_table[sum | ((blr >> 2) & 0x7ff)];
		*g = bldiv_hwaccurate_table[sum | ((blg >> 2) & 0x7ff)];
		*b = bldiv_hwaccurate_table[sum | ((blb >> 2) & 0x7ff)];
	}
	else
	{
		*r = (blr >> 5) & 0xff;	
		*g = (blg >> 5) & 0xff; 
		*b = (blb >> 5) & 0xff;
	}	
}

static inline void blender_equation_cycle0_2(int* r, int* g, int* b)
{
	int blend1a, blend2a;
	blend1a = *blender1b_a[0] >> 3;
	blend2a = *blender2b_a[0] >> 3;

	if (other_modes.f.special_bsel0)
	{
		blend1a = (blend1a >> pastblshifta) & 0x3C;
		blend2a = (blend2a >> pastblshiftb) | 3;
	}
	
	blend2a += 1;
	*r = (((*blender1a_r[0]) * blend1a + (*blender2a_r[0]) * blend2a) >> 5) & 0xff;
	*g = (((*blender1a_g[0]) * blend1a + (*blender2a_g[0]) * blend2a) >> 5) & 0xff;
	*b = (((*blender1a_b[0]) * blend1a + (*blender2a_b[0]) * blend2a) >> 5) & 0xff;
}

static inline void blender_equation_cycle1(int* r, int* g, int* b)
{
	int blend1a, blend2a;
	int blr, blg, blb, sum;
	blend1a = *blender1b_a[1] >> 3;
	blend2a = *blender2b_a[1] >> 3;

	int mulb;
	if (other_modes.f.special_bsel1)
	{
		blend1a = (blend1a >> blshifta) & 0x3C;
		blend2a = (blend2a >> blshiftb) | 3;
	}
	
	mulb = blend2a + 1;
	blr = (*blender1a_r[1]) * blend1a + (*blender2a_r[1]) * mulb;
	blg = (*blender1a_g[1]) * blend1a + (*blender2a_g[1]) * mulb;
	blb = (*blender1a_b[1]) * blend1a + (*blender2a_b[1]) * mulb;

	if (!other_modes.force_blend)
	{
		sum = ((blend1a & ~3) + (blend2a & ~3) + 4) << 9;
		*r = bldiv_hwaccurate_table[sum | ((blr >> 2) & 0x7ff)];
		*g = bldiv_hwaccurate_table[sum | ((blg >> 2) & 0x7ff)];
		*b = bldiv_hwaccurate_table[sum | ((blb >> 2) & 0x7ff)];
	}
	else
	{
		*r = (blr >> 5) & 0xff;	
		*g = (blg >> 5) & 0xff; 
		*b = (blb >> 5) & 0xff;
	}
}




static inline uint32_t rightcvghex(uint32_t x, uint32_t fmask)
{
	uint32_t covered = ((x & 7) + 1) >> 1;
	
	covered = 0xf0 >> covered;
	return (covered & fmask);
}

static inline uint32_t leftcvghex(uint32_t x, uint32_t fmask) 
{
	uint32_t covered = ((x & 7) + 1) >> 1;
	covered = 0xf >> covered;
	return (covered & fmask);
}

static inline void compute_cvg_flip(int32_t scanline)
{
	int32_t purgestart, purgeend;
	int i, length, fmask, maskshift, fmaskshifted;
	int32_t minorcur, majorcur, minorcurint, majorcurint, samecvg;

	purgestart = span[scanline].rx;
	purgeend = span[scanline].lx;
	length = purgeend - purgestart;
	if (length >= 0)
	{

		memset(&cvgbuf[purgestart], 0xff, length + 1);
		for(i = 0; i < 4; i++)
		{
				fmask = 0xa >> (i & 1);


				maskshift = (i - 2) & 4;
				fmaskshifted = fmask << maskshift;

				if (!span[scanline].invalyscan[i])
				{
					minorcur = span[scanline].minorx[i];
					majorcur = span[scanline].majorx[i];
					minorcurint = minorcur >> 3;
					majorcurint = majorcur >> 3;

					for (int k = purgestart; k <= majorcurint; k++)
						cvgbuf[k] &= ~fmaskshifted;
					for (int k = minorcurint; k <= purgeend; k++)
						cvgbuf[k] &= ~fmaskshifted;







					if (minorcurint > majorcurint)
					{
						cvgbuf[minorcurint] |= (rightcvghex(minorcur, fmask) << maskshift);
						cvgbuf[majorcurint] |= (leftcvghex(majorcur, fmask) << maskshift);
					}
					else if (minorcurint == majorcurint)
					{
						samecvg = rightcvghex(minorcur, fmask) & leftcvghex(majorcur, fmask);
						cvgbuf[majorcurint] |= (samecvg << maskshift);
					}
				}
				else
				{
					for (int k = purgestart; k <= purgeend; k++)
						cvgbuf[k] &= ~fmaskshifted;
				}

		}
	}
}

static inline void compute_cvg_noflip(int32_t scanline)
{
	int32_t purgestart, purgeend;
	int i, length, fmask, maskshift, fmaskshifted;
	int32_t minorcur, majorcur, minorcurint, majorcurint, samecvg;

	purgestart = span[scanline].lx;
	purgeend = span[scanline].rx;
	length = purgeend - purgestart;

	if (length >= 0)
	{
		memset(&cvgbuf[purgestart], 0xff, length + 1);

		for(i = 0; i < 4; i++)
		{
			fmask = 0xa >> (i & 1);
			maskshift = (i - 2) & 4;
			fmaskshifted = fmask << maskshift;

			if (!span[scanline].invalyscan[i])
			{
				minorcur = span[scanline].minorx[i];
				majorcur = span[scanline].majorx[i];
				minorcurint = minorcur >> 3;
				majorcurint = majorcur >> 3;

				for (int k = purgestart; k <= minorcurint; k++)
					cvgbuf[k] &= ~fmaskshifted;
				for (int k = majorcurint; k <= purgeend; k++)
					cvgbuf[k] &= ~fmaskshifted;

				if (majorcurint > minorcurint)
				{
					cvgbuf[minorcurint] |= (leftcvghex(minorcur, fmask) << maskshift);
					cvgbuf[majorcurint] |= (rightcvghex(majorcur, fmask) << maskshift);
				}
				else if (minorcurint == majorcurint)
				{
					samecvg = leftcvghex(minorcur, fmask) & rightcvghex(majorcur, fmask);
					cvgbuf[majorcurint] |= (samecvg << maskshift);
				}
			}
			else
			{
				for (int k = purgestart; k <= purgeend; k++)
					cvgbuf[k] &= ~fmaskshifted;
			}

		}
	}
}

int rdp_close()
{
	return 0;
}

static void fbwrite_4(uint32_t curpixel, uint32_t r, uint32_t g, uint32_t b, uint32_t blend_en, uint32_t curpixel_cvg, uint32_t curpixel_memcvg)
{
	uint32_t fb = fb_address + curpixel;
	RWRITEADDR8(fb, 0);
}

static void fbwrite_8(uint32_t curpixel, uint32_t r, uint32_t g, uint32_t b, uint32_t blend_en, uint32_t curpixel_cvg, uint32_t curpixel_memcvg)
{
	uint32_t fb = fb_address + curpixel;
	PAIRWRITE8(fb, r & 0xff, (r & 1) ? 3 : 0);
}

static void fbwrite_16(uint32_t curpixel, uint32_t r, uint32_t g, uint32_t b, uint32_t blend_en, uint32_t curpixel_cvg, uint32_t curpixel_memcvg)
{
#undef CVG_DRAW
#ifdef CVG_DRAW
	int covdraw = (curpixel_cvg - 1) << 5;
	r=covdraw; g=covdraw; b=covdraw;
#endif

	uint32_t fb;
	uint16_t rval;
	uint8_t hval;
	fb = (fb_address >> 1) + curpixel;	

	int32_t finalcvg = finalize_spanalpha(blend_en, curpixel_cvg, curpixel_memcvg);
	int16_t finalcolor; 

	if (fb_format == FORMAT_RGBA)
	{
		finalcolor = ((r & ~7) << 8) | ((g & ~7) << 3) | ((b & ~7) >> 2);
	}
	else
	{
		finalcolor = (r << 8) | (finalcvg << 5);
		finalcvg = 0;
	}

	
	rval = finalcolor|(finalcvg >> 2);
	hval = finalcvg & 3;
	PAIRWRITE16(fb, rval, hval);
}

static void fbwrite_32(uint32_t curpixel, uint32_t r, uint32_t g, uint32_t b, uint32_t blend_en, uint32_t curpixel_cvg, uint32_t curpixel_memcvg)
{
	uint32_t fb = (fb_address >> 2) + curpixel;

	int32_t finalcolor;
	int32_t finalcvg = finalize_spanalpha(blend_en, curpixel_cvg, curpixel_memcvg);
		
	finalcolor = (r << 24) | (g << 16) | (b << 8);
	finalcolor |= (finalcvg << 5);

	PAIRWRITE32(fb, finalcolor, (g & 1) ? 3 : 0, 0);
}

static void fbfill_4(uint32_t curpixel)
{
	rdp_pipeline_crashed = 1;
}

static void fbfill_8(uint32_t curpixel)
{
	uint32_t fb = fb_address + curpixel;
	uint32_t val = (fill_color >> (((fb & 3) ^ 3) << 3)) & 0xff;
	uint8_t hval = ((val & 1) << 1) | (val & 1);
	PAIRWRITE8(fb, val, hval);
}

static void fbfill_16(uint32_t curpixel)
{
	uint16_t val;
	uint8_t hval;
	uint32_t fb = (fb_address >> 1) + curpixel;
	if (fb & 1)
		val = fill_color & 0xffff;
	else
		val = (fill_color >> 16) & 0xffff;
	hval = ((val & 1) << 1) | (val & 1);
	PAIRWRITE16(fb, val, hval);
}

static void fbfill_32(uint32_t curpixel)
{
	uint32_t fb = (fb_address >> 2) + curpixel;
	PAIRWRITE32(fb, fill_color, (fill_color & 0x10000) ? 3 : 0, (fill_color & 0x1) ? 3 : 0);
}

static void fbread_4(uint32_t curpixel, uint32_t* curpixel_memcvg)
{
	memory_color[0] = memory_color[1] = memory_color[2] = 0;
	
	*curpixel_memcvg = 7;
	memory_color[3] = 0xe0;
}

static void fbread2_4(uint32_t curpixel, uint32_t* curpixel_memcvg)
{
	pre_memory_color[0] = pre_memory_color[1] = pre_memory_color[2] = 0;
	pre_memory_color[3] = 0xe0;
	*curpixel_memcvg = 7;
}

static void fbread_8(uint32_t curpixel, uint32_t* curpixel_memcvg)
{
	uint8_t mem;
	uint32_t addr = fb_address + curpixel;
	RREADADDR8(mem, addr);
	memory_color[0] = memory_color[1] = memory_color[2] = mem;
	*curpixel_memcvg = 7;
	memory_color[3] = 0xe0;
}

static void fbread2_8(uint32_t curpixel, uint32_t* curpixel_memcvg)
{
	uint8_t mem;
	uint32_t addr = fb_address + curpixel;
	RREADADDR8(mem, addr);
	pre_memory_color[0] = pre_memory_color[1] = pre_memory_color[2] = mem;
	pre_memory_color[3] = 0xe0;
	*curpixel_memcvg = 7;
}

static void fbread_16(uint32_t curpixel, uint32_t* curpixel_memcvg)
{
	uint16_t fword;
	uint8_t hbyte;
	uint32_t addr = (fb_address >> 1) + curpixel;

	uint8_t lowbits;


	if (other_modes.image_read_en)
	{
		PAIRREAD16(fword, hbyte, addr);

		if (fb_format == FORMAT_RGBA)
		{
			memory_color[0] = GET_HI(fword);
			memory_color[1] = GET_MED(fword);
			memory_color[2] = GET_LOW(fword);
			lowbits = ((fword & 1) << 2) | hbyte;
		}
		else
		{
			memory_color[0] = memory_color[1] = memory_color[2] = fword >> 8;
			lowbits = (fword >> 5) & 7;
		}

		*curpixel_memcvg = lowbits;
		memory_color[3] = lowbits << 5;
	}
	else
	{
		RREADIDX16(fword, addr);

		if (fb_format == FORMAT_RGBA)
		{
			memory_color[0] = GET_HI(fword);
			memory_color[1] = GET_MED(fword);
			memory_color[2] = GET_LOW(fword);
		}
		else
			memory_color[0] = memory_color[1] = memory_color[2] = fword >> 8;

		*curpixel_memcvg = 7;
		memory_color[3] = 0xe0;
	}
}

static void fbread2_16(uint32_t curpixel, uint32_t* curpixel_memcvg)
{
	uint16_t fword;
	uint8_t hbyte;
	uint32_t addr = (fb_address >> 1) + curpixel;

	uint8_t lowbits;


	if (other_modes.image_read_en)
	{
		PAIRREAD16(fword, hbyte, addr);

		if (fb_format == FORMAT_RGBA)
		{
			pre_memory_color[0] = GET_HI(fword);
			pre_memory_color[1] = GET_MED(fword);
			pre_memory_color[2] = GET_LOW(fword);
			lowbits = ((fword & 1) << 2) | hbyte;
		}
		else
		{
			pre_memory_color[0] = pre_memory_color[1] = pre_memory_color[2] = fword >> 8;
			lowbits = (fword >> 5) & 7;
		}

		*curpixel_memcvg = lowbits;
		pre_memory_color[3] = lowbits << 5;
	}
	else
	{
		RREADIDX16(fword, addr);

		if (fb_format == FORMAT_RGBA)
		{
			pre_memory_color[0] = GET_HI(fword);
			pre_memory_color[1] = GET_MED(fword);
			pre_memory_color[2] = GET_LOW(fword);
		}
		else
			pre_memory_color[0] = pre_memory_color[1] = pre_memory_color[2] = fword >> 8;

		*curpixel_memcvg = 7;
		pre_memory_color[3] = 0xe0;
	}
	
}

static void fbread_32(uint32_t curpixel, uint32_t* curpixel_memcvg)
{
	uint32_t mem, addr = (fb_address >> 2) + curpixel;
	RREADIDX32(mem, addr);
	memory_color[0] = (mem >> 24) & 0xff;
	memory_color[1] = (mem >> 16) & 0xff;
	memory_color[2] = (mem >> 8) & 0xff;
	if (other_modes.image_read_en)
	{
		*curpixel_memcvg = (mem >> 5) & 7;
		memory_color[3] = (mem) & 0xe0;
	}
	else
	{
		*curpixel_memcvg = 7;
		memory_color[3] = 0xe0;
	}
}

static void fbread2_32(uint32_t curpixel, uint32_t* curpixel_memcvg)
{
	uint32_t mem, addr = (fb_address >> 2) + curpixel; 
	RREADIDX32(mem, addr);
	pre_memory_color[0] = (mem >> 24) & 0xff;
	pre_memory_color[1] = (mem >> 16) & 0xff;
	pre_memory_color[2] = (mem >> 8) & 0xff;
	if (other_modes.image_read_en)
	{
		*curpixel_memcvg = (mem >> 5) & 7;
		pre_memory_color[3] = (mem) & 0xe0;
	}
	else
	{
		*curpixel_memcvg = 7;
		pre_memory_color[3] = 0xe0;
	}
}

static inline uint32_t z_decompress(uint32_t zb)
{
	return z_complete_dec_table[(zb >> 2) & 0x3fff];
}

static void z_build_com_table(void)
{

	uint16_t altmem = 0;
	for(int z = 0; z < 0x40000; z++)
	{
	switch((z >> 11) & 0x7f)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1a:
	case 0x1b:
	case 0x1c:
	case 0x1d:
	case 0x1e:
	case 0x1f:
	case 0x20:
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
	case 0x25:
	case 0x26:
	case 0x27:
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	case 0x2e:
	case 0x2f:
	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
	case 0x38:
	case 0x39:
	case 0x3a:
	case 0x3b:
	case 0x3c:
	case 0x3d:
	case 0x3e:
	case 0x3f:
		altmem = (z >> 4) & 0x1ffc;
		break;
	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
	case 0x48:
	case 0x49:
	case 0x4a:
	case 0x4b:
	case 0x4c:
	case 0x4d:
	case 0x4e:
	case 0x4f:
	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
	case 0x58:
	case 0x59:
	case 0x5a:
	case 0x5b:
	case 0x5c:
	case 0x5d:
	case 0x5e:
	case 0x5f:
		altmem = ((z >> 3) & 0x1ffc) | 0x2000;
		break;
	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
	case 0x68:
	case 0x69:
	case 0x6a:
	case 0x6b:
	case 0x6c:
	case 0x6d:
	case 0x6e:
	case 0x6f:
		altmem = ((z >> 2) & 0x1ffc) | 0x4000;
		break;
	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	case 0x76:
	case 0x77:
		altmem = ((z >> 1) & 0x1ffc) | 0x6000;
		break;
	case 0x78:
	case 0x79:
	case 0x7a:
	case 0x7b:
		altmem = (z & 0x1ffc) | 0x8000;
		break;
	case 0x7c:
	case 0x7d:
		altmem = ((z << 1) & 0x1ffc) | 0xa000;
		break;
	case 0x7e:
		altmem = ((z << 2) & 0x1ffc) | 0xc000;
		break;
	case 0x7f:
		altmem = ((z << 2) & 0x1ffc) | 0xe000;
		break;
	default:
		debug("z_build_com_table failed");
		break;
	}

    z_com_table[z] = altmem;

    }
}

static void precalc_cvmask_derivatives(void)
{
	int i = 0, k = 0;
	uint16_t mask = 0, maskx = 0, masky = 0;
	uint8_t offx = 0, offy = 0;
	const uint8_t yarray[16] = {0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};
	const uint8_t xarray[16] = {0, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};

	
	for (; i < 0x100; i++)
	{
		mask = decompress_cvmask_frombyte(i);
		cvarray[i].cvg = cvarray[i].cvbit = 0;
		cvarray[i].cvbit = (i >> 7) & 1;
		for (k = 0; k < 8; k++)
			cvarray[i].cvg += ((i >> k) & 1);

		
		masky = maskx = offx = offy = 0;
		for (k = 0; k < 4; k++)
			masky |= ((mask & (0xf000 >> (k << 2))) > 0) << k;

		offy = yarray[masky];
		
		maskx = (mask & (0xf000 >> (offy << 2))) >> ((offy ^ 3) << 2);
		
		
		offx = xarray[maskx];
		
		cvarray[i].xoff = offx;
		cvarray[i].yoff = offy;
	}
}

static inline uint16_t decompress_cvmask_frombyte(uint8_t x)
{
	uint16_t y = (x & 1) | ((x & 2) << 4) | (x & 4) | ((x & 8) << 4) |
		((x & 0x10) << 4) | ((x & 0x20) << 8) | ((x & 0x40) << 4) | ((x & 0x80) << 8);
	return y;
}

static inline void lookup_cvmask_derivatives(uint32_t mask, uint8_t* offx, uint8_t* offy, uint32_t* curpixel_cvg, uint32_t* curpixel_cvbit)
{
	CVtcmaskDERIVATIVE temp = cvarray[mask];
	*curpixel_cvg = temp.cvg;
	*curpixel_cvbit = temp.cvbit;
	*offx = temp.xoff;
	*offy = temp.yoff;
}

static inline void z_store(uint32_t zcurpixel, uint32_t z, int dzpixenc)
{
	uint16_t zval = z_com_table[z & 0x3ffff]|(dzpixenc >> 2);
	uint8_t hval = dzpixenc & 3;
	PAIRWRITE16(zcurpixel, zval, hval);
}

static inline uint32_t dz_decompress(uint32_t dz_compressed)
{
	return (1 << dz_compressed);
}

static inline uint32_t dz_compress(uint32_t value)
{
	int j = 0;
	if (value & 0xff00)
		j |= 8;
	if (value & 0xf0f0)
		j |= 4;
	if (value & 0xcccc)
		j |= 2;
	if (value & 0xaaaa)
		j |= 1;
	return j;
}

static inline uint32_t z_compare(uint32_t zcurpixel, uint32_t sz, uint16_t dzpix, int dzpixenc, uint32_t* blend_en, uint32_t* prewrap, uint32_t* curpixel_cvg, uint32_t curpixel_memcvg)
{


	int force_coplanar = 0;
	sz &= 0x3ffff;

	uint32_t oz, dzmem, zval, hval;
	int32_t rawdzmem;
  uint32_t dzmemmodifier;
  uint32_t dznew;
  uint32_t dznotshift;
  uint32_t farther;

	if (other_modes.z_compare_en)
	{
		PAIRREAD16(zval, hval, zcurpixel);
		oz = z_decompress(zval);
		rawdzmem = ((zval & 3) << 2) | hval;
		dzmem = dz_decompress(rawdzmem);


		if (other_modes.f.realblendershiftersneeded)
		{
			blshifta = CLIP(dzpixenc - rawdzmem, 0, 4);
			blshiftb = CLIP(rawdzmem - dzpixenc, 0, 4);

		}


		if (other_modes.f.interpixelblendershiftersneeded)
		{
			pastblshifta = CLIP(dzpixenc - pastrawdzmem, 0, 4);
			pastblshiftb = CLIP(pastrawdzmem - dzpixenc, 0, 4);
		}

		pastrawdzmem = rawdzmem;


		int precision_factor = (zval >> 13) & 0xf;


		if (precision_factor < 3)
		{
			if (dzmem != 0x8000)
			{
				dzmemmodifier = 16 >> precision_factor;
				dzmem <<= 1;
				if (dzmem < dzmemmodifier)
					dzmem = dzmemmodifier;
			}
			else
			{
				force_coplanar = 1;
				dzmem = 0xffff;
			}
			
		}



		dznew = (uint32_t)deltaz_comparator_lut[dzpix | dzmem];

		dznotshift = dznew;
		dznew <<= 3;

		farther = force_coplanar || ((sz + dznew) >= oz);
		
		int overflow = (curpixel_memcvg + *curpixel_cvg) & 8;
		*blend_en = other_modes.force_blend || (!overflow && other_modes.antialias_en && farther);
		
		*prewrap = overflow;

		
		
		int cvgcoeff = 0;
		uint32_t dzenc = 0;
	
		int32_t diff;
		uint32_t nearer, max, infront;

		switch(other_modes.z_mode)
		{
		case ZMODE_OPAQUE: 
			infront = sz < oz;
			diff = (int32_t)sz - (int32_t)dznew;
			nearer = force_coplanar || (diff <= (int32_t)oz);
			max = (oz == 0x3ffff);
			return (max || (overflow ? infront : nearer));
			break;
		case ZMODE_INTERPENETRATING: 
			infront = sz < oz;
			if (!infront || !farther || !overflow)
			{
				diff = (int32_t)sz - (int32_t)dznew;
				nearer = force_coplanar || (diff <= (int32_t)oz);
				max = (oz == 0x3ffff);
				return (max || (overflow ? infront : nearer)); 
			}
			else
			{
				dzenc = dz_compress(dznotshift & 0xffff);
				cvgcoeff = ((oz >> dzenc) - (sz >> dzenc)) & 0xf;
				*curpixel_cvg = ((cvgcoeff * (*curpixel_cvg)) >> 3) & 0xf;
				return 1;
			}
			break;
		case ZMODE_TRANSPARENT: 
			infront = sz < oz;
			max = (oz == 0x3ffff);
			return (infront || max); 
			break;
		case ZMODE_DECAL: 
			diff = (int32_t)sz - (int32_t)dznew;
			nearer = force_coplanar || (diff <= (int32_t)oz);
			max = (oz == 0x3ffff);
			return (farther && nearer && !max); 
			break;
		}
		return 0;
	}
	else
	{

		blshifta = CLIP(dzpixenc - 0xf, 0, 4);
		blshiftb = CLIP(0xf - dzpixenc, 0, 4);

		if (other_modes.f.realblendershiftersneeded)
		{
			blshifta = 0;
			if (dzpixenc < 0xb)
				blshiftb = 4;
			else
				blshiftb = 0xf - dzpixenc;
		}

		if (other_modes.f.interpixelblendershiftersneeded)
		{
			pastblshifta = 0;
			if (dzpixenc < 0xb)
				pastblshiftb = 4;
			else
				pastblshiftb = 0xf - dzpixenc;
		}
		pastrawdzmem = 0xf;

		int overflow = (curpixel_memcvg + *curpixel_cvg) & 8;
		*blend_en = other_modes.force_blend || (!overflow && other_modes.antialias_en);
		*prewrap = overflow;

		return 1;
	}
}

static inline int finalize_spanalpha(uint32_t blend_en, uint32_t curpixel_cvg, uint32_t curpixel_memcvg)
{
	int finalcvg;

	
	
	switch(other_modes.cvg_dest)
	{
	case CVG_CLAMP: 
		if (!blend_en)
		{
			finalcvg = curpixel_cvg - 1;
			
			
		}
		else
		{
			finalcvg = curpixel_cvg + curpixel_memcvg;
		}
		if (!(finalcvg & 8))
			finalcvg &= 7;
		else
			finalcvg = 7;
		break;
	case CVG_WRAP:
		finalcvg = (curpixel_cvg + curpixel_memcvg) & 7;
		break;
	case CVG_ZAP: 
		finalcvg = 7;
		break;
	case CVG_SAVE: 
		finalcvg = curpixel_memcvg;
		break;
	}

	return finalcvg;
}

static inline int32_t normalize_dzpix(int32_t sum)
{
	if (sum & 0xc000)
		return 0x8000;
	if (!(sum & 0xffff))
		return 1;
	if (sum == 1)
		return 3;
	for(int count = 0x2000; count > 0; count >>= 1)
    {
      if (sum & count)
        return(count << 1);
    }
	debug("normalize_dzpix: invalid codepath taken");
	return 0;
}

static inline int32_t CLIP(int32_t value,int32_t min,int32_t max)
{
	if (value < min)
		return min;
	else if (value > max)
		return max;
	else
		return value;
}


static inline void video_filter16(int* endr, int* endg, int* endb, uint32_t fboffset, uint32_t num, uint32_t hres, uint32_t centercvg, uint32_t fetchbugstate)
{


	uint32_t penumaxr, penumaxg, penumaxb, penuminr, penuming, penuminb;
	uint16_t pix;
	uint32_t numoffull = 1;
	uint32_t hidval;
	uint32_t r, g, b; 
	uint32_t backr[7], backg[7], backb[7];

	r = *endr;
	g = *endg;
	b = *endb;

	backr[0] = r;
	backg[0] = g;
	backb[0] = b;


	uint32_t idx = (fboffset >> 1) + num;

	uint32_t toleft = idx - 2;
	uint32_t toright = idx + 2;

	uint32_t leftup, rightup, leftdown, rightdown;

	leftup = idx - hres - 1;
	rightup = idx - hres + 1;



	if (fetchbugstate != 1)
	{
		leftdown = idx + hres - 1;
		rightdown = idx + hres + 1;
	}
	else
	{
		leftdown = toleft;
		rightdown = toright;
	}


#define VI_ANDER(x) {													\
			PAIRREAD16(pix, hidval, (x));									\
			if (hidval == 3 && (pix & 1))								\
			{															\
				backr[numoffull] = GET_HI(pix);							\
				backg[numoffull] = GET_MED(pix);						\
				backb[numoffull] = GET_LOW(pix);						\
				numoffull++;											\
			}															\
}
	
	VI_ANDER(leftup);
	VI_ANDER(rightup);
	VI_ANDER(toleft);
	VI_ANDER(toright);
	VI_ANDER(leftdown);
	VI_ANDER(rightdown);

	uint32_t colr, colg, colb;
	
	
	
	
	video_max_optimized(backr, &penuminr, &penumaxr, numoffull);
	video_max_optimized(backg, &penuming, &penumaxg, numoffull);
	video_max_optimized(backb, &penuminb, &penumaxb, numoffull);

	
	

	
	uint32_t coeff = 7 - centercvg;
	colr = penuminr + penumaxr - (r << 1);
	colg = penuming + penumaxg - (g << 1);
	colb = penuminb + penumaxb - (b << 1);

	colr = (((colr * coeff) + 4) >> 3) + r;
	colg = (((colg * coeff) + 4) >> 3) + g;
	colb = (((colb * coeff) + 4) >> 3) + b;

	*endr = colr & 0xff;
	*endg = colg & 0xff;
	*endb = colb & 0xff;

	
	
}

static inline void video_filter32(int* endr, int* endg, int* endb, uint32_t fboffset, uint32_t num, uint32_t hres, uint32_t centercvg, uint32_t fetchbugstate)
{

	uint32_t penumaxr, penumaxg, penumaxb, penuminr, penuming, penuminb;
	uint32_t numoffull = 1;
	uint32_t pix = 0, pixcvg = 0;
	uint32_t r, g, b; 
	uint32_t backr[7], backg[7], backb[7];

	r = *endr;
	g = *endg;
	b = *endb;

	backr[0] = r;
	backg[0] = g;
	backb[0] = b;

	uint32_t idx = (fboffset >> 2) + num;

	uint32_t toleft = idx - 2;
	uint32_t toright = idx + 2;

	uint32_t leftup, rightup, leftdown, rightdown;

	leftup = idx - hres - 1;
	rightup = idx - hres + 1;

	if (fetchbugstate != 1)
	{
		leftdown = idx + hres - 1;
		rightdown = idx + hres + 1;
	}
	else
	{
		leftdown = toleft;
		rightdown = toright;
	}

#define VI_ANDER32(x) {													\
			RREADIDX32(pix, (x));										\
			pixcvg = (pix >> 5) & 7;									\
			if (pixcvg == 7)											\
			{															\
				backr[numoffull] = (pix >> 24) & 0xff;					\
				backg[numoffull] = (pix >> 16) & 0xff;					\
				backb[numoffull] = (pix >> 8) & 0xff;					\
				numoffull++;											\
			}															\
}
	
	VI_ANDER32(leftup);
	VI_ANDER32(rightup);
	VI_ANDER32(toleft);
	VI_ANDER32(toright);
	VI_ANDER32(leftdown);
	VI_ANDER32(rightdown);

	uint32_t colr, colg, colb;

	video_max_optimized(backr, &penuminr, &penumaxr, numoffull);
	video_max_optimized(backg, &penuming, &penumaxg, numoffull);
	video_max_optimized(backb, &penuminb, &penumaxb, numoffull);

	uint32_t coeff = 7 - centercvg;
	colr = penuminr + penumaxr - (r << 1);
	colg = penuming + penumaxg - (g << 1);
	colb = penuminb + penumaxb - (b << 1);

	colr = (((colr * coeff) + 4) >> 3) + r;
	colg = (((colg * coeff) + 4) >> 3) + g;
	colb = (((colb * coeff) + 4) >> 3) + b;

	*endr = colr & 0xff;
	*endg = colg & 0xff;
	*endb = colb & 0xff;
}

static inline void divot_filter(CCVG final, CCVG centercolor, CCVG leftcolor, CCVG rightcolor)
{







	uint32_t leftr, leftg, leftb, rightr, rightg, rightb, centerr, centerg, centerb;

  memcpy(final, centercolor, sizeof(CCVG));
	
	if ((centercolor[3] & leftcolor[3] & rightcolor[3]) == 7)
	
	
	
	{
		return;
	}

	leftr = leftcolor[0];	
	leftg = leftcolor[1];	
	leftb = leftcolor[2];
	rightr = rightcolor[0];	
	rightg = rightcolor[1];	
	rightb = rightcolor[2];
	centerr = centercolor[0];
	centerg = centercolor[1];
	centerb = centercolor[2];


	if ((leftr >= centerr && rightr >= leftr) || (leftr >= rightr && centerr >= leftr))
		final[0] = leftr;
	else if ((rightr >= centerr && leftr >= rightr) || (rightr >= leftr && centerr >= rightr))
		final[0] = rightr;

	if ((leftg >= centerg && rightg >= leftg) || (leftg >= rightg && centerg >= leftg))
		final[1] = leftg;
	else if ((rightg >= centerg && leftg >= rightg) || (rightg >= leftg && centerg >= rightg))
		final[1] = rightg;

	if ((leftb >= centerb && rightb >= leftb) || (leftb >= rightb && centerb >= leftb))
		final[2] = leftb;
	else if ((rightb >= centerb && leftb >= rightb) || (rightb >= leftb && centerb >= rightb))
		final[2] = rightb;
}

static inline void restore_filter16(int* r, int* g, int* b, uint32_t fboffset, uint32_t num, uint32_t hres, uint32_t fetchbugstate)
{

	uint32_t idx = (fboffset >> 1) + num;

	uint32_t toleftpix = idx - 1;

	uint32_t leftuppix, leftdownpix, maxpix;

	leftuppix = idx - hres - 1;

	if (fetchbugstate != 1)
	{
		leftdownpix = idx + hres - 1;
		maxpix = idx + hres + 1;
	}
	else
	{
		leftdownpix = toleftpix;
		maxpix = toleftpix + 2;
	}

	int rend = *r;
	int gend = *g;
	int bend = *b;
	const int* redptr = &vi_restore_table[(rend << 2) & 0x3e0];
	const int* greenptr = &vi_restore_table[(gend << 2) & 0x3e0];
	const int* blueptr = &vi_restore_table[(bend << 2) & 0x3e0];

	uint32_t tempr, tempg, tempb;
	uint16_t pix;
	uint32_t addr;


#define VI_COMPARE(x)												\
{																	\
	addr = (x);														\
	RREADIDX16(pix, addr);											\
	tempr = (pix >> 11) & 0x1f;										\
	tempg = (pix >> 6) & 0x1f;										\
	tempb = (pix >> 1) & 0x1f;										\
	rend += redptr[tempr];											\
	gend += greenptr[tempg];										\
	bend += blueptr[tempb];											\
}

#define VI_COMPARE_OPT(x)											\
{																	\
	addr = (x);														\
	pix = byteswap_16(rdram_16[addr]);							\
	tempr = (pix >> 11) & 0x1f;										\
	tempg = (pix >> 6) & 0x1f;										\
	tempb = (pix >> 1) & 0x1f;										\
	rend += redptr[tempr];											\
	gend += greenptr[tempg];										\
	bend += blueptr[tempb];											\
}

	if (maxpix <= idxlim16 && leftuppix <= idxlim16)
	{
		VI_COMPARE_OPT(leftuppix);
		VI_COMPARE_OPT(leftuppix + 1);
		VI_COMPARE_OPT(leftuppix + 2);
		VI_COMPARE_OPT(leftdownpix);
		VI_COMPARE_OPT(leftdownpix + 1);
		VI_COMPARE_OPT(maxpix);
		VI_COMPARE_OPT(toleftpix);
		VI_COMPARE_OPT(toleftpix + 2);
	}
	else
	{
		VI_COMPARE(leftuppix);
		VI_COMPARE(leftuppix + 1);
		VI_COMPARE(leftuppix + 2);
		VI_COMPARE(leftdownpix);
		VI_COMPARE(leftdownpix + 1);
		VI_COMPARE(maxpix);
		VI_COMPARE(toleftpix);
		VI_COMPARE(toleftpix + 2);
	}

	
	*r = rend;
	*g = gend;
	*b = bend;
}

static inline void restore_filter32(int* r, int* g, int* b, uint32_t fboffset, uint32_t num, uint32_t hres, uint32_t fetchbugstate)
{
	uint32_t idx = (fboffset >> 2) + num;

	uint32_t toleftpix = idx - 1;

	uint32_t leftuppix, leftdownpix, maxpix;

	leftuppix = idx - hres - 1;

	if (fetchbugstate != 1)
	{
		leftdownpix = idx + hres - 1;
		maxpix = idx +hres + 1;
	}
	else
	{
		leftdownpix = toleftpix;
		maxpix = toleftpix + 2;
	}

	int rend = *r;
	int gend = *g;
	int bend = *b;
	const int* redptr = &vi_restore_table[(rend << 2) & 0x3e0];
	const int* greenptr = &vi_restore_table[(gend << 2) & 0x3e0];
	const int* blueptr = &vi_restore_table[(bend << 2) & 0x3e0];

	uint32_t tempr, tempg, tempb;
	uint32_t pix, addr;

#define VI_COMPARE32(x)													\
{																		\
	addr = (x);															\
	RREADIDX32(pix, addr);												\
	tempr = (pix >> 27) & 0x1f;											\
	tempg = (pix >> 19) & 0x1f;											\
	tempb = (pix >> 11) & 0x1f;											\
	rend += redptr[tempr];												\
	gend += greenptr[tempg];											\
	bend += blueptr[tempb];												\
}

#define VI_COMPARE32_OPT(x)													\
{																		\
	addr = (x);															\
	pix = byteswap_32(rdram[addr]);												\
	tempr = (pix >> 27) & 0x1f;											\
	tempg = (pix >> 19) & 0x1f;											\
	tempb = (pix >> 11) & 0x1f;											\
	rend += redptr[tempr];												\
	gend += greenptr[tempg];											\
	bend += blueptr[tempb];												\
}

	if (maxpix <= idxlim32 && leftuppix <= idxlim32)
	{
		VI_COMPARE32_OPT(leftuppix);
		VI_COMPARE32_OPT(leftuppix + 1);
		VI_COMPARE32_OPT(leftuppix + 2);
		VI_COMPARE32_OPT(leftdownpix);
		VI_COMPARE32_OPT(leftdownpix + 1);
		VI_COMPARE32_OPT(maxpix);
		VI_COMPARE32_OPT(toleftpix);
		VI_COMPARE32_OPT(toleftpix + 2);
	}
	else
	{
		VI_COMPARE32(leftuppix);
		VI_COMPARE32(leftuppix + 1);
		VI_COMPARE32(leftuppix + 2);
		VI_COMPARE32(leftdownpix);
		VI_COMPARE32(leftdownpix + 1);
		VI_COMPARE32(maxpix);
		VI_COMPARE32(toleftpix);
		VI_COMPARE32(toleftpix + 2);
	}

	*r = rend;
	*g = gend;
	*b = bend;
}

static inline void gamma_filters(int* r, int* g, int* b, int gamma_and_dither)
{
	int cdith, dith;
	
	

	switch(gamma_and_dither)
	{
	case 0:
		return;
		break;
	case 1:
		cdith = irand();
		dith = cdith & 1;
		if (*r < 255)
			*r += dith;
		dith = (cdith >> 1) & 1;
		if (*g < 255)
			*g += dith;
		dith = (cdith >> 2) & 1;
		if (*b < 255)
			*b += dith;
		break;
	case 2:
		*r = gamma_table[*r];
		*g = gamma_table[*g];
		*b = gamma_table[*b];
		break;
	case 3:
		cdith = irand();
		dith = cdith & 0x3f;
		*r = gamma_dither_table[((*r) << 6)|dith];
		dith = (cdith >> 6) & 0x3f;
		*g = gamma_dither_table[((*g) << 6)|dith];
		dith = ((cdith >> 9) & 0x38) | (cdith & 7);
		*b = gamma_dither_table[((*b) << 6)|dith];
		break;
	}
}

static inline void adjust_brightness(int* r, int* g, int* b, int brightcoeff)
{
	brightcoeff &= 7;
	switch(brightcoeff)
	{
	case 0:	
		break;
	case 1: 
	case 2:
	case 3:
		*r += (*r >> (4 - brightcoeff));
		*g += (*g >> (4 - brightcoeff));
		*b += (*b >> (4 - brightcoeff));
		if (*r > 0xff)
			*r = 0xff;
		if (*g > 0xff)
			*g = 0xff;
		if (*b > 0xff)
			*b = 0xff;
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		*r = (*r + 1) << (brightcoeff - 3);
		*g = (*g + 1) << (brightcoeff - 3);
		*b = (*b + 1) << (brightcoeff - 3);
		if (*r > 0xff)
			*r = 0xff;
		if (*g > 0xff)
			*g = 0xff;
		if (*b > 0xff)
			*b = 0xff;
		break;
	}
}


static inline void video_max_optimized(uint32_t* pixels, uint32_t* penumin, uint32_t* penumax, int numofels)
{



	int i;
	int posmax = 0, posmin = 0;
	uint32_t curpenmax = pixels[0], curpenmin = pixels[0];
	uint32_t max, min;

	for (i = 1; i < numofels; i++)
	{
	    if (pixels[i] > pixels[posmax])
		{
			curpenmax = pixels[posmax];
			posmax = i;			
		}
		else if (pixels[i] < pixels[posmin])
		{
			curpenmin = pixels[posmin];
			posmin = i;
		}
	}
	max = pixels[posmax];
	min = pixels[posmin];
	if (curpenmax != max)
	{
		for (i = posmax + 1; i < numofels; i++)
		{
			if (pixels[i] > curpenmax)
				curpenmax = pixels[i];
		}
	}
	if (curpenmin != min)
	{
		for (i = posmin + 1; i < numofels; i++)
		{
			if (pixels[i] < curpenmin)
				curpenmin = pixels[i];
		}
	}
	*penumax = curpenmax;
	*penumin = curpenmin;
}


static void calculate_clamp_diffs(uint32_t i)
{
	tile[i].f.clampdiffs = ((tile[i].sh >> 2) - (tile[i].sl >> 2)) & 0x3ff;
	tile[i].f.clampdifft = ((tile[i].th >> 2) - (tile[i].tl >> 2)) & 0x3ff;
}


static void calculate_tile_derivs(uint32_t i)
{
	tile[i].f.clampens = tile[i].cs || !tile[i].mask_s;
	tile[i].f.clampent = tile[i].ct || !tile[i].mask_t;
	tile[i].f.masksclamped = tile[i].mask_s <= 10 ? tile[i].mask_s : 10;
	tile[i].f.masktclamped = tile[i].mask_t <= 10 ? tile[i].mask_t : 10;
	tile[i].f.notlutswitch = (tile[i].format << 2) | tile[i].size;
	tile[i].f.tlutswitch = (tile[i].size << 2) | ((tile[i].format + 2) & 3);
}

static void rgb_dither_complete(int* r, int* g, int* b, int dith)
{

	int32_t newr = *r, newg = *g, newb = *b;
	int32_t rcomp, gcomp, bcomp;


	if (newr > 247)
		newr = 255;
	else
		newr = (newr & 0xf8) + 8;
	if (newg > 247)
		newg = 255;
	else
		newg = (newg & 0xf8) + 8;
	if (newb > 247)
		newb = 255;
	else
		newb = (newb & 0xf8) + 8;

	if (other_modes.rgb_dither_sel != 2)
		rcomp = gcomp = bcomp = dith;
	else
	{
		rcomp = dith & 7;
		gcomp = (dith >> 3) & 7;
		bcomp = (dith >> 6) & 7;
	}





	int32_t replacesign = (rcomp - (*r & 7)) >> 31;

	int32_t ditherdiff = newr - *r;
	*r = *r + (ditherdiff & replacesign);

	replacesign = (gcomp - (*g & 7)) >> 31;
	ditherdiff = newg - *g;
	*g = *g + (ditherdiff & replacesign);

	replacesign = (bcomp - (*b & 7)) >> 31;
	ditherdiff = newb - *b;
	*b = *b + (ditherdiff & replacesign);

}

static void rgb_dither_nothing(int* r, int* g, int* b, int dith)
{
}


static void get_dither_noise_complete(int x, int y, int* cdith, int* adith)
{

	
	noise = ((irand() & 7) << 6) | 0x20;
	
	
	int dithindex; 
	switch(other_modes.f.rgb_alpha_dither)
	{
	case 0:
		dithindex = ((y & 3) << 2) | (x & 3);
		*adith = *cdith = magic_matrix[dithindex];
		break;
	case 1:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = magic_matrix[dithindex];
		*adith = (~(*cdith)) & 7;
		break;
	case 2:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = magic_matrix[dithindex];
		*adith = (noise >> 6) & 7;
		break;
	case 3:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = magic_matrix[dithindex];
		*adith = 0;
		break;
	case 4:
		dithindex = ((y & 3) << 2) | (x & 3);
		*adith = *cdith = bayer_matrix[dithindex];
		break;
	case 5:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = bayer_matrix[dithindex];
		*adith = (~(*cdith)) & 7;
		break;
	case 6:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = bayer_matrix[dithindex];
		*adith = (noise >> 6) & 7;
		break;
	case 7:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = bayer_matrix[dithindex];
		*adith = 0;
		break;
	case 8:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = irand();
		*adith = magic_matrix[dithindex];
		break;
	case 9:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = irand();
		*adith = (~magic_matrix[dithindex]) & 7;
		break;
	case 10:
		*cdith = irand();
		*adith = (noise >> 6) & 7;
		break;
	case 11:
		*cdith = irand();
		*adith = 0;
		break;
	case 12:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = 7;
		*adith = bayer_matrix[dithindex];
		break;
	case 13:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = 7;
		*adith = (~bayer_matrix[dithindex]) & 7;
		break;
	case 14:
		*cdith = 7;
		*adith = (noise >> 6) & 7;
		break;
	case 15:
		*cdith = 7;
		*adith = 0;
		break;
	}
}


static void get_dither_only(int x, int y, int* cdith, int* adith)
{
	int dithindex; 
	switch(other_modes.f.rgb_alpha_dither)
	{
	case 0:
		dithindex = ((y & 3) << 2) | (x & 3);
		*adith = *cdith = magic_matrix[dithindex];
		break;
	case 1:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = magic_matrix[dithindex];
		*adith = (~(*cdith)) & 7;
		break;
	case 2:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = magic_matrix[dithindex];
		*adith = (noise >> 6) & 7;
		break;
	case 3:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = magic_matrix[dithindex];
		*adith = 0;
		break;
	case 4:
		dithindex = ((y & 3) << 2) | (x & 3);
		*adith = *cdith = bayer_matrix[dithindex];
		break;
	case 5:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = bayer_matrix[dithindex];
		*adith = (~(*cdith)) & 7;
		break;
	case 6:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = bayer_matrix[dithindex];
		*adith = (noise >> 6) & 7;
		break;
	case 7:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = bayer_matrix[dithindex];
		*adith = 0;
		break;
	case 8:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = irand();
		*adith = magic_matrix[dithindex];
		break;
	case 9:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = irand();
		*adith = (~magic_matrix[dithindex]) & 7;
		break;
	case 10:
		*cdith = irand();
		*adith = (noise >> 6) & 7;
		break;
	case 11:
		*cdith = irand();
		*adith = 0;
		break;
	case 12:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = 7;
		*adith = bayer_matrix[dithindex];
		break;
	case 13:
		dithindex = ((y & 3) << 2) | (x & 3);
		*cdith = 7;
		*adith = (~bayer_matrix[dithindex]) & 7;
		break;
	case 14:
		*cdith = 7;
		*adith = (noise >> 6) & 7;
		break;
	case 15:
		*cdith = 7;
		*adith = 0;
		break;
	}
}

static void get_dither_nothing(int x, int y, int* cdith, int* adith)
{
}

static inline void vi_vl_lerp(CCVG up, CCVG down, uint32_t frac)
{
	uint32_t r0, g0, b0;
	if (!frac)
		return;

	r0 = up[0];
	g0 = up[1];
	b0 = up[2];
	
	up[0] = ((((down[0] - r0) * frac + 16) >> 5) + r0) & 0xff;
	up[1] = ((((down[1] - g0) * frac + 16) >> 5) + g0) & 0xff;
	up[2] = ((((down[2] - b0) * frac + 16) >> 5) + b0) & 0xff;

}

static inline void rgbaz_correct_clip_sse(int offx, int offy, __m128i rgba, int *z, uint32_t curpixel_cvg, __m128i spans_cdrgba_drgbady_v) {
  __m128i rgba2;
  int sz = *z;
  int zanded;

  if (curpixel_cvg == 8) {
    rgba = _mm_slli_epi32(rgba, 21);
    sz >>= 3;
  }

  else {
    __m128i off_v = _mm_set1_epi32(offx + (offy << 16));
    __m128i summand_rgba = _mm_madd_epi16(off_v, spans_cdrgba_drgbady_v);

    int summand_z = offx * spans_cdz + offy * spans_dstwzdy[3];
    sz = ((sz << 2) + summand_z) >> 5;

    rgba = _mm_add_epi32(_mm_slli_epi32(rgba, 2), summand_rgba);
    rgba = _mm_slli_epi32(rgba, 19);
  }

  rgba2 = _mm_adds_epu16(rgba, rgba);
  rgba = _mm_srai_epi16(_mm_add_epi32(rgba, rgba), 15);
  rgba = _mm_cmpeq_epi16(rgba, rgba2);
  rgba = _mm_andnot_si128(rgba, rgba2);
  _mm_store_si128(shade_color, _mm_srli_epi32(rgba, 24));

	zanded = (sz & 0x60000) >> 17;	

	switch(zanded)
	{
		case 0: *z = sz & 0x3ffff;						break;
		case 1:	*z = sz & 0x3ffff;						break;
		case 2: *z = 0x3ffff;							break;
		case 3: *z = 0;									break;
	}
}

static inline void rgbaz_correct_clip(int offx, int offy, int r, int g, int b, int a, int* z, uint32_t curpixel_cvg)
{
	int summand_r, summand_b, summand_g, summand_a;
	int summand_z;
	int sz = *z;
	int zanded;




	if (curpixel_cvg == 8)
	{
		r >>= 2;
		g >>= 2;
		b >>= 2;
		a >>= 2;
		sz = sz >> 3;
	}
	else
	{
		summand_r = offx * spans_cdrgba_drgbady[0] + offy * spans_cdrgba_drgbady[1];
		summand_g = offx * spans_cdrgba_drgbady[2] + offy * spans_cdrgba_drgbady[3];
		summand_b = offx * spans_cdrgba_drgbady[4] + offy * spans_cdrgba_drgbady[5];
		summand_a = offx * spans_cdrgba_drgbady[6] + offy * spans_cdrgba_drgbady[7];
		summand_z = offx * spans_cdz + offy * spans_dstwzdy[3];

		r = ((r << 2) + summand_r) >> 4;
		g = ((g << 2) + summand_g) >> 4;
		b = ((b << 2) + summand_b) >> 4;
		a = ((a << 2) + summand_a) >> 4;
		sz = ((sz << 2) + summand_z) >> 5;
	}

	
	shade_color[0] = special_9bit_clamptable[r & 0x1ff];
	shade_color[1] = special_9bit_clamptable[g & 0x1ff];
	shade_color[2] = special_9bit_clamptable[b & 0x1ff];
	shade_color[3] = special_9bit_clamptable[a & 0x1ff];
	
	
	
	zanded = (sz & 0x60000) >> 17;

	
	switch(zanded)
	{
		case 0: *z = sz & 0x3ffff;						break;
		case 1:	*z = sz & 0x3ffff;						break;
		case 2: *z = 0x3ffff;							break;
		case 3: *z = 0;									break;
	}
}

uint32_t vi_integer_sqrt(uint32_t a)
{
	unsigned long op = a, res = 0, one = 1 << 30;

    while (one > op) 
		one >>= 2;

    while (one != 0) 
	{
        if (op >= res + one) 
		{
            op -= res + one;
            res += one << 1;
        }
        res >>= 1;
        one >>= 2;
    }
    return res;
}

static void clearfb16(uint16_t* fb, uint32_t width,uint32_t height)
{
	uint16_t* d;
	uint32_t j;
	int i = width << 1;
	for (j = 0; j < height; j++)
	{
		d = &fb[j*width];
		memset(d,0,i);
	}
}

static void tcdiv_nopersp_sse(__m128i stwz, int32_t ssst[2]) {
  _mm_storel_epi64(ssst, _mm_srli_epi32(_mm_srai_epi32(_mm_slli_epi32(stwz, 16), 1), 15));
}

static void tcdiv_nopersp(int32_t ss, int32_t st, int32_t sw, int32_t* sss, int32_t* sst)
{

	*sss = (SIGN16(ss)) & 0x1ffff;
	*sst = (SIGN16(st)) & 0x1ffff;
}

static void tcdiv_persp_sse(__m128i stwz, int32_t ssst[2]) {
  int32_t ss, st, sw;

  ss = _mm_extract_epi32(stwz, 0);
  st = _mm_extract_epi32(stwz, 1);
  sw = _mm_extract_epi32(stwz, 2);

	int w_carry = 0;
	int shift; 
	int tlu_rcp;
    int sprod, tprod;
	int outofbounds_s, outofbounds_t;
	int tempmask;
	int shift_value;
	int32_t temps, tempt;

	
	
	int overunder_s = 0, overunder_t = 0;
	
	
	if (SIGN16(sw) <= 0)
		w_carry = 1;

	sw &= 0x7fff;

	
	
	shift = tcdiv_table[sw];
	tlu_rcp = shift >> 4;
	shift &= 0xf;

	sprod = SIGN16(ss) * tlu_rcp;
	tprod = SIGN16(st) * tlu_rcp;

	
	
	
	tempmask = ((1 << 30) - 1) & -((1 << 29) >> shift);
	
	outofbounds_s = sprod & tempmask;
	outofbounds_t = tprod & tempmask;
	
	if (shift != 0xe)
	{
		shift_value = 13 - shift;
		temps = sprod = (sprod >> shift_value);
		tempt = tprod = (tprod >> shift_value);
	}
	else
	{
		temps = sprod << 1;
		tempt = tprod << 1;
	}

	if (outofbounds_s != tempmask && outofbounds_s != 0)
	{
		if (!(sprod & (1 << 29)))
			overunder_s = 2 << 17;
		else
			overunder_s = 1 << 17;
	}

	if (outofbounds_t != tempmask && outofbounds_t != 0)
	{
		if (!(tprod & (1 << 29)))
			overunder_t = 2 << 17;
		else
			overunder_t = 1 << 17;
	}

	if (w_carry)
	{
		overunder_s |= (2 << 17);
		overunder_t |= (2 << 17);
	}

	ssst[0] = (temps & 0x1ffff) | overunder_s;
	ssst[1] = (tempt & 0x1ffff) | overunder_t;
}

static void tcdiv_persp(int32_t ss, int32_t st, int32_t sw, int32_t* sss, int32_t* sst)
{


	int w_carry = 0;
	int shift; 
	int tlu_rcp;
    int sprod, tprod;
	int outofbounds_s, outofbounds_t;
	int tempmask;
	int shift_value;
	int32_t temps, tempt;

	
	
	int overunder_s = 0, overunder_t = 0;
	
	
	if (SIGN16(sw) <= 0)
		w_carry = 1;

	sw &= 0x7fff;

	
	
	shift = tcdiv_table[sw];
	tlu_rcp = shift >> 4;
	shift &= 0xf;

	sprod = SIGN16(ss) * tlu_rcp;
	tprod = SIGN16(st) * tlu_rcp;

	
	
	
	tempmask = ((1 << 30) - 1) & -((1 << 29) >> shift);
	
	outofbounds_s = sprod & tempmask;
	outofbounds_t = tprod & tempmask;
	
	if (shift != 0xe)
	{
		shift_value = 13 - shift;
		temps = sprod = (sprod >> shift_value);
		tempt = tprod = (tprod >> shift_value);
	}
	else
	{
		temps = sprod << 1;
		tempt = tprod << 1;
	}
	
	if (outofbounds_s != tempmask && outofbounds_s != 0)
	{
		if (!(sprod & (1 << 29)))
			overunder_s = 2 << 17;
		else
			overunder_s = 1 << 17;
	}

	if (outofbounds_t != tempmask && outofbounds_t != 0)
	{
		if (!(tprod & (1 << 29)))
			overunder_t = 2 << 17;
		else
			overunder_t = 1 << 17;
	}

	if (w_carry)
	{
		overunder_s |= (2 << 17);
		overunder_t |= (2 << 17);
	}

	*sss = (temps & 0x1ffff) | overunder_s;
	*sst = (tempt & 0x1ffff) | overunder_t;
}

static inline void tclod_2cycle_current(int32_t* sss, int32_t* sst, int32_t nexts, int32_t nextt, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t prim_tile, int32_t* t1, int32_t* t2)
{








	int nextys, nextyt, nextysw;
	int lodclamp = 0;
	int32_t lod = 0;
	uint32_t l_tile;
	uint32_t magnify = 0;
	uint32_t distant = 0;
	int inits = *sss, initt = *sst;

	tclod_tcclamp(sss, sst);

	if (other_modes.f.dolod)
	{
		
		
		
		
		
		
		nextys = (s + spans_dstwzdy[0]) >> 16;
		nextyt = (t + spans_dstwzdy[1]) >> 16;
		nextysw = (w + spans_dstwzdy[2]) >> 16;

		tcdiv_ptr(nextys, nextyt, nextysw, &nextys, &nextyt);

		lodclamp = (initt & 0x60000) || (nextt & 0x60000) || (inits & 0x60000) || (nexts & 0x60000) || (nextys & 0x60000) || (nextyt & 0x60000);
		
		

		
		tclod_4x17_to_15(inits, nexts, initt, nextt, 0, &lod);
		tclod_4x17_to_15(inits, nextys, initt, nextyt, lod, &lod);

		lodfrac_lodtile_signals(lodclamp, lod, &l_tile, &magnify, &distant);

		
		if (other_modes.tex_lod_en)
		{
			if (distant)
				l_tile = max_level;
			if (!other_modes.detail_tex_en)
			{
				*t1 = (prim_tile + l_tile) & 7;
				if (!(distant || (!other_modes.sharpen_tex_en && magnify)))
					*t2 = (*t1 + 1) & 7;
				else
					*t2 = *t1;
			}
			else 
			{
				if (!magnify)
					*t1 = (prim_tile + l_tile + 1);
				else
					*t1 = (prim_tile + l_tile);
				*t1 &= 7;
				if (!distant && !magnify)
					*t2 = (prim_tile + l_tile + 2) & 7;
				else
					*t2 = (prim_tile + l_tile + 1) & 7;
			}
		}
	}
}

static inline void tclod_2cycle_current_simple_sse(int32_t ssst[2], __m128i stwz, __m128i dstwzinc, int32_t prim_tile, int32_t* t1, int32_t* t2) {
	int nextys, nextyt, nextysw, nexts, nextt, nextsw;
	int lodclamp = 0;
	int32_t lod = 0;
	uint32_t l_tile;
	uint32_t magnify = 0;
	uint32_t distant = 0;
	int inits = ssst[0], initt = ssst[1];

	tclod_tcclamp(&ssst[0], &ssst[1]);

	if (other_modes.f.dolod)
	{
    int temp[2][2] __attribute__((aligned(16)));
    int nextystw[4] __attribute__ ((aligned(16)));
    int nextstw[4] __attribute__ ((aligned(16)));

    __m128i nextstw_v = _mm_srai_epi32(_mm_add_epi32(stwz, dstwzinc), 16);
    __m128i nextystw_v = _mm_srai_epi32(_mm_add_epi32(stwz, _mm_load_si128(spans_dstwzdy)), 16);

    _mm_store_si128(nextystw, nextystw_v);
    _mm_store_si128(nextstw, nextstw_v);

    if (tcdiv_ptr == tcdiv_nopersp) {
      tcdiv_nopersp_sse(nextstw_v, temp[0]);
      tcdiv_nopersp_sse(nextystw_v, temp[1]);
    } else {
      tcdiv_persp_sse(nextstw_v, temp[0]);
      tcdiv_persp_sse(nextystw_v, temp[1]);
    }

    nextstw[0] = temp[0][0];
    nextt = temp[0][1];
    nextystw[0] = temp[1][0];
    nextyt = temp[1][1];

#if 0
		tcdiv_ptr(nextstw[0], nextstw[1], nextstw[2], &nextstw[0], &nextt);
		tcdiv_ptr(nextystw[0], nextystw[1], nextystw[2], &nextystw[0], &nextyt);
#endif

		lodclamp = (initt & 0x60000) || (nextt & 0x60000) || (inits & 0x60000) || (nexts & 0x60000) || (nextys & 0x60000) || (nextyt & 0x60000);

		tclod_4x17_to_15(inits, nextstw[0], initt, nextstw[1], 0, &lod);
		tclod_4x17_to_15(inits, nextystw[0], initt, nextystw[1], lod, &lod);

		lodfrac_lodtile_signals(lodclamp, lod, &l_tile, &magnify, &distant);
	
		if (other_modes.tex_lod_en)
		{
			if (distant)
				l_tile = max_level;
			if (!other_modes.detail_tex_en)
			{
				*t1 = (prim_tile + l_tile) & 7;
				if (!(distant || (!other_modes.sharpen_tex_en && magnify)))
					*t2 = (*t1 + 1) & 7;
				else
					*t2 = *t1;
			}
			else 
			{
				if (!magnify)
					*t1 = (prim_tile + l_tile + 1);
				else
					*t1 = (prim_tile + l_tile);
				*t1 &= 7;
				if (!distant && !magnify)
					*t2 = (prim_tile + l_tile + 2) & 7;
				else
					*t2 = (prim_tile + l_tile + 1) & 7;
			}
		}
	}
}

static inline void tclod_2cycle_current_simple(int32_t* sss, int32_t* sst, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t prim_tile, int32_t* t1, int32_t* t2)
{
	int nextys, nextyt, nextysw, nexts, nextt, nextsw;
	int lodclamp = 0;
	int32_t lod = 0;
	uint32_t l_tile;
	uint32_t magnify = 0;
	uint32_t distant = 0;
	int inits = *sss, initt = *sst;

	tclod_tcclamp(sss, sst);

	if (other_modes.f.dolod)
	{
		nextsw = (w + dwinc) >> 16;
		nexts = (s + dsinc) >> 16;
		nextt = (t + dtinc) >> 16;
		nextys = (s + spans_dstwzdy[0]) >> 16;
		nextyt = (t + spans_dstwzdy[1]) >> 16;
		nextysw = (w + spans_dstwzdy[2]) >> 16;

		tcdiv_ptr(nexts, nextt, nextsw, &nexts, &nextt);
		tcdiv_ptr(nextys, nextyt, nextysw, &nextys, &nextyt);

		lodclamp = (initt & 0x60000) || (nextt & 0x60000) || (inits & 0x60000) || (nexts & 0x60000) || (nextys & 0x60000) || (nextyt & 0x60000);

		tclod_4x17_to_15(inits, nexts, initt, nextt, 0, &lod);
		tclod_4x17_to_15(inits, nextys, initt, nextyt, lod, &lod);

		lodfrac_lodtile_signals(lodclamp, lod, &l_tile, &magnify, &distant);
	
		if (other_modes.tex_lod_en)
		{
			if (distant)
				l_tile = max_level;
			if (!other_modes.detail_tex_en)
			{
				*t1 = (prim_tile + l_tile) & 7;
				if (!(distant || (!other_modes.sharpen_tex_en && magnify)))
					*t2 = (*t1 + 1) & 7;
				else
					*t2 = *t1;
			}
			else 
			{
				if (!magnify)
					*t1 = (prim_tile + l_tile + 1);
				else
					*t1 = (prim_tile + l_tile);
				*t1 &= 7;
				if (!distant && !magnify)
					*t2 = (prim_tile + l_tile + 2) & 7;
				else
					*t2 = (prim_tile + l_tile + 1) & 7;
			}
		}
	}
}


static inline void tclod_2cycle_current_notexel1(int32_t* sss, int32_t* sst, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t prim_tile, int32_t* t1)
{
	int nextys, nextyt, nextysw, nexts, nextt, nextsw;
	int lodclamp = 0;
	int32_t lod = 0;
	uint32_t l_tile;
	uint32_t magnify = 0;
	uint32_t distant = 0;
	int inits = *sss, initt = *sst;

	tclod_tcclamp(sss, sst);

	if (other_modes.f.dolod)
	{
		nextsw = (w + dwinc) >> 16;
		nexts = (s + dsinc) >> 16;
		nextt = (t + dtinc) >> 16;
		nextys = (s + spans_dstwzdy[0]) >> 16;
		nextyt = (t + spans_dstwzdy[1]) >> 16;
		nextysw = (w + spans_dstwzdy[2]) >> 16;

		tcdiv_ptr(nexts, nextt, nextsw, &nexts, &nextt);
		tcdiv_ptr(nextys, nextyt, nextysw, &nextys, &nextyt);

		lodclamp = (initt & 0x60000) || (nextt & 0x60000) || (inits & 0x60000) || (nexts & 0x60000) || (nextys & 0x60000) || (nextyt & 0x60000);

		tclod_4x17_to_15(inits, nexts, initt, nextt, 0, &lod);
		tclod_4x17_to_15(inits, nextys, initt, nextyt, lod, &lod);

		lodfrac_lodtile_signals(lodclamp, lod, &l_tile, &magnify, &distant);
	
		if (other_modes.tex_lod_en)
		{
			if (distant)
				l_tile = max_level;
			if (!other_modes.detail_tex_en || magnify)
				*t1 = (prim_tile + l_tile) & 7;
			else
				*t1 = (prim_tile + l_tile + 1) & 7;
		}
		
	}
}

static inline void tclod_2cycle_next(int32_t* sss, int32_t* sst, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t prim_tile, int32_t* t1, int32_t* t2, int32_t* prelodfrac)
{
	int nexts, nextt, nextsw, nextys, nextyt, nextysw;
	int lodclamp = 0;
	int32_t lod = 0;
	uint32_t l_tile;
	uint32_t magnify = 0;
	uint32_t distant = 0;
	int inits = *sss, initt = *sst;

	tclod_tcclamp(sss, sst);

	if (other_modes.f.dolod)
	{
		nextsw = (w + dwinc) >> 16;
		nexts = (s + dsinc) >> 16;
		nextt = (t + dtinc) >> 16;
		nextys = (s + spans_dstwzdy[0]) >> 16;
		nextyt = (t + spans_dstwzdy[1]) >> 16;
		nextysw = (w + spans_dstwzdy[2]) >> 16;

		tcdiv_ptr(nexts, nextt, nextsw, &nexts, &nextt);
		tcdiv_ptr(nextys, nextyt, nextysw, &nextys, &nextyt);
	
		lodclamp = (initt & 0x60000) || (nextt & 0x60000) || (inits & 0x60000) || (nexts & 0x60000) || (nextys & 0x60000) || (nextyt & 0x60000);

		tclod_4x17_to_15(inits, nexts, initt, nextt, 0, &lod);
		tclod_4x17_to_15(inits, nextys, initt, nextyt, lod, &lod);

		
		if ((lod & 0x4000) || lodclamp)
			lod = 0x7fff;
		else if (lod < min_level)
			lod = min_level;
						
		magnify = (lod < 32) ? 1: 0;
		l_tile =  log2table[(lod >> 5) & 0xff];
		distant = ((lod & 0x6000) || (l_tile >= max_level)) ? 1 : 0;

		*prelodfrac = ((lod << 3) >> l_tile) & 0xff;

		
		if(!other_modes.sharpen_tex_en && !other_modes.detail_tex_en)
		{
			if (distant)
				*prelodfrac = 0xff;
			else if (magnify)
				*prelodfrac = 0;
		}

		
		

		if(other_modes.sharpen_tex_en && magnify)
			*prelodfrac |= 0x100;

		if (other_modes.tex_lod_en)
		{
			if (distant)
				l_tile = max_level;
			if (!other_modes.detail_tex_en)
			{
				*t1 = (prim_tile + l_tile) & 7;
				if (!(distant || (!other_modes.sharpen_tex_en && magnify)))
					*t2 = (*t1 + 1) & 7;
				else
					*t2 = *t1;
			}
			else 
			{
				if (!magnify)
					*t1 = (prim_tile + l_tile + 1);
				else
					*t1 = (prim_tile + l_tile);
				*t1 &= 7;
				if (!distant && !magnify)
					*t2 = (prim_tile + l_tile + 2) & 7;
				else
					*t2 = (prim_tile + l_tile + 1) & 7;
			}
		}
	}
}

static inline void tclod_1cycle_current(int32_t* sss, int32_t* sst, int32_t nexts, int32_t nextt, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t scanline, int32_t prim_tile, int32_t* t1, SPANSIGS* sigs)
{









	int fars, fart, farsw;
	int lodclamp = 0;
	int32_t lod = 0;
	uint32_t l_tile = 0, magnify = 0, distant = 0;
	
	tclod_tcclamp(sss, sst);

	if (other_modes.f.dolod)
	{
		int nextscan = scanline + 1;

		
		if (span[nextscan].validline)
		{
			if (!sigs->endspan || !sigs->longspan)
			{
				if (!(sigs->preendspan && sigs->longspan) && !(sigs->endspan && sigs->midspan))
				{
					farsw = (w + (dwinc << 1)) >> 16;
					fars = (s + (dsinc << 1)) >> 16;
					fart = (t + (dtinc << 1)) >> 16;
				}
				else
				{
					farsw = (w - dwinc) >> 16;
					fars = (s - dsinc) >> 16;
					fart = (t - dtinc) >> 16;
				}
			}
			else
			{
				fart = (span[nextscan].rgbastwz[5] + dtinc) >> 16; 
				fars = (span[nextscan].rgbastwz[4] + dsinc) >> 16; 
				farsw = (span[nextscan].rgbastwz[6] + dwinc) >> 16;
			}
		}
		else
		{
			farsw = (w + (dwinc << 1)) >> 16;
			fars = (s + (dsinc << 1)) >> 16;
			fart = (t + (dtinc << 1)) >> 16;
		}

		tcdiv_ptr(fars, fart, farsw, &fars, &fart);

		lodclamp = (fart & 0x60000) || (nextt & 0x60000) || (fars & 0x60000) || (nexts & 0x60000);
		
		

		
		tclod_4x17_to_15(nexts, fars, nextt, fart, 0, &lod);

		lodfrac_lodtile_signals(lodclamp, lod, &l_tile, &magnify, &distant);
	
		if (other_modes.tex_lod_en)
		{
			if (distant)
				l_tile = max_level;

			
			
			if (!other_modes.detail_tex_en || magnify)
				*t1 = (prim_tile + l_tile) & 7;
			else
				*t1 = (prim_tile + l_tile + 1) & 7;
		}
	}
}



static inline void tclod_1cycle_current_simple(int32_t* sss, int32_t* sst, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t scanline, int32_t prim_tile, int32_t* t1, SPANSIGS* sigs)
{
	int fars, fart, farsw, nexts, nextt, nextsw;
	int lodclamp = 0;
	int32_t lod = 0;
	uint32_t l_tile = 0, magnify = 0, distant = 0;
	
	tclod_tcclamp(sss, sst);

	if (other_modes.f.dolod)
	{

		int nextscan = scanline + 1;
		if (span[nextscan].validline)
		{
			if (!sigs->endspan || !sigs->longspan)
			{
				nextsw = (w + dwinc) >> 16;
				nexts = (s + dsinc) >> 16;
				nextt = (t + dtinc) >> 16;
				
				if (!(sigs->preendspan && sigs->longspan) && !(sigs->endspan && sigs->midspan))
				{
					farsw = (w + (dwinc << 1)) >> 16;
					fars = (s + (dsinc << 1)) >> 16;
					fart = (t + (dtinc << 1)) >> 16;
				}
				else
				{
					farsw = (w - dwinc) >> 16;
					fars = (s - dsinc) >> 16;
					fart = (t - dtinc) >> 16;
				}
			}
			else
			{
				nextt = span[nextscan].rgbastwz[5] >> 16;
				nexts = span[nextscan].rgbastwz[4] >> 16;
				nextsw = span[nextscan].rgbastwz[6] >> 16;
				fart = (span[nextscan].rgbastwz[5] + dtinc) >> 16; 
				fars = (span[nextscan].rgbastwz[4] + dsinc) >> 16; 
				farsw = (span[nextscan].rgbastwz[6] + dwinc) >> 16;
			}
		}
		else
		{
			nextsw = (w + dwinc) >> 16;
			nexts = (s + dsinc) >> 16;
			nextt = (t + dtinc) >> 16;
			farsw = (w + (dwinc << 1)) >> 16;
			fars = (s + (dsinc << 1)) >> 16;
			fart = (t + (dtinc << 1)) >> 16;
		}

		tcdiv_ptr(nexts, nextt, nextsw, &nexts, &nextt);
		tcdiv_ptr(fars, fart, farsw, &fars, &fart);

		lodclamp = (fart & 0x60000) || (nextt & 0x60000) || (fars & 0x60000) || (nexts & 0x60000);

		tclod_4x17_to_15(nexts, fars, nextt, fart, 0, &lod);

		lodfrac_lodtile_signals(lodclamp, lod, &l_tile, &magnify, &distant);
	
		if (other_modes.tex_lod_en)
		{
			if (distant)
				l_tile = max_level;
			if (!other_modes.detail_tex_en || magnify)
				*t1 = (prim_tile + l_tile) & 7;
			else
				*t1 = (prim_tile + l_tile + 1) & 7;
		}
	}
}

static inline void tclod_1cycle_next(int32_t* sss, int32_t* sst, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t scanline, int32_t prim_tile, int32_t* t1, SPANSIGS* sigs, int32_t* prelodfrac)
{
	int nexts, nextt, nextsw, fars, fart, farsw;
	int lodclamp = 0;
	int32_t lod = 0;
	uint32_t l_tile = 0, magnify = 0, distant = 0;
	
	tclod_tcclamp(sss, sst);

	if (other_modes.f.dolod)
	{
		
		int nextscan = scanline + 1;
		
		if (span[nextscan].validline)
		{
			if (!sigs->nextspan)
			{
				if (!sigs->endspan || !sigs->longspan)
				{
					nextsw = (w + dwinc) >> 16;
					nexts = (s + dsinc) >> 16;
					nextt = (t + dtinc) >> 16;
					
					if (!(sigs->preendspan && sigs->longspan) && !(sigs->endspan && sigs->midspan))
					{
						farsw = (w + (dwinc << 1)) >> 16;
						fars = (s + (dsinc << 1)) >> 16;
						fart = (t + (dtinc << 1)) >> 16;
					}
					else
					{
						farsw = (w - dwinc) >> 16;
						fars = (s - dsinc) >> 16;
						fart = (t - dtinc) >> 16;
					}
				}
				else
				{
					nextt = span[nextscan].rgbastwz[5];
					nexts = span[nextscan].rgbastwz[4];
					nextsw = span[nextscan].rgbastwz[6];
					fart = (nextt + dtinc) >> 16; 
					fars = (nexts + dsinc) >> 16; 
					farsw = (nextsw + dwinc) >> 16;
					nextt >>= 16;
					nexts >>= 16;
					nextsw >>= 16;
				}
			}
			else
			{
				if (!sigs->onelessthanmid)
				{
					nextt = span[nextscan].rgbastwz[5] + dtinc;
					nexts = span[nextscan].rgbastwz[4] + dsinc;
					nextsw = span[nextscan].rgbastwz[6] + dwinc;
					fart = (nextt + dtinc) >> 16; 
					fars = (nexts + dsinc) >> 16; 
					farsw = (nextsw + dwinc) >> 16;
					nextt >>= 16;
					nexts >>= 16;
					nextsw >>= 16;
				}
				else
				{
					nextsw = (w + dwinc) >> 16;
					nexts = (s + dsinc) >> 16;
					nextt = (t + dtinc) >> 16;
					farsw = (w - dwinc) >> 16;
					fars = (s - dsinc) >> 16;
					fart = (t - dtinc) >> 16;
				}
			}
		}
		else
		{
			nextsw = (w + dwinc) >> 16;
			nexts = (s + dsinc) >> 16;
			nextt = (t + dtinc) >> 16;
			farsw = (w + (dwinc << 1)) >> 16;
			fars = (s + (dsinc << 1)) >> 16;
			fart = (t + (dtinc << 1)) >> 16;
		}

		tcdiv_ptr(nexts, nextt, nextsw, &nexts, &nextt);
		tcdiv_ptr(fars, fart, farsw, &fars, &fart);

		lodclamp = (fart & 0x60000) || (nextt & 0x60000) || (fars & 0x60000) || (nexts & 0x60000);
		
		
		tclod_4x17_to_15(nexts, fars, nextt, fart, 0, &lod);

		
		if ((lod & 0x4000) || lodclamp)
			lod = 0x7fff;
		else if (lod < min_level)
			lod = min_level;
					
		magnify = (lod < 32) ? 1: 0;
		l_tile =  log2table[(lod >> 5) & 0xff];
		distant = ((lod & 0x6000) || (l_tile >= max_level)) ? 1 : 0;

		*prelodfrac = ((lod << 3) >> l_tile) & 0xff;

		
		if(!other_modes.sharpen_tex_en && !other_modes.detail_tex_en)
		{
			if (distant)
				*prelodfrac = 0xff;
			else if (magnify)
				*prelodfrac = 0;
		}

		if(other_modes.sharpen_tex_en && magnify)
			*prelodfrac |= 0x100;

		if (other_modes.tex_lod_en)
		{
			if (distant)
				l_tile = max_level;
			if (!other_modes.detail_tex_en || magnify)
				*t1 = (prim_tile + l_tile) & 7;
			else
				*t1 = (prim_tile + l_tile + 1) & 7;
		}
	}
}

static inline void tclod_copy(int32_t* sss, int32_t* sst, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t prim_tile, int32_t* t1)
{




	int nexts, nextt, nextsw, fars, fart, farsw;
	int lodclamp = 0;
	int32_t lod = 0;
	uint32_t l_tile = 0, magnify = 0, distant = 0;

	tclod_tcclamp(sss, sst);

	if (other_modes.tex_lod_en)
	{
		
		
		
		nextsw = (w + dwinc) >> 16;
		nexts = (s + dsinc) >> 16;
		nextt = (t + dtinc) >> 16;
		farsw = (w + (dwinc << 1)) >> 16;
		fars = (s + (dsinc << 1)) >> 16;
		fart = (t + (dtinc << 1)) >> 16;
	
		tcdiv_ptr(nexts, nextt, nextsw, &nexts, &nextt);
		tcdiv_ptr(fars, fart, farsw, &fars, &fart);

		lodclamp = (fart & 0x60000) || (nextt & 0x60000) || (fars & 0x60000) || (nexts & 0x60000);

		tclod_4x17_to_15(nexts, fars, nextt, fart, 0, &lod);

		if ((lod & 0x4000) || lodclamp)
			lod = 0x7fff;
		else if (lod < min_level)
			lod = min_level;
						
		magnify = (lod < 32) ? 1: 0;
		l_tile =  log2table[(lod >> 5) & 0xff];
		distant = ((lod & 0x6000) || (l_tile >= max_level)) ? 1 : 0;

		if (distant)
			l_tile = max_level;
	
		if (!other_modes.detail_tex_en || magnify)
			*t1 = (prim_tile + l_tile) & 7;
		else
			*t1 = (prim_tile + l_tile + 1) & 7;
	}

}

static inline void get_texel1_1cycle(int32_t* s1, int32_t* t1, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc, int32_t scanline, SPANSIGS* sigs)
{
	int32_t nexts, nextt, nextsw;
	
	if (!sigs->endspan || !sigs->longspan || !span[scanline + 1].validline)
	{
	
	
		nextsw = (w + dwinc) >> 16;
		nexts = (s + dsinc) >> 16;
		nextt = (t + dtinc) >> 16;
	}
	else
	{
		int32_t nextscan = scanline + 1;
		nextt = span[nextscan].rgbastwz[5] >> 16;
		nexts = span[nextscan].rgbastwz[4] >> 16;
		nextsw = span[nextscan].rgbastwz[6] >> 16;
	}

	tcdiv_ptr(nexts, nextt, nextsw, s1, t1);
}

static inline void get_nexttexel0_2cycle(int32_t* s1, int32_t* t1, int32_t s, int32_t t, int32_t w, int32_t dsinc, int32_t dtinc, int32_t dwinc)
{
	int32_t nexts, nextt, nextsw;
	nextsw = (w + dwinc) >> 16;
	nexts = (s + dsinc) >> 16;
	nextt = (t + dtinc) >> 16;

	tcdiv_ptr(nexts, nextt, nextsw, s1, t1);
}



static inline void tclod_4x17_to_15(int32_t scurr, int32_t snext, int32_t tcurr, int32_t tnext, int32_t previous, int32_t* lod)
{



	int dels = SIGN(snext, 17) - SIGN(scurr, 17);
	if (dels & 0x20000)
		dels = ~dels & 0x1ffff;
	int delt = SIGN(tnext, 17) - SIGN(tcurr, 17);
	if(delt & 0x20000)
		delt = ~delt & 0x1ffff;
	

	dels = (dels > delt) ? dels : delt;
	dels = (previous > dels) ? previous : dels;
	*lod = dels & 0x7fff;
	if (dels & 0x1c000)
		*lod |= 0x4000;
}

static inline void tclod_tcclamp(int32_t* sss, int32_t* sst)
{
	int32_t tempanded, temps = *sss, tempt = *sst;

	
	
	
	
	if (!(temps & 0x40000))
	{
		if (!(temps & 0x20000))
		{
			tempanded = temps & 0x18000;
			if (tempanded != 0x8000)
			{
				if (tempanded != 0x10000)
					*sss &= 0xffff;
				else
					*sss = 0x8000;
			}
			else
				*sss = 0x7fff;
		}
		else
			*sss = 0x8000;
	}
	else
		*sss = 0x7fff;

	if (!(tempt & 0x40000))
	{
		if (!(tempt & 0x20000))
		{
			tempanded = tempt & 0x18000;
			if (tempanded != 0x8000)
			{
				if (tempanded != 0x10000)
					*sst &= 0xffff;
				else
					*sst = 0x8000;
			}
			else
				*sst = 0x7fff;
		}
		else
			*sst = 0x8000;
	}
	else
		*sst = 0x7fff;

}


static inline void lodfrac_lodtile_signals(int lodclamp, int32_t lod, uint32_t* l_tile, uint32_t* magnify, uint32_t* distant)
{
	uint32_t ltil, dis, mag;
	int32_t lf;

	
	if ((lod & 0x4000) || lodclamp)
		lod = 0x7fff;
	else if (lod < min_level)
		lod = min_level;
						
	mag = (lod < 32) ? 1: 0;
	ltil=  log2table[(lod >> 5) & 0xff];
	dis = ((lod & 0x6000) || (ltil >= max_level)) ? 1 : 0;
						
	lf = ((lod << 3) >> ltil) & 0xff;

	
	if(!other_modes.sharpen_tex_en && !other_modes.detail_tex_en)
	{
		if (dis)
			lf = 0xff;
		else if (mag)
			lf = 0;
	}

	
	

	if(other_modes.sharpen_tex_en && mag)
		lf |= 0x100;

	*distant = dis;
	*l_tile = ltil;
	*magnify = mag;
	lod_frac = lf;
}

