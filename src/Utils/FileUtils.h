#ifndef FILEUTILS_H
#define FILEUTILS_H

int FILE_exists(const char * path);
int FILE_delete_file(const char * nuke_path, int is_dir);
int FILE_rename_file(const char * old_name, const char * new_name);
int FILE_make_dir(const char * in_dir);

int FILE_make_dir_exist(const char * in_dir);

#endif
