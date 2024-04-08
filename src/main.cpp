#include "evlk_Terminal.h"
#include "evlk_Terminal_fontImpl.h"
#include "evlk_cli.h"
#include "evlk_Shell.h"
#include "evlk_stand_syscli.h"
#include "CStream.h"
#include "def_vt100.h"
using namespace _EVLK_TERMINAL_;
using namespace _EVLK_SHELL_;

#include "Wire.h"
#include "SPI.h"
#include "OneButtonTiny.h"
#include "Arduino.h"

#define Ed2 26
#define Ed3 15
#define Ed4 2
#define Ed5 0
#define Ed6 4
#define Ed7 19
#define Ed8 23
#define Ed9 18
#define Ed10 5
#define BAT_PIN 34
#define CHG_PIN 33
#define SDA_PIN 21
#define SCL_PIN 22
#define GPIO_BTN_PIN GPIO_NUM_14
#define BTN_PIN 14
#define BTNU_PIN 27
#define BTND_PIN 12
#define RGB_PIN 13
#define LSMINT1_PIN 35
#define LSMINT2_PIN 32
#define PCFINT_PIN 25

#define O_EN Ed2
#define O_IR Ed3
#define O_IE Ed4
#define O_BUZ Ed7
#define O_GPSEN Ed8
#define O_GPSTX Ed9
#define O_GPSRX Ed10
#define O_SDA SDA_PIN
#define O_SCL SCL_PIN

#define en_rtc
#define en_lsm
#define en_rgb

#define en_ExpanBoard
// #define en_beep
#define en_oled

#define timer(time) \
	for (unsigned long start = millis(), pass = 0; pass < time; pass = millis() - start)
#define test(num) Serial.print(num)
#define beep_fre 300

// ********************CLI_TOOLS*********************

#define cli_s [](Shell & sh) -> int
#define cli_c [](Shell & sh, int argc) -> int
#define cli_cv [](Shell & sh, int argc, char *argv[]) -> int
#define cli_cve [](Shell & sh, int argc, char *argv[], char *envp[]) -> int
#define clib(name) cli name(#name,cli_cv
#define clie )
#define set_eat                                                   \
	auto eat = [&argc, &argv]() -> char * {if (!argc) return NULL;argc--;argv++;return *(argv - 1); };                    \
	auto eat_ = [&argc, &argv, eat](const char *key) -> bool {if (!argc || String(argv[0]) != key)return false;return eat(); }; \
	eat()
#define Arg(key, num) eat_(key) && argc == num

// ***********************BTN************************
struct btnE
{
	enum dim
	{
		up,
		center,
		down,
	} Dim;
	enum op
	{
		click,
		dclick,
		mclick,
		pressb,
		press,
		presse
	} Op;
	bool change = false;
	bool get()
	{
		if (!change)
			return false;
		change = false;
		return true;
	};
	void set(dim dim, op op)
	{
		change = true;
		this->Dim = dim;
		this->Op = op;
	}
} btnE;
void btn_c_click() { btnE.set(btnE::center, btnE::click); };
void btn_c_dclick() { btnE.set(btnE::center, btnE::dclick); };
void btn_u_click() { btnE.set(btnE::up, btnE::click); };
void btn_d_click() { btnE.set(btnE::down, btnE::click); }
#define c_click btnE.Dim == btnE::center &&btnE.Op == btnE::click
#define c_dclick btnE.Dim == btnE::center &&btnE.Op == btnE::dclick
#define u_click btnE.Dim == btnE::up &&btnE.Op == btnE::click
#define d_click btnE.Dim == btnE::down &&btnE.Op == btnE::click
OneButtonTiny btn_u(BTNU_PIN);
OneButtonTiny btn_c(BTN_PIN);
OneButtonTiny btn_d(BTND_PIN);
void btnTick()
{
	btn_u.tick();
	btn_c.tick();
	btn_d.tick();
};

#include "ecli_begin.h"
// ***********************OLED************************
#ifdef en_oled
#include "GFXTerminal_bin.h"
#include <Adafruit_SH110X.h>
const size_t width = 128; // px
const size_t height = 64; // px
const size_t log_len = 2000;
const size_t style_len = 400;
Adafruit_SH1106G Oled(width, height, &Wire);
GFXTerminal_Bin terminal(width, height, log_len, style_len, Oled, u8g2_font_4x6_tr);

