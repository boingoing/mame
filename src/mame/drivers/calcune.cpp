// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    Calcune (Japan, prototype)

    CPUs are HD68HC000CP8 and TMPZ84C00AU-6 QFP types. Other ICs include
    two Sega 315-5660-02 VDPs and a YMZ280B-F for sound.

    Oscillators: 53.693MHz (OSC1), 16.9444 (XL1), 14.318 (XL3).

    Printed on board: "VDP LICENSE FROM SEGA, MANUFACTURED BY ADO."

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "sound/ymz280b.h"
#include "video/315_5313.h"
#include "machine/nvram.h"
#include "emupal.h"
#include "speaker.h"

#define MASTER_CLOCK_NTSC 53693175

class calcune_state : public driver_device
{
public:
	calcune_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		vdp_state(0),
		m_maincpu(*this, "maincpu"),
		m_vdp(*this, "gen_vdp"),
		m_vdp2(*this, "gen_vdp2"),
		m_palette(*this, "palette")
	{ }

	WRITE_LINE_MEMBER(vdp_sndirqline_callback_genesis_z80);
	WRITE_LINE_MEMBER(vdp_lv6irqline_callback_genesis_68k);
	WRITE_LINE_MEMBER(vdp_lv4irqline_callback_genesis_68k);

	DECLARE_MACHINE_START(calcune);
	DECLARE_MACHINE_RESET(calcune);

	IRQ_CALLBACK_MEMBER(genesis_int_callback);

	void init_calcune();

	DECLARE_READ16_MEMBER(cal_700000_r);
	DECLARE_WRITE16_MEMBER(cal_770000_w);
	DECLARE_READ16_MEMBER(cal_vdp_r);
	DECLARE_WRITE16_MEMBER(cal_vdp_w);

	uint32_t screen_update_calcune(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void calcune(machine_config &config);
	void calcune_map(address_map &map);
private:
	int vdp_state;

	required_device<cpu_device> m_maincpu;
	required_device<sega315_5313_device> m_vdp;
	required_device<sega315_5313_device> m_vdp2;
	required_device<palette_device> m_palette;
};


static INPUT_PORTS_START( calcune )
	PORT_START("710000")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0200, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("720000")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xffc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


READ16_MEMBER(calcune_state::cal_700000_r)
{
	vdp_state = 0;
	return 0;
}

WRITE16_MEMBER(calcune_state::cal_770000_w)
{
	vdp_state = data;
}

READ16_MEMBER(calcune_state::cal_vdp_r)
{
	if (!vdp_state)
		return m_vdp->vdp_r(space, offset, mem_mask);
	else
		return m_vdp2->vdp_r(space, offset, mem_mask);
}

WRITE16_MEMBER(calcune_state::cal_vdp_w)
{
	if (!vdp_state)
		m_vdp->vdp_w(space, offset, data, mem_mask);
	else
		m_vdp2->vdp_w(space, offset, data, mem_mask);
}

void calcune_state::calcune_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();

	map(0x700000, 0x700001).r(FUNC(calcune_state::cal_700000_r));

	map(0x710000, 0x710001).portr("710000");
	map(0x720000, 0x720001).portr("720000");
//  AM_RANGE(0x730000, 0x730001) possible Z80 control?
	map(0x760000, 0x760003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0xff00);

	map(0x770000, 0x770001).w(FUNC(calcune_state::cal_770000_w));

	map(0xA14100, 0xA14101).noprw(); // on startup, possible z80 control

	map(0xc00000, 0xc0001f).rw(FUNC(calcune_state::cal_vdp_r), FUNC(calcune_state::cal_vdp_w));

	map(0xff0000, 0xffffff).ram().share("nvram"); // battery on PCB
}


