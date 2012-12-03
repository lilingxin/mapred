#ifndef _MAPRED_FILES_H_
#define _MAPRED_FILES_H_

int mapred_read_fd(int fd, void* buf, uint32_t size);
int mapred_write_fd(int fd, void* buf, uint32_t size);
int mapred_spawn_process(const char* cmd, int* in, int* out);

#endif /* _MAPRED_FILES_H */