clib(oled)
{
	auto help = [&sh]() -> int
	{
		cout << "usage" << endl;
		cout << "  oled [options]" << endl;
		cout << "options" << endl;
		cout << "  -print <str>    =print" << endl;
		cout << "  -clear          =show" << endl;
		cout << "  -set <0~100>    =contrast" << endl;
		cout << "do not use" << endl;
		cout << "  -begin <addr>   =init";
		return 0;
	};
	set_eat;

	if (Arg("-print", 1))
	{
		String str = argv[0];
		const uint8_t size = 1;
		int16_t x = (128 - str.length() * 6 * size) / 2;
		int16_t y = (64 - 7 * size) / 2;
		Oled.clearDisplay();
		Oled.setTextSize(size);
		Oled.setTextColor(0x1);
		Oled.setCursor(x, y);
		Oled.print(str);
		Oled.display();
		return 0;
	}
	if (Arg("-clear", 0))
	{
		Oled.clearDisplay();
		Oled.display();
		return 0;
	}
	if (Arg("-begin", 1))
	{
		auto error = [&sh]() -> int
		{cout << "error: input a valuable iic address.";return -1; };
		auto ok = [&sh]() -> int
		{cout << "ok";return 0; };
		int addr = atoi(argv[0]);
		if (!addr && argv[0] != "0")
			return error();
		return Oled.begin(addr) ? ok() : error();
	}
	if (Arg("-set", 1))
	{
		auto error = [&sh]() -> int
		{
			cout << "error:input -set <0~100>";
			return -1;
		};
		int n = atoi(argv[0]);
		if (!n && (String)argv[0] != "0")
			return error();
		if (n < 0 || n > 100)
			return error();
		uint8_t per = map(n, 0, 100, 0, 127);
		Oled.setContrast(per);
		return 0;
	}
	return help();
}
clie;
#endif

// ***********************RGB*************************
#ifdef en_rgb
#include <FastLED.h>
CRGB Rgb[1];

clib(rgb)
{
	auto help = [&sh]() -> int
	{
		cout << "usage" << endl;
		cout << "  rgb [options]" << endl;
		cout << "options" << endl;
		cout << "  -show <color>   =show" << endl;
		cout << "    color=int(0=black/1=red..)" << endl;
		cout << "  -enable (bool)" << endl;
		cout << "do not use" << endl;
		cout << "  -begin <pin>    =init";
		return 0;
	};
	set_eat;

	static bool dynamic_en_rgb = false;

	if (Arg("-show", 1))
	{
		auto error = [&sh]() -> int
		{
			cout << "input rgb -show (int)" << endl;
			cout << "0=black" << endl;
			cout << "1=red" << endl;
			cout << "2=green" << endl;
			cout << "3=yellow" << endl;
			cout << "4=blue" << endl;
			cout << "5=magenta" << endl;
			cout << "6=cyan" << endl;
			cout << "7=white" << endl;
			cout << "9=black" << endl;
			return -1;
		};

		int c = atoi(argv[0]);
		if (!c && String(argv[0]) != "0")
			return error();
		switch (c)
		{
		case 0:
			Rgb[0] = CRGB::Black;
			break;
		case 1:
			Rgb[0] = CRGB::Red;
			break;
		case 2:
			Rgb[0] = CRGB::Green;
			break;
		case 3:
			Rgb[0] = CRGB::Yellow;
			break;
		case 4:
			Rgb[0] = CRGB::Blue;
			break;
		case 5:
			Rgb[0] = CRGB::Magenta;
			break;
		case 6:
			Rgb[0] = CRGB::Cyan;
			break;
		case 7:
			Rgb[0] = CRGB::White;
			break;
		case 9:
			Rgb[0] = CRGB::Black;
			break;
		default:
			return error();
		}
		if (dynamic_en_rgb)
			FastLED.show();
		return 0;
	}
	if (Arg("-enable", 1))
	{
		auto error = [&sh]() -> int
		{cout << "error: -enable (bool)";return -1; };
		String v = (String)argv[0];
		if (v == "true")
		{
			dynamic_en_rgb = true;
			return 0;
		}
		else if (v == "false")
		{
			dynamic_en_rgb = false;
			return 0;
		}
		else
		{
			return error();
		}
	}
	if (Arg("-begin", 1))
	{
		auto error = [&sh]() -> int
		{cout << "error: -begin (int)";return -1; };
		uint8_t pin = atoi(argv[0]);
		if (!pin && String(argv[0]) != "0")
			return error();
		FastLED.addLeds<WS2812B, RGB_PIN>(Rgb, 1);
		cout << "ok";
		return 0;
	}
	return help();
}
clie;
#endif