uint32_t calcune_state::screen_update_calcune(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *paldata = m_palette->pens();

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t *dst = &bitmap.pix32(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int pix;

			pix = m_vdp2->m_render_line_raw[x] & 0x3f;

			switch (pix & 0xc0)
			{
			case 0x00:
				dst[x] = paldata[pix + 0x0000 + 0xc0];
				break;
			case 0x40:
			case 0x80:
				dst[x] = paldata[pix + 0x0040 + 0xc0];
				break;
			case 0xc0:
				dst[x] = paldata[pix + 0x0080 + 0xc0];
				break;

			}

			if (m_vdp->m_render_line_raw[x] & 0x100)
			{
				pix = m_vdp->m_render_line_raw[x] & 0x3f;
				if (pix & 0xf)
				{
					switch (pix & 0xc0)
					{
					case 0x00:
						dst[x] = paldata[pix + 0x0000];
						break;
					case 0x40:
					case 0x80:
						dst[x] = paldata[pix + 0x0040];
						break;
					case 0xc0:
						dst[x] = paldata[pix + 0x0080];
						break;
					}
				}
			}
		}
	}

	return 0;
}

MACHINE_RESET_MEMBER(calcune_state,calcune)
{
	m_vdp->device_reset_old();
	m_vdp2->device_reset_old();
}


IRQ_CALLBACK_MEMBER(calcune_state::genesis_int_callback)
{
	if (irqline==4)
	{
		m_vdp->vdp_clear_irq4_pending();
	}

	if (irqline==6)
	{
		m_vdp->vdp_clear_irq6_pending();
	}

	return (0x60+irqline*4)/4; // vector address
}

WRITE_LINE_MEMBER(calcune_state::vdp_sndirqline_callback_genesis_z80)
{
}

WRITE_LINE_MEMBER(calcune_state::vdp_lv6irqline_callback_genesis_68k)
{
	// this looks odd but is the logic the Genesis code requires
	if (state == ASSERT_LINE)
		m_maincpu->set_input_line(6, HOLD_LINE);
	else
		m_maincpu->set_input_line(6, CLEAR_LINE);
}

WRITE_LINE_MEMBER(calcune_state::vdp_lv4irqline_callback_genesis_68k)
{
	// this looks odd but is the logic the Genesis code requires
	if (state == ASSERT_LINE)
		m_maincpu->set_input_line(4, HOLD_LINE);
	else
		m_maincpu->set_input_line(4, CLEAR_LINE);
}

MACHINE_START_MEMBER(calcune_state,calcune)
{
	m_vdp->stop_timers();
	m_vdp2->stop_timers();
}

