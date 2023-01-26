#ifndef __MONITOR_H__
#define __MONITOR_H__

#define MAX_OPTION_LENGTH 10
#define MAX_IMG_PATH_LENGTH 1000
extern char opt[MAX_OPTION_LENGTH];
extern char img_path[MAX_IMG_PATH_LENGTH];

void cmd_c();

void cmd_h();

void cmd_r();

void cmd_s();
#endif