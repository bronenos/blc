#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <sys/param.h>


static const char *_cmdPath = "/sys/class/backlight/";
static char *_cmdVendor = "intel_backlight";


char _cmdOutput[1024];
char *execute_cmd(const char *cmdAction)
{
	char cmd[1024];
	strcpy(cmd, _cmdPath);
	strcat(cmd, _cmdVendor);
	chdir(cmd);

	FILE *sp = popen(cmdAction, "r");
	if (sp) {
		fgets(_cmdOutput, sizeof(_cmdOutput), sp);
		pclose(sp);
	
		return _cmdOutput;
	}
	
	return NULL;	
}

bool eq(const char *arg, const char *longval, const char *shortval)
{
	if (! strcmp(arg, longval)) {
		return true;
	}

	if (! strcmp(arg, shortval)) {
		return true;
	}

	return false;
}

void print_usage()
{
	printf(
		"USAGE:\n"
		"	command [value]\n\n"
		"COMMANDS:\n"
		"	Set custom vendor\n"
		"	--vendor -v, argument value Identifier, default is %s\n\n"
		"	Get backlight level\n"
		"	--get -g, no arguments, returns [0..1]\n\n"
		"	Set backlight level\n"
		"	--set -s, argument value [0..1]\n\n"
		"	Increase backlight level by 0.1\n"
		"	--inc -i, no arguments\n\n"
		"	Decrease backlight level by 0.1\n"
		"	--dec -d, no arguments\n\n",
		_cmdVendor
	);
}

float max_level()
{
	char *maxValueStr = execute_cmd("cat ./max_brightness");
	float maxValue = 0;
	sscanf(maxValueStr, "%f", &maxValue);
	return maxValue;
}

float get_level()
{
	char *curValueStr = execute_cmd("cat ./brightness");
	float curValue = 0;
	sscanf(curValueStr, "%f", &curValue);

	float value = curValue / max_level();
	return value;
}

void print_level()
{
	float level = get_level();
	printf("%f\n", level);
}

void set_level(float level)
{
	level = MAX(0.1, level);
	level = MIN(1.0, level);
	int value = (int) (max_level() * level);

	char cmd[1024];
	sprintf(cmd, "echo %d | tee ./brightness", value);
	execute_cmd(cmd);
}

void set_level_str(const char *levelStr)
{
	float level = 0;
	sscanf(levelStr, "%f", &level);
	set_level(level);
}

void inc_level()
{
	float level = MIN(1.0, get_level() + 0.1);
	set_level(level);
}

void dec_level()
{
	float level = MAX(0.1, get_level() - 0.1);
	set_level(level);
}

int main(int argc, char *argv[])
{
	if (argc <= 1) {
		print_usage();
		return 0;
	}

#	define assert_last_arg() if (i == argc - 1) return 1
	for (int i=1; i<argc; i++) {
		const char *arg = argv[i];

		if (eq(arg, "--vendor", "-v")) {
			assert_last_arg();
			_cmdVendor = argv[++i];
		}
		else if (eq(arg, "--get", "-g")) {
			print_level();
			break;
		}
		else if (eq(arg, "--set", "-s")) {
			assert_last_arg();
			set_level_str(argv[++i]);
			break;
		}
		else if (eq(arg, "--inc", "-i")) {
			inc_level();
			break;
		}
		else if (eq(arg, "--dec", "-d")) {
			dec_level();
			break;
		}
	}
#	undef assert_last_arg

	return 0;
}