// ***********************RTC**********************
#ifdef en_rtc
#include <RTClib.h>
RTC_PCF8563 Rtc;

String n2s(uint num)
{
	String str(num);
	if (num < 10)
		str = '0' + str;
	return str;
}

clib(rtc)
{
	auto help = [&sh]() -> int
	{
		cout << "usage" << endl;
		cout << "  rtc [options]" << endl;
		cout << "options" << endl;
		cout << "  -print         =printTime" << endl;
		cout << "  -set <time>    =setting" << endl;
		cout << "    time = YY MM DD hh mm ss" << endl;
		cout << "  -lowpwr        =lowPwrMode" << endl;
		cout << "  -lose          =isLosePwr" << endl;
		cout << "  -loseAdj       =recoverTime" << endl;
		cout << "do not use" << endl;
		cout << "  -begin         =init";
		return 0;
	};
	set_eat;

	if (Arg("-print", 0))
	{
		DateTime t = Rtc.now();
		cout << n2s(t.year()) + '-';
		cout << n2s(t.month()) + '-';
		cout << n2s(t.day()) + 'T';
		cout << n2s(t.hour()) + ':';
		cout << n2s(t.minute()) + ':';
		cout << n2s(t.second());
		return 0;
	}
	if (Arg("-set", 6))
	{
		int Y = atoi(eat()),
			M = atoi(eat()),
			D = atoi(eat()),
			h = atoi(eat()),
			m = atoi(eat()),
			s = atoi(eat());

		DateTime t(Y, M, D, h, m, s);

		Rtc.adjust(t);
		return 0;
	}
	if (Arg("-lowpwr", 0))
	{
		Rtc.writeSqwPinMode(PCF8563_SquareWaveOFF);
		return 0;
	}
	if (Arg("-lose", 0))
	{
		if (Rtc.lostPower())
			cout << "true";
		else
			cout << "false";
		return 0;
	}
	if (Arg("-loseAdj", 0))
	{
		Rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
		return 0;
	}
	if (Arg("-begin", 0))
	{
		auto error = [&sh]() -> int
		{cout << "error";return -1; };
		auto ok = [&sh]() -> int
		{cout << "ok";return 0; };
		return Rtc.begin() ? ok() : error();
	}
	return help();
}
clie;
#endif

// ***********************GS**************************
#ifdef en_lsm
#include <Adafruit_LSM6DS3TRC.h>
#include "Adafruit_Sensor.h"
Adafruit_LSM6DS3TRC Lsm;

clib(lsm)
{
	auto help = [&sh]() -> int
	{
		cout << "usage" << endl;
		cout << "  lsm [options]" << endl;
		cout << "options" << endl;
		cout << "  -print         =printDatas" << endl;
		cout << "  -lowpwr        =lowPwrMode" << endl;
		cout << "do not use" << endl;
		cout << "  -begin <addr>  =init";
		return 0;
	};
	set_eat;

	if (Arg("-print", 1))
	{
		/* code */
		return 0;
	}
	if (Arg("-lowpwr", 0))
	{
		Lsm.enableI2CMasterPullups(false);
		Lsm.enablePedometer(false);
		Lsm.enableWakeup(false);
		Lsm.configIntOutputs(false, true);
		Lsm.setAccelDataRate(LSM6DS_RATE_SHUTDOWN);
		Lsm.setGyroDataRate(LSM6DS_RATE_SHUTDOWN);
		return 0;
	}
	if (Arg("-begin", 1))
	{
		auto error = [&sh]() -> int
		{cout << "error: input a valuable iic address.";return -1; };
		auto ok = [&sh]() -> int
		{cout << "ok";return 0; };
		int addr = atoi(argv[0]);
		if (!addr && argv[0] != "0")
			return error();
		return Lsm.begin_I2C(addr) ? ok() : error();
	}
	return help();
}
clie;
#endif

// ***********************SYSTEM**********************
CStream neoTerminal(terminal, Serial);