MACHINE_CONFIG_START(calcune_state::calcune)
	MCFG_DEVICE_ADD("maincpu", M68000, MASTER_CLOCK_NTSC / 7) /* 7.67 MHz */
	MCFG_DEVICE_PROGRAM_MAP(calcune_map)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DRIVER(calcune_state,genesis_int_callback)

	MCFG_DEVICE_ADD("z80", Z80, MASTER_CLOCK_NTSC / 15) /* 3.58 MHz */
	MCFG_DEVICE_DISABLE() /* no code is ever uploaded for the Z80, so it's unused here even if it is present on the PCB */

	MCFG_MACHINE_START_OVERRIDE(calcune_state,calcune)
	MCFG_MACHINE_RESET_OVERRIDE(calcune_state,calcune)

	MCFG_SCREEN_ADD("megadriv", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)) // Vblank handled manually.
	MCFG_SCREEN_SIZE(64*8, 620)
	MCFG_SCREEN_VISIBLE_AREA(0, 40*8-1, 0, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(calcune_state, screen_update_calcune)

	MCFG_DEVICE_ADD("gen_vdp", SEGA315_5313, MASTER_CLOCK_NTSC, "maincpu")
	MCFG_SEGA315_5313_IS_PAL(false)
	MCFG_SEGA315_5313_SND_IRQ_CALLBACK(WRITELINE(*this, calcune_state, vdp_sndirqline_callback_genesis_z80));
	MCFG_SEGA315_5313_LV6_IRQ_CALLBACK(WRITELINE(*this, calcune_state, vdp_lv6irqline_callback_genesis_68k));
	MCFG_SEGA315_5313_LV4_IRQ_CALLBACK(WRITELINE(*this, calcune_state, vdp_lv4irqline_callback_genesis_68k));
	MCFG_SEGA315_5313_ALT_TIMING(1);
	MCFG_SEGA315_5313_PAL_WRITE_BASE(0x0000);
	MCFG_SEGA315_5313_PALETTE("palette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker",0.25)

	MCFG_DEVICE_ADD("gen_vdp2", SEGA315_5313, MASTER_CLOCK_NTSC, "maincpu")
	MCFG_SEGA315_5313_IS_PAL(false)
//  are these not hooked up or should they OR with the other lines?
//  MCFG_SEGA315_5313_SND_IRQ_CALLBACK(WRITELINE(*this, calcune_state, vdp_sndirqline_callback_genesis_z80));
//  MCFG_SEGA315_5313_LV6_IRQ_CALLBACK(WRITELINE(*this, calcune_state, vdp_lv6irqline_callback_genesis_68k));
//  MCFG_SEGA315_5313_LV4_IRQ_CALLBACK(WRITELINE(*this, calcune_state, vdp_lv4irqline_callback_genesis_68k));
	MCFG_SEGA315_5313_ALT_TIMING(1);
	MCFG_SEGA315_5313_PAL_WRITE_BASE(0x0c0);
	MCFG_SEGA315_5313_PALETTE("palette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker",0.25)

	MCFG_TIMER_DEVICE_ADD_SCANLINE("scantimer", "gen_vdp", sega315_5313_device, megadriv_scanline_timer_callback_alt_timing, "megadriv", 0, 1)
	MCFG_TIMER_DEVICE_ADD_SCANLINE("scantimer2", "gen_vdp2", sega315_5313_device, megadriv_scanline_timer_callback_alt_timing, "megadriv", 0, 1)

	MCFG_PALETTE_ADD("palette", 0xc0*2)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MCFG_DEVICE_ADD("ymz", YMZ280B, XTAL(16'934'400))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

void calcune_state::init_calcune()
{
	m_vdp->set_use_cram(1);
	m_vdp->set_vdp_pal(false);
	m_vdp->set_framerate(60);
	m_vdp->set_total_scanlines(262);

	m_vdp2->set_use_cram(1);
	m_vdp2->set_vdp_pal(false);
	m_vdp2->set_framerate(60);
	m_vdp2->set_total_scanlines(262);
}

ROM_START( calcune )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* 68000 Code - ROM check fails, but probably due to prototype status, not bad ROM */
	ROM_LOAD16_BYTE( "1.ic101.27c4001", 0x000001, 0x080000, CRC(2d25544c) SHA1(ef778cbf2aa4fcec43ce2dce9bcefb964f458c0a) )
	ROM_LOAD16_BYTE( "2.ic102.27c4001", 0x000000, 0x080000, CRC(09330dc9) SHA1(fd888a7469a6290cd372a4b1eed577c2444f0c80) )
	ROM_LOAD16_BYTE( "3.ic103.27c4001", 0x100001, 0x080000, CRC(736a356d) SHA1(372fa5989a67746efc439d76df1733cf70df57e7) )
	ROM_LOAD16_BYTE( "4.ic104.27c4001", 0x100000, 0x080000, CRC(0bec031a) SHA1(69b255743f20408b2f14bddf0b85c85a13f29615) )

	ROM_REGION( 0x200000, "ymz", 0 )
	ROM_LOAD( "sound1.1c.27c4001", 0x000000, 0x080000, CRC(5fb63e84) SHA1(6fad8e76162c81b2cfa4778effb81b78ed4fa299) )
	ROM_LOAD( "sound2.2d.27c4001", 0x080000, 0x080000, CRC(8924c6cc) SHA1(c76e6cfcf92a2c2834de62d7136c69e6edda46cc) )
	ROM_LOAD( "sound3.3a.27c4001", 0x100000, 0x080000, CRC(18cfa7f4) SHA1(201ea186eb3af9138db6699c9dcf527795f7c0db) )
	ROM_LOAD( "sound4.4b.27c4001", 0x180000, 0x080000, CRC(61a8510b) SHA1(177e56c374aa5697545ede28140cb42b5a4b737b) )
ROM_END



GAME( 1996, calcune, 0, calcune, calcune, calcune_state, init_calcune, ROT0, "Yuvo", "Calcune (Japan, prototype)", 0 )
