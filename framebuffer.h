#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

extern void fb_init(void);
extern void console_write(char *text);

/* Control characters for the console */
#define FG_RED "\001"
#define FG_GREEN "\002"
#define FG_BLUE "\003"
#define FG_YELLOW "\004"
#define FG_MAGENTA "\005"
#define FG_CYAN "\006"
#define FG_WHITE "\007"
#define FG_BLACK "\010"
#define FG_HALF "\011"

#define COLOUR_PUSH "\013"
#define COLOUR_POP "\014"

#define BG_RED "\021"
#define BG_GREEN "\022"
#define BG_BLUE "\023"
#define BG_YELLOW "\024"
#define BG_MAGENTA "\025"
#define BG_CYAN "\026"
#define BG_WHITE "\027"
#define BG_BLACK "\030"
#define BG_HALF "\031"

#endif	/* FRAMEBUFFER_H */
