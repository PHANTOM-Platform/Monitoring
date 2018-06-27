/*
 * Copyright 2018 High Performance Computing Center, Stuttgart
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