#include "esp_adc_cal.h"
static esp_adc_cal_characteristics_t adc1_chars;
static bool cali_enable = false;
void adc_cal_init()
{
	esp_err_t ret = esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP);
	if (ret == ESP_OK)
	{
		cali_enable = true;
		esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db, ADC_WIDTH_12Bit, 0, &adc1_chars);
	}
	analogSetWidth(12);
	analogSetPinAttenuation(BAT_PIN, ADC_11db);
}
double adc()
{
	float v = (float)analogReadMilliVolts(BAT_PIN) * 1e-3;
	double vot = v * 2;
	return vot;
}
double per(double vot)
{
	if (vot <= 2.5)
		return 0;
	if (vot <= 3.68f)
		return 5 * vot - 12.5;
	return -263.2 * vot * vot + 2249.2 * vot - 4705.2;
}

clib(sys)
{
	auto help = [&sh]() -> int
	{
		cout << "usage" << endl;
		cout << "  sys [options]" << endl;
		cout << "options" << endl;
		cout << "  -wire <sda> <scl>  =startIIC";
		return -1;
	};
	set_eat;

	if (eat_("-wire"))
	{
		if (argc != 2)
			return help();
		int sda = atoi(eat());
		int scl = atoi(eat());
		if (sda == scl)
			return help();
		Wire.begin(sda, scl);
		cout << "ok";
		return 0;
	}

	return help();
}
clie;

clib(elp13)
{
	auto help = [&sh]() -> int
	{
		cout << "usage" << endl;
		cout << "  elp13 [options]" << endl;
		cout << "options" << endl;
		cout << "  -settime    =setting" << endl;
		cout << "  -sleep      =sleep" << endl;
		cout << "  -battery    =printPWR" << endl;
		cout << "do not use" << endl;
		cout << "  -begin      =init" << endl;
		cout << "  -loseadj    =adj";
		return -1;
	};
	set_eat;

	if (Arg("-settime", 0))
	{
		bool mode = false; // false : 位移, true : 增改
		system("rgb -show 2");
		cout << def_vt100_clean << def_vt100_home;
		cout << "settime...\n>";
		system("rtc -print");
		String t = sh.getCatch();
		const int len = 19;
		uint times[6], i = 11; // Y M D h m s
		const uint8_t bits[] = {4, 2, 2, 2, 2, 2};
		auto parse = [&t, &times, &bits]()
		{
			const char *b = t.c_str();
			for (size_t i = 0; i < 6; i++)
			{
				times[i] = atoi(String(b, bits[i]).c_str());
				b += bits[i] + 1;
			}
		};
		auto pos = [&i, &bits](int &n, int &p)
		{
			n = 0, p = 0;
			for (size_t j = 0, a = bits[j] + 1; j < 6; j++, a += bits[j] + 1)
			{
				if (i < a)
				{
					int last = a - bits[j] - 1;
					n = j;
					p = bits[j] - (i - last) - 1; // 反向
					break;
				}
			}
		};
		auto recur = [&sh, &i]()
		{ cout << (String) "\033[" + (i + 2) + 'G'; };
		auto up = [&t, &i, &sh, recur]()
		{
			int n = t[i] - '0';
			if (n == 9)
				n = 0;
			else
				n++;
			t[i] = n + '0';
			cout << (String)n;
			recur();
		};
		auto down = [&t, &i, &sh, recur]()
		{
			int n = t[i] - '0';
			if (n == 0)
				n = 9;
			else
				n--;
			t[i] = n + '0';
			cout << (String)n;
			recur();
		};
		auto right = [&i, &sh, pos, recur]()
		{
			if (i == len - 1)
				return;
			int n, p;
			pos(n, p);
			if (p == 0)
				i += 2;
			else
				i++;
			recur();
		};
		auto left = [&i, &bits, &sh, pos, recur]()
		{
			if (i == 0)
				return;
			int n, p;
			pos(n, p);
			if (p == bits[n] - 1)
				i -= 2;
			else
				i--;
			recur();
		};
		// cout << "\ndouble click to ensure" << (String)def_vt100_cup('2', '2');
		recur();

		while (1)
		{
			btnTick();
			if (btnE.get())
			{
				if (c_dclick)
				{
					system("rgb -show 4");
					cout << (String)def_vt100_cup(3, 1) << "adjusting...";
					parse();
					system(((String) "rtc -set " +
							times[0] + ' ' +
							times[1] + ' ' +
							times[2] + ' ' +
							times[3] + ' ' +
							times[4] + ' ' +
							times[5])
							   .c_str());
					cout << "ok!";
					delay(500);
					break;
				}
				if (c_click)
				{
					if (mode)
						system("rgb -show 2");
					else
						system("rgb -show 3");
					mode = !mode;
				}
				if (u_click)
					if (mode)
						up();
					else
						right();
				if (d_click)
					if (mode)
						down();
					else
						left();
				cout.flush();
			}
		}
		system("elp13 -sleep");

		return 0;
	}
	if (Arg("-sleep", 0))
	{
		system("rgb -show 9");
#ifdef en_ExpanBoard
		pinMode(O_EN, OUTPUT);
		digitalWrite(O_EN, LOW);
#endif
		esp_sleep_enable_ext0_wakeup(GPIO_BTN_PIN, 0);
		esp_deep_sleep_start();
	}
	if (Arg("-battery", 0))
	{
		cout << String("vot:") + adc() + String(" pwr:") + per(adc());
		return 0;
	}
	if (Arg("-begin", 0))
	{
#ifdef en_ExpanBoard
		pinMode(O_EN, OUTPUT);
		digitalWrite(O_EN, HIGH);
#endif
		cout << "<<system init>>" << endl;
		cout << ">iic:   ";
		system((String("sys -wire ") + SDA_PIN + ' ' + SCL_PIN).c_str());
		cout << endl;
#ifdef en_oled
		delay(100);
		cout << ">oled:  ";
		system("oled -begin 60");
		system("oled -clear");
		cout << endl;
#endif
#ifdef en_rgb
		delay(100);
		cout << ">rgb:   ";
		system((String("rgb -begin ") + String(RGB_PIN)).c_str());
		cout << endl;
#endif
#ifdef en_rtc
		delay(100);
		cout << ">rtc:   ";
		system("rtc -begin");
		cout << endl;
#endif
#ifdef en_lsm
		delay(100);
		cout << ">lsm:   ";
		system("lsm -begin 106");
		cout << endl;
#endif
		return 0;
	}
	if (Arg("-loseadj", 0))
	{
#ifdef en_rtc
		cout << "pwr lost?  ";
		system("rtc -lose");
		String r = sh.getCatch();
		cout << endl;
		if (r == "true")
		{
			cout << "adjust...";
			system("rtc -loseAdj");
			system("rtc -lowpwr");
#ifdef en_lsm
			system("lsm -lowpwr");
#endif
			cout << "ok";
		}
#endif
		return 0;
	}
	return help();
}
clie;

