#ifndef BOT_FILTER_H
#define BOT_FILTER_H

/* Returns 1 if the request looks like a bot/scanner, 0 if it looks real. */
int is_bot_request(const char *user_agent, const char *path);

#endif /* BOT_FILTER_H */
