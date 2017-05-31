#ifndef MAIN_H
#define MAIN_H

/*
 * Path of current directory
 */
extern char *pwd;

/*
 * Name and location of the configuration file
 */
extern char *confFile;
extern char *logFileName;

/* IDs for the specific application */
extern char *application_id;
extern char *task_id;
extern char *experiment_id;
extern char *platform_id;

/* URL for publishing metrics */
extern char *metrics_publish_URL;

#endif /* MAIN_H */