#include "ecli_end.h"

// ***********************MAIN***********************

void setup()
{
	pinMode(BTN_PIN, INPUT);
	Serial.begin(115200);
	load(sysh_cli_pool);
	load(sysh_var_pool);
	Shell sh(neoTerminal);
	sh << sys << elp13;
#ifdef en_oled
	sh << oled;
#endif
#ifdef en_rgb
	sh << rgb;
#endif
#ifdef en_rtc
	sh << rtc;
#endif
#ifdef en_lsm
	sh << lsm;
#endif
	btn_c.attachClick(btn_c_click);
	btn_c.attachDoubleClick(btn_c_dclick);
	btn_u.attachClick(btn_u_click);
	btn_d.attachClick(btn_d_click);

	//***************** 先给屏幕供电
	pinMode(O_EN, OUTPUT);
	digitalWrite(O_EN, HIGH);
	Wire.begin(SDA_PIN, SCL_PIN);
	Oled.begin(0x3C);
	//*****************

	sh.system("oled -set 0");
	sh.system("elp13 -begin");
	sh.system("elp13 -loseadj");
	delay(1000);

	sh.cout << def_vt100_clean << def_vt100_home;
	sh.cout << ">el psy congroo\n\n>";
	sh.system("elp13 -battery");
	sh.cout << (String)def_vt100_cup(2, 1) + ">";

	const size_t wakeTIME = 10000;
	sh.cout << def_vt100_civis;
	timer(wakeTIME)
	{
		static size_t t = 0;
		if (pass > t * 100)
		{
			sh.cout << "\033[2G";
			sh.system("rtc -print");
			t++;
		}

		btnTick();
		if (btnE.get() && c_click)
		{
			sh.cout << def_vt100_cvvis;
			sh.system("elp13 -settime");
		}
	}
	sh.system("elp13 -sleep");
}

void loop() {